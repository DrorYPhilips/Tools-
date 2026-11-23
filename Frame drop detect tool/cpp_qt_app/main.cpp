#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QAction>
#include <QGroupBox>
#include <QMenu>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QColor>
#include <QImage>
#include <QLabel>
#include <QPalette>
#include <QStyle>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QCheckBox>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QUrl>
#include <QtEndian>
#include <QThread>
#include <QVBoxLayout>
#include <QVector>
#include <QFrame>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <tesseract/baseapi.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace {

const QStringList imagePatterns = {
    QStringLiteral("*.png"),
    QStringLiteral("*.jpg"),
    QStringLiteral("*.jpeg"),
    QStringLiteral("*.bmp")
};

} // namespace

struct MissingRange {
    long long first;
    long long last;
};

struct CounterReading {
    QString fileName;
    long long number;
    double timestampMs;
    int index;
};

struct CounterGap {
    MissingRange range;
    CounterReading before;
    CounterReading after;
};

struct SequenceAnalysis {
    QVector<CounterGap> gaps;
    QStringList duplicates;
    QStringList unresolved;
    unsigned long long missingCount = 0;
};

namespace {

SequenceAnalysis analyzeSequence(const QVector<CounterReading>& frames)
{
    SequenceAnalysis result;
    std::set<qsizetype> suspect;
    for (qsizetype i = 1; i + 1 < frames.size(); ++i) {
        const auto& before = frames.at(i - 1);
        const auto& current = frames.at(i);
        const auto& after = frames.at(i + 1);
        if (before.index + 1 == current.index && current.index + 1 == after.index
            && after.number >= before.number && after.number - before.number <= 2
            && (current.number < before.number || current.number > after.number)) {
            suspect.insert(i);
            result.unresolved.append(
                QStringLiteral("%1: suspect OCR counter %2 between %3 and %4; "
                               "not counted as a drop or duplicate.")
                    .arg(current.fileName).arg(current.number).arg(before.number).arg(after.number));
        }
    }

    for (qsizetype i = 1; i < frames.size(); ++i) {
        const auto& before = frames.at(i - 1);
        const auto& after = frames.at(i);
        if (before.index + 1 != after.index) {
            result.unresolved.append(QStringLiteral("%1 to %2: unreadable/unrecognized intervening frames; "
                                                   "drop count is unknown.")
                                         .arg(before.fileName, after.fileName));
            continue;
        }
        if (suspect.count(i - 1) || suspect.count(i)) {
            continue;
        }
        if (after.number < before.number) {
            result.unresolved.append(QStringLiteral("%1: counter went backwards from %2 to %3; "
                                                   "possible OCR error, reset, or reordered frame.")
                                         .arg(after.fileName).arg(before.number).arg(after.number));
        } else if (after.number == before.number) {
            result.duplicates.append(QStringLiteral("%1: %2").arg(after.fileName).arg(after.number));
        } else if (after.number - before.number > 1) {
            result.gaps.append({{before.number + 1, after.number - 1}, before, after});
            result.missingCount += static_cast<unsigned long long>(after.number - before.number - 1);
        }
    }
    return result;
}

cv::Mat readImage(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QByteArray encoded = file.readAll();
    const std::vector<unsigned char> bytes(encoded.begin(), encoded.end());
    try {
        return cv::imdecode(bytes, cv::IMREAD_COLOR);
    } catch (const cv::Exception&) {
        return {};
    }
}

QImage toQImage(const cv::Mat& image)
{
    if (image.empty()) {
        return {};
    }

    cv::Mat rgb;
    cv::cvtColor(image, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                  QImage::Format_RGB888).copy();
}

QString uniqueReportPath(const QString& folder, const QString& baseName)
{
    const QString first = QDir(folder).filePath(baseName + QStringLiteral(".txt"));
    if (!QFileInfo::exists(first)) {
        return first;
    }

    for (int revision = 1; revision < std::numeric_limits<int>::max(); ++revision) {
        const QString candidate = QDir(folder).filePath(
            baseName + QStringLiteral("_rev%1.txt").arg(revision));
        if (!QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

QString formatMissing(const QVector<MissingRange>& ranges)
{
    QStringList items;
    for (const MissingRange& range : ranges) {
        if (range.first == range.last) {
            items.append(QString::number(range.first));
        } else {
            items.append(QStringLiteral("%1-%2").arg(range.first).arg(range.last));
        }
    }
    return items.join(QStringLiteral(", "));
}

QStringList imageFiles(const QString& folder)
{
    const QFileInfoList entries = QDir(folder).entryInfoList(
        imagePatterns, QDir::Files, QDir::Name | QDir::IgnoreCase);

    QStringList paths;
    paths.reserve(entries.size());
    for (const QFileInfo& entry : entries) {
        paths.append(entry.absoluteFilePath());
    }
    return paths;
}

bool isVideoFile(const QFileInfo& info)
{
    const QString suffix = info.suffix().toLower();
    return info.isFile() && (suffix == QStringLiteral("avi") || suffix == QStringLiteral("mp4")
                             || suffix == QStringLiteral("mov"));
}

// The bundled OpenCV FFmpeg decoder crashes on uncompressed (BI_RGB) AVI files whose
// stream header has a positive (bottom-up) height, so those are rejected before opening.
bool isBottomUpUncompressedAvi(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QByteArray head = file.read(65536);
    if (!head.startsWith("RIFF") || head.mid(8, 4) != "AVI ") {
        return false;
    }
    const int at = head.indexOf("strf");
    if (at < 0 || at + 28 > head.size()) {
        return false;
    }
    const auto* p = reinterpret_cast<const uchar*>(head.constData() + at);
    const auto le32 = [](const uchar* q) {
        return static_cast<quint32>(q[0]) | (static_cast<quint32>(q[1]) << 8)
               | (static_cast<quint32>(q[2]) << 16) | (static_cast<quint32>(q[3]) << 24);
    };
    const auto height = static_cast<qint32>(le32(p + 16));
    const quint32 compression = le32(p + 24);
    return height > 0 && compression == 0;
}

QString unreadableUncompressedAviMessage()
{
    return QStringLiteral("This uncompressed AVI could not be read. Only 24-bit and 32-bit "
                          "uncompressed AVI files are supported; re-encode other files "
                          "(for example to MJPEG or MP4) and try again.");
}

// Reads uncompressed 24/32-bit AVI frames directly, because the bundled OpenCV FFmpeg
// decoder crashes on bottom-up rows. Frame chunks are indexed by walking the RIFF/LIST tree,
// which also covers OpenDML files split into several RIFF AVIX segments.
class UncompressedAviReader
{
public:
    bool open(const QString& path)
    {
        m_file.setFileName(path);
        if (!m_file.open(QIODevice::ReadOnly)) {
            return false;
        }
        scan(0, m_file.size());
        if (!m_formatFound) {
            return false;
        }
        const int stride = ((m_width * m_bytesPerPixel + 3) / 4) * 4;
        m_frameBytes = static_cast<qint64>(stride) * m_height;
        m_stride = stride;
        for (const FrameChunk& chunk : m_candidates) {
            if (static_cast<qint64>(chunk.size) >= m_frameBytes) {
                m_chunks.append(chunk);
            }
        }
        return !m_chunks.isEmpty();
    }

    bool read(cv::Mat& image)
    {
        if (m_next >= m_chunks.size() || !m_file.seek(m_chunks[m_next].offset)) {
            return false;
        }
        QByteArray data = m_file.read(m_frameBytes);
        if (data.size() != m_frameBytes) {
            return false;
        }
        ++m_next;
        const cv::Mat raw(m_height, m_width, CV_8UC(m_bytesPerPixel), data.data(), m_stride);
        cv::Mat bgr;
        if (m_bytesPerPixel == 4) {
            cv::cvtColor(raw, bgr, cv::COLOR_BGRA2BGR);
        } else {
            bgr = raw.clone();
        }
        if (m_bottomUp) {
            cv::flip(bgr, image, 0);
        } else {
            image = bgr;
        }
        return true;
    }

    int frameCount() const { return m_chunks.size(); }

    double fps() const { return m_scale > 0 ? static_cast<double>(m_rate) / m_scale : 0; }

    double positionMs() const
    {
        return fps() > 0 && m_next > 0 ? (m_next - 1) * 1000.0 / fps() : -1;
    }

private:
    struct FrameChunk
    {
        qint64 offset;
        quint32 size;
    };

    QByteArray readAt(qint64 pos, qint64 length)
    {
        return m_file.seek(pos) ? m_file.read(length) : QByteArray();
    }

    void scan(qint64 begin, qint64 end)
    {
        qint64 pos = begin;
        while (pos + 8 <= end) {
            const QByteArray header = readAt(pos, 8);
            if (header.size() != 8) {
                return;
            }
            const QByteArray id = header.left(4);
            const auto size = qFromLittleEndian<quint32>(header.constData() + 4);
            const qint64 bodyStart = pos + 8;
            const qint64 chunkEnd = bodyStart + size;
            if (id == "RIFF" || id == "LIST") {
                const qint64 listEnd = qMin(chunkEnd, end);
                if (listEnd - bodyStart >= 4) {
                    scan(bodyStart + 4, listEnd);
                }
            } else if (id == "strh") {
                readStreamHeader(bodyStart, size);
            } else if (id == "strf" && m_expectFormat) {
                readFormat(bodyStart, size);
            } else if (m_videoFound && id.left(2) == m_videoPrefix
                       && (id.mid(2) == "db" || id.mid(2) == "dc")) {
                m_candidates.append({bodyStart, size});
            }
            pos = chunkEnd + (size & 1);
        }
    }

    void readStreamHeader(qint64 bodyStart, quint32 size)
    {
        const int streamIndex = m_streamCount++;
        const QByteArray body = size >= 28 ? readAt(bodyStart, 28) : QByteArray();
        m_expectFormat = body.size() == 28 && body.left(4) == "vids" && !m_videoFound;
        if (m_expectFormat) {
            m_videoFound = true;
            m_videoPrefix = QByteArray::number(streamIndex).rightJustified(2, '0');
            m_scale = qFromLittleEndian<quint32>(body.constData() + 20);
            m_rate = qFromLittleEndian<quint32>(body.constData() + 24);
        }
    }

    void readFormat(qint64 bodyStart, quint32 size)
    {
        m_expectFormat = false;
        const QByteArray body = size >= 40 ? readAt(bodyStart, 40) : QByteArray();
        if (body.size() != 40) {
            return;
        }
        const auto* p = reinterpret_cast<const uchar*>(body.constData());
        const auto width = qFromLittleEndian<qint32>(p + 4);
        const auto height = qFromLittleEndian<qint32>(p + 8);
        const auto bitCount = qFromLittleEndian<quint16>(p + 14);
        const auto compression = qFromLittleEndian<quint32>(p + 16);
        if (width <= 0 || height == 0 || compression != 0 || (bitCount != 24 && bitCount != 32)) {
            return;
        }
        m_width = width;
        m_height = height < 0 ? -height : height;
        m_bottomUp = height > 0;
        m_bytesPerPixel = bitCount / 8;
        m_formatFound = true;
    }

    QFile m_file;
    bool m_expectFormat = false;
    bool m_formatFound = false;
    bool m_videoFound = false;
    bool m_bottomUp = true;
    int m_streamCount = 0;
    QByteArray m_videoPrefix;
    quint32 m_scale = 0;
    quint32 m_rate = 0;
    int m_width = 0;
    int m_height = 0;
    int m_bytesPerPixel = 0;
    int m_stride = 0;
    qint64 m_frameBytes = 0;
    QVector<FrameChunk> m_candidates;
    QVector<FrameChunk> m_chunks;
    int m_next = 0;
};

// Opens a video with the raw AVI reader for uncompressed bottom-up files and with OpenCV otherwise.
class VideoSource
{
public:
    bool open(const QString& path)
    {
        m_raw = isBottomUpUncompressedAvi(path);
        if (m_raw) {
            return m_rawReader.open(path);
        }
        return m_capture.open(path.toUtf8().constData(), cv::CAP_FFMPEG);
    }

    bool read(cv::Mat& image, double& timestampMs)
    {
        if (m_raw) {
            timestampMs = -1;
            if (!m_rawReader.read(image)) {
                return false;
            }
            timestampMs = m_rawReader.positionMs();
            return true;
        }
        if (!m_capture.read(image)) {
            return false;
        }
        timestampMs = m_capture.get(cv::CAP_PROP_POS_MSEC);
        return true;
    }

    double frameCount() { return m_raw ? m_rawReader.frameCount() : m_capture.get(cv::CAP_PROP_FRAME_COUNT); }

    double fps() { return m_raw ? m_rawReader.fps() : m_capture.get(cv::CAP_PROP_FPS); }

private:
    bool m_raw = false;
    cv::VideoCapture m_capture;
    UncompressedAviReader m_rawReader;
};

QString readOcr(tesseract::TessBaseAPI& reader, const cv::Mat& image)
{
    reader.SetImage(image.data, image.cols, image.rows, 1, static_cast<int>(image.step));
    std::unique_ptr<char[]> text(reader.GetUTF8Text());
    if (!text) {
        return {};
    }

    QString digits;
    for (const unsigned char character : QByteArray(text.get())) {
        if (character >= '0' && character <= '9') {
            digits.append(QChar(character));
        }
    }
    return digits;
}

QString extractDigits(tesseract::TessBaseAPI& reader, const cv::Mat& crop,
                      std::map<QByteArray, QString>& glyphCache)
{
    cv::Mat gray;
    cv::Mat mask;
    cv::cvtColor(crop, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, mask, 150, 255, cv::THRESH_BINARY);

    const double edgeBrightness = (cv::mean(mask.row(0))[0] + cv::mean(mask.row(mask.rows - 1))[0]
                                   + cv::mean(mask.col(0))[0] + cv::mean(mask.col(mask.cols - 1))[0]) / 4;
    if (edgeBrightness > 127) {
        cv::bitwise_not(mask, mask);
    }

    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    const int count = cv::connectedComponentsWithStats(mask, labels, stats, centroids);
    int tallest = 0;
    for (int i = 1; i < count; ++i) {
        tallest = std::max(tallest, stats.at<int>(i, cv::CC_STAT_HEIGHT));
    }
    std::vector<cv::Rect> glyphs;
    bool separable = tallest >= 3;
    for (int i = 1; i < count; ++i) {
        const int height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
        if (height * 10 < tallest * 6) {
            if (height * 10 >= tallest * 3) {
                separable = false;
            }
            continue;
        }
        glyphs.emplace_back(stats.at<int>(i, cv::CC_STAT_LEFT), stats.at<int>(i, cv::CC_STAT_TOP),
                            stats.at<int>(i, cv::CC_STAT_WIDTH), height);
    }
    std::sort(glyphs.begin(), glyphs.end(), [](const cv::Rect& a, const cv::Rect& b) {
        return a.x < b.x;
    });
    if (glyphs.size() < 2 || glyphs.size() > 19) {
        separable = false;
    }
    for (size_t i = 0; i < glyphs.size(); ++i) {
        if (glyphs[i].width > tallest * 1.5
            || (i > 0 && glyphs[i - 1].x + glyphs[i - 1].width > glyphs[i].x)) {
            separable = false;
            break;
        }
    }

    if (separable) {
        reader.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
        QString result;
        for (const cv::Rect& glyph : glyphs) {
            cv::Mat digit;
            cv::bitwise_not(mask(glyph), digit);
            const int padding = std::max(glyph.width, glyph.height) / 2;
            cv::copyMakeBorder(digit, digit, padding, padding, padding, padding,
                               cv::BORDER_CONSTANT, cv::Scalar(255));
            const QByteArray key = QByteArray::number(digit.cols) + ':'
                + QByteArray(reinterpret_cast<const char*>(digit.data),
                             static_cast<qsizetype>(digit.total()));
            const auto cached = glyphCache.find(key);
            QString recognized;
            if (cached != glyphCache.end()) {
                recognized = cached->second;
            } else {
                recognized = readOcr(reader, digit);
                if (recognized.size() != 1 || reader.MeanTextConf() < 50) {
                    separable = false;
                    break;
                }
                if (glyphCache.size() < 256) {
                    glyphCache.emplace(key, recognized);
                }
            }
            result += recognized;
        }
        if (separable) {
            return result;
        }
    }

    reader.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
    cv::Mat line;
    cv::bitwise_not(mask, line);
    cv::copyMakeBorder(line, line, 15, 15, 15, 15, cv::BORDER_CONSTANT, cv::Scalar(255));
    return readOcr(reader, line);
}

bool writeReport(const QString& path, const QString& contents, QString* error)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        *error = QStringLiteral("Cannot create report %1: %2").arg(path, file.errorString());
        return false;
    }

    QTextStream stream(&file);
    stream << contents;
    stream.flush();
    if (stream.status() != QTextStream::Ok || !file.commit()) {
        *error = QStringLiteral("Cannot save report %1: %2").arg(path, file.errorString());
        return false;
    }
    return true;
}

QString videoTime(double milliseconds)
{
    if (!std::isfinite(milliseconds) || milliseconds < 0
        || milliseconds >= static_cast<double>(std::numeric_limits<qint64>::max())) {
        return QStringLiteral("unavailable");
    }
    const qint64 total = qRound64(milliseconds);
    return QStringLiteral("%1:%2:%3.%4")
        .arg(total / 3600000, 2, 10, QLatin1Char('0'))
        .arg((total / 60000) % 60, 2, 10, QLatin1Char('0'))
        .arg((total / 1000) % 60, 2, 10, QLatin1Char('0'))
        .arg(total % 1000, 3, 10, QLatin1Char('0'));
}

QString reportToHtml(const QString& report)
{
    const QString droppedPrefix = QStringLiteral("Total dropped frames: ");
    const QString duplicatedPrefix = QStringLiteral("Total duplicated frames: ");
    const QStringList lines = report.split(QLatin1Char('\n'));

    long long droppedTotal = 0;
    long long duplicatedTotal = 0;
    for (const QString& line : lines) {
        if (line.startsWith(droppedPrefix)) {
            droppedTotal = line.mid(droppedPrefix.size()).toLongLong();
        } else if (line.startsWith(duplicatedPrefix)) {
            duplicatedTotal = line.mid(duplicatedPrefix.size()).toLongLong();
        }
    }

    QStringList html;
    html.reserve(lines.size());
    for (const QString& line : lines) {
        const bool droppedLine = line.startsWith(droppedPrefix)
            || line.startsWith(QStringLiteral("Dropped frame numbers: "));
        const bool duplicatedLine = line.startsWith(duplicatedPrefix)
            || line.startsWith(QStringLiteral("Duplicated frames: "));
        const bool red = (droppedLine && droppedTotal > 1) || (duplicatedLine && duplicatedTotal > 1);
        const QString escaped = line.toHtmlEscaped();
        html.append(red ? QStringLiteral("<span style='color:red;'>%1</span>").arg(escaped) : escaped);
    }
    return html.join(QStringLiteral("<br>"));
}

} // namespace

class RoiCanvas final : public QWidget {
public:
    RoiCanvas(QImage image, const QRect& selection, QWidget* parent = nullptr)
        : QWidget(parent), m_image(std::move(image)), m_selection(selection)
    {
        setMinimumSize(320, 240);
        setCursor(Qt::CrossCursor);
    }

