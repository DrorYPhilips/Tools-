#include "main.cpp"

#include <QPainter>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QTimer>

#include <iostream>

namespace {

bool check(bool condition, const QString& message)
{
    if (!condition) {
        std::cerr << message.toStdString() << '\n';
    }
    return condition;
}

cv::Mat numberedFrame(int number)
{
    QImage image(480, 160, QImage::Format_RGB888);
    image.fill(Qt::white);
    QPainter painter(&image);
    QFont font(QStringLiteral("Arial"));
    font.setPixelSize(100);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(image.rect(), Qt::AlignCenter, QString::number(number));
    painter.end();
    cv::Mat rgb(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());
    cv::Mat bgr;
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
    return bgr;
}

bool verifySeparatedDigits()
{
    tesseract::TessBaseAPI reader;
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("tesseract"));
    const QString dataPath = QDir(QFileInfo(executable).absolutePath())
                                 .absoluteFilePath(QStringLiteral("../share/tessdata"));
    const QByteArray encodedPath = QFile::encodeName(dataPath);
    if (!check(reader.Init(encodedPath.constData(), "eng", tesseract::OEM_LSTM_ONLY) == 0,
               QStringLiteral("Cannot initialize OCR for digit test"))) {
        return false;
    }
    reader.SetVariable("tessedit_char_whitelist", "0123456789");
    std::map<QByteArray, QString> cache;
    for (bool lightText : {true, false}) {
        for (const QString& number : {QStringLiteral("11002"), QStringLiteral("11122"),
                                      QStringLiteral("11717"), QStringLiteral("12777")}) {
            QImage image(269, 64, QImage::Format_RGB888);
            image.fill(lightText ? QColor(72, 72, 72) : Qt::white);
            QPainter painter(&image);
            QFont font(QStringLiteral("Arial"));
            font.setPixelSize(58);
            font.setBold(true);
            painter.setFont(font);
            painter.setPen(lightText ? Qt::white : Qt::black);
            painter.drawText(image.rect(), Qt::AlignCenter, number);
            painter.fillRect(QRect(0, 29, 4, 9), lightText ? Qt::white : Qt::black);
            painter.end();
            cv::Mat rgb(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());
            cv::Mat bgr;
            cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
            const QString actual = extractDigits(reader, bgr, cache);
            if (!check(actual == number,
                       QStringLiteral("Expected counter %1 but read %2").arg(number, actual))) {
                return false;
            }
        }
    }
    return true;
}

struct Result {
    QString message;
    QString report;
    QString error;
};

Result runTask(TaskWorker::Task task, const QString& path)
{
    Result result;
    TaskWorker worker(task, path, 0, 480, 0, 160);
    QObject::connect(&worker, &TaskWorker::completed, &worker,
                     [&result](const QString& message, const QString& report, const QString& error) {
                         result = {message, report, error};
                     }, Qt::DirectConnection);
    worker.start();
    worker.wait();
    return result;
}

bool verifyVideo(const QString& dir, const QString& extension, int fourcc)
{
    const QString path = QDir(dir).filePath(QStringLiteral("input.") + extension);
    cv::VideoWriter writer(path.toUtf8().constData(), cv::CAP_FFMPEG, fourcc, 5.0,
                           cv::Size(480, 160));
    if (!check(writer.isOpened(), QStringLiteral("Cannot create %1 test video").arg(extension))) {
        return false;
    }
    for (int number : {100, 101, 103, 103}) {
        writer.write(numberedFrame(number));
    }
    writer.release();

    const Result result = runTask(TaskWorker::Task::AnalyzeFrames, path);
    if (!check(result.error.isEmpty(), QStringLiteral("%1: %2").arg(extension, result.error))) {
        return false;
    }
    QFile report(result.report);
    if (!check(report.open(QIODevice::ReadOnly), QStringLiteral("Cannot read video report"))) {
        return false;
    }
    const QString contents = QString::fromUtf8(report.readAll());
    return check(contents.contains(QStringLiteral("Total video frames processed: 4"))
                 && contents.contains(QStringLiteral("Frame 3: 103"))
                 && contents.contains(QStringLiteral("Dropped frame numbers: [102]"))
                 && contents.contains(QStringLiteral("Counter 102 | Estimated video time: 00:00:00.300 | Next observed: Frame 2 at 00:00:00.400"))
                 && contents.contains(QStringLiteral("Duplicated frames: [Frame 3: 103]"))
                 && contents.contains(QStringLiteral("Total duplicated frames: 1"))
                 && contents.contains(QStringLiteral("Total dropped frames: 1"))
                 && QFileInfo(result.report).absolutePath() == QFileInfo(path).absolutePath(),
                 QStringLiteral("%1 report was unexpected:\n%2").arg(extension, contents));
}

