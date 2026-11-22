#include "../src/main.cpp"

#include <fstream>
#include <iostream>

namespace {

class TempDirectory {
public:
    TempDirectory() {
        wchar_t tempPath[MAX_PATH]{};
        wchar_t tempFile[MAX_PATH]{};
        if (!GetTempPathW(MAX_PATH, tempPath) ||
            !GetTempFileNameW(tempPath, L"ndr", 0, tempFile)) {
            throw std::runtime_error("Could not allocate a temporary test path.");
        }
        path_ = std::wstring(tempFile) + L".dir";
        DeleteFileW(tempFile);
        if (!CreateDirectoryW(path_.c_str(), nullptr)) {
            throw std::runtime_error("Could not create the temporary test directory.");
        }
    }

    ~TempDirectory() {
        DeleteFileW(File(L"test.mp4").c_str());
        DeleteFileW(File(L"hevc.mp4").c_str());
        DeleteFileW(File(L"test.avi").c_str());
        DeleteFileW(File(L"camera.avi").c_str());
        DeleteFileW(File(L"overflow.avi").c_str());
        DeleteFileW(File(L"lagarith.avi").c_str());
        DeleteFileW(File(L"bitrate.mp4").c_str());
        DeleteFileW(File(L"low-bitrate.mp4").c_str());
        DeleteFileW(File(L"high-bitrate.mp4").c_str());
        DeleteFileW(File(L"frame-rate.avi").c_str());
        DeleteFileW(File(L"frame-rate.mp4").c_str());
        DeleteFileW(File(L"existing.mp4").c_str());
        DeleteFileW(File(L"existing_1.mp4").c_str());
        RemoveDirectoryW(File(L"existing_2.mp4").c_str());
        RemoveDirectoryW(path_.c_str());
    }

    std::wstring File(const wchar_t* name) const {
        return path_ + L"\\" + name;
    }

private:
    std::wstring path_;
};

void Require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void CheckNdiSettings(const TempDirectory& temp) {
    Require(DeviceWebPageUrl("172.22.2.2:5961") == L"http://172.22.2.2/" &&
            DeviceWebPageUrl("[fe80::1]:5961") == L"http://[fe80::1]/" &&
            DeviceWebPageUrl("").empty() && DeviceWebPageUrl("device.local:5961").empty(),
            "The NDI device web page address is incorrect.");

    const std::wstring path = temp.File(L"settings\\settings.ini");
    NdiSettings settings;
    settings.deviceIp = L"172.22.2.2";
    SaveAppSettings(path, settings);
    NdiSettings loaded;
    LoadAppSettings(path, loaded);
    Require(loaded.deviceIp == settings.deviceIp, "The NDI device IP was not saved for the next start.");
    SaveAppSettings(path, NdiSettings{});
    LoadAppSettings(path, loaded);
    Require(loaded.deviceIp.empty(), "The cleared NDI device IP was not saved.");
    Require(AppSettingsPath().size() > 13 &&
            AppSettingsPath().rfind(APP_NAME_W L"\\settings.ini") ==
                AppSettingsPath().size() - std::wcslen(APP_NAME_W L"\\settings.ini"),
            "The settings file is not stored per edition.");
    DeleteFileW(path.c_str());
    RemoveDirectoryW(temp.File(L"settings").c_str());
}

void CheckOutputCollisions(const TempDirectory& temp) {
    const auto path = temp.File(L"existing.mp4");
    Require(!OutputPathAttributes(path), "An unused output path was reported as existing.");
    for (const auto& filePath : {path, temp.File(L"existing_1.mp4")}) {
        HANDLE file = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0, nullptr,
                                  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        Require(file != INVALID_HANDLE_VALUE, "Could not create an existing-output test file.");
        const char marker = 'X';
        DWORD written = 0;
        const bool saved = WriteFile(file, &marker, 1, &written, nullptr) && written == 1;
        CloseHandle(file);
        Require(saved, "Could not write an existing-output test file.");
    }
    Require(CreateDirectoryW(temp.File(L"existing_2.mp4").c_str(), nullptr) != FALSE,
            "Could not create an occupied numbered output directory.");
    Require(NumberedOutputPath(path) == temp.File(L"existing_3.mp4") &&
            NumberedOutputPath(temp.File(L"capture.avi")) == temp.File(L"capture_1.avi") &&
            NumberedOutputPath(temp.File(L"capture")) == temp.File(L"capture_1"),
            "Numbered output did not preserve the extension or skip occupied names.");
    bool rejected = false;
    try { OutputPathAttributes(temp.File(L"invalid<name>.mp4")); }
    catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "An invalid output path was treated as an unused filename.");

    static int action = IDCANCEL;
    static bool safeDefault = false;
    for (const int choice : {IDCANCEL, kOverwriteOutput, kNumberedOutput, kChooseOutput, 0}) {
        action = choice;
        safeDefault = false;
        const UINT_PTR timer = SetTimer(nullptr, 0, 10, [](HWND, UINT, UINT_PTR, DWORD) {
            HWND dialog = GetActiveWindow();
            if (!GetDlgItem(dialog, kNumberedOutput)) return;
            safeDefault = LOWORD(SendMessageW(dialog, DM_GETDEFID, 0, 0)) == IDCANCEL &&
                GetFocus() == GetDlgItem(dialog, IDCANCEL);
            if (action == 0) SendMessageW(dialog, WM_CLOSE, 0, 0);
            else SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(action, BN_CLICKED), 0);
        });
        Require(timer != 0, "Could not schedule the existing-output dialog test.");
        struct StopTimer {
            UINT_PTR timer;
            ~StopTimer() { KillTimer(nullptr, timer); }
        } stop{timer};
        const INT_PTR result = ChooseExistingOutputAction(path);
        Require(safeDefault && result == (choice == 0 ? IDCANCEL : choice),
                "Existing-output dialog did not default to Cancel or return the chosen action.");
    }
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    Require(file != INVALID_HANDLE_VALUE, "Existing recording disappeared during collision handling.");
    char marker = 0;
    DWORD read = 0;
    const bool unchanged = ReadFile(file, &marker, 1, &read, nullptr) && read == 1 &&
        marker == 'X' && GetFileSize(file, nullptr) == 1;
    CloseHandle(file);
    Require(unchanged, "Choosing an existing-output action modified the existing file prematurely.");
}

void CheckFrameRateConversion() {
    Require(ParseFrameRate(L"1").numerator == 1 &&
            ParseFrameRate(L"85").numerator == 85 &&
            ParseFrameRate(L"29.970").numerator == 29970 &&
            ParseFrameRate(L"29.970").denominator == 1000 &&
            ParseFrameRate(L"7.5").numerator == 75 &&
            ParseFrameRate(L"7.5").denominator == 10,
            "Custom FPS did not preserve valid integer and fractional rates.");
    for (const wchar_t* invalid : {L"", L"0", L"0.999", L"85.001", L"86", L"-1",
                                   L"NaN", L"1e1", L"1.", L".5", L"1.0001", L"1..2",
                                   L" 30", L"30fps", L"999999999999999999"}) {
        bool rejected = false;
        try { ParseFrameRate(invalid); } catch (const std::runtime_error&) { rejected = true; }
        Require(rejected, "Custom FPS accepted an invalid value.");
    }

    class FrameCollector final : public VideoWriter {
    public:
        void Write(const NDIlib_video_frame_v2_t& frame) override {
            pixels.push_back(frame.p_data[0]);
            rate = {frame.frame_rate_N, frame.frame_rate_D};
        }
        void Finish() override {}
        std::vector<uint8_t> pixels;
        FrameRate rate{};
    };
    const auto makeFrame = [](uint8_t pixel, FrameRate rate) {
        auto frame = std::make_shared<CameraFrame>();
        frame->width = 2;
        frame->height = 2;
        frame->rateN = rate.numerator;
        frame->rateD = rate.denominator;
        frame->pixels.assign(16, pixel);
        return frame;
    };
    const auto start = std::chrono::steady_clock::time_point{};
    struct ConversionCase { FrameRate source; FrameRate target; int frames; };
    for (const auto& test : std::vector<ConversionCase>{
             {{60, 1}, {30, 1}, 120}, {{60, 1}, {15, 1}, 120},
             {{60, 1}, {15, 2}, 120}, {{15, 1}, {85, 1}, 30},
             {{60000, 1001}, {29970, 1000}, 120}, {{60, 1}, {1, 1}, 120}}) {
        FrameCollector writer;
        FrameRateConverter converter(writer, test.target);
        for (int i = 0; i < test.frames; ++i) {
            converter.Write(makeFrame(static_cast<uint8_t>(i), test.source),
                start + std::chrono::nanoseconds(FrameTime(
                    i, test.source.numerator, test.source.denominator, 1'000'000'000)));
        }
        const auto duration = std::chrono::nanoseconds(FrameTime(
            test.frames, test.source.numerator, test.source.denominator, 1'000'000'000));
        converter.Finish(start + duration);
        const int64_t countNumerator =
            int64_t{test.frames} * test.source.denominator * test.target.numerator;
        const int64_t countDenominator =
            int64_t{test.source.numerator} * test.target.denominator;
        Require(writer.pixels.size() ==
                    static_cast<size_t>((countNumerator + countDenominator - 1) / countDenominator) &&
                writer.rate.numerator == test.target.numerator &&
                writer.rate.denominator == test.target.denominator,
                "FPS conversion produced the wrong frame count or output rate.");
        for (size_t i = 0; i < writer.pixels.size(); ++i) {
            const auto expected = i * test.source.numerator * test.target.denominator /
                (int64_t{test.source.denominator} * test.target.numerator);
            Require(writer.pixels[i] == expected,
                    "FPS conversion selected the wrong source frame.");
        }
    }

    FrameCollector writer;
    FrameRateConverter converter(writer, {10, 1});
    converter.Write(makeFrame(0, {60, 1}), start);
    converter.Write(makeFrame(1, {60, 1}), start + std::chrono::milliseconds(100));
    converter.Write(makeFrame(2, {60, 1}), start + std::chrono::milliseconds(900));
    converter.Finish(start + std::chrono::seconds(1));
    Require(writer.pixels == std::vector<uint8_t>({0, 1, 1, 1, 1, 1, 1, 1, 1, 2}),
            "FPS conversion did not preserve duration across irregular capture intervals.");
    auto changed = makeFrame(3, {60, 1});
    changed->width = 1;
    bool rejected = false;
    try { converter.Write(changed, start + std::chrono::seconds(1)); }
    catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "FPS conversion hid a source size change.");
}

void CheckPrivacyMasks() {
    class FrameCollector final : public VideoWriter {
    public:
        void Write(const NDIlib_video_frame_v2_t& frame) override {
            width = frame.xres;
            height = frame.yres;
            stride = VideoStride(frame);
            pixels.resize(static_cast<size_t>(stride) * height);
            for (int y = 0; y < height; ++y) {
                std::copy_n(frame.p_data + static_cast<size_t>(y) * stride,
                            stride, pixels.data() + static_cast<size_t>(y) * stride);
            }
        }
        void Finish() override { finished = true; }
        int width = 0;
        int height = 0;
        int stride = 0;
        bool finished = false;
        std::vector<uint8_t> pixels;
    };

    constexpr int width = 4;
    constexpr int height = 3;
    constexpr int stride = 20;
    std::vector<uint8_t> source(static_cast<size_t>(stride) * height, 99);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            auto* pixel = source.data() + static_cast<size_t>(y) * stride + x * 4;
            pixel[0] = static_cast<uint8_t>(x + 1);
            pixel[1] = static_cast<uint8_t>(y + 2);
            pixel[2] = static_cast<uint8_t>(x + y + 3);
            pixel[3] = 255;
        }
    }
    NDIlib_video_frame_v2_t frame{};
    frame.xres = width;
    frame.yres = height;
    frame.FourCC = NDIlib_FourCC_video_type_BGRX;
    frame.line_stride_in_bytes = stride;
    frame.p_data = source.data();

    auto collector = std::make_unique<FrameCollector>();
    auto* captured = collector.get();
    PrivacyMaskWriter writer(std::move(collector), {
        {0.25, 1.0 / 3.0, 0.75, 2.0 / 3.0},
        {0.75, 0, 1, 1}
    });
    writer.Write(frame);
    writer.Finish();

    Require(captured->finished && captured->width == width && captured->height == height &&
            captured->stride == width * 4,
            "Privacy mask writer did not preserve the frame dimensions or finish.");
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto* output = captured->pixels.data() +
                static_cast<size_t>(y) * captured->stride + x * 4;
            const bool masked = (y == 1 && (x == 1 || x == 2)) || x == 3;
            if (masked) {
                Require(output[0] == 128 && output[1] == 128 &&
                        output[2] == 128 && output[3] == 255,
                        "A selected privacy mask did not produce an opaque gray rectangle.");
            } else {
                const auto* original = source.data() + static_cast<size_t>(y) * stride + x * 4;
                Require(std::equal(output, output + 4, original),
                        "Privacy masking changed pixels outside the selected rectangles.");
            }
        }
    }
    Require(source[static_cast<size_t>(stride) + 4] == 2,
            "Privacy masking modified the original capture frame.");
}