    QRect selection() const { return m_selection; }
    std::function<void(bool)> selectionChanged;


    QRectF imageBounds() const
    {
        const QSizeF scaled = QSizeF(m_image.size()).scaled(size(), Qt::KeepAspectRatio);
        return QRectF((width() - scaled.width()) / 2.0, (height() - scaled.height()) / 2.0,
                      scaled.width(), scaled.height());
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::darkGray);
        const QRectF bounds = imageBounds();
        painter.drawImage(bounds, m_image);
        if (!m_selection.isEmpty()) {
            const double sx = bounds.width() / m_image.width();
            const double sy = bounds.height() / m_image.height();
            const QRectF selected(bounds.left() + m_selection.x() * sx,
                                  bounds.top() + m_selection.y() * sy,
                                  m_selection.width() * sx, m_selection.height() * sy);
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(selected);
        }
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton || !imageBounds().contains(event->position())) {
            return;
        }
        m_dragging = true;
        m_start = event->position();
        m_selection = {};
        notifySelection();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (m_dragging) {
            updateSelection(event->position());
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (m_dragging && event->button() == Qt::LeftButton) {
            updateSelection(event->position());
            m_dragging = false;
        }
    }

private:
    void updateSelection(const QPointF& end)
    {
        const QRectF bounds = imageBounds();
        auto imagePoint = [&](const QPointF& point) {
            const double x = std::clamp((point.x() - bounds.left()) / bounds.width(), 0.0, 1.0);
            const double y = std::clamp((point.y() - bounds.top()) / bounds.height(), 0.0, 1.0);
            return QPointF(x * m_image.width(), y * m_image.height());
        };
        const QPointF first = imagePoint(m_start);
        const QPointF last = imagePoint(end);
        if (first.x() == last.x() || first.y() == last.y()) {
            m_selection = {};
        } else {
            const int left = qRound(std::min(first.x(), last.x()));
            const int top = qRound(std::min(first.y(), last.y()));
            const int right = qRound(std::max(first.x(), last.x()));
            const int bottom = qRound(std::max(first.y(), last.y()));
            m_selection = QRect(left, top, right - left, bottom - top);
        }
        notifySelection();
    }