bool verifyImageFolder(const QString& dir)
{
    const QString folder = QDir(dir).filePath(QStringLiteral("images"));
    if (!QDir().mkpath(folder)) {
        return check(false, QStringLiteral("Cannot create image folder"));
    }
    for (int number : {100, 101, 103}) {
        std::vector<unsigned char> png;
        if (!cv::imencode(".png", numberedFrame(number), png)) {
            return check(false, QStringLiteral("Cannot encode test image"));
        }
        QFile image(QDir(folder).filePath(QStringLiteral("%1.png").arg(number)));
        if (!image.open(QIODevice::WriteOnly)
            || image.write(reinterpret_cast<const char*>(png.data()), png.size())
                != static_cast<qint64>(png.size())) {
            return check(false, QStringLiteral("Cannot save test image"));
        }
    }
    QFile broken(QDir(folder).filePath(QStringLiteral("broken.png")));
    if (!broken.open(QIODevice::WriteOnly) || broken.write("invalid") != 7) {
        return check(false, QStringLiteral("Cannot create invalid image"));
    }
    broken.close();

    const Result analysis = runTask(TaskWorker::Task::AnalyzeFrames, folder);
    QFile report(analysis.report);
    if (!check(analysis.error.isEmpty() && report.open(QIODevice::ReadOnly),
               QStringLiteral("Image analysis failed: %1").arg(analysis.error))) {
        return false;
    }
    const QByteArray text = report.readAll();
    if (!check(text.contains("Total images processed: 4")
               && text.contains("Dropped frame numbers: [102]")
               && text.contains("Missing frame details:")
               && text.contains("Counter 102")
               && text.contains("broken.png: could not decode image"),
               QStringLiteral("Unexpected image analysis report"))) {
        return false;
    }
    const Result health = runTask(TaskWorker::Task::ScanImages, folder);
    QFile healthReport(health.report);
    if (!check(health.error.isEmpty() && healthReport.open(QIODevice::ReadOnly),
               QStringLiteral("Image health scan failed: %1").arg(health.error))) {
        return false;
    }
    return check(healthReport.readAll().contains("Total corrupted files: 1"),
                 QStringLiteral("Image health report did not identify corrupt image"));
}

bool verifyBottomUpRawAvi()
{
    QTemporaryDir temp;
    if (!check(temp.isValid(), QStringLiteral("Cannot create AVI header test folder"))) {
        return false;
    }
    // Writes a RIFF/AVI file whose video stream format header has the given height and fourcc.
    auto writeHeader = [&temp](const QString& name, qint32 height, const char* compression) {
        QByteArray data("RIFF\0\0\0\0AVI LIST\0\0\0\0hdrlstrf", 28);
        data.append("\x28\0\0\0", 4);
        data.append("\x28\0\0\0\x80\x07\0\0", 8);
        data.append(reinterpret_cast<const char*>(&height), 4);
        data.append("\x01\0\x18\0", 4);
        data.append(compression, 4);
        data.append(QByteArray(20, '\0'));
        const QString path = QDir(temp.path()).filePath(name);
        QFile file(path);
        return file.open(QIODevice::WriteOnly) && file.write(data) == data.size() ? path : QString();
    };
    const QString bottomUp = writeHeader(QStringLiteral("bottom-up.avi"), 1080, "\0\0\0\0");
    const QString topDown = writeHeader(QStringLiteral("top-down.avi"), -1080, "\0\0\0\0");
    const QString compressed = writeHeader(QStringLiteral("mjpg.avi"), 1080, "MJPG");
    if (!check(!bottomUp.isEmpty() && !topDown.isEmpty() && !compressed.isEmpty(),
               QStringLiteral("Cannot write AVI header test files"))) {
        return false;
    }
    return check(isBottomUpUncompressedAvi(bottomUp), QStringLiteral("Bottom-up raw AVI was not detected"))
           && check(!isBottomUpUncompressedAvi(topDown), QStringLiteral("Top-down raw AVI was rejected"))
           && check(!isBottomUpUncompressedAvi(compressed), QStringLiteral("Compressed AVI was rejected"));
}