void CheckQueuedFrameRate(const std::wstring& path, RecordingFormat format,
                         std::optional<FrameRate> rate) {
    std::cout << "Checking queued FPS: format " << static_cast<int>(format)
              << ", rate " << (rate ? std::to_string(rate->numerator) + "/" +
                  std::to_string(rate->denominator) : "source") << '\n';
    CameraQueue queue;
    queue.recording = true;
    const auto start = std::chrono::steady_clock::now();
    const int width = format == RecordingFormat::HevcMp4 ? 1280 : 96;
    const int height = format == RecordingFormat::HevcMp4 ? 720 : 64;
    for (int i = 0; i < 120; ++i) {
        auto frame = std::make_shared<CameraFrame>();
        frame->width = width;
        frame->height = height;
        frame->rateN = 60;
        frame->rateD = 1;
        frame->pixels.assign(static_cast<size_t>(width) * height * 4, static_cast<uint8_t>(i));
        Require(QueueFrame(queue, frame), "FPS test queue rejected a frame.");
        queue.frames.back().received = start + std::chrono::nanoseconds(
            FrameTime(i, 60, 1, 1'000'000'000));
    }
    StopQueue(queue, {}, start + std::chrono::seconds(2));
    RecordingOptions options;
    options.path = path;
    options.format = format;
    options.frameRate = rate;
    g_closing.store(true);
    struct RestoreClosing { ~RestoreClosing() { g_closing.store(false); } } restore;
    Require(WriteQueuedFrames(queue, options), "FPS test did not write a recording.");
    const FrameRate expectedRate = rate.value_or(FrameRate{60, 1});
    const int64_t expectedCount =
        (int64_t{2} * expectedRate.numerator + expectedRate.denominator - 1) /
        expectedRate.denominator;
    if (format == RecordingFormat::UncompressedAvi || format == RecordingFormat::LagarithAvi) {
        AVIFileInit();
        PAVIFILE file = nullptr;
        PAVISTREAM stream = nullptr;
        struct CloseAvi {
            PAVIFILE& file;
            PAVISTREAM& stream;
            ~CloseAvi() {
                if (stream) AVIStreamRelease(stream);
                if (file) AVIFileRelease(file);
                AVIFileExit();
            }
        } close{file, stream};
        CheckAviResult(AVIFileOpen(&file, path.c_str(), OF_READ, nullptr), L"Opening FPS test AVI");
        CheckAviResult(AVIFileGetStream(file, &stream, streamtypeVIDEO, 0), L"Reading FPS test AVI");
        AVISTREAMINFO info{};
        CheckAviResult(AVIStreamInfo(stream, &info, sizeof(info)), L"Reading FPS stream info");
        Require(int64_t{info.dwRate} * expectedRate.denominator ==
                    int64_t{info.dwScale} * expectedRate.numerator &&
                AVIStreamLength(stream) == expectedCount,
                "AVI did not save the selected FPS and duration.");
    } else {
        ComPtr<IMFSourceReader> reader;
        CheckHResult(MFCreateSourceReaderFromURL(path.c_str(), nullptr, &reader),
                     L"Opening FPS test MP4");
        const DWORD stream = static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
        ComPtr<IMFMediaType> type;
        CheckHResult(reader->GetNativeMediaType(stream, 0, &type), L"Reading FPS test format");
        UINT32 numerator = 0, denominator = 0;
        CheckHResult(MFGetAttributeRatio(type.Get(), MF_MT_FRAME_RATE, &numerator, &denominator),
                     L"Reading recorded FPS");
        Require(numerator > 0 && denominator > 0 &&
                std::abs(10'000'000.0 * denominator / numerator -
                    10'000'000.0 * expectedRate.denominator / expectedRate.numerator) <= 1.0,
                "MP4 FPS differs from the requested rate by more than one 100 ns tick per frame.");
        int64_t count = 0, end = 0;
        for (;;) {
            DWORD flags = 0;
            ComPtr<IMFSample> sample;
            CheckHResult(reader->ReadSample(stream, 0, nullptr, &flags, nullptr, &sample),
                         L"Reading FPS test sample");
            if (sample) {
                LONGLONG time = 0, duration = 0;
                CheckHResult(sample->GetSampleTime(&time), L"Reading FPS sample time");
                CheckHResult(sample->GetSampleDuration(&duration), L"Reading FPS sample duration");
                Require(time >= end, "MP4 FPS sample timestamps overlap.");
                end = time + duration;
                ++count;
            }
            if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
        }
        const int64_t expectedEnd = FrameTime(expectedCount, expectedRate.numerator,
                                               expectedRate.denominator, 10'000'000);
        Require(count == expectedCount && std::abs(end - expectedEnd) <= 10'000,
                "MP4 FPS conversion changed the frame count or recording duration.");
    }
}

#ifdef HAVE_EPIPHAN_SDK
void CheckEpiphanFrames() {
    std::vector<uint8_t> pixels{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    V2U_GrabFrame2 frame{};
    frame.pixbuf = pixels.data();
    frame.pixbuflen = static_cast<V2U_UINT32>(pixels.size());
    frame.imagelen = frame.pixbuflen;
    frame.palette = V2U_GRABFRAME_FORMAT_BGR24;
    frame.crop.width = 2;
    frame.crop.height = 2;
    frame.mode.vfreq = 60000;
    auto copy = CopyEpiphanFrame(frame, 25000);
    Require(copy->pixels == std::vector<uint8_t>({1, 2, 3, 255, 4, 5, 6, 255,
                                                  7, 8, 9, 255, 10, 11, 12, 255}) &&
            copy->rateN == 25000 && copy->rateD == 1000 && copy->signalRateMilliHz == 60000,
            "Epiphan BGR frame conversion or rate metadata is incorrect.");
    for (int invalid = 0; invalid < 4; ++invalid) {
        V2U_GrabFrame2 bad = frame;
        if (invalid == 0) bad.imagelen--;
        if (invalid == 1) bad.pixbuflen--;
        if (invalid == 2) bad.palette |= V2U_GRABFRAME_BOTTOM_UP_FLAG;
        if (invalid == 3) bad.retcode = V2UERROR_NOSIGNAL;
        bool rejected = false;
        try { CopyEpiphanFrame(bad, 25000); } catch (const std::runtime_error&) { rejected = true; }
        Require(rejected, "Invalid Epiphan frame was accepted.");
    }

    EpiphanRate rate;
    const auto start = std::chrono::steady_clock::time_point{};
    Require(rate.Add(start) == 0 &&
            rate.Add(start + std::chrono::milliseconds(960)) == 0,
            "Epiphan rate should wait for a measurement interval.");
    EpiphanRate regular;
    int measured = 0;
    for (int i = 0; i <= 50; ++i)
        measured = regular.Add(start + std::chrono::milliseconds(i * 40));
    Require(measured == 25000, "Epiphan capture rate measurement is incorrect.");
    V2U_GrabParameters params{};
    V2UAdjRange ranges{};
    params.hshift = -2;
    ranges.hshift_min = -10;
    ranges.hshift_max = 10;
    const auto adjustment = EpiphanAdjustmentDetails(EpiphanAdjustment::Horizontal, params, ranges);
    Require(adjustment.value == -2 && adjustment.minimum == -10 && adjustment.maximum == 10 &&
            adjustment.flag == V2U_FLAG_VALID_HSHIFT,
            "Epiphan adjustment did not use the reported range.");

    EpiphanSettingsContext context;
    context.signal = L"Input: digital";
    UsbDialogTemplate definition;
    HWND dialog = CreateDialogIndirectParamW(GetModuleHandleW(nullptr), &definition.base,
        nullptr, EpiphanSettingsDialogProc, reinterpret_cast<LPARAM>(&context));
    Require(dialog != nullptr, "Could not create Epiphan settings dialog.");
    struct DestroyDialog { HWND window; ~DestroyDialog() { DestroyWindow(window); } } cleanup{dialog};
    Require(!IsWindowEnabled(GetDlgItem(dialog, kUsbPropertyApply)) &&
            !IsWindowEnabled(GetDlgItem(dialog, kUsbPropertyChoice)),
            "Epiphan analog adjustments should be unavailable for digital input.");
}
#endif

#ifdef HAVE_DATAPATH_SDK
long g_fakeDatapathBrightness = 0;
unsigned long g_fakeDatapathWidth = 640;
int g_fakeDatapathSets = 0;
bool g_fakeDatapathRejectSet = false;

unsigned long RGBAPI FakeDatapathSignedMinimum(HRGB, signed long* value) { *value = -10; return 0; }
unsigned long RGBAPI FakeDatapathSignedMaximum(HRGB, signed long* value) { *value = 10; return 0; }
unsigned long RGBAPI FakeDatapathUnsignedMinimum(HRGB, unsigned long* value) { *value = 320; return 0; }
unsigned long RGBAPI FakeDatapathUnsignedMaximum(HRGB, unsigned long* value) { *value = 1920; return 0; }
unsigned long RGBAPI FakeDatapathSetSigned(HRGB, signed long value) {
    if (g_fakeDatapathRejectSet) return 123;
    g_fakeDatapathBrightness = value;
    ++g_fakeDatapathSets;
    return 0;
}
unsigned long RGBAPI FakeDatapathSetUnsigned(HRGB, unsigned long value) {
    if (g_fakeDatapathRejectSet) return 123;
    g_fakeDatapathWidth = value;
    ++g_fakeDatapathSets;
    return 0;
}

struct FakeDatapathControlApi {
    bool missingSetter = false;
    template <typename T> T OptionalFunction(const char* name) {
        const std::string function(name);
        FARPROC address = nullptr;
        if (function == "RGBGetBrightnessMinimum") address = reinterpret_cast<FARPROC>(&FakeDatapathSignedMinimum);
        else if (function == "RGBGetBrightnessMaximum") address = reinterpret_cast<FARPROC>(&FakeDatapathSignedMaximum);
        else if (function == "RGBSetBrightness" && !missingSetter) address = reinterpret_cast<FARPROC>(&FakeDatapathSetSigned);
        else if (function == "RGBGetCaptureWidthMinimum") address = reinterpret_cast<FARPROC>(&FakeDatapathUnsignedMinimum);
        else if (function == "RGBGetCaptureWidthMaximum") address = reinterpret_cast<FARPROC>(&FakeDatapathUnsignedMaximum);
        else if (function == "RGBSetCaptureWidth") address = reinterpret_cast<FARPROC>(&FakeDatapathSetUnsigned);
        return reinterpret_cast<T>(address);
    }
};

int g_fakeDatapathLoads = 0;
int g_fakeDatapathFrees = 0;
unsigned long RGBAPI FakeDatapathLoad(PHRGBDLL library) {
    ++g_fakeDatapathLoads;
    *library = 1;
    return 0;
}
unsigned long RGBAPI FakeDatapathFree(HRGBDLL) {
    ++g_fakeDatapathFrees;
    return 0;
}
struct FakeDatapathRuntimeApi {
    decltype(&RGBLoad) load = FakeDatapathLoad;
    decltype(&RGBFree) free = FakeDatapathFree;
    bool kept = false;
    void KeepLoaded() { kept = true; }
};

void CheckDatapathSharedRuntime() {
    FakeDatapathRuntimeApi api;
    DatapathSharedRuntime runtime;
    g_fakeDatapathLoads = g_fakeDatapathFrees = 0;
    runtime.Acquire(api);
    runtime.Acquire(api);
    Require(g_fakeDatapathLoads == 1, "Datapath views did not share the driver load.");
    Require(runtime.Release(api, false).empty() && g_fakeDatapathFrees == 0,
            "Datapath driver was released while another input was active.");
    Require(runtime.Release(api, false).empty() && g_fakeDatapathFrees == 1,
            "Datapath driver was not released after the last input closed.");
    runtime.Acquire(api);
    runtime.Acquire(api);
    runtime.Release(api, true);
    runtime.Release(api, false);
    Require(api.kept && g_fakeDatapathFrees == 1,
            "An unsafe Datapath close did not keep the shared runtime loaded.");
    bool rejected = false;
    try { runtime.Acquire(api); } catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "Datapath reopened after an unsafe driver close.");
}

void CheckDatapathSettings() {
    FakeDatapathControlApi api;
    g_fakeDatapathSets = 0;
    g_fakeDatapathRejectSet = false;
    std::vector<DatapathSetting> saved;
    SetDatapathControl(api, 0, 0, -5);
    RememberDatapathSetting(saved, 0, -5);
    SetDatapathControl(api, 0, 9, 1280);
    RememberDatapathSetting(saved, 9, 1280);
    RememberDatapathSetting(saved, 0, 7);
    g_fakeDatapathBrightness = 0;
    g_fakeDatapathWidth = 640;
    ApplyDatapathSettings(api, 0, saved);
    Require(saved.size() == 2 && saved[0].control == 9 && saved[1].control == 0 &&
            g_fakeDatapathBrightness == 7 && g_fakeDatapathWidth == 1280,
            "Datapath saved controls were not reapplied in the latest application order.");
    const int applied = g_fakeDatapathSets;
    for (const auto& setting : std::vector<DatapathSetting>{{0, -11}, {0, 11}, {9, -1}, {9, 1921}}) {
        bool rejected = false;
        try { SetDatapathControl(api, 0, setting.control, setting.value); }
        catch (const std::runtime_error&) { rejected = true; }
        Require(rejected, "Datapath accepted a value outside the driver range.");
    }
    Require(g_fakeDatapathSets == applied, "Datapath called the driver for an invalid value.");
    api.missingSetter = true;
    bool rejected = false;
    try { SetDatapathControl(api, 0, 0, 0); }
    catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "Datapath accepted a control unsupported by the runtime.");
    api.missingSetter = false;
    g_fakeDatapathRejectSet = true;
    rejected = false;
    try { ApplyDatapathSettings(api, 0, saved); }
    catch (const std::runtime_error& error) { rejected = std::string(error.what()).find("123") != std::string::npos; }
    Require(rejected, "Datapath did not report a driver failure while restoring settings.");
    g_fakeDatapathRejectSet = false;
    Require(ParseDatapathSetting(L"-10") == -10 && ParseDatapathSetting(L"4294967295") == 4294967295LL,
            "Datapath did not parse signed and unsigned driver values.");
    for (const wchar_t* text : {L"", L"1.5", L"1x", L"99999999999999999999"}) {
        rejected = false;
        try { ParseDatapathSetting(text); } catch (const std::runtime_error&) { rejected = true; }
        Require(rejected, "Datapath accepted an invalid control value.");
    }
}

void CheckDatapathFrames() {
    BITMAPINFOHEADER bitmap{};
    bitmap.biSize = sizeof(bitmap);
    bitmap.biWidth = 2;
    bitmap.biHeight = 2;
    bitmap.biPlanes = 1;
    bitmap.biBitCount = 24;
    bitmap.biCompression = BI_RGB;
    bitmap.biSizeImage = 16;
    std::vector<uint8_t> pixels{7, 8, 9, 10, 11, 12, 0, 0,
                                1, 2, 3, 4, 5, 6, 0, 0};
    auto copy = CopyDatapathFrame(bitmap, pixels.data(), 25000);
    Require(copy->pixels == std::vector<uint8_t>({1, 2, 3, 255, 4, 5, 6, 255,
                                                  7, 8, 9, 255, 10, 11, 12, 255}) &&
            copy->rateN == 25000 && copy->rateD == 1000,
            "Datapath bitmap orientation, stride or rate is incorrect.");
    BITMAPINFOHEADER invalid = bitmap;
    invalid.biSizeImage = 15;
    bool rejected = false;
    try { CopyDatapathFrame(invalid, pixels.data(), 25000); }
    catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "Datapath accepted a truncated bitmap.");
    struct BitmapWithMasks { BITMAPINFOHEADER header; DWORD masks[3]; } fields{};
    fields.header = bitmap;
    fields.header.biCompression = BI_BITFIELDS;
    fields.masks[0] = 0xff;
    fields.masks[1] = 0xff00;
    fields.masks[2] = 0xff0000;
    rejected = false;
    try { CopyDatapathFrame(fields.header, pixels.data(), 25000); }
    catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "Datapath accepted unsupported channel masks.");

    DatapathCapture context;
    context.reportedRate.store(25);
    context.rate.Add(std::chrono::steady_clock::now() - std::chrono::seconds(2));
    RGBFRAMEDATA frame{};
    frame.Size = sizeof(frame);
    frame.PBitmapInfo = &bitmap;
    frame.PBitmapBits = pixels.data();
    frame.TimeStamp = 10'000'000;
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.frames.clear();
        g_camera.bytes = 0;
        g_camera.error.clear();
        g_camera.recording = true;
    }
    const ULONG_PTR userData = reinterpret_cast<ULONG_PTR>(&context);
    DatapathFrameCallback(nullptr, 0, &frame, userData);
    frame.TimeStamp += 400'000;
    DatapathFrameCallback(nullptr, 0, &frame, userData);
    Require(context.error.empty() && g_camera.frames.size() == 2,
            "Datapath did not queue continuous timestamped frames.");
    frame.TimeStamp += 1'200'000;
    DatapathFrameCallback(nullptr, 0, &frame, userData);
    Require(context.error.find("gap") != std::string::npos && g_camera.frames.size() == 2 &&
            !QueueIsRecording(g_camera),
            "Datapath did not detect a frame timestamp gap.");
    DatapathCapture missingTimestamp;
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.recording = true;
        g_camera.error.clear();
    }
    frame.TimeStamp = 0;
    DatapathFrameCallback(nullptr, 0, &frame, reinterpret_cast<ULONG_PTR>(&missingTimestamp));
    Require(missingTimestamp.error.find("timestamp") != std::string::npos &&
            !QueueIsRecording(g_camera), "Datapath accepted a frame without a timestamp.");
    DatapathCapture missingRate;
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.recording = true;
        g_camera.error.clear();
    }
    frame.TimeStamp = 10'000'000;
    DatapathFrameCallback(nullptr, 0, &frame, reinterpret_cast<ULONG_PTR>(&missingRate));
    Require(missingRate.error.find("rate") != std::string::npos &&
            !QueueIsRecording(g_camera), "Datapath accepted an unmonitorable frame rate.");
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.recording = true;
        g_camera.error.clear();
    }
    DatapathErrorCallback(nullptr, 0, 123, reinterpret_cast<ULONG_PTR>(&missingRate), nullptr);
    Require(missingRate.driverError.load() == 123 && !QueueIsRecording(g_camera),
            "Datapath driver error did not stop recording.");
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.recording = false;
        g_camera.frames.clear();
        g_camera.bytes = 0;
        g_camera.error.clear();
    }
}
#endif