    void notifySelection()
    {
        update();
        if (selectionChanged) {
            selectionChanged(!m_selection.isEmpty());
        }
    }

    QImage m_image;
    QRect m_selection;
    QPointF m_start;
    bool m_dragging = false;
};

class TaskWorker final : public QThread {
    Q_OBJECT

public:
    enum class Task {
        AnalyzeFrames,
        ScanImages
    };

    TaskWorker(Task task, QString folder, int x1, int x2, int y1, int y2)
        : m_task(task)
        , m_folder(std::move(folder))
        , m_x1(x1)
        , m_x2(x2)
        , m_y1(y1)
        , m_y2(y2)
    {
    }

signals:
    void progressChanged(int completed, int total, const QString& fileName);
    void completed(const QString& message, const QString& reportPath, const QString& error);

protected:
    void run() override
    {
        try {
            const QFileInfo source(m_folder);
            if (isVideoFile(source)) {
                if (m_task != Task::AnalyzeFrames) {
                    emit completed(QString(), QString(),
                                   QStringLiteral("Image health scanning requires an image folder."));
                    return;
                }
                analyzeFrames({}, true);
                return;
            }
            if (!source.isDir()) {
                emit completed(QString(), QString(), QStringLiteral("The input source no longer exists."));
                return;
            }
            const QStringList files = imageFiles(m_folder);
            if (files.isEmpty()) {
                emit completed(QString(), QString(),
                               QStringLiteral("No PNG, JPG, JPEG, or BMP images were found."));
                return;
            }
            if (m_task == Task::AnalyzeFrames) {
                analyzeFrames(files, false);
            } else {
                scanImages(files);
            }
        } catch (const cv::Exception& error) {
            emit completed(QString(), QString(),
                           QStringLiteral("OpenCV failed: %1").arg(QString::fromUtf8(error.what())));
        }
    }

private:
    void analyzeFrames(const QStringList& files, bool video)
    {
        VideoSource source;
        int estimatedFrames = 0;
        double fps = 0;
        if (video) {
            if (!source.open(m_folder)) {
                emit completed(QString(), QString(),
                               isBottomUpUncompressedAvi(m_folder)
                                   ? unreadableUncompressedAviMessage()
                                   : QStringLiteral("Could not open video. Check the file and its codec."));
                return;
            }
            const double count = source.frameCount();
            fps = source.fps();
            if (std::isfinite(count) && count >= 1 && count <= std::numeric_limits<int>::max()) {
                estimatedFrames = static_cast<int>(count);
            }
        }

        tesseract::TessBaseAPI reader;
        QByteArray dataPath;
        const QString portableData = QDir(QCoreApplication::applicationDirPath())
                                         .absoluteFilePath(QStringLiteral("tessdata"));
        if (QFileInfo::exists(QDir(portableData).filePath(QStringLiteral("eng.traineddata")))) {
            dataPath = QFile::encodeName(portableData);
        } else if (qEnvironmentVariableIsEmpty("TESSDATA_PREFIX")) {
            const QString executable = QStandardPaths::findExecutable(QStringLiteral("tesseract"));
            const QString bundledData = QDir(QFileInfo(executable).absolutePath())
                                            .absoluteFilePath(QStringLiteral("../share/tessdata"));
            if (!executable.isEmpty()
                && QFileInfo::exists(QDir(bundledData).filePath(QStringLiteral("eng.traineddata")))) {
                dataPath = QFile::encodeName(bundledData);
            }
        }
        if (reader.Init(dataPath.isEmpty() ? nullptr : dataPath.constData(),
                        "eng", tesseract::OEM_LSTM_ONLY) != 0) {
            emit completed(QString(), QString(),
                           QStringLiteral("Tesseract could not initialize the English OCR data. "
                                          "Install Tesseract and its English traineddata."));
            return;
        }
        reader.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
        reader.SetVariable("tessedit_char_whitelist", "0123456789");

        QVector<CounterReading> extracted;
        QStringList failures;
        std::set<long long> seen;
        QStringList duplicates;
        QStringList warnings;
        std::map<QByteArray, QString> glyphCache;
        int processed = 0;

        auto processFrame = [&](const cv::Mat& image, const QString& fileName, double timestampMs) {
            if (image.empty()) {
                failures.append(QStringLiteral("%1: could not decode image")
                                    .arg(fileName));
                return;
            }
            if (m_x1 < 0 || m_y1 < 0 || m_x2 <= m_x1 || m_y2 <= m_y1
                || m_x2 > image.cols || m_y2 > image.rows) {
                failures.append(QStringLiteral("%1: crop box is outside the image")
                                    .arg(fileName));
                return;
            }

            const cv::Rect bounds(m_x1, m_y1, m_x2 - m_x1, m_y2 - m_y1);
            const QString digits = extractDigits(reader, image(bounds), glyphCache);
            bool ok = false;
            const long long number = digits.toLongLong(&ok);
            if (!ok) {
                failures.append(QStringLiteral("%1: no frame number recognized")
                                    .arg(fileName));
                return;
            }

            if (!video && !seen.insert(number).second) {
                duplicates.append(QStringLiteral("%1: %2").arg(fileName).arg(number));
            }
            extracted.append({fileName, number, timestampMs, video ? processed - 1 : processed});
        };

        if (video) {
            cv::Mat image;
            double capturedMs = -1;
            while (source.read(image, capturedMs) && !image.empty()) {
                if (processed == std::numeric_limits<int>::max()) {
                    emit completed(QString(), QString(), QStringLiteral("Video has too many frames to process."));
                    return;
                }
                ++processed;
                const QString name = QStringLiteral("Frame %1").arg(processed - 1);
                double timestamp = capturedMs;
                if (!std::isfinite(timestamp) || timestamp < 0 || (processed > 1 && timestamp == 0)) {
                    timestamp = std::isfinite(fps) && fps > 0
                        ? (processed - 1) * 1000.0 / fps : -1;
                }
                processFrame(image, name, timestamp);
                emit progressChanged(processed, estimatedFrames, name);
            }
            if (processed == 0) {
                emit completed(QString(), QString(),
                               QStringLiteral("The video contains no decodable frames."));
                return;
            }
            if (estimatedFrames > processed) {
                warnings.append(QStringLiteral("The video metadata reported %1 frames, but only %2 "
                                               "were decoded. The video may be truncated or damaged.")
                                    .arg(estimatedFrames).arg(processed));
            }
        } else {
            for (int index = 0; index < files.size(); ++index) {
                const QString path = files.at(index);
                const QString name = QFileInfo(path).fileName();
                processFrame(readImage(path), name, -1);
                ++processed;
                emit progressChanged(processed, files.size(), name);
            }
        }

        QVector<long long> sortedNumbers;
        sortedNumbers.reserve(extracted.size());
        for (const CounterReading& frame : extracted) {
            sortedNumbers.append(frame.number);
        }
        std::sort(sortedNumbers.begin(), sortedNumbers.end());

        QVector<MissingRange> missingRanges;
        unsigned long long missingCount = 0;
        const SequenceAnalysis sequence = video ? analyzeSequence(extracted) : SequenceAnalysis();
        if (video) {
            duplicates = sequence.duplicates;
            missingCount = sequence.missingCount;
            for (const CounterGap& gap : sequence.gaps) {
                missingRanges.append(gap.range);
            }
        }
        for (int i = 1; !video && i < sortedNumbers.size(); ++i) {
            const long long previous = sortedNumbers.at(i - 1);
            const long long current = sortedNumbers.at(i);
            if (current > previous && current - previous > 1) {
                const long long first = previous + 1;
                const long long last = current - 1;
                missingRanges.append({first, last});
                missingCount += static_cast<unsigned long long>(last - first) + 1;
            }
        }

        const QString reportFolder = video ? QFileInfo(m_folder).absolutePath() : m_folder;
        const QString reportPath = uniqueReportPath(reportFolder, QStringLiteral("frame_check_results"));
        if (reportPath.isEmpty()) {
            emit completed(QString(), QString(), QStringLiteral("Could not choose a report filename."));
            return;
        }

        QString report;
        QTextStream output(&report);
        const long long expectedFrames = video ? estimatedFrames : files.size();
        QString processedLine = (video ? QStringLiteral("Total video frames processed: ")
                                       : QStringLiteral("Total images processed: "))
            + QString::number(processed);
        if (expectedFrames > 0) {
            processedLine += QStringLiteral(" out of %1 (%2%)")
                                 .arg(expectedFrames)
                                 .arg(qRound(100.0 * processed / expectedFrames));
        }

        output << "Report\n"
               << "Total dropped frames: " << missingCount << '\n'
               << "Dropped frame numbers: [" << formatMissing(missingRanges) << "]\n"
               << "Duplicated frames: [" << duplicates.join(QStringLiteral(", ")) << "]\n"
               << "Total duplicated frames: " << duplicates.size() << '\n'
               << processedLine << '\n'
               << "OCR-recognized frame numbers (not independently verified): " << extracted.size() << '\n'
               << "Unresolved OCR/sequence issues: " << failures.size() + sequence.unresolved.size() << '\n'
               << ((!failures.isEmpty() || !sequence.unresolved.isEmpty())
                       ? "Incomplete analysis: totals exclude unresolved transitions; they are not final.\n" : "")
               << "OCR can misread numbers; review unexpected gaps against the original frames.\n\n"
               << "Analysis details\n"
               << "====================================\n"
               << "OCR method: separated digits with line fallback\n"
               << (video ? "Analysis: adjacent video frames; expected counter increment +1; zero-based indices\n"
                         : "Analysis: sorted image counter numbers\n")
               << (video ? "Video Path: " : "Folder Path: ") << m_folder << '\n'
               << "Crop Box (x1:x2, y1:y2): " << m_x1 << ':' << m_x2 << ", "
               << m_y1 << ':' << m_y2 << "\n\n"
               << "Missing frame details:\n";
        if (missingRanges.isEmpty()) {
            output << "None\n";
        }
        for (qsizetype gapIndex = 0; gapIndex < missingRanges.size(); ++gapIndex) {
            const MissingRange& range = missingRanges.at(gapIndex);
            const CounterGap* gap = video ? &sequence.gaps.at(gapIndex) : nullptr;
            const bool hasLocation = gap && gap->before.timestampMs >= 0
                && gap->after.timestampMs > gap->before.timestampMs;
            if (range.last - range.first > 10000) {
                output << "Counters " << range.first << '-' << range.last
                       << ": large observed gap; verify OCR before treating it as a real drop.\n";
                continue;
            }
            for (long long number = range.first; number <= range.last; ++number) {
                output << "Counter " << number;
                if (video) {
                    if (hasLocation) {
                        const double fraction = static_cast<double>(number - range.first + 1)
                            / (static_cast<double>(range.last - range.first) + 2.0);
                        output << " | Estimated video time: "
                               << videoTime(gap->before.timestampMs
                                            + (gap->after.timestampMs - gap->before.timestampMs) * fraction)
                               << " | Next observed: " << gap->after.fileName
                               << " at " << videoTime(gap->after.timestampMs);
                    } else {
                        output << " | Estimated video time: unavailable (missing timestamps)";
                    }
                }
                output << '\n';
            }
        }
        output << "\nExtracted frame numbers by file:\n";
        for (const CounterReading& frame : extracted) {
            output << frame.fileName << ": " << frame.number;
            if (video) {
                output << " | Video time: " << videoTime(frame.timestampMs);
            }
            output << '\n';
        }
        output << "\nSorted frame number list:\n";
        for (const long long number : sortedNumbers) {
            output << number << '\n';
        }
        if (!failures.isEmpty()) {
            output << "\n" << (video ? "Video frames" : "Images") << " without a usable frame number:\n";
            for (const QString& failure : failures) {
                output << failure << '\n';
            }
        }
        if (!sequence.unresolved.isEmpty()) {
            output << "\nUnresolved OCR/sequence transitions:\n";
            for (const QString& issue : sequence.unresolved) {
                output << issue << '\n';
            }
        }
        for (const QString& warning : warnings) {
            output << "\nWarning: " << warning << '\n';
        }
        output.flush();

        QString error;
        if (!writeReport(reportPath, report, &error)) {
            emit completed(QString(), QString(), error);
            return;
        }

        const QString summary = QStringLiteral(
            "Processed %1 frames; recognized %2 frame numbers, found %3 duplicate(s) "
            "and %4 missing frame number(s).")
                                    .arg(processed)
                                    .arg(extracted.size())
                                    .arg(duplicates.size())
                                    .arg(missingCount);
        if (!failures.isEmpty() || !sequence.unresolved.isEmpty()) {
            warnings.append(QStringLiteral("Analysis is incomplete: %1 unreadable or suspect OCR/sequence "
                                           "issue(s). Drop/duplicate totals are provisional.")
                                .arg(failures.size() + sequence.unresolved.size()));
        }
        emit completed(warnings.isEmpty() ? summary : summary + QStringLiteral("\nWarning: ")
                                                          + warnings.join(QStringLiteral("\n")),
                       reportPath, QString());
    }