bool verifyUncompressedAviDecode()
{
    QTemporaryDir temp;
    if (!check(temp.isValid(), QStringLiteral("Cannot create AVI decode test folder"))) {
        return false;
    }
    const auto le32 = [](quint32 value) {
        return QByteArray(reinterpret_cast<const char*>(&value), 4);
    };
    const auto chunk = [&le32](const char* id, const QByteArray& body) {
        QByteArray out(id, 4);
        out.append(le32(static_cast<quint32>(body.size())));
        out.append(body);
        if (body.size() % 2 != 0) {
            out.append('\0');
        }
        return out;
    };
    const auto list = [&chunk](const char* type, const QByteArray& body) {
        return chunk("LIST", QByteArray(type, 4) + body);
    };
    // 4x3 pixels, 24-bit, stored bottom-up: stored row k is filled with value k (+10 in frame 2).
    const QByteArray strh = QByteArray("vids", 4) + QByteArray("DIB ", 4) + le32(0) + le32(0) + le32(0)
                            + le32(1) + le32(30) + QByteArray(28, '\0');
    const QByteArray strf = le32(40) + le32(4) + le32(3) + QByteArray("\x01\0\x18\0", 4) + le32(0)
                            + QByteArray(20, '\0');
    const auto frame = [](int offset) {
        QByteArray pixels;
        for (int row = 0; row < 3; ++row) {
            pixels.append(QByteArray(12, static_cast<char>(row + offset)));
        }
        return pixels;
    };
    const QByteArray file = chunk("RIFF", QByteArray("AVI ", 4)
                                      + list("hdrl", list("strl", chunk("strh", strh) + chunk("strf", strf)))
                                      + chunk("JUNK", QByteArray(6, '\0'))
                                      + list("movi", chunk("00db", frame(0)) + chunk("00db", frame(10))));
    const QString path = QDir(temp.path()).filePath(QStringLiteral("bottom-up-decode.avi"));
    QFile out(path);
    if (!check(out.open(QIODevice::WriteOnly) && out.write(file) == file.size(),
               QStringLiteral("Cannot write bottom-up decode test file"))) {
        return false;
    }
    out.close();

    VideoSource source;
    if (!check(isBottomUpUncompressedAvi(path) && source.open(path) && source.frameCount() == 2
                   && source.fps() == 30,
               QStringLiteral("Bottom-up AVI was not opened by the raw reader"))) {
        return false;
    }
    cv::Mat image;
    double timestamp = -1;
    if (!check(source.read(image, timestamp) && image.rows == 3 && image.cols == 4
                   && image.type() == CV_8UC3 && image.at<cv::Vec3b>(0, 0)[0] == 2
                   && image.at<cv::Vec3b>(2, 0)[0] == 0 && timestamp == 0,
               QStringLiteral("First bottom-up frame was not flipped to top-down"))) {
        return false;
    }
    return check(source.read(image, timestamp) && image.at<cv::Vec3b>(0, 0)[0] == 12
                     && std::abs(timestamp - 1000.0 / 30.0) < 0.01,
                 QStringLiteral("Second bottom-up frame was not decoded"))
           && check(!source.read(image, timestamp), QStringLiteral("Reader returned more frames than indexed"));
}