class FakeUsbVideoControl final :
    public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IAMVideoProcAmp> {
public:
    STDMETHODIMP GetRange(long property, long* minimum, long* maximum, long* step,
                          long* defaultValue, long* flags) override {
        if (property != VideoProcAmp_Brightness) return E_INVALIDARG;
        *minimum = 0;
        *maximum = 10;
        *step = 2;
        *defaultValue = 4;
        *flags = VideoProcAmp_Flags_Auto | VideoProcAmp_Flags_Manual;
        return S_OK;
    }
    STDMETHODIMP Get(long property, long* outputValue, long* flags) override {
        if (property != VideoProcAmp_Brightness) return E_INVALIDARG;
        *outputValue = value_;
        *flags = flags_;
        return S_OK;
    }
    STDMETHODIMP Set(long property, long newValue, long flags) override {
        if (property != VideoProcAmp_Brightness) return E_INVALIDARG;
        value_ = newValue;
        flags_ = flags;
        ++sets_;
        return S_OK;
    }
    LONG value_ = 4;
    LONG flags_ = VideoProcAmp_Flags_Manual;
    int sets_ = 0;
};

void CheckUsbSettingsDialog() {
    UsbSettingsContext context;
    context.modes.push_back({1920, 1080, 60, 1});
    context.selected = context.modes.front();
    auto fake = Microsoft::WRL::Make<FakeUsbVideoControl>();
    Require(fake != nullptr, "Could not create USB control test.");
    context.video = fake;
    context.properties.push_back({L"Brightness", false, VideoProcAmp_Brightness});
    UsbDialogTemplate definition;
    HWND dialog = CreateDialogIndirectParamW(GetModuleHandleW(nullptr), &definition.base,
        nullptr, UsbSettingsDialogProc, reinterpret_cast<LPARAM>(&context));
    Require(dialog != nullptr, "Could not create USB settings dialog.");
    struct DestroyDialog { HWND window; ~DestroyDialog() { DestroyWindow(window); } } cleanup{dialog};
    Require(SendDlgItemMessageW(dialog, kUsbModeChoice, CB_GETCOUNT, 0, 0) == 2 &&
            SendDlgItemMessageW(dialog, kUsbModeChoice, CB_GETCURSEL, 0, 0) == 1,
            "USB format choices or saved selection are incorrect.");
    SetDlgItemInt(dialog, kUsbPropertyValue, 8, TRUE);
    ApplyUsbProperty(dialog, context);
    Require(fake->value_ == 8 && fake->flags_ == VideoProcAmp_Flags_Manual && fake->sets_ == 1,
            "USB manual control setting was not applied.");
    SetDlgItemInt(dialog, kUsbPropertyValue, 9, TRUE);
    bool rejected = false;
    try { ApplyUsbProperty(dialog, context); } catch (const std::runtime_error&) { rejected = true; }
    Require(rejected && fake->sets_ == 1, "Invalid USB control value was applied.");
    CheckDlgButton(dialog, kUsbPropertyAuto, BST_CHECKED);
    ApplyUsbProperty(dialog, context);
    Require(fake->flags_ == VideoProcAmp_Flags_Auto && fake->sets_ == 2,
            "USB automatic control setting was not applied.");
}