    void scanImages(const QStringList& files)
    {
        QStringList corrupted;
        for (int index = 0; index < files.size(); ++index) {
            const QString path = files.at(index);
            emit progressChanged(index + 1, files.size(), QFileInfo(path).fileName());
            if (readImage(path).empty()) {
                corrupted.append(QStringLiteral("%1 - Failed to load or decode image")
                                     .arg(QFileInfo(path).fileName()));
            }
        }

        const QString reportPath = uniqueReportPath(m_folder, QStringLiteral("file_tester_report"));
        if (reportPath.isEmpty()) {
            emit completed(QString(), QString(), QStringLiteral("Could not choose a report filename."));
            return;
        }

        QString report;
        QTextStream output(&report);
        output << "Corrupted Image File Report\n"
               << "====================================\n\n"
               << "Scanned folder: " << m_folder << '\n'
               << "Total files scanned: " << files.size() << "\n\n"
               << "Corrupted files:\n";
        for (const QString& file : corrupted) {
            output << file << '\n';
        }
        output << "\nTotal corrupted files: " << corrupted.size() << '\n';
        output.flush();

        QString error;
        if (!writeReport(reportPath, report, &error)) {
            emit completed(QString(), QString(), error);
            return;
        }

        emit completed(QStringLiteral("Scanned %1 images; found %2 corrupted image(s).")
                           .arg(files.size())
                           .arg(corrupted.size()),
                       reportPath, QString());
    }