bool verifyVideoPreview(const QString& path)
{
    MainWindow window;
    if (!check(window.windowTitle().contains(QStringLiteral("Improved OCR")),
               QStringLiteral("Updated application title missing"))) {
        return false;
    }
    const auto edits = window.findChildren<QLineEdit*>();
    const auto spins = window.findChildren<QSpinBox*>();
    QPushButton* preview = nullptr;
    QPushButton* analyzeButton = nullptr;
    for (QPushButton* button : window.findChildren<QPushButton*>()) {
        if (button->text() == QStringLiteral("Preview crop")) {
            preview = button;
        } else if (button->text() == QStringLiteral("Analyze frame numbers")) {
            analyzeButton = button;
        }
    }
    if (!check(edits.size() >= 1 && spins.size() == 4 && preview && analyzeButton,
               QStringLiteral("Could not locate preview controls"))) {
        return false;
    }
    edits.first()->setText(path);
    spins[0]->setValue(0);
    spins[1]->setValue(480);
    spins[2]->setValue(0);
    spins[3]->setValue(160);
    QTimer::singleShot(1000, [] {
        if (QWidget* dialog = QApplication::activeModalWidget()) {
            dialog->close();
        }
    });
    preview->click();
    bool hasPreview = false;
    for (QLabel* label : window.findChildren<QLabel*>()) {
        if (!label->pixmap().isNull()) {
            hasPreview = true;
        }
    }
    if (!check(hasPreview, QStringLiteral("Video crop preview was not shown"))) {
        return false;
    }

    analyzeButton->click();
    QElapsedTimer timer;
    timer.start();
    while (!analyzeButton->isEnabled() && timer.elapsed() < 30000) {
        QApplication::processEvents();
        QThread::msleep(10);
    }
    if (TaskWorker* worker = window.findChild<TaskWorker*>()) {
        worker->wait();
        QApplication::processEvents();
    }
    const auto logs = window.findChildren<QTextEdit*>();
    return check(analyzeButton->isEnabled() && !logs.isEmpty()
                 && logs.first()->toPlainText().contains(QStringLiteral("Processed 4 frames"))
                 && logs.first()->toPlainText().contains(QStringLiteral("Report saved to:"))
                 && logs.first()->toPlainText().contains(QStringLiteral("Full report:"))
                 && logs.first()->toPlainText().contains(QStringLiteral("Counter 102 | Estimated video time: 00:00:00.300")),
                 QStringLiteral("GUI video analysis did not complete successfully"));
}

bool verifyAmbiguousGap(const QString& dir)
{
    const QString path = QDir(dir).filePath(QStringLiteral("ambiguous.mp4"));
    cv::VideoWriter writer(path.toUtf8().constData(), cv::CAP_FFMPEG,
                           cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 5.0,
                           cv::Size(480, 160));
    if (!check(writer.isOpened(), QStringLiteral("Cannot create ambiguous test video"))) {
        return false;
    }
    for (int number : {100, 103, 100, 103}) {
        writer.write(numberedFrame(number));
    }
    writer.release();
    const Result result = runTask(TaskWorker::Task::AnalyzeFrames, path);
    QFile report(result.report);
    if (!check(result.error.isEmpty() && report.open(QIODevice::ReadOnly),
               QStringLiteral("Ambiguous video analysis failed: %1").arg(result.error))) {
        return false;
    }
    const QByteArray text = report.readAll();
    return check(text.contains("Unresolved OCR/sequence transitions:")
                 && text.contains("Incomplete analysis:")
                 && text.contains("Total dropped frames: 0"),
                 QStringLiteral("Counter reset must be reported as unresolved:\n%1").arg(QString::fromUtf8(text)));
}

bool verifySequenceRules()
{
    auto sequence = [](std::initializer_list<long long> numbers) {
        QVector<CounterReading> frames;
        for (long long number : numbers) {
            const int index = static_cast<int>(frames.size());
            frames.append({QStringLiteral("Frame %1").arg(index), number, index * 1000.0 / 60, index});
        }
        return analyzeSequence(frames);
    };
    const auto drop = sequence({10808, 10809, 10811});
    const auto duplicate = sequence({10810, 10810, 10811});
    const auto repeated = sequence({10810, 10810, 10810, 10811});
    const auto spike = sequence({10808, 41001, 10810});
    const auto reset = sequence({10810, 10811, 10808, 10809});
    QVector<CounterReading> unreadableGap = {
        {QStringLiteral("Frame 0"), 10808, 0, 0},
        {QStringLiteral("Frame 2"), 10810, 33, 2}
    };
    const auto unknown = analyzeSequence(unreadableGap);
    return check(drop.missingCount == 1 && drop.gaps.size() == 1
                 && drop.gaps.first().range.first == 10810 && drop.duplicates.isEmpty()
                 && duplicate.missingCount == 0 && duplicate.duplicates.size() == 1
                 && repeated.duplicates.size() == 2
                 && spike.missingCount == 0 && spike.duplicates.isEmpty() && !spike.unresolved.isEmpty()
                 && reset.duplicates.isEmpty() && !reset.unresolved.isEmpty()
                 && unknown.missingCount == 0 && !unknown.unresolved.isEmpty(),
                 QStringLiteral("Sequential drop/duplicate/uncertain rules failed"));
}