void CheckAvi(const std::wstring& path, size_t frameBytes, size_t rowBytes) {
    AVIFileInit();
    PAVIFILE file = nullptr;
    PAVISTREAM stream = nullptr;
    try {
        CheckAviResult(AVIFileOpen(&file, path.c_str(), OF_READ, nullptr), L"Opening test AVI");
        CheckAviResult(AVIFileGetStream(file, &stream, streamtypeVIDEO, 0), L"Reading test AVI stream");
        std::vector<uint8_t> pixels(frameBytes);
        LONG bytes = 0;
        LONG samples = 0;
        CheckAviResult(AVIStreamRead(stream, 0, 1, pixels.data(), static_cast<LONG>(pixels.size()),
                                     &bytes, &samples), L"Reading test AVI frame");
        Require(samples == 1 && bytes == frameBytes, "AVI frame is incomplete.");
        Require(pixels[0] < 20 && pixels[frameBytes - rowBytes] > 230,
                "AVI frame is vertically inverted.");
    } catch (...) {
        if (stream) AVIStreamRelease(stream);
        if (file) AVIFileRelease(file);
        AVIFileExit();
        throw;
    }
    AVIStreamRelease(stream);
    AVIFileRelease(file);
    AVIFileExit();
}

void CheckMp4(const std::wstring& path) {
    ComPtr<IMFAttributes> attributes;
    CheckHResult(MFCreateAttributes(&attributes, 1), L"Creating reader attributes");
    CheckHResult(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE),
                 L"Enabling video processing");
    ComPtr<IMFSourceReader> reader;
    CheckHResult(MFCreateSourceReaderFromURL(path.c_str(), attributes.Get(), &reader),
                 L"Opening test MP4");
    ComPtr<IMFMediaType> type;
    CheckHResult(MFCreateMediaType(&type), L"Creating decoder type");
    CheckHResult(type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), L"Setting decoder type");
    CheckHResult(type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12), L"Selecting decoded NV12");
    const DWORD videoStream = static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
    CheckHResult(reader->SetCurrentMediaType(videoStream, nullptr, type.Get()),
                 L"Configuring decoder");
    ComPtr<IMFMediaType> actualType;
    CheckHResult(reader->GetCurrentMediaType(videoStream, &actualType),
                 L"Getting decoded video format");
    UINT32 width = 0;
    UINT32 height = 0;
    CheckHResult(MFGetAttributeSize(actualType.Get(), MF_MT_FRAME_SIZE, &width, &height),
                 L"Getting decoded frame size");
    Require(width == 96 && height == 64, "Decoded MP4 has the wrong dimensions.");

    ComPtr<IMFSample> sample;
    DWORD flags = 0;
    for (int attempts = 0; attempts < 100 && !sample; ++attempts) {
        CheckHResult(reader->ReadSample(videoStream, 0, nullptr,
                                        &flags, nullptr, &sample), L"Reading test MP4 frame");
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            break;
        }
    }
    Require(sample != nullptr, "MP4 did not contain a decodable frame.");
    ComPtr<IMFMediaBuffer> buffer;
    CheckHResult(sample->ConvertToContiguousBuffer(&buffer), L"Getting decoded frame");
    BYTE* pixels = nullptr;
    DWORD length = 0;
    CheckHResult(buffer->Lock(&pixels, nullptr, &length), L"Locking decoded frame");
    UINT32 stride = width;
    actualType->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride);
    const bool valid = stride >= width && length >= static_cast<uint64_t>(stride) * height;
    const bool upright = valid && pixels[stride * 8 + width / 2] >
        pixels[stride * (height - 8) + width / 2] + 100;
    CheckHResult(buffer->Unlock(), L"Unlocking decoded frame");
    Require(valid, "Decoded MP4 frame has an unsupported layout.");
    Require(upright, "Decoded MP4 frame is vertically inverted.");
}

void CheckMonochromeRecording(const std::wstring& aviPath, const std::wstring& mp4Path) {
    // Strongly colored pixels prove the output uses only the green channel.
    const auto makeFrame = [](int width, int height, std::vector<uint8_t>& pixels, auto green) {
        pixels.assign(static_cast<size_t>(width) * height * 4, 0);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                auto* pixel = pixels.data() + (static_cast<size_t>(y) * width + x) * 4;
                pixel[0] = 10;
                pixel[1] = green(x, y);
                pixel[2] = 250;
                pixel[3] = 255;
            }
        }
        NDIlib_video_frame_v2_t frame{};
        frame.xres = width;
        frame.yres = height;
        frame.FourCC = NDIlib_FourCC_video_type_BGRX;
        frame.frame_rate_N = 30;
        frame.frame_rate_D = 1;
        frame.line_stride_in_bytes = width * 4;
        frame.p_data = pixels.data();
        return frame;
    };

    constexpr int aviWidth = 98;
    constexpr int aviHeight = 64;
    constexpr size_t aviRowBytes = 100;
    const auto aviGreen = [](int x, int y) { return static_cast<uint8_t>(x * 2 + y); };
    std::vector<uint8_t> aviPixels;
    const auto aviFrame = makeFrame(aviWidth, aviHeight, aviPixels, aviGreen);
    RecordingOptions options;
    options.path = aviPath;
    options.format = RecordingFormat::UncompressedAvi;
    options.monochrome = true;
    {
        auto writer = CreateWriter(options, aviFrame);
        writer->Write(aviFrame);
        writer->Finish();
    }
    AVIFileInit();
    PAVIFILE file = nullptr;
    PAVISTREAM stream = nullptr;
    struct CloseAvi {
        PAVIFILE& file;
        PAVISTREAM& stream;
        ~CloseAvi() {
            if (stream) AVIStreamRelease(stream);
            if (file) AVIFileRelease(file);
            AVIFileExit();
        }
    } close{file, stream};
    CheckAviResult(AVIFileOpen(&file, aviPath.c_str(), OF_READ, nullptr), L"Opening monochrome AVI");
    CheckAviResult(AVIFileGetStream(file, &stream, streamtypeVIDEO, 0), L"Reading monochrome AVI");
    struct {
        BITMAPINFOHEADER header;
        RGBQUAD colors[256];
    } format{};
    LONG formatSize = sizeof(format);
    CheckAviResult(AVIStreamReadFormat(stream, 0, &format, &formatSize), L"Reading monochrome format");
    bool grayPalette = formatSize == sizeof(format);
    for (int level = 0; level < 256 && grayPalette; ++level) {
        const auto& color = format.colors[level];
        grayPalette = color.rgbBlue == level && color.rgbGreen == level && color.rgbRed == level;
    }
    Require(format.header.biBitCount == 8 && format.header.biCompression == BI_RGB &&
            format.header.biClrUsed == 256 && grayPalette,
            "Monochrome AVI is not 8-bit grayscale.");
    std::vector<uint8_t> gray(aviRowBytes * aviHeight);
    LONG bytes = 0;
    LONG samples = 0;
    CheckAviResult(AVIStreamRead(stream, 0, 1, gray.data(), static_cast<LONG>(gray.size()),
                                 &bytes, &samples), L"Reading monochrome AVI frame");
    bool greenLevels = samples == 1 && bytes == static_cast<LONG>(gray.size());
    for (int y = 0; y < aviHeight && greenLevels; ++y) {
        for (int x = 0; x < aviWidth && greenLevels; ++x) {
            greenLevels = gray[static_cast<size_t>(y) * aviRowBytes + x] ==
                          aviGreen(x, aviHeight - 1 - y);
        }
    }
    Require(greenLevels, "Monochrome AVI does not contain the source green levels.");

    constexpr int mp4Width = 96;
    constexpr int mp4Height = 64;
    std::vector<uint8_t> mp4Pixels;
    const auto mp4Frame = makeFrame(mp4Width, mp4Height, mp4Pixels,
        [](int, int y) { return static_cast<uint8_t>(y < mp4Height / 2 ? 200 : 40); });
    options.path = mp4Path;
    options.format = RecordingFormat::H264Mp4;
    {
        auto writer = CreateWriter(options, mp4Frame);
        for (int i = 0; i < 5; ++i) writer->Write(mp4Frame);
        writer->Finish();
    }
    ComPtr<IMFAttributes> attributes;
    CheckHResult(MFCreateAttributes(&attributes, 1), L"Creating monochrome reader attributes");
    CheckHResult(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE),
                 L"Enabling monochrome video processing");
    ComPtr<IMFSourceReader> reader;
    CheckHResult(MFCreateSourceReaderFromURL(mp4Path.c_str(), attributes.Get(), &reader),
                 L"Opening monochrome MP4");
    ComPtr<IMFMediaType> type;
    CheckHResult(MFCreateMediaType(&type), L"Creating monochrome decoder type");
    CheckHResult(type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), L"Setting monochrome decoder type");
    CheckHResult(type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12), L"Selecting monochrome NV12");
    const DWORD videoStream = static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
    CheckHResult(reader->SetCurrentMediaType(videoStream, nullptr, type.Get()),
                 L"Configuring monochrome decoder");
    ComPtr<IMFMediaType> actualType;
    CheckHResult(reader->GetCurrentMediaType(videoStream, &actualType), L"Reading monochrome format");
    UINT32 width = 0, height = 0;
    CheckHResult(MFGetAttributeSize(actualType.Get(), MF_MT_FRAME_SIZE, &width, &height),
                 L"Reading monochrome size");
    ComPtr<IMFSample> sample;
    for (int attempts = 0; attempts < 100 && !sample; ++attempts) {
        DWORD flags = 0;
        CheckHResult(reader->ReadSample(videoStream, 0, nullptr, &flags, nullptr, &sample),
                     L"Reading monochrome MP4 frame");
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
    }
    Require(sample != nullptr && width == mp4Width && height == mp4Height,
            "Monochrome MP4 did not contain a decodable frame.");
    ComPtr<IMFMediaBuffer> buffer;
    CheckHResult(sample->ConvertToContiguousBuffer(&buffer), L"Getting monochrome frame");
    BYTE* pixels = nullptr;
    DWORD length = 0;
    CheckHResult(buffer->Lock(&pixels, nullptr, &length), L"Locking monochrome frame");
    UINT32 stride = width;
    actualType->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride);
    const bool valid = stride >= width && length >= static_cast<uint64_t>(stride) * height * 3 / 2;
    int topLuma = 0, bottomLuma = 0, maxChromaOffset = 255;
    if (valid) {
        topLuma = pixels[stride * 8 + width / 2];
        bottomLuma = pixels[stride * (height - 8) + width / 2];
        maxChromaOffset = 0;
        const BYTE* chroma = pixels + static_cast<size_t>(stride) * height;
        for (UINT32 y = 2; y + 2 < height / 2; ++y) {
            for (UINT32 x = 4; x + 4 < width; ++x) {
                maxChromaOffset = std::max(maxChromaOffset,
                    std::abs(static_cast<int>(chroma[y * stride + x]) - 128));
            }
        }
    }
    CheckHResult(buffer->Unlock(), L"Unlocking monochrome frame");
    Require(valid, "Decoded monochrome MP4 frame has an unsupported layout.");
    // Limited-range BT.601/709 luma for gray levels 200 and 40 is about 188 and 50.
    Require(std::abs(topLuma - 188) <= 10 && std::abs(bottomLuma - 50) <= 10,
            "Monochrome MP4 brightness does not follow the source green channel.");
    Require(maxChromaOffset <= 6, "Monochrome MP4 still contains color.");
}