    Task m_task;
    QString m_folder;
    int m_x1;
    int m_x2;
    int m_y1;
    int m_y2;
};

void applyTheme(bool dark)
{
    if (!dark) {
        QPalette light = QApplication::style()->standardPalette();
        light.setColor(QPalette::Window, QColor(0xf1, 0xf5, 0xf9));
        light.setColor(QPalette::Highlight, QColor(0x0f, 0x76, 0x6e));
        light.setColor(QPalette::HighlightedText, QColor(0xff, 0xff, 0xff));
        qApp->setPalette(light);
        qApp->setStyleSheet(QStringLiteral(R"(
            QGroupBox { background: #ffffff; border: 1px solid #d8dee8; border-radius: 10px; margin-top: 14px; padding: 12px; }
            QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; color: #0f766e; font-weight: 600; }
            QLineEdit, QSpinBox, QDoubleSpinBox, QTextEdit, QPlainTextEdit {
                background: #ffffff; border: 1px solid #cbd5e1; border-radius: 4px; padding: 4px 6px; color: #1e293b;
                selection-background-color: #0f766e; selection-color: #ffffff; }
            QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus { border: 1px solid #0f766e; }
            QPushButton { background: #f8fafc; border: 1px solid #cbd5e1; border-radius: 4px; padding: 5px 12px; color: #1e293b; }
            QPushButton:hover { background: #e2e8f0; }
            QPushButton:pressed { background: #cbd5e1; }
            QPushButton:disabled { color: #94a3b8; background: #f1f5f9; border-color: #e2e8f0; }
            QPushButton#primaryButton { background: #0f766e; border-color: #0f766e; color: #ffffff; font-weight: 600; }
            QPushButton#primaryButton:hover { background: #115e59; }
            QPushButton#primaryButton:disabled { background: #cbd5e1; border-color: #cbd5e1; color: #64748b; }
            QPushButton#helpButton { border-radius: 15px; padding: 0; font-weight: 700; }
            QProgressBar { background: #e2e8f0; border: none; border-radius: 4px; height: 8px; text-align: center; color: transparent; }
            QProgressBar::chunk { background: #14b8a6; border-radius: 4px; }
            QLabel { background: transparent; }
            QLabel#previewLabel { background: #f8fafc; border: 1px dashed #cbd5e1; border-radius: 6px; color: #64748b; }
            QLabel#statusLabel { color: #0f766e; font-weight: 600; }
        )"));
        return;
    }

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(0x1a, 0x1f, 0x27));
    palette.setColor(QPalette::WindowText, QColor(0xe2, 0xe8, 0xf0));
    palette.setColor(QPalette::Base, QColor(0x1a, 0x1f, 0x27));
    palette.setColor(QPalette::AlternateBase, QColor(0x24, 0x2b, 0x35));
    palette.setColor(QPalette::Text, QColor(0xe2, 0xe8, 0xf0));
    palette.setColor(QPalette::Button, QColor(0x2a, 0x32, 0x3e));
    palette.setColor(QPalette::ButtonText, QColor(0xe2, 0xe8, 0xf0));
    palette.setColor(QPalette::Highlight, QColor(0x0f, 0x76, 0x6e));
    palette.setColor(QPalette::HighlightedText, QColor(0xff, 0xff, 0xff));
    palette.setColor(QPalette::PlaceholderText, QColor(0x64, 0x74, 0x8b));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(0x64, 0x74, 0x8b));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x64, 0x74, 0x8b));
    qApp->setPalette(palette);
    qApp->setStyleSheet(QStringLiteral(R"(
        QGroupBox { background: #242b35; border: 1px solid #333c48; border-radius: 10px; margin-top: 14px; padding: 12px; }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; color: #2dd4bf; font-weight: 600; }
        QLineEdit, QSpinBox, QDoubleSpinBox, QTextEdit, QPlainTextEdit {
            background: #1a1f27; border: 1px solid #3b4656; border-radius: 4px; padding: 4px 6px; color: #e2e8f0;
            selection-background-color: #0f766e; selection-color: #ffffff; }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus { border: 1px solid #2dd4bf; }
        QPushButton { background: #2a323e; border: 1px solid #3b4656; border-radius: 4px; padding: 5px 12px; color: #e2e8f0; }
        QPushButton:hover { background: #334050; }
        QPushButton:pressed { background: #1f2630; }
        QPushButton:disabled { color: #64748b; background: #222831; border-color: #2c3440; }
        QPushButton#primaryButton { background: #0f766e; border-color: #0f766e; color: #ffffff; font-weight: 600; }
        QPushButton#primaryButton:hover { background: #115e59; }
        QPushButton#primaryButton:disabled { background: #2c3440; border-color: #2c3440; color: #64748b; }
        QPushButton#helpButton { border-radius: 15px; padding: 0; font-weight: 700; }
        QCheckBox { spacing: 6px; }
        QProgressBar { background: #334155; border: none; border-radius: 4px; height: 8px; text-align: center; color: transparent; }
        QProgressBar::chunk { background: #14b8a6; border-radius: 4px; }
        QLabel { background: transparent; }
        QLabel#previewLabel { background: #1a1f27; border: 1px dashed #3b4656; border-radius: 6px; color: #94a3b8; }
        QLabel#statusLabel { color: #2dd4bf; font-weight: 600; }
        QMenuBar, QMenu { background: #242b35; color: #e2e8f0; }
        QMenu::item:selected { background: #0f766e; color: #ffffff; }
    )"));
}

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    MainWindow()
    {
        setWindowTitle(QStringLiteral("Frame Check - Improved OCR"));
        resize(820, 620);

        QSettings settings(QStringLiteral("FrameCheck"), QStringLiteral("FrameCheck"));
        const bool dark = settings.value(QStringLiteral("darkMode"), true).toBool();
        applyTheme(dark);
        auto* viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
        auto* darkAction = viewMenu->addAction(QStringLiteral("Dark mode"));
        darkAction->setCheckable(true);
        darkAction->setChecked(dark);
        connect(darkAction, &QAction::toggled, this, [](bool checked) {
            applyTheme(checked);
            QSettings(QStringLiteral("FrameCheck"), QStringLiteral("FrameCheck"))
                .setValue(QStringLiteral("darkMode"), checked);
        });

        auto* central = new QWidget(this);
        auto* layout = new QVBoxLayout(central);
        auto* inputCard = new QGroupBox(QStringLiteral("1  Choose input"), central);
        auto* folderLayout = new QHBoxLayout(inputCard);
        m_folder = new QLineEdit(central);
        m_folder->setPlaceholderText(QStringLiteral("Choose an image folder or AVI/MP4/MOV video"));
        auto* browse = new QPushButton(QStringLiteral("Image folder..."), central);
        auto* browseVideo = new QPushButton(QStringLiteral("Video file..."), central);
        auto* helpButton = new QPushButton(QStringLiteral("?"), central);
        helpButton->setToolTip(QStringLiteral("Open the user guide"));
        helpButton->setObjectName(QStringLiteral("helpButton"));
        helpButton->setFixedSize(30, 30);
        folderLayout->addWidget(m_folder, 1);
        folderLayout->addWidget(browse);
        folderLayout->addWidget(browseVideo);
        folderLayout->addWidget(helpButton);
        layout->addWidget(inputCard);

        auto* stepTwo = new QGroupBox(QStringLiteral("2  Select counter area"), central);
        auto* stepTwoLayout = new QVBoxLayout(stepTwo);
        auto* cropBox = new QWidget(stepTwo);
        auto* cropLayout = new QGridLayout(cropBox);
        cropLayout->setContentsMargins(0, 0, 0, 0);
        m_x1 = makeCoordinateInput(cropBox);
        m_x2 = makeCoordinateInput(cropBox);
        m_y1 = makeCoordinateInput(cropBox);
        m_y2 = makeCoordinateInput(cropBox);
        cropLayout->addWidget(new QLabel(QStringLiteral("Left (x1):"), cropBox), 0, 0);
        cropLayout->addWidget(m_x1, 0, 1);
        cropLayout->addWidget(new QLabel(QStringLiteral("Right (x2):"), cropBox), 0, 2);
        cropLayout->addWidget(m_x2, 0, 3);
        cropLayout->addWidget(new QLabel(QStringLiteral("Top (y1):"), cropBox), 1, 0);
        cropLayout->addWidget(m_y1, 1, 1);
        cropLayout->addWidget(new QLabel(QStringLiteral("Bottom (y2):"), cropBox), 1, 2);
        cropLayout->addWidget(m_y2, 1, 3);
        stepTwoLayout->addWidget(cropBox);

        auto* cropActions = new QHBoxLayout();
        m_selectButton = new QPushButton(QStringLiteral("Select counter area..."), stepTwo);
        m_previewButton = new QPushButton(QStringLiteral("Preview crop"), stepTwo);
        auto* showCropCheck = new QCheckBox(QStringLiteral("Show crop box"), stepTwo);
        showCropCheck->setChecked(true);
        cropActions->addWidget(m_selectButton);
        cropActions->addWidget(m_previewButton);
        cropActions->addStretch(1);
        cropActions->addWidget(showCropCheck);
        stepTwoLayout->addLayout(cropActions);

        m_preview = new QLabel(QStringLiteral("Crop preview appears here"), stepTwo);
        m_preview->setObjectName(QStringLiteral("previewLabel"));
        m_preview->setAlignment(Qt::AlignCenter);
        m_preview->setMinimumHeight(140);
        stepTwoLayout->addWidget(m_preview);
        layout->addWidget(stepTwo);

        auto* stepThree = new QGroupBox(QStringLiteral("3  Run analysis"), central);
        auto* stepThreeLayout = new QVBoxLayout(stepThree);
        auto* runLayout = new QHBoxLayout();
        m_analyzeButton = new QPushButton(QStringLiteral("Analyze frame numbers"), stepThree);
        m_analyzeButton->setObjectName(QStringLiteral("primaryButton"));
        m_scanButton = new QPushButton(QStringLiteral("Scan image health"), stepThree);
        runLayout->addWidget(m_analyzeButton);
        runLayout->addWidget(m_scanButton);
        runLayout->addStretch(1);
        stepThreeLayout->addLayout(runLayout);

        m_progress = new QProgressBar(stepThree);
        m_progress->setRange(0, 100);
        stepThreeLayout->addWidget(m_progress);
        m_status = new QLabel(QStringLiteral("Select an image folder or video to begin."), stepThree);
        m_status->setObjectName(QStringLiteral("statusLabel"));
        stepThreeLayout->addWidget(m_status);

        m_log = new QTextEdit(stepThree);
        m_log->setReadOnly(true);
        stepThreeLayout->addWidget(m_log, 1);
        layout->addWidget(stepThree, 1);
        setCentralWidget(central);

        connect(browse, &QPushButton::clicked, this, &MainWindow::chooseFolder);
        connect(browseVideo, &QPushButton::clicked, this, &MainWindow::chooseVideo);
        connect(helpButton, &QPushButton::clicked, this, &MainWindow::openUserGuide);
        connect(m_selectButton, &QPushButton::clicked, this, &MainWindow::selectCrop);
        connect(m_previewButton, &QPushButton::clicked, this, &MainWindow::previewCrop);
        connect(m_analyzeButton, &QPushButton::clicked, this, [this] {
            startTask(TaskWorker::Task::AnalyzeFrames);
        });
        connect(m_scanButton, &QPushButton::clicked, this, [this] {
            startTask(TaskWorker::Task::ScanImages);
        });
        connect(showCropCheck, &QCheckBox::toggled, cropBox, &QWidget::setVisible);

        loadSettings();
    }

protected:
    void closeEvent(QCloseEvent* event) override
    {
        if (m_worker && m_worker->isRunning()) {
            m_status->setText(QStringLiteral("Wait for the current scan to finish before closing."));
            event->ignore();
            return;
        }
        QMainWindow::closeEvent(event);
    }

private:
    static QSpinBox* makeCoordinateInput(QWidget* parent)
    {
        auto* input = new QSpinBox(parent);
        input->setRange(0, 1000000);
        return input;
    }

    void loadSettings()
    {
        QSettings settings(QStringLiteral("FrameCheck"), QStringLiteral("FrameCheck"));
        m_folder->setText(settings.value(QStringLiteral("folder")).toString());
        m_x1->setValue(settings.value(QStringLiteral("x1"), 0).toInt());
        m_x2->setValue(settings.value(QStringLiteral("x2"), 0).toInt());
        m_y1->setValue(settings.value(QStringLiteral("y1"), 0).toInt());
        m_y2->setValue(settings.value(QStringLiteral("y2"), 0).toInt());
    }

    void saveSettings()
    {
        QSettings settings(QStringLiteral("FrameCheck"), QStringLiteral("FrameCheck"));
        settings.setValue(QStringLiteral("folder"), m_folder->text().trimmed());
        settings.setValue(QStringLiteral("x1"), m_x1->value());
        settings.setValue(QStringLiteral("x2"), m_x2->value());
        settings.setValue(QStringLiteral("y1"), m_y1->value());
        settings.setValue(QStringLiteral("y2"), m_y2->value());
        settings.sync();
    }

    void chooseFolder()
    {
        const QString folder = QFileDialog::getExistingDirectory(
            this, QStringLiteral("Select image folder"), m_folder->text());
        if (!folder.isEmpty()) {
            m_folder->setText(folder);
            saveSettings();
        }
    }

    // Installed builds ship the guide beside the executable; the build tree keeps it in the project folder.
    void openUserGuide()
    {
        const QString exeDir = QCoreApplication::applicationDirPath();
        const QStringList candidates = {
            QDir(exeDir).filePath(QStringLiteral("USER_GUIDE.html")),
            QDir(exeDir).filePath(QStringLiteral("../USER_GUIDE.html"))
        };
        for (const QString& candidate : candidates) {
            if (QFileInfo::exists(candidate)
                && QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(candidate).absoluteFilePath()))) {
                return;
            }
        }
        QMessageBox::warning(this, QStringLiteral("User guide not found"),
                             QStringLiteral("USER_GUIDE.html could not be found or opened."));
    }

    void chooseVideo()
    {
        const QString video = QFileDialog::getOpenFileName(
            this, QStringLiteral("Select video"), m_folder->text(),
            QStringLiteral("Video files (*.avi *.mp4 *.mov)"));
        if (!video.isEmpty()) {
            m_folder->setText(video);
            saveSettings();
        }
    }

    QString selectedSource() const
    {
        const QString source = m_folder->text().trimmed();
        const QFileInfo info(source);
        if (source.isEmpty() || (!info.isDir() && !isVideoFile(info))) {
            return {};
        }
        return source;
    }

    bool cropFits(const cv::Mat& image) const
    {
        return !image.empty() && m_x1->value() < m_x2->value()
            && m_y1->value() < m_y2->value() && m_x2->value() <= image.cols
            && m_y2->value() <= image.rows;
    }

    bool loadFirstFrame(cv::Mat& image, QString& previewName)
    {
        const QString source = selectedSource();
        if (source.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Input required"),
                                 QStringLiteral("Choose an existing image folder or AVI/MP4/MOV video."));
            return false;
        }

        try {
            if (isVideoFile(QFileInfo(source))) {
                VideoSource frames;
                if (!frames.open(source)) {
                    QMessageBox::warning(this, QStringLiteral("Cannot read video"),
                                         isBottomUpUncompressedAvi(source)
                                             ? unreadableUncompressedAviMessage()
                                             : QStringLiteral("Could not open video. Check the file and its codec."));
                    return false;
                }
                double timestamp = -1;
                if (!frames.read(image, timestamp) || image.empty()) {
                    QMessageBox::warning(this, QStringLiteral("Cannot read video"),
                                         QStringLiteral("Could not decode the first video frame."));
                    return false;
                }
                previewName = QStringLiteral("%1, frame 1").arg(QFileInfo(source).fileName());
            } else {
                const QStringList files = imageFiles(source);
                if (files.isEmpty()) {
                    QMessageBox::warning(this, QStringLiteral("No images"),
                                         QStringLiteral("No PNG, JPG, JPEG, or BMP images were found."));
                    return false;
                }
                image = readImage(files.first());
                if (image.empty()) {
                    QMessageBox::warning(this, QStringLiteral("Cannot read image"),
                                         QStringLiteral("Could not decode the first image."));
                    return false;
                }
                previewName = QFileInfo(files.first()).fileName();
            }
        } catch (const cv::Exception& error) {
            QMessageBox::warning(this, QStringLiteral("Preview failed"),
                                 QString::fromUtf8(error.what()));
            return false;
        }
        return true;
    }

    void selectCrop()
    {
        cv::Mat image;
        QString previewName;
        if (!loadFirstFrame(image, previewName)) {
            return;
        }

        const QRect current = cropFits(image)
            ? QRect(m_x1->value(), m_y1->value(), m_x2->value() - m_x1->value(),
                    m_y2->value() - m_y1->value())
            : QRect();
        QDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("Select counter area"));
        dialog.resize(900, 700);
        auto* layout = new QVBoxLayout(&dialog);
        layout->addWidget(new QLabel(
            QStringLiteral("Drag a rectangle around the counter in %1.").arg(previewName), &dialog));
        auto* canvas = new RoiCanvas(toQImage(image), current, &dialog);
        layout->addWidget(canvas, 1);
        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        buttons->button(QDialogButtonBox::Ok)->setEnabled(!current.isEmpty());
        canvas->selectionChanged = [buttons](bool valid) {
            buttons->button(QDialogButtonBox::Ok)->setEnabled(valid);
        };
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttons);
        if (dialog.exec() != QDialog::Accepted || canvas->selection().isEmpty()) {
            return;
        }

        const QRect selection = canvas->selection();
        m_x1->setMaximum(qMax(m_x1->maximum(), image.cols));
        m_x2->setMaximum(qMax(m_x2->maximum(), image.cols));
        m_y1->setMaximum(qMax(m_y1->maximum(), image.rows));
        m_y2->setMaximum(qMax(m_y2->maximum(), image.rows));
        m_x1->setValue(selection.x());
        m_x2->setValue(selection.x() + selection.width());
        m_y1->setValue(selection.y());
        m_y2->setValue(selection.y() + selection.height());
        showCrop(image, previewName);
    }

    void previewCrop()
    {
        cv::Mat image;
        QString previewName;
        if (!loadFirstFrame(image, previewName)) {
            return;
        }
        if (!cropFits(image)) {
            QMessageBox::warning(this, QStringLiteral("Invalid crop"),
                                 QStringLiteral("The crop box must be inside the first frame."));
            return;
        }
        showCrop(image, previewName);
    }

    void showCrop(const cv::Mat& image, const QString& previewName)
    {
        const cv::Rect bounds(m_x1->value(), m_y1->value(),
                              m_x2->value() - m_x1->value(),
                              m_y2->value() - m_y1->value());
        const QImage preview = toQImage(image(bounds));
        m_preview->setPixmap(QPixmap::fromImage(preview).scaled(
            m_preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_status->setText(QStringLiteral("Preview from %1").arg(previewName));
        saveSettings();
    }

    void startTask(TaskWorker::Task task)
    {
        const QString source = selectedSource();
        if (source.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Input required"),
                                 QStringLiteral("Choose an existing image folder or AVI/MP4/MOV video."));
            return;
        }
        if (task == TaskWorker::Task::ScanImages && isVideoFile(QFileInfo(source))) {
            QMessageBox::warning(this, QStringLiteral("Image folder required"),
                                 QStringLiteral("Image health scanning only supports image folders."));
            return;
        }
        if (task == TaskWorker::Task::AnalyzeFrames
            && (m_x1->value() >= m_x2->value() || m_y1->value() >= m_y2->value())) {
            QMessageBox::warning(this, QStringLiteral("Invalid crop"),
                                 QStringLiteral("Set x2 greater than x1 and y2 greater than y1."));
            return;
        }

        saveSettings();
        m_log->clear();
        m_progress->setValue(0);
        m_status->setText(QStringLiteral("Preparing scan..."));
        setBusy(true);

        m_worker = new TaskWorker(task, source, m_x1->value(), m_x2->value(),
                                  m_y1->value(), m_y2->value());
        m_worker->setParent(this);
        connect(m_worker, &TaskWorker::progressChanged, this,
                [this](int completed, int total, const QString& fileName) {
                    m_progress->setRange(0, total > 0 ? qMax(completed, total) : 0);
                    m_progress->setValue(completed);
                    m_status->setText(total > 0
                        ? QStringLiteral("Processed %1 of approximately %2: %3")
                              .arg(completed).arg(total).arg(fileName)
                        : QStringLiteral("Processed %1: %2").arg(completed).arg(fileName));
                });
        connect(m_worker, &TaskWorker::completed, this,
                [this](const QString& message, const QString& reportPath, const QString& error) {
                    if (!error.isEmpty()) {
                        m_log->append(QStringLiteral("Error: %1").arg(error));
                        m_status->setText(QStringLiteral("Scan failed."));
                        QMessageBox::critical(this, QStringLiteral("Scan failed"), error);
                        return;
                    }
                    m_progress->setRange(0, 100);
                    m_progress->setValue(100);
                    const QString summary = message + QStringLiteral("\nReport saved to: %1").arg(reportPath);
                    m_log->setPlainText(summary);
                    QFile report(reportPath);
                    if (!report.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        const QString error = QStringLiteral("Report saved but could not be displayed: %1")
                                                  .arg(report.errorString());
                        m_log->append(error);
                        m_status->setText(QStringLiteral("Scan complete; report display failed."));
                        QMessageBox::warning(this, QStringLiteral("Report display failed"), error);
                        return;
                    }
                    const QString contents = QString::fromUtf8(report.readAll());
                    if (report.error() != QFileDevice::NoError) {
                        const QString error = QStringLiteral("Could not read the full report: %1")
                                                  .arg(report.errorString());
                        m_log->append(error);
                        m_status->setText(QStringLiteral("Scan complete; report display failed."));
                        QMessageBox::warning(this, QStringLiteral("Report display failed"), error);
                        return;
                    }
                    m_log->setHtml(reportToHtml(QStringLiteral("Full report:\n\n") + contents
                                                + QStringLiteral("\n") + summary));
                    m_status->setText(QStringLiteral("Scan complete."));
                });
        connect(m_worker, &QThread::finished, this, [this] {
            if (m_worker) {
                m_worker->deleteLater();
                m_worker = nullptr;
            }
            setBusy(false);
        });
        m_worker->start();
    }

    void setBusy(bool busy)
    {
        m_selectButton->setEnabled(!busy);
        m_previewButton->setEnabled(!busy);
        m_analyzeButton->setEnabled(!busy);
        m_scanButton->setEnabled(!busy);
    }

    QLineEdit* m_folder = nullptr;
    QSpinBox* m_x1 = nullptr;
    QSpinBox* m_x2 = nullptr;
    QSpinBox* m_y1 = nullptr;
    QSpinBox* m_y2 = nullptr;
    QPushButton* m_selectButton = nullptr;
    QPushButton* m_previewButton = nullptr;
    QPushButton* m_analyzeButton = nullptr;
    QPushButton* m_scanButton = nullptr;
    QLabel* m_preview = nullptr;
    QLabel* m_status = nullptr;
    QProgressBar* m_progress = nullptr;
    QTextEdit* m_log = nullptr;
    TaskWorker* m_worker = nullptr;
};

#ifndef FRAMECHECK_TEST
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("FrameCheck"));
    QCoreApplication::setApplicationName(QStringLiteral("FrameCheck"));

    app.setStyle(QStringLiteral("Fusion"));

    MainWindow window;
    window.show();
    return app.exec();
}
#endif

#include "main.moc"