bool verifyReportHighlighting()
{
    const QString many = reportToHtml(QStringLiteral(
        "Total dropped frames: 2\nDropped frame numbers: [3585, 4186]\n"
        "Duplicated frames: [Frame 1: 7, Frame 2: 7]\nTotal duplicated frames: 2\n"
        "Total video frames processed: 4 out of 4 (100%)"));
    const QString single = reportToHtml(QStringLiteral(
        "Total dropped frames: 1\nDropped frame numbers: [102]\nTotal duplicated frames: 1"));
    return check(many.contains(QStringLiteral("<span style='color:red;'>Total dropped frames: 2</span>"))
                 && many.contains(QStringLiteral("<span style='color:red;'>Dropped frame numbers: [3585, 4186]</span>"))
                 && many.contains(QStringLiteral("<span style='color:red;'>Duplicated frames: [Frame 1: 7, Frame 2: 7]</span>"))
                 && many.contains(QStringLiteral("<span style='color:red;'>Total duplicated frames: 2</span>"))
                 && many.contains(QStringLiteral("Total video frames processed: 4 out of 4 (100%)"))
                 && !single.contains(QStringLiteral("color:red")),
                 QStringLiteral("Report lines must be red only when the count is above 1"));
}

void sendMouse(QWidget* widget, QEvent::Type type, const QPointF& point,
               Qt::MouseButton button, Qt::MouseButtons buttons)
{
    QMouseEvent event(type, point, point, button, buttons, Qt::NoModifier);
    QApplication::sendEvent(widget, &event);
}

bool verifyRoiMapping()
{
    QImage image(1000, 500, QImage::Format_RGB888);
    image.fill(Qt::white);
    RoiCanvas canvas(image, {});
    canvas.resize(500, 500);
    const QRectF bounds = canvas.imageBounds();
    const QPointF start(bounds.left() + bounds.width() * 0.8,
                        bounds.top() + bounds.height() * 0.8);
    const QPointF end(bounds.left() + bounds.width() * 0.2,
                      bounds.top() + bounds.height() * 0.2);
    sendMouse(&canvas, QEvent::MouseButtonPress, start, Qt::LeftButton, Qt::LeftButton);
    sendMouse(&canvas, QEvent::MouseMove, end, Qt::NoButton, Qt::LeftButton);
    sendMouse(&canvas, QEvent::MouseButtonRelease, end, Qt::LeftButton, Qt::NoButton);
    if (!check(canvas.selection() == QRect(200, 100, 600, 300),
               QStringLiteral("Reverse drag did not map to image pixels"))) {
        return false;
    }

    sendMouse(&canvas, QEvent::MouseButtonPress, bounds.center(), Qt::LeftButton, Qt::LeftButton);
    sendMouse(&canvas, QEvent::MouseButtonRelease, QPointF(-50, -50), Qt::LeftButton, Qt::NoButton);
    if (!check(canvas.selection() == QRect(0, 0, 500, 250),
               QStringLiteral("Out-of-image drag was not clamped"))) {
        return false;
    }
    sendMouse(&canvas, QEvent::MouseButtonPress, bounds.center(), Qt::LeftButton, Qt::LeftButton);
    sendMouse(&canvas, QEvent::MouseButtonRelease, bounds.center(), Qt::LeftButton, Qt::NoButton);
    return check(canvas.selection().isEmpty(), QStringLiteral("A click should not create a crop"));
}