void CheckMultipleNdiViews();

void CheckViewOnlyLayout() {
    const std::wstring title = kWindowTitle;
    const std::wstring build = APP_BUILD_NUMBER;
    Require(title.rfind(APP_NAME_W L" v" APP_VERSION, 0) == 0 &&
            title.find(L"(Build " + build + L")") != std::wstring::npos &&
            !build.empty() && build != L"0" &&
            build.find_first_not_of(L"0123456789") == std::wstring::npos,
            "The application title does not contain its name, version, and build number.");
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSW previewClass{};
    previewClass.hInstance = instance;
    previewClass.lpfnWndProc = PreviewProcedure;
    previewClass.lpszClassName = kPreviewClass;
    Require(RegisterClassW(&previewClass) != 0, "Could not register test preview class.");
    WNDCLASSW viewClass{};
    viewClass.hInstance = instance;
    viewClass.lpfnWndProc = ViewOnlyProcedure;
    viewClass.lpszClassName = kViewClass;
    Require(RegisterClassW(&viewClass) != 0, "Could not register test view class.");
    HWND parent = CreateWindowW(L"STATIC", L"", WS_OVERLAPPED, 0, 0, 800, 700,
                                nullptr, nullptr, instance, nullptr);
    Require(parent != nullptr, "Could not create view test parent.");
    g_viewCount = 2;
    g_viewPanel = CreateWindowW(kViewClass, L"", WS_CHILD, 0, 0, 800, 700,
                                parent, nullptr, instance, nullptr);
    Require(g_viewPanel != nullptr, "Could not create view-only panel.");
    struct Cleanup {
        HWND parent;
        HINSTANCE instance;
        HWND previousWindow = g_window;
        HWND previousSource = g_sourceControl;
        HWND previousViewButton = g_viewButton;
        ~Cleanup() {
            KillTimer(parent, 3);
            g_viewOnly = false;
            g_viewFastTimer = false;
            g_hiddenRecordingControls.clear();
            StopViewSessions();
            DestroyWindow(parent);
            for (auto& slot : g_viewSlots) {
                slot.choice = nullptr;
                slot.preview = nullptr;
                slot.selected.reset();
            }
            g_viewPanel = nullptr;
            g_viewLayout = nullptr;
            g_viewCount = 2;
            g_window = previousWindow;
            g_sourceControl = previousSource;
            g_viewButton = previousViewButton;
            UnregisterClassW(kViewClass, instance);
            UnregisterClassW(kPreviewClass, instance);
        }
    } cleanup{parent, instance};
    g_window = parent;
    g_sourceControl = CreateWindowW(WC_COMBOBOXW, L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 10, 10, 200, 100,
        parent, nullptr, instance, nullptr);
    g_viewButton = CreateWindowW(L"BUTTON", L"View only", WS_CHILD | WS_VISIBLE,
        10, 120, 100, 30, parent, nullptr, instance, nullptr);
    HWND conditional = CreateWindowW(L"EDIT", L"29.97", WS_CHILD,
        10, 160, 100, 25, parent, nullptr, instance, nullptr);
    Require(g_sourceControl && g_viewButton && conditional, "Could not create recording test controls.");
    EnableWindow(g_sourceControl, FALSE);
    EnterViewOnly();
    Require(g_viewOnly && (GetWindowLongW(g_viewPanel, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(g_sourceControl, GWL_STYLE) & WS_VISIBLE) == 0 &&
            (GetWindowLongW(g_viewButton, GWL_STYLE) & WS_VISIBLE) == 0,
            "Recording controls remain visible beneath view-only mode.");
    Require((GetWindowLongW(g_viewSlots[0].preview, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(g_viewSlots[1].preview, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(g_viewSlots[2].preview, GWL_STYLE) & WS_VISIBLE) == 0,
            "The two-view layout shows the wrong number of tiles.");
    RECT ipRect{}, connectRect{}, previewRect{};
    GetWindowRect(g_viewSlots[0].ip, &ipRect);
    GetWindowRect(g_viewSlots[0].connect, &connectRect);
    GetWindowRect(g_viewSlots[0].preview, &previewRect);
    Require((GetWindowLongW(g_viewSlots[0].ip, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(g_viewSlots[1].connect, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(g_viewSlots[2].ip, GWL_STYLE) & WS_VISIBLE) == 0 &&
            ipRect.right <= connectRect.left && ipRect.bottom <= previewRect.top &&
            connectRect.bottom <= previewRect.top,
            "Each visible view needs its own IP field and Connect button above the video.");
    SendMessageW(g_viewLayout, CB_SETCURSEL, 1, 0);
    SendMessageW(g_viewPanel, WM_COMMAND, MAKEWPARAM(kViewLayoutControl, CBN_SELCHANGE), 0);
    Require(g_viewCount == 4 &&
            (GetWindowLongW(g_viewSlots[3].preview, GWL_STYLE) & WS_VISIBLE) != 0,
            "The four-view layout did not show all tiles.");
    RECT first{}, second{}, third{}, fourth{};
    GetWindowRect(g_viewSlots[0].preview, &first);
    GetWindowRect(g_viewSlots[1].preview, &second);
    GetWindowRect(g_viewSlots[2].preview, &third);
    GetWindowRect(g_viewSlots[3].preview, &fourth);
    Require(first.right <= second.left && first.bottom <= third.top &&
            third.right <= fourth.left && second.bottom <= fourth.top,
            "The four-view tiles overlap.");
    CheckMultipleNdiViews();
    g_viewSlots[2].selected = NdiSource{};
    SendMessageW(g_viewLayout, CB_SETCURSEL, 0, 0);
    SendMessageW(g_viewPanel, WM_COMMAND, MAKEWPARAM(kViewLayoutControl, CBN_SELCHANGE), 0);
    Require(g_viewCount == 2 && !g_viewSlots[2].selected &&
            (GetWindowLongW(g_viewSlots[2].preview, GWL_STYLE) & WS_VISIBLE) == 0,
            "Switching to two views did not close the lower views.");
    ExitViewOnly();
    Require(!g_viewOnly && (GetWindowLongW(g_viewPanel, GWL_STYLE) & WS_VISIBLE) == 0 &&
            (GetWindowLongW(g_sourceControl, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(g_viewButton, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(conditional, GWL_STYLE) & WS_VISIBLE) == 0 &&
            !IsWindowEnabled(g_sourceControl),
            "Returning to recording did not preserve conditional visibility and enabled state.");
    EnterViewOnly();
    ExitViewOnly();
    Require((GetWindowLongW(g_viewButton, GWL_STYLE) & WS_VISIBLE) != 0 &&
            (GetWindowLongW(conditional, GWL_STYLE) & WS_VISIBLE) == 0,
            "Repeated screen switches changed recording control visibility.");
}

void CheckViewPreviewCadence() {
    auto frame = std::make_shared<CameraFrame>();
    frame->width = 2;
    frame->height = 2;
    frame->rateN = 30;
    frame->rateD = 1;
    frame->pixels.assign(16, 10);
    PreviewSession view;
    PreviewSession recording(&g_camera);
    PublishCameraFrame(frame, view);
    PublishCameraFrame(frame, recording);
    const auto firstView = view.frame;
    const auto firstRecording = recording.frame;
    view.lastUpdate = std::chrono::steady_clock::now() - std::chrono::milliseconds(40);
    recording.lastUpdate = view.lastUpdate;
    PublishCameraFrame(frame, view);
    PublishCameraFrame(frame, recording);
    Require(view.frame != firstView && recording.frame == firstRecording,
            "View-only previews should update faster without changing recording preview cadence.");
}

void CheckMultipleNdiViews() {
    Require(NDIlib_initialize(), "Could not initialize NDI for view-only test.");
    struct Cleanup {
        std::array<NDIlib_send_instance_t, 4> senders{};
        std::vector<NdiSource> previousSources = g_sources;
        ~Cleanup() {
            StopViewSessions();
            for (auto& slot : g_viewSlots) {
                slot.selected.reset();
                slot.pendingIp.clear();
            }
            DestroyIpFinders();
            for (auto sender : senders) if (sender) NDIlib_send_destroy(sender);
            g_sources = std::move(previousSources);
            UpdateViewSources();
            NDIlib_destroy();
        }
    } state;
    const char* names[] = {"View test A", "View test B", "View test C", "View test D"};
    std::array<std::vector<uint8_t>, 4> pixels;
    std::array<NDIlib_video_frame_v2_t, 4> frames{};
    g_sources.clear();
    for (size_t i = 0; i < g_viewSlots.size(); ++i) {
        NDIlib_send_create_t settings(names[i], nullptr, false, false);
        state.senders[i] = NDIlib_send_create(&settings);
        Require(state.senders[i] != nullptr, "Could not create a view-only test source.");
        const auto* info = NDIlib_send_get_source_name(state.senders[i]);
        Require(info && info->p_ndi_name, "A view-only test source has no name.");
        NdiSource source;
        source.name = Utf8ToWide(info->p_ndi_name);
        source.ndiName = info->p_ndi_name;
        source.address = info->p_url_address ? info->p_url_address : "";
        g_sources.push_back(std::move(source));
        pixels[i].assign(32 * 32 * 4, static_cast<uint8_t>(30 + i * 65));
        frames[i].xres = 32;
        frames[i].yres = 32;
        frames[i].frame_rate_N = 30;
        frames[i].frame_rate_D = 1;
        frames[i].FourCC = NDIlib_FourCC_video_type_BGRX;
        frames[i].line_stride_in_bytes = 32 * 4;
        frames[i].p_data = pixels[i].data();
    }
    UpdateViewSources();
    for (size_t i = 0; i < g_viewSlots.size(); ++i) {
        SendMessageW(g_viewSlots[i].choice, CB_SETCURSEL, i + 1, 0);
        SendMessageW(g_viewPanel, WM_COMMAND,
                     MAKEWPARAM(kViewSourceFirst + static_cast<int>(i), CBN_SELCHANGE), 0);
    }
    std::array<bool, 4> received{};
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (std::chrono::steady_clock::now() < deadline) {
        for (size_t i = 0; i < g_viewSlots.size(); ++i) {
            NDIlib_send_send_video_v2(state.senders[i], &frames[i]);
            std::lock_guard<std::mutex> lock(g_viewSlots[i].session.mutex);
            const auto& preview = g_viewSlots[i].session.frame;
            if (preview && !preview->pixels.empty() &&
                std::abs(static_cast<int>(preview->pixels[0]) - static_cast<int>(30 + i * 65)) < 25)
                received[i] = true;
        }
        if (std::all_of(received.begin(), received.end(), [](bool value) { return value; })) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    Require(std::all_of(received.begin(), received.end(), [](bool value) { return value; }),
            "Four distinct NDI sources did not reach their corresponding views.");
    SendMessageW(g_viewSlots[1].choice, CB_SETCURSEL, 0, 0);
    SendMessageW(g_viewPanel, WM_COMMAND, MAKEWPARAM(kViewSourceFirst + 1, CBN_SELCHANGE), 0);
    CameraFailed("Synthetic view error", g_viewSlots[1].session);
    Require(g_viewSlots[0].session.worker.joinable() && g_viewSlots[2].session.worker.joinable() &&
            g_viewSlots[3].session.worker.joinable() && !g_viewSlots[1].session.worker.joinable(),
            "Stopping one view affected the other sessions.");
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        Require(g_camera.error.empty(), "A view-only error affected the recording queue.");
    }
    // All test senders share this machine's IP; Connect must pick the one not already shown.
    const std::wstring senderIp = NumericSourceAddress(g_sources[1].address);
    if (!senderIp.empty()) {
        SetWindowTextW(g_viewSlots[1].ip, senderIp.c_str());
        SendMessageW(g_viewPanel, WM_COMMAND, MAKEWPARAM(kViewConnectFirst + 1, BN_CLICKED), 0);
        Require(g_viewSlots[1].selected && g_viewSlots[1].selected->ndiName == g_sources[1].ndiName &&
                g_viewSlots[1].pendingIp.empty() && g_viewSlots[1].session.worker.joinable() &&
                g_ipFinders.size() == 1 &&
                SendMessageW(g_viewSlots[1].choice, CB_GETCURSEL, 0, 0) == 2,
                "Connecting a view by IP did not select the free NDI source at that address.");
    }
}

void CheckNdiPreviewAndRecording(const NDIlib_video_frame_v2_t& frame,
                                 const std::wstring& outputPath) {
    Require(NDIlib_initialize(), "Could not initialize NDI for preview test.");
    NDIlib_send_create_t settings("NDI Recorder Preview Test", nullptr, false, false);
    NDIlib_send_instance_t sender = NDIlib_send_create(&settings);
    if (!sender) {
        NDIlib_destroy();
        throw std::runtime_error("Could not create the NDI test source.");
    }
    const NDIlib_source_t* info = NDIlib_send_get_source_name(sender);
    if (!info || !info->p_ndi_name) {
        NDIlib_send_destroy(sender);
        NDIlib_destroy();
        throw std::runtime_error("NDI test source has no name.");
    }
    NdiSource source;
    source.ndiName = info->p_ndi_name;
    source.address = info->p_url_address ? info->p_url_address : "";
    g_stopPreview.store(false);
    g_stopRequested.store(false);
    g_recordStart.store(0);
    g_recordEnd.store(0);
    try {
        g_previewWorker = std::thread(Preview, source, &g_recordPreview);
        g_worker = std::thread(Record, RecordingOptions{source, outputPath, RecordingFormat::H264Mp4});
    } catch (...) {
        StopPreview();
        NDIlib_send_destroy(sender);
        NDIlib_destroy();
        throw;
    }
    std::shared_ptr<const PreviewFrame> preview;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < deadline) {
        NDIlib_send_send_video_v2(sender, &frame);
        {
            std::lock_guard<std::mutex> lock(g_previewMutex);
            preview = g_previewFrame;
        }
        const ULONGLONG start = g_recordStart.load();
        if (preview && start && GetTickCount64() - start >= 1200) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    g_stopRequested.store(true);
    g_worker.join();
    StopPreview();
    NDIlib_send_destroy(sender);
    NDIlib_destroy();
    Require(preview != nullptr, "Preview did not receive video from the local NDI source.");
    Require(g_recordStart.load() != 0 && g_recordEnd.load() >= g_recordStart.load() + 1000,
            "The live NDI recording timer did not start or stop correctly.");
    const size_t top = static_cast<size_t>(preview->width) * 4 * (preview->height / 4);
    const size_t bottom = static_cast<size_t>(preview->width) * 4 * (preview->height * 3 / 4);
    Require(preview->pixels[top] > preview->pixels[bottom] + 100,
            "Live NDI preview is vertically inverted.");
}

void CheckCameraFramesAndRecording(const std::wstring& outputPath) {
    constexpr int width = 96;
    constexpr int height = 64;
    constexpr int stride = width * 3 + 4;
    std::vector<BYTE> pixels(stride * height, 0);
    for (int y = height / 2; y < height; ++y) {
        std::fill_n(pixels.data() + y * stride, width * 3, static_cast<BYTE>(255));
    }
    auto frame = CopyCameraFrame(pixels.data() + (height - 1) * stride,
                                 -stride, width, height, 24, 30, 1);
    Require(frame->pixels[0] == 255 &&
            frame->pixels[static_cast<size_t>(width) * (height - 1) * 4] == 0,
            "Bottom-up VFW RGB24 frame was not converted upright.");

    ComPtr<IMFMediaBuffer> buffer;
    CheckHResult(MFCreateMemoryBuffer(width * height * 4, &buffer), L"Allocating USB test frame");
    BYTE* data = nullptr;
    CheckHResult(buffer->Lock(&data, nullptr, nullptr), L"Locking USB test frame");
    for (int y = 0; y < height; ++y) {
        std::fill_n(data + static_cast<size_t>(y) * width * 4, width * 4,
                    static_cast<BYTE>(y >= height / 2 ? 255 : 0));
    }
    CheckHResult(buffer->Unlock(), L"Unlocking USB test frame");
    CheckHResult(buffer->SetCurrentLength(width * height * 4), L"Setting USB test frame length");
    ComPtr<IMFSample> sample;
    CheckHResult(MFCreateSample(&sample), L"Creating USB test sample");
    CheckHResult(sample->AddBuffer(buffer.Get()), L"Adding USB test buffer");
    ComPtr<IMFMediaType> type;
    CheckHResult(MFCreateMediaType(&type), L"Creating USB test format");
    CheckHResult(type->SetUINT32(MF_MT_DEFAULT_STRIDE, static_cast<UINT32>(-width * 4)),
                 L"Setting USB test stride");
    const auto usbFrame = CopyUsbSample(sample.Get(), type.Get(), width, height, 30, 1);
    Require(usbFrame->pixels[0] == 255 &&
            usbFrame->pixels[static_cast<size_t>(width) * (height - 1) * 4] == 0,
            "Bottom-up USB RGB32 frame was not converted upright.");

    g_closing.store(true);
    g_stopRequested.store(false);
    g_recordStart.store(0);
    g_recordEnd.store(0);
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.error.clear();
        g_camera.frames.clear();
        g_camera.bytes = 0;
        g_camera.recording = true;
    }
    NdiSource camera;
    camera.kind = SourceKind::Usb;
    std::thread recorder(RecordCamera,
        RecordingOptions{camera, outputPath, RecordingFormat::UncompressedAvi});
    PublishCameraFrame(frame);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!g_recordStart.load() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.recording = false;
        g_stopRequested.store(true);
    }
    g_camera.changed.notify_all();
    recorder.join();
    g_closing.store(false);
    Require(g_recordStart.load() != 0, "Shared camera stream did not record a frame.");
    CheckAvi(outputPath, static_cast<size_t>(width) * height * 4, width * 4);

    {
        std::lock_guard<std::mutex> lock(g_previewMutex);
        g_previewFrame.reset();
        g_previewError.clear();
    }
}

void CheckQueueOverflowDrain(const std::wstring& outputPath) {
    CameraQueue queue;
    queue.recording = true;
    constexpr int width = 32;
    constexpr int height = 16;
    auto first = std::make_shared<CameraFrame>();
    first->width = width;
    first->height = height;
    first->rateN = 60;
    first->rateD = 1;
    first->pixels.assign(width * height * 4, 10);
    auto second = std::make_shared<CameraFrame>(*first);
    second->pixels.assign(width * height * 4, 20);
    queue.maxBytes = first->pixels.size() * 2;
    Require(QueueFrame(queue, first) && QueueFrame(queue, second),
            "Recording queue rejected frames within its limit.");
    Require(!QueueFrame(queue, first) && !queue.recording && !queue.error.empty(),
            "Recording queue did not report overflow.");

    g_closing.store(true);
    g_recordStart.store(0);
    g_recordEnd.store(0);
    bool reported = false;
    try {
        WriteQueuedFrames(queue, RecordingOptions{NdiSource{}, outputPath,
                                                    RecordingFormat::UncompressedAvi});
    } catch (const std::runtime_error& error) {
        reported = std::string(error.what()).find("buffer is full") != std::string::npos;
    }
    g_closing.store(false);
    Require(reported, "Queue overflow was not reported after draining.");

    AVIFileInit();
    PAVIFILE file = nullptr;
    PAVISTREAM stream = nullptr;
    try {
        CheckAviResult(AVIFileOpen(&file, outputPath.c_str(), OF_READ, nullptr), L"Opening overflow AVI");
        CheckAviResult(AVIFileGetStream(file, &stream, streamtypeVIDEO, 0), L"Reading overflow AVI");
        Require(AVIStreamLength(stream) == 2, "Overflow discarded accepted frames.");
        std::vector<uint8_t> pixels(width * height * 4);
        for (LONG index = 0; index < 2; ++index) {
            LONG bytes = 0, samples = 0;
            CheckAviResult(AVIStreamRead(stream, index, 1, pixels.data(),
                static_cast<LONG>(pixels.size()), &bytes, &samples), L"Reading overflow frame");
            Require(samples == 1 && bytes == static_cast<LONG>(pixels.size()) &&
                    pixels[0] == (index == 0 ? 10 : 20),
                    "Accepted frames were not saved in order.");
        }
    } catch (...) {
        if (stream) AVIStreamRelease(stream);
        if (file) AVIFileRelease(file);
        AVIFileExit();
        throw;
    }
    AVIStreamRelease(stream);
    AVIFileRelease(file);
    AVIFileExit();
}

void CheckLagarith(const std::wstring& outputPath, HIC codec) {
    Require(ICQueryConfigure(codec) != FALSE, "Lagarith codec settings dialog is unavailable.");
    const LRESULT stateSize = ICGetStateSize(codec);
    Require(stateSize > 0 && stateSize <= 1024 * 1024, "Lagarith state size is invalid.");
    std::vector<uint8_t> state(static_cast<size_t>(stateSize));
    Require(ICGetState(codec, state.data(), static_cast<DWORD>(state.size())) >= 0,
            "Could not retrieve Lagarith state.");
    const LRESULT stateResult = ICSetState(codec, state.data(), static_cast<DWORD>(state.size()));
    std::vector<uint8_t> restored(state.size());
    if (stateResult < 0 ||
        ICGetState(codec, restored.data(), static_cast<DWORD>(restored.size())) < 0 ||
        restored != state) {
        throw std::runtime_error("Lagarith state restore returned " + std::to_string(stateResult));
    }
    constexpr int width = 640;
    constexpr int height = 480;
    std::vector<uint8_t> pixels(width * height * 4);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const size_t offset = static_cast<size_t>(y * width + x) * 4;
            pixels[offset] = static_cast<uint8_t>(x * 200 / width);
            pixels[offset + 1] = static_cast<uint8_t>(y * 200 / height);
            pixels[offset + 2] = static_cast<uint8_t>((x + y) * 200 / (width + height));
            pixels[offset + 3] = 255;
        }
    }
    NDIlib_video_frame_v2_t frame{};
    frame.xres = width;
    frame.yres = height;
    frame.frame_rate_N = 60;
    frame.frame_rate_D = 1;
    frame.FourCC = NDIlib_FourCC_video_type_BGRX;
    frame.line_stride_in_bytes = width * 4;
    frame.p_data = pixels.data();
    RecordingOptions options;
    options.path = outputPath;
    options.format = RecordingFormat::LagarithAvi;
    options.codecState = state;
    {
        auto writer = CreateWriter(options, frame);
        writer->Write(frame);
        writer->Write(frame);
        writer->Finish();
    }

    AVIFileInit();
    PAVIFILE file = nullptr;
    PAVISTREAM stream = nullptr;
    HIC decoder = nullptr;
    bool decompressStarted = false;
    try {
        CheckAviResult(AVIFileOpen(&file, outputPath.c_str(), OF_READ, nullptr), L"Opening Lagarith AVI");
        CheckAviResult(AVIFileGetStream(file, &stream, streamtypeVIDEO, 0), L"Reading Lagarith AVI");
        AVISTREAMINFO info{};
        CheckAviResult(AVIStreamInfo(stream, &info, sizeof(info)), L"Reading Lagarith stream info");
        if (info.fccHandler != kLagarithFourCC || AVIStreamLength(stream) != 2) {
            throw std::runtime_error("Unexpected AVI handler " + std::to_string(info.fccHandler) +
                                     " or frame count " + std::to_string(AVIStreamLength(stream)));
        }
        LONG formatSize = 0;
        CheckAviResult(AVIStreamReadFormat(stream, 0, nullptr, &formatSize), L"Reading compressed format size");
        Require(formatSize >= sizeof(BITMAPINFOHEADER), "Lagarith AVI format is incomplete.");
        std::vector<uint8_t> format(static_cast<size_t>(formatSize));
        CheckAviResult(AVIStreamReadFormat(stream, 0, format.data(), &formatSize), L"Reading compressed format");
        auto* compressed = reinterpret_cast<BITMAPINFOHEADER*>(format.data());
        Require(compressed->biCompression == kLagarithFourCC, "AVI format is not Lagarith.");
        BITMAPINFOHEADER desired{};
        desired.biSize = sizeof(desired);
        desired.biWidth = width;
        desired.biHeight = height;
        desired.biPlanes = 1;
        desired.biBitCount = 32;
        desired.biCompression = BI_RGB;
        desired.biSizeImage = width * height * 4;
        decoder = ICOpen(ICTYPE_VIDEO, kLagarithFourCC, ICMODE_DECOMPRESS);
        Require(decoder != nullptr, "Could not open Lagarith decoder.");
        Require(ICDecompressBegin(decoder, compressed, &desired) == ICERR_OK,
                "Could not start Lagarith decoder.");
        decompressStarted = true;
        std::vector<uint8_t> decoded(static_cast<size_t>(desired.biSizeImage));
        const size_t stride = static_cast<size_t>(width) * 4;
        for (LONG index = 0; index < 2; ++index) {
            LONG compressedBytes = 0, compressedSamples = 0;
            CheckAviResult(AVIStreamRead(stream, index, 1, nullptr, 0, &compressedBytes, &compressedSamples),
                           L"Reading compressed frame size");
            Require(compressedBytes > 0 && compressedSamples == 1, "Lagarith frame is empty.");
            std::vector<uint8_t> encoded(static_cast<size_t>(compressedBytes));
            CheckAviResult(AVIStreamRead(stream, index, 1, encoded.data(), compressedBytes,
                                         &compressedBytes, &compressedSamples), L"Reading Lagarith frame");
            Require(ICDecompress(decoder, 0, compressed, encoded.data(), &desired, decoded.data()) == ICERR_OK,
                    "Could not decode Lagarith frame.");
            for (int y = 0; y < height; ++y) {
                const int row = height - 1 - y;
                for (int x = 0; x < width; ++x) {
                    const auto* actual = decoded.data() + static_cast<size_t>(row) * stride + x * 4;
                    const auto* expected = pixels.data() + static_cast<size_t>(y * width + x) * 4;
                    for (int channel = 0; channel < 3; ++channel) {
                        Require(std::abs(static_cast<int>(actual[channel]) - expected[channel]) <= 4,
                                "Lagarith frame colors or orientation are incorrect.");
                    }
                }
            }
        }
    } catch (...) {
        if (decompressStarted) ICDecompressEnd(decoder);
        if (decoder) ICClose(decoder);
        if (stream) AVIStreamRelease(stream);
        if (file) AVIFileRelease(file);
        AVIFileExit();
        throw;
    }
    ICDecompressEnd(decoder);
    ICClose(decoder);
    AVIStreamRelease(stream);
    AVIFileRelease(file);
    AVIFileExit();
}

void CheckMp4Bitrate(const std::wstring& lowPath, const std::wstring& highPath) {
    constexpr int width = 320;
    constexpr int height = 180;
    std::vector<uint8_t> pixels(width * height * 4);
    NDIlib_video_frame_v2_t frame{};
    frame.xres = width;
    frame.yres = height;
    frame.frame_rate_N = 30;
    frame.frame_rate_D = 1;
    frame.FourCC = NDIlib_FourCC_video_type_BGRX;
    frame.line_stride_in_bytes = width * 4;
    frame.p_data = pixels.data();
    const auto writeRecording = [&](const std::wstring& path, UINT32 bitrate) {
        RecordingOptions options;
        options.path = path;
        options.bitrate = bitrate;
        auto writer = CreateWriter(options, frame);
        uint32_t random = 1234567;
        for (int i = 0; i < 90; ++i) {
            for (auto& pixel : pixels) {
                random ^= random << 13;
                random ^= random >> 17;
                random ^= random << 5;
                pixel = static_cast<uint8_t>(random);
            }
            writer->Write(frame);
        }
        writer->Finish();
        WIN32_FILE_ATTRIBUTE_DATA attributes{};
        Require(GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attributes) != FALSE,
                "Could not inspect bitrate test recording.");
        return (static_cast<uint64_t>(attributes.nFileSizeHigh) << 32) | attributes.nFileSizeLow;
    };
    const uint64_t low = writeRecording(lowPath, 2'000'000);
    const uint64_t high = writeRecording(highPath, 8'000'000);
    Require(high > low * 3 / 2, "MP4 encoder did not respond to the configured bitrate.");
}

void CheckUserGuide(const std::wstring& guide) {
    std::ifstream stream(guide, std::ios::binary);
    Require(stream.good(), "The user guide was not copied next to the application.");
    const std::string html((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    const std::wstring folder = guide.substr(0, guide.find_last_of(L"\\/") + 1);
    Require(html.find("<title>" APP_NAME_A " User Guide</title>") != std::string::npos,
            "The user guide is not titled for this edition.");
    Require(html.find("<!--") == std::string::npos, "The user guide still contains edition markers.");
    for (const char* other : kNdiOnly ? std::vector<const char*>{"Multi-Recorder", "Epiphan", "Datapath", "USB settings"}
                                      : std::vector<const char*>{"NDI-Recorder"}) {
        Require(html.find(other) == std::string::npos,
                (std::string("The user guide mentions another edition: ") + other).c_str());
    }

    int images = 0;
    for (size_t at = html.find("src=\""); at != std::string::npos; at = html.find("src=\"", at + 5)) {
        const size_t start = at + 5;
        const std::string source = html.substr(start, html.find('"', start) - start);
        std::wstring path = folder;
        for (const char character : source) path.push_back(character == '/' ? L'\\' : static_cast<wchar_t>(character));
        Require(GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES,
                ("The user guide references a missing image: " + source).c_str());
        ++images;
    }
    Require(images >= 6, "The user guide should include screenshots.");

    for (size_t at = html.find("href=\"#"); at != std::string::npos; at = html.find("href=\"#", at + 7)) {
        const size_t start = at + 7;
        const std::string anchor = html.substr(start, html.find('"', start) - start);
        Require(html.find("id=\"" + anchor + "\"") != std::string::npos,
                ("The user guide links to a missing section: " + anchor).c_str());
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        ComApartment apartment;
        CheckHResult(MFStartup(MF_VERSION), L"Starting Media Foundation");
        try {
            TempDirectory temp;
            if (argc > 2 && std::string(argv[1]) == "--user-guide") {
                const std::string path = argv[2];
                CheckUserGuide(std::wstring(path.begin(), path.end()));
                MFShutdown();
                std::cout << "User guide and screenshot checks passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--ndi-settings") {
                CheckNdiSettings(temp);
                MFShutdown();
                std::cout << "NDI device page and settings checks passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--ndi-edition") {
                Require(kNdiOnly, "This test binary was not built as the NDI-Recorder edition.");
                g_cameraSources.push_back(NdiSource{});
                RefreshCameraSources();
                Require(g_cameraSources.empty(), "NDI-Recorder must not list USB, VFW or capture-card sources.");
#if defined(HAVE_EPIPHAN_SDK) || defined(HAVE_DATAPATH_SDK)
                Require(false, "NDI-Recorder must not include capture-card SDK support.");
#endif
                MFShutdown();
                std::cout << "NDI-Recorder edition checks passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--frame-rate") {
                CheckFrameRateConversion();
                for (const auto& rate : std::vector<std::optional<FrameRate>>{
                         std::nullopt, FrameRate{30, 1}, FrameRate{15, 1},
                         FrameRate{15, 2}, FrameRate{29970, 1000}, FrameRate{1, 1}, FrameRate{85, 1}}) {
                    CheckQueuedFrameRate(temp.File(L"frame-rate.avi"),
                                         RecordingFormat::UncompressedAvi, rate);
                    CheckQueuedFrameRate(temp.File(L"frame-rate.mp4"), RecordingFormat::H264Mp4, rate);
                }
                MFShutdown();
                std::cout << "Recording FPS parsing, resampling, AVI and MP4 checks passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--monochrome") {
                CheckMonochromeRecording(temp.File(L"monochrome.avi"), temp.File(L"monochrome.mp4"));
                MFShutdown();
                std::cout << "Monochrome AVI and MP4 recording checks passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--privacy-masks") {
                CheckPrivacyMasks();
                MFShutdown();
                std::cout << "Privacy mask frame checks passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--lagarith") {
                HIC codec = ICOpen(ICTYPE_VIDEO, kLagarithFourCC, ICMODE_COMPRESS);
                if (!codec) {
                    MFShutdown();
                    std::cout << "Lagarith check skipped: 64-bit encoder unavailable.\n";
                    return 77;
                }
                try {
                    CheckLagarith(temp.File(L"lagarith.avi"), codec);
                    CheckQueuedFrameRate(temp.File(L"frame-rate.avi"),
                                         RecordingFormat::LagarithAvi, FrameRate{15, 2});
                } catch (...) {
                    ICClose(codec);
                    throw;
                }
                ICClose(codec);
                MFShutdown();
                std::cout << "Lagarith encoding and decoding check passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--bitrate") {
                CheckMp4Bitrate(temp.File(L"low-bitrate.mp4"), temp.File(L"high-bitrate.mp4"));
                MFShutdown();
                std::cout << "MP4 bitrate configuration check passed.\n";
                return 0;
            }
            if (argc > 1 && std::string(argv[1]) == "--hevc") {
                MFT_REGISTER_TYPE_INFO outputInfo{MFMediaType_Video, MFVideoFormat_HEVC};
                IMFActivate** encoders = nullptr;
                UINT32 encoderCount = 0;
                CheckHResult(MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER, MFT_ENUM_FLAG_ALL,
                                       nullptr, &outputInfo, &encoders, &encoderCount),
                             L"Enumerating HEVC encoders");
                for (UINT32 i = 0; i < encoderCount; ++i) encoders[i]->Release();
                CoTaskMemFree(encoders);
                if (!encoderCount) {
                    MFShutdown();
                    std::cout << "HEVC encoding check skipped: no HEVC encoder is installed.\n";
                    return 77;
                }
                constexpr int width = 1280;
                constexpr int height = 720;
                std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4, 128);
                NDIlib_video_frame_v2_t frame;
                frame.xres = width;
                frame.yres = height;
                frame.FourCC = NDIlib_FourCC_video_type_BGRX;
                frame.frame_rate_N = 30;
                frame.frame_rate_D = 1;
                frame.line_stride_in_bytes = width * 4;
                frame.p_data = pixels.data();
                const auto path = temp.File(L"hevc.mp4");
                {
                    Mp4Writer writer(path, frame, true);
                    for (int i = 0; i < 30; ++i) writer.Write(frame);
                    writer.Finish();
                }
                ComPtr<IMFSourceReader> reader;
                CheckHResult(MFCreateSourceReaderFromURL(path.c_str(), nullptr, &reader),
                             L"Opening HEVC test recording");
                ComPtr<IMFMediaType> type;
                CheckHResult(reader->GetNativeMediaType(
                    static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), 0, &type),
                    L"Reading HEVC test format");
                GUID subtype{};
                CheckHResult(type->GetGUID(MF_MT_SUBTYPE, &subtype), L"Reading HEVC subtype");
                Require(subtype == MFVideoFormat_HEVC, "MP4 is not encoded as HEVC.");
                reader.Reset();
                CheckQueuedFrameRate(temp.File(L"frame-rate.mp4"),
                                     RecordingFormat::HevcMp4, FrameRate{15, 2});
                MFShutdown();
                std::cout << "HEVC encoding check passed.\n";
                return 0;
            }
            constexpr int width = 96;
            constexpr int height = 64;
            constexpr int stride = width * 4 + 16;
            std::vector<uint8_t> pixels(stride * height, 0);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    auto* pixel = pixels.data() + y * stride + x * 4;
                    pixel[0] = pixel[1] = pixel[2] = y < height / 2 ? 255 : 0;
                    pixel[3] = 255;
                }
            }
            NDIlib_video_frame_v2_t frame;
            frame.xres = width;
            frame.yres = height;
            frame.FourCC = NDIlib_FourCC_video_type_BGRX;
            frame.frame_rate_N = 30;
            frame.frame_rate_D = 1;
            frame.line_stride_in_bytes = stride;
            frame.p_data = pixels.data();

            const auto preview = MakePreviewFrame(frame);
            CheckUsbSettingsDialog();
            CheckOutputCollisions(temp);
#ifdef HAVE_EPIPHAN_SDK
            CheckEpiphanFrames();
#endif
#ifdef HAVE_DATAPATH_SDK
            CheckDatapathFrames();
            CheckDatapathSettings();
            CheckDatapathSharedRuntime();
#endif
            CheckViewPreviewCadence();
            CheckViewOnlyLayout();
            const UsbMode selectedMode{1920, 1080, 60000, 1001};
            const auto requestedUsb = UsbRequestedType(selectedMode);
            UINT32 usbWidth = 0, usbHeight = 0, usbRateN = 0, usbRateD = 0;
            CheckHResult(MFGetAttributeSize(requestedUsb.Get(), MF_MT_FRAME_SIZE, &usbWidth, &usbHeight),
                         L"Reading requested USB size");
            CheckHResult(MFGetAttributeRatio(requestedUsb.Get(), MF_MT_FRAME_RATE, &usbRateN, &usbRateD),
                         L"Reading requested USB rate");
            Require(SameUsbMode(selectedMode, {usbWidth, usbHeight, usbRateN, usbRateD}) &&
                    SameUsbMode({1920, 1080, 60, 1}, {1920, 1080, 120, 2}) &&
                    !SameUsbMode(selectedMode, {1920, 1080, 60, 1}) &&
                    !SameUsbMode(selectedMode, {1280, 720, 60000, 1001}),
                    "USB format selection did not preserve the requested resolution and frame rate.");
            const auto defaultUsb = UsbRequestedType(std::nullopt);
            Require(FAILED(MFGetAttributeSize(defaultUsb.Get(), MF_MT_FRAME_SIZE, &usbWidth, &usbHeight)),
                    "USB default format unexpectedly forces a resolution.");
            Require(IsValidUsbManualValue(-4, -10, 10, 3) &&
                    !IsValidUsbManualValue(-3, -10, 10, 3) &&
                    !IsValidUsbManualValue(-11, -10, 10, 3) &&
                    !IsValidUsbManualValue(0, -10, 10, 0),
                    "USB control range validation failed.");
            Require(!ParseBitrate(L"").has_value() &&
                    ParseBitrate(L"1") == 1'000'000u &&
                    ParseBitrate(L"1000") == 1'000'000'000u,
                    "MP4 bitrate parsing failed for valid values.");
            for (const wchar_t* invalid : {L"0", L"1001", L"-1", L"1.5", L"999999999999"}) {
                bool rejected = false;
                try { ParseBitrate(invalid); } catch (const std::runtime_error&) { rejected = true; }
                Require(rejected, "MP4 bitrate parsing accepted an invalid value.");
            }
            Require(preview->sourceWidth == width && preview->sourceHeight == height &&
                    preview->frameRateN == 30 && preview->frameRateD == 1,
                    "Preview did not preserve source video information.");
            Require(NumericSourceAddress("192.168.1.23:5960") == L"192.168.1.23" &&
                    NumericSourceAddress("tcp://[2001:db8::1]:5960") == L"2001:db8::1" &&
                    NumericSourceAddress("tcp://camera.local:5960").empty() &&
                    NumericSourceAddress("").empty(),
                    "Source IP parsing accepted an invalid address or rejected a numeric address.");
            Require(IsNumericIp(L"192.168.1.23") && IsNumericIp(L"2001:db8::1") &&
                    !IsNumericIp(L"camera.local") && !IsNumericIp(L"192.168.1.23:5960") &&
                    !IsNumericIp(L"999.1.2.3") && !IsNumericIp(L""),
                    "NDI discovery IP validation failed.");
            Require(preview->pixels[0] == 255 &&
                    preview->pixels[static_cast<size_t>(width) * (height - 1) * 4] == 0,
                    "Preview frame is vertically inverted or has an incorrect stride.");
            Require(FormatRecordingTime(0, 0, 100000) == L"Recording time: 00:00:00",
                    "Timer should not run before the first frame.");
            Require(FormatRecordingTime(1000, 0, 3'662'000) == L"Recording time: 01:01:01",
                    "Timer did not show elapsed recording time.");
            Require(FormatRecordingTime(1000, 5000, 100000) == L"Recording time: 00:00:04",
                    "Timer did not retain the stopped recording time.");

            const auto aviPath = temp.File(L"test.avi");
            {
                AviWriter writer(aviPath, frame);
                writer.Write(frame);
                writer.Finish();
            }
            CheckAvi(aviPath, static_cast<size_t>(width) * height * 4, width * 4);
            CheckCameraFramesAndRecording(temp.File(L"camera.avi"));
            CheckQueueOverflowDrain(temp.File(L"overflow.avi"));

            RecordingOptions configuredMp4;
            configuredMp4.path = temp.File(L"bitrate.mp4");
            configuredMp4.bitrate = ParseBitrate(L"4");
            {
                auto writer = CreateWriter(configuredMp4, frame);
                for (int i = 0; i < 30; ++i) writer->Write(frame);
                writer->Finish();
            }
            CheckMp4(configuredMp4.path);

            const auto mp4Path = temp.File(L"test.mp4");
            CheckNdiPreviewAndRecording(frame, mp4Path);
            CheckMp4(mp4Path);
        } catch (...) {
            MFShutdown();
            throw;
        }
        MFShutdown();
        std::cout << "Preview, AVI and MP4 orientation checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