bool verifyRoiDialog(const QString& source)
{
    MainWindow window;
    window.findChildren<QLineEdit*>().first()->setText(source);
    const auto spins = window.findChildren<QSpinBox*>();
    QPushButton* select = nullptr;
    for (QPushButton* button : window.findChildren<QPushButton*>()) {
        if (button->text() == QStringLiteral("Select counter area...")) {
            select = button;
            break;
        }
    }
    if (!check(select && spins.size() == 4, QStringLiteral("ROI controls not found"))) {
        return false;
    }
    bool interacted = false;
    QTimer::singleShot(0, [&] {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dialog) {
            return;
        }
        RoiCanvas* canvas = nullptr;
        for (QWidget* widget : dialog->findChildren<QWidget*>()) {
            if ((canvas = dynamic_cast<RoiCanvas*>(widget))) {
                break;
            }
        }
        auto* buttons = dialog->findChild<QDialogButtonBox*>();
        if (!canvas || !buttons) {
            dialog->reject();
            return;
        }
        const QRectF bounds = canvas->imageBounds();
        const QPointF start(bounds.left() + bounds.width() * 0.25,
                            bounds.top() + bounds.height() * 0.25);
        const QPointF end(bounds.left() + bounds.width() * 0.75,
                          bounds.top() + bounds.height() * 0.75);
        sendMouse(canvas, QEvent::MouseButtonPress, start, Qt::LeftButton, Qt::LeftButton);
        sendMouse(canvas, QEvent::MouseMove, end, Qt::NoButton, Qt::LeftButton);
        sendMouse(canvas, QEvent::MouseButtonRelease, end, Qt::LeftButton, Qt::NoButton);
        interacted = buttons->button(QDialogButtonBox::Ok)->isEnabled();
        buttons->button(QDialogButtonBox::Ok)->click();
    });
    select->click();
    if (!check(interacted && spins[0]->value() == 120 && spins[1]->value() == 360
               && spins[2]->value() == 40 && spins[3]->value() == 120,
               QStringLiteral("ROI selection did not update pixel coordinates: %1 %2 %3 %4; interacted=%5")
                   .arg(spins[0]->value()).arg(spins[1]->value())
                   .arg(spins[2]->value()).arg(spins[3]->value()).arg(interacted))) {
        return false;
    }

    QTimer::singleShot(0, [] {
        if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) {
            dialog->reject();
        }
    });
    select->click();
    return check(spins[0]->value() == 120 && spins[1]->value() == 360
                 && spins[2]->value() == 40 && spins[3]->value() == 120,
                 QStringLiteral("Cancel changed the selected ROI"));
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QTemporaryDir temp;
    if (!check(temp.isValid(), QStringLiteral("Cannot create temporary test directory"))) {
        return 1;
    }
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, temp.path());
    const QString dir = QDir(temp.path()).filePath(QString::fromUtf8("video-\xc3\xa9"));
    if (!check(QDir().mkpath(dir), QStringLiteral("Cannot create test folder"))) {
        return 1;
    }
    if (!verifySequenceRules()
        || !verifyReportHighlighting()
        || !verifyBottomUpRawAvi()
        || !verifyUncompressedAviDecode()
        || !verifySeparatedDigits()
        || !verifyVideo(dir, QStringLiteral("avi"), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'))
        || !verifyVideo(dir, QStringLiteral("mp4"), cv::VideoWriter::fourcc('m', 'p', '4', 'v'))
        || !verifyVideo(dir, QStringLiteral("mov"), cv::VideoWriter::fourcc('m', 'p', '4', 'v'))
        || !verifyImageFolder(dir)
        || !verifyVideoPreview(QDir(dir).filePath(QStringLiteral("input.mp4")))
        || !verifyVideoPreview(QDir(dir).filePath(QStringLiteral("input.mov")))
        || !verifyAmbiguousGap(dir)
        || !verifyRoiMapping()
        || !verifyRoiDialog(QDir(dir).filePath(QStringLiteral("input.mp4")))
        || !verifyRoiDialog(QDir(dir).filePath(QStringLiteral("images")))) {
        return 1;
    }
    const QString invalidPath = QDir(dir).filePath(QStringLiteral("invalid.mp4"));
    QFile invalid(invalidPath);
    if (!check(invalid.open(QIODevice::WriteOnly), QStringLiteral("Cannot create invalid video"))) {
        return 1;
    }
    invalid.write("not a video");
    invalid.close();
    const Result invalidResult = runTask(TaskWorker::Task::AnalyzeFrames, invalidPath);
    if (!check(!invalidResult.error.isEmpty() && invalidResult.report.isEmpty(),
               QStringLiteral("Invalid video should fail without a report"))) {
        return 1;
    }
    const Result unsupported = runTask(TaskWorker::Task::ScanImages,
                                       QDir(dir).filePath(QStringLiteral("input.mp4")));
    if (!check(!unsupported.error.isEmpty() && unsupported.report.isEmpty(),
               QStringLiteral("Image health scan should reject video input"))) {
        return 1;
    }
    std::cout << "AVI, MP4, MOV, preview, image folders, Unicode paths, and error checks passed.\n";
    return 0;
}
