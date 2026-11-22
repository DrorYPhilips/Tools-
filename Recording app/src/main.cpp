#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <ws2tcpip.h>
#include <commctrl.h>
#include <commdlg.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <vfw.h>
#include <dshow.h>
#include <olectl.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <mftransform.h>
#include <wrl/client.h>
#include <wrl/implements.h>

#include "app_version.h"

// NDI_ONLY builds the NDI-Recorder edition: same app, but only NDI sources.
#ifdef NDI_ONLY
#undef HAVE_EPIPHAN_SDK
#undef HAVE_DATAPATH_SDK
#define APP_NAME_A "NDI-Recorder"
#define APP_NAME_W L"NDI-Recorder"
#else
#define APP_NAME_A "Multi-Recorder"
#define APP_NAME_W L"Multi-Recorder"
#endif
#include <Processing.NDI.Lib.h>
#ifdef HAVE_EPIPHAN_SDK
#include <frmgrab.h>
#endif
#ifdef HAVE_DATAPATH_SDK
#include <rgb.h>
#include <rgbapi.h>
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#include <deque>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "vfw32.lib")

using Microsoft::WRL::ComPtr;

namespace {

constexpr wchar_t kWindowClass[] = L"NdiRecorderWindow";
constexpr wchar_t kPreviewClass[] = L"NdiRecorderPreview";
constexpr wchar_t kViewClass[] = L"NdiRecorderViewOnly";
constexpr wchar_t kAppName[] = APP_NAME_W;
#ifdef NDI_ONLY
constexpr bool kNdiOnly = true;
#else
constexpr bool kNdiOnly = false;
#endif
constexpr wchar_t kWindowTitle[] = APP_NAME_W L" v" APP_VERSION L" (Build " APP_BUILD_NUMBER L")";
constexpr UINT kStatusMessage = WM_APP + 1;
constexpr int kSourceControl = 1001;
constexpr int kRefreshControl = 1002;
constexpr int kPathControl = 1003;
constexpr int kBrowseControl = 1004;
constexpr int kFormatControl = 1005;
constexpr int kRecordControl = 1006;
constexpr int kStatusControl = 1007;
constexpr int kTimerControl = 1008;
constexpr int kStreamInfoControl = 1009;
constexpr int kAddressInfoControl = 1010;
constexpr int kPerformanceInfoControl = 1011;
constexpr int kDiscoveryIpControl = 1012;
constexpr int kApplyIpControl = 1013;
constexpr int kCodecSettingsControl = 1014;
constexpr int kUsbSettingsControl = 1015;
constexpr int kViewOnlyControl = 1016;
constexpr int kFrameRateControl = 1017;
constexpr int kCustomFrameRate = 4;
constexpr int kColorControl = 1018;
constexpr int kColorMonochrome = 1;
constexpr int kHelpControl = 1019;
constexpr int kNdiSettingsControl = 1020;
constexpr int kPrivacyMaskControl = 1021;
constexpr int kClearPrivacyMasksControl = 1022;
constexpr int kViewHelpControl = 1104;
constexpr int kViewLayoutControl = 1101;
constexpr int kViewRefreshControl = 1102;
constexpr int kViewBackControl = 1103;
constexpr int kViewSourceFirst = 1110;
constexpr int kViewIpFirst = 1120;
constexpr int kViewConnectFirst = 1130;
constexpr int kFormatAvi = 1;
constexpr int kFormatHevc = 2;
constexpr int kFormatLagarith = 3;
constexpr DWORD kLagarithFourCC = mmioFOURCC('L', 'A', 'G', 'S');

enum class RecordingFormat {
    H264Mp4,
    UncompressedAvi,
    HevcMp4,
    LagarithAvi
};

enum class SourceKind { Ndi, Usb, Vfw, Epiphan, Datapath };

struct NdiSource {
    std::wstring name;
    std::string ndiName;
    std::string address;
    SourceKind kind = SourceKind::Ndi;
    std::wstring deviceId;
    unsigned int driverIndex = 0;
};

class ComApartment {
public:
    ComApartment() {
        const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(result)) {
            throw std::runtime_error("Initializing recording thread COM failed with HRESULT " +
                                     std::to_string(static_cast<unsigned int>(result)));
        }
        initialized_ = true;
    }

    ~ComApartment() {
        if (initialized_) {
            CoUninitialize();
        }
    }

private:
    bool initialized_ = false;
};

struct FrameRate {
    int numerator;
    int denominator;
};

struct PrivacyMask {
    double left = 0;
    double top = 0;
    double right = 0;
    double bottom = 0;
};

int64_t FrameTime(int64_t index, int64_t numerator, int64_t denominator,
                  int64_t ticksPerSecond) {
    // Split the calculation to avoid overflow in long, fractional-rate recordings.
    const int64_t scale = ticksPerSecond * denominator;
    return (index / numerator) * scale + (index % numerator) * scale / numerator;
}

struct RecordingOptions {
    NdiSource source;
    std::wstring path;
    RecordingFormat format = RecordingFormat::H264Mp4;
    std::optional<UINT32> bitrate;
    std::vector<uint8_t> codecState;
    std::optional<FrameRate> frameRate;
    bool monochrome = false;
    std::vector<PrivacyMask> privacyMasks;
};

HWND g_window = nullptr;
HWND g_sourceControl = nullptr;
HWND g_pathControl = nullptr;
HWND g_formatControl = nullptr;
HWND g_recordControl = nullptr;
HWND g_statusControl = nullptr;
HWND g_timerControl = nullptr;
HWND g_previewControl = nullptr;
HWND g_streamInfoControl = nullptr;
HWND g_addressInfoControl = nullptr;
HWND g_performanceInfoControl = nullptr;
HWND g_discoveryIpControl = nullptr;
HWND g_bitrateLabel = nullptr;
HWND g_bitrateControl = nullptr;
HWND g_codecSettingsControl = nullptr;
HWND g_usbSettingsControl = nullptr;
HWND g_ndiSettingsControl = nullptr;
HWND g_frameRateControl = nullptr;
HWND g_customFrameRateControl = nullptr;
HWND g_colorControl = nullptr;
HWND g_privacyMaskControl = nullptr;
HWND g_clearPrivacyMasksControl = nullptr;
bool g_monochromePreview = false;
constexpr size_t kMaxPrivacyMasks = 2;
std::vector<PrivacyMask> g_privacyMasks;
std::optional<PrivacyMask> g_dragPrivacyMask;
std::optional<NdiSource> g_privacyMaskSource;
bool g_drawingPrivacyMask = false;
double g_dragPrivacyStartX = 0;
double g_dragPrivacyStartY = 0;
std::vector<uint8_t> g_lagarithState;
void UpdatePrivacyMaskControls();
void ClearPrivacyMasks();
struct UsbMode {
    UINT32 width;
    UINT32 height;
    UINT32 rateN;
    UINT32 rateD;
};
std::unordered_map<std::wstring, UsbMode> g_usbModes;
#ifdef HAVE_DATAPATH_SDK
struct DatapathSetting {
    size_t control;
    int64_t value;
};
std::unordered_map<std::wstring, std::vector<DatapathSetting>> g_datapathSettings;
#endif
std::vector<NdiSource> g_sources;
std::vector<NdiSource> g_cameraSources;
bool SamePrivacyMaskSource(const NdiSource& left, const NdiSource& right) {
    if (left.kind != right.kind) return false;
    return left.kind == SourceKind::Ndi ? left.ndiName == right.ndiName
                                        : left.deviceId == right.deviceId;
}
NDIlib_find_instance_t g_finder = nullptr;
// Extra finders for IPs entered in view-only mode; they add sources without
// replacing the main finder, so views that are already running keep going.
std::vector<std::pair<std::wstring, NDIlib_find_instance_t>> g_ipFinders;

struct NdiSettings {
    std::wstring deviceIp;
};

NdiSettings g_ndiSettings;
std::thread g_worker;
std::atomic<bool> g_stopRequested{false};
std::atomic<int64_t> g_requestedStopTime{0};
std::atomic<bool> g_closing{false};
std::atomic<ULONGLONG> g_recordStart{0};
std::atomic<ULONGLONG> g_recordEnd{0};
bool g_recording = false;
bool g_configuringUsb = false;

struct PreviewFrame {
    int width = 0;
    int height = 0;
    int sourceWidth = 0;
    int sourceHeight = 0;
    int frameRateN = 0;
    int frameRateD = 0;
    int signalRateMilliHz = 0;
    bool interlaced = false;
    std::vector<uint8_t> pixels;
    std::chrono::steady_clock::time_point received;
};

std::string g_previewName;
std::string g_previewAddress;
std::wstring g_previewDeviceId;
SourceKind g_previewKind = SourceKind::Ndi;

struct CameraFrame {
    int width = 0;
    int height = 0;
    int rateN = 0;
    int rateD = 0;
    int signalRateMilliHz = 0;
    bool interlaced = false;
    std::vector<uint8_t> pixels;

    NDIlib_video_frame_v2_t VideoFrame() {
        NDIlib_video_frame_v2_t frame{};
        frame.xres = width;
        frame.yres = height;
        frame.frame_rate_N = rateN;
        frame.frame_rate_D = rateD;
        frame.FourCC = NDIlib_FourCC_video_type_BGRX;
        frame.line_stride_in_bytes = width * 4;
        frame.p_data = pixels.data();
        return frame;
    }
};

struct QueuedCameraFrame {
    std::shared_ptr<CameraFrame> frame;
    std::chrono::steady_clock::time_point received;
};

struct CameraQueue {
    std::mutex mutex;
    std::condition_variable changed;
    std::deque<QueuedCameraFrame> frames;
    size_t bytes = 0;
    size_t maxBytes = size_t{4} * 1024 * 1024 * 1024;
    bool recording = false;
    std::optional<std::chrono::steady_clock::time_point> stopped;
    std::string error;
};

CameraQueue g_camera;

struct PreviewSession {
    explicit PreviewSession(CameraQueue* recordingQueue = nullptr) : queue(recordingQueue) {}
    std::atomic<bool> stop{false};
    std::thread worker;
    std::mutex mutex;
    std::shared_ptr<const PreviewFrame> frame;
    std::wstring error;
    std::chrono::steady_clock::time_point lastUpdate{};
    CameraQueue* queue;
};

PreviewSession g_recordPreview(&g_camera);
std::atomic<bool>& g_stopPreview = g_recordPreview.stop;
std::thread& g_previewWorker = g_recordPreview.worker;
std::mutex& g_previewMutex = g_recordPreview.mutex;
std::shared_ptr<const PreviewFrame>& g_previewFrame = g_recordPreview.frame;
std::wstring& g_previewError = g_recordPreview.error;

struct ViewSlot {
    PreviewSession session;
    HWND choice = nullptr;
    HWND preview = nullptr;
    HWND ipLabel = nullptr;
    HWND ip = nullptr;
    HWND connect = nullptr;
    std::optional<NdiSource> selected;
    std::wstring pendingIp;
    std::chrono::steady_clock::time_point pendingDeadline{};
};

std::array<ViewSlot, 4> g_viewSlots;
HWND g_viewPanel = nullptr;
HWND g_viewLayout = nullptr;
HWND g_viewButton = nullptr;
std::vector<HWND> g_hiddenRecordingControls;
bool g_viewOnly = false;
bool g_viewFastTimer = false;
int g_viewCount = 2;

class PerformanceMonitor {
public:
    PerformanceMonitor() {
        if (PdhOpenQueryW(nullptr, 0, &query_) != ERROR_SUCCESS) {
            return;
        }
        PdhAddEnglishCounterW(query_, L"\\PhysicalDisk(_Total)\\% Disk Time", 0, &disk_);
        PdhAddEnglishCounterW(query_, L"\\GPU Engine(*)\\Utilization Percentage", 0, &gpu_);
        PdhCollectQueryData(query_);
    }

    ~PerformanceMonitor() {
        if (query_) PdhCloseQuery(query_);
    }

    std::wstring Sample() {
        std::optional<double> cpu;
        FILETIME idle{}, kernel{}, user{};
        if (GetSystemTimes(&idle, &kernel, &user)) {
            const auto value = [](const FILETIME& time) {
                return (static_cast<uint64_t>(time.dwHighDateTime) << 32) | time.dwLowDateTime;
            };
            const uint64_t idleTime = value(idle);
            const uint64_t totalTime = value(kernel) + value(user);
            if (previousTotal_ && totalTime > previousTotal_ && idleTime >= previousIdle_) {
                const uint64_t totalDelta = totalTime - previousTotal_;
                const uint64_t idleDelta = idleTime - previousIdle_;
                if (idleDelta <= totalDelta) {
                    cpu = 100.0 * static_cast<double>(totalDelta - idleDelta) /
                        static_cast<double>(totalDelta);
                }
            }
            previousIdle_ = idleTime;
            previousTotal_ = totalTime;
        }

        MEMORYSTATUSEX memory{sizeof(memory)};
        const bool memoryAvailable = GlobalMemoryStatusEx(&memory) != FALSE;
        std::optional<double> disk;
        std::optional<double> gpu;
        if (query_ && PdhCollectQueryData(query_) == ERROR_SUCCESS) {
            PDH_FMT_COUNTERVALUE value{};
            if (disk_ && PdhGetFormattedCounterValue(disk_, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                                     nullptr, &value) == ERROR_SUCCESS &&
                (value.CStatus == PDH_CSTATUS_VALID_DATA || value.CStatus == PDH_CSTATUS_NEW_DATA)) {
                disk = value.doubleValue;
            }
            gpu = GpuUtilization();
        }
        const auto percent = [](const std::optional<double>& value) {
            if (!value) return std::wstring(L"N/A");
            wchar_t text[32]{};
            swprintf_s(text, L"%.0f%%", *value);
            return std::wstring(text);
        };
        const std::optional<double> memoryPercent = memoryAvailable
            ? std::optional<double>(memory.dwMemoryLoad) : std::nullopt;
        return L"System: CPU " + percent(cpu) + L" | Memory " + percent(memoryPercent) +
            L" | Disk active " + percent(disk) + L" | GPU busiest engine " + percent(gpu);
    }

private:
    std::optional<double> GpuUtilization() {
        if (!gpu_) return std::nullopt;
        DWORD bytes = 0;
        DWORD count = 0;
        if (PdhGetFormattedCounterArrayW(gpu_, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                         &bytes, &count, nullptr) != PDH_MORE_DATA || !bytes) {
            return std::nullopt;
        }
        std::vector<uint8_t> buffer(bytes);
        auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
        if (PdhGetFormattedCounterArrayW(gpu_, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                         &bytes, &count, items) != ERROR_SUCCESS) {
            return std::nullopt;
        }
        std::unordered_map<std::wstring, double> engines;
        for (DWORD i = 0; i < count; ++i) {
            if (!items[i].szName ||
                (items[i].FmtValue.CStatus != PDH_CSTATUS_VALID_DATA &&
                 items[i].FmtValue.CStatus != PDH_CSTATUS_NEW_DATA)) continue;
            const std::wstring name(items[i].szName);
            const auto luid = name.find(L"luid_");
            if (luid == std::wstring::npos) continue;
            engines[name.substr(luid)] += std::max(0.0, items[i].FmtValue.doubleValue);
        }
        if (engines.empty()) return std::nullopt;
        double busiest = 0;
        for (const auto& engine : engines) busiest = std::max(busiest, engine.second);
        return std::min(busiest, 100.0);
    }

    PDH_HQUERY query_ = nullptr;
    PDH_HCOUNTER disk_ = nullptr;
    PDH_HCOUNTER gpu_ = nullptr;
    uint64_t previousIdle_ = 0;
    uint64_t previousTotal_ = 0;
};

std::unique_ptr<PerformanceMonitor> g_performanceMonitor;

std::wstring Utf8ToWide(const char* value) {
    if (!value || !*value) {
        return {};
    }

    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value, -1, nullptr, 0);
    if (size <= 0) {
        return L"(unreadable source name)";
    }

    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value, -1, result.data(), size);
    result.pop_back();
    return result;
}

std::wstring NumericSourceAddress(const std::string& address) {
    std::wstring value = Utf8ToWide(address.c_str());
    const auto scheme = value.find(L"://");
    if (scheme != std::wstring::npos) value.erase(0, scheme + 3);
    const auto end = value.find_first_of(L"/?#");
    if (end != std::wstring::npos) value.resize(end);
    if (!value.empty() && value.front() == L'[') {
        const auto close = value.find(L']');
        if (close == std::wstring::npos ||
            (close + 1 < value.size() && value[close + 1] != L':')) return {};
        value = value.substr(1, close - 1);
    } else {
        const auto firstColon = value.find(L':');
        if (firstColon != std::wstring::npos && firstColon == value.rfind(L':')) {
            value.resize(firstColon);
        }
    }
    IN_ADDR ipv4{};
    IN6_ADDR ipv6{};
    return InetPtonW(AF_INET, value.c_str(), &ipv4) == 1 ||
           InetPtonW(AF_INET6, value.c_str(), &ipv6) == 1 ? value : std::wstring{};
}

bool IsNumericIp(const std::wstring& address) {
    IN_ADDR ipv4{};
    IN6_ADDR ipv6{};
    return InetPtonW(AF_INET, address.c_str(), &ipv4) == 1 ||
           InetPtonW(AF_INET6, address.c_str(), &ipv6) == 1;
}

std::wstring HResultMessage(const wchar_t* operation, HRESULT result) {
    wchar_t message[96]{};
    swprintf_s(message, L"%s failed (0x%08X).", operation, static_cast<unsigned int>(result));
    return message;
}

void CheckHResult(HRESULT result, const wchar_t* operation) {
    if (FAILED(result)) {
        std::string operationText;
        while (*operation) {
            operationText.push_back(static_cast<char>(*operation++));
        }
        throw std::runtime_error(operationText + " failed with HRESULT " +
                                 std::to_string(static_cast<unsigned int>(result)));
    }
}

void CheckAviResult(HRESULT result, const wchar_t* operation) {
    if (result != AVIERR_OK) {
        std::string operationText;
        while (*operation) {
            operationText.push_back(static_cast<char>(*operation++));
        }
        throw std::runtime_error(operationText + " failed with AVI error " +
                                 std::to_string(static_cast<unsigned int>(result)));
    }
}

void SetStatus(const std::wstring& text) {
    if (g_statusControl) {
        SetWindowTextW(g_statusControl, text.c_str());
    }
}

void PostStatus(std::wstring text) {
    if (g_closing.load()) {
        return;
    }
    auto* message = new std::wstring(std::move(text));
    if (!PostMessageW(g_window, kStatusMessage, 0, reinterpret_cast<LPARAM>(message))) {
        delete message;
    }
}

struct ReceiverDeleter {
    void operator()(NDIlib_recv_instance_t receiver) const {
        if (receiver) {
            NDIlib_recv_destroy(receiver);
        }
    }
};

struct VideoFrameGuard {
    NDIlib_recv_instance_t receiver;
    NDIlib_video_frame_v2_t* frame;

    ~VideoFrameGuard() {
        NDIlib_recv_free_video_v2(receiver, frame);
    }
};

class VideoWriter {
public:
    virtual ~VideoWriter() = default;
    virtual void Write(const NDIlib_video_frame_v2_t& frame) = 0;
    virtual void Finish() = 0;
};

int VideoStride(const NDIlib_video_frame_v2_t& frame) {
    return frame.line_stride_in_bytes == 0 ? frame.xres * 4 : frame.line_stride_in_bytes;
}

size_t ValidateFrame(const NDIlib_video_frame_v2_t& frame) {
    if (frame.xres <= 0 || frame.yres <= 0 || frame.xres > 16384 || frame.yres > 16384) {
        throw std::runtime_error("The NDI source reported an invalid video size.");
    }
    if (!frame.p_data ||
        (frame.FourCC != NDIlib_FourCC_video_type_BGRX &&
         frame.FourCC != NDIlib_FourCC_video_type_BGRA)) {
        throw std::runtime_error("The NDI receiver did not provide BGRX video.");
    }

    const auto rowBytes = static_cast<size_t>(frame.xres) * 4;
    const int stride = VideoStride(frame);
    if (stride < 0 || static_cast<size_t>(stride) < rowBytes) {
        throw std::runtime_error("The NDI video frame has an unsupported row stride.");
    }
    return rowBytes;
}

void CopyBottomUp(const NDIlib_video_frame_v2_t& frame, uint8_t* destination,
                  size_t rowBytes) {
    for (int y = 0; y < frame.yres; ++y) {
        const auto* source = frame.p_data +
            static_cast<size_t>(frame.yres - 1 - y) * VideoStride(frame);
        std::copy_n(source, rowBytes, destination + static_cast<size_t>(y) * rowBytes);
    }
}

class AviWriter final : public VideoWriter {
public:
    AviWriter(const std::wstring& path, const NDIlib_video_frame_v2_t& frame,
              bool lagarith = false, const std::vector<uint8_t>& codecState = {},
              bool monochrome = false)
        : width_(frame.xres), height_(frame.yres), rowBytes_(ValidateFrame(frame)),
          codecState_(codecState), monochrome_(monochrome && !lagarith) {
        if (width_ > LONG_MAX / 4 ||
            static_cast<size_t>(height_) > std::numeric_limits<size_t>::max() / rowBytes_) {
            throw std::runtime_error("The video frame is too large to write.");
        }
        // 8-bit DIB rows are padded to a multiple of four bytes.
        outputRowBytes_ = monochrome_ ? (static_cast<size_t>(width_) + 3) / 4 * 4 : rowBytes_;
        const size_t frameBytes = outputRowBytes_ * height_;
        if (frameBytes > static_cast<size_t>(LONG_MAX) ||
            frameBytes > std::numeric_limits<DWORD>::max()) {
            throw std::runtime_error("The video frame is too large for AVI.");
        }
        buffer_.resize(frameBytes);

        if (lagarith) {
            HIC codec = ICOpen(ICTYPE_VIDEO, kLagarithFourCC, ICMODE_COMPRESS);
            if (!codec) {
                throw std::runtime_error(
                    "Lagarith encoder is unavailable. Install the 64-bit Lagarith VFW codec.");
            }
            ICClose(codec);
        }
        AVIFileInit();
        initialized_ = true;

        const HRESULT openResult = AVIFileOpen(&file_, path.c_str(),
                                               OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE, nullptr);
        if (openResult != AVIERR_OK) {
            FinishResources();
            CheckAviResult(openResult, L"Opening AVI file");
        }

        AVISTREAMINFO streamInfo{};
        streamInfo.fccType = streamtypeVIDEO;
        streamInfo.fccHandler = lagarith ? kLagarithFourCC : BI_RGB;
        streamInfo.dwScale = static_cast<DWORD>(std::max(frame.frame_rate_D, 1));
        streamInfo.dwRate = static_cast<DWORD>(std::max(frame.frame_rate_N, 1));
        streamInfo.dwSuggestedBufferSize = static_cast<DWORD>(frameBytes);
        SetRect(&streamInfo.rcFrame, 0, 0, width_, height_);

        const HRESULT streamResult = AVIFileCreateStream(file_, &stream_, &streamInfo);
        if (streamResult != AVIERR_OK) {
            FinishResources();
            CheckAviResult(streamResult, L"Creating AVI video stream");
        }
        if (lagarith) {
            AVICOMPRESSOPTIONS compression{};
            compression.fccType = streamtypeVIDEO;
            compression.fccHandler = kLagarithFourCC;
            compression.dwQuality = static_cast<DWORD>(ICQUALITY_DEFAULT);
            compression.lpParms = codecState_.empty() ? nullptr : codecState_.data();
            compression.cbParms = static_cast<DWORD>(codecState_.size());
            const HRESULT result = AVIMakeCompressedStream(&compressedStream_, stream_, &compression, nullptr);
            if (result != AVIERR_OK) {
                FinishResources();
                CheckAviResult(result, L"Creating Lagarith AVI stream");
            }
        }

        bitmap_ = {};
        bitmap_.biSize = sizeof(bitmap_);
        bitmap_.biWidth = width_;
        bitmap_.biHeight = height_;
        bitmap_.biPlanes = 1;
        bitmap_.biBitCount = monochrome_ ? 8 : 32;
        bitmap_.biCompression = BI_RGB;
        bitmap_.biSizeImage = static_cast<DWORD>(frameBytes);

        struct {
            BITMAPINFOHEADER header;
            RGBQUAD colors[256];
        } grayFormat{};
        if (monochrome_) {
            bitmap_.biClrUsed = 256;
            grayFormat.header = bitmap_;
            for (int level = 0; level < 256; ++level) {
                const auto value = static_cast<BYTE>(level);
                grayFormat.colors[level] = {value, value, value, 0};
            }
        }
        const HRESULT formatResult = monochrome_
            ? AVIStreamSetFormat(stream_, 0, &grayFormat, sizeof(grayFormat))
            : AVIStreamSetFormat(compressedStream_ ? compressedStream_ : stream_, 0,
                                 &bitmap_, sizeof(bitmap_));
        if (formatResult != AVIERR_OK) {
            FinishResources();
            CheckAviResult(formatResult, L"Setting AVI video format");
        }
    }

    ~AviWriter() override {
        FinishResources();
    }

    void Write(const NDIlib_video_frame_v2_t& frame) override {
        EnsureMatchingFrame(frame);
        if (monochrome_) {
            for (int y = 0; y < height_; ++y) {
                const auto* source = frame.p_data +
                    static_cast<size_t>(height_ - 1 - y) * VideoStride(frame);
                auto* destination = buffer_.data() + static_cast<size_t>(y) * outputRowBytes_;
                for (int x = 0; x < width_; ++x) destination[x] = source[x * 4 + 1];
            }
        } else {
            CopyBottomUp(frame, buffer_.data(), rowBytes_);
        }

        if (frameIndex_ == LONG_MAX) {
            throw std::runtime_error("The AVI recording reached its frame limit.");
        }
        const HRESULT result = AVIStreamWrite(
            compressedStream_ ? compressedStream_ : stream_, frameIndex_, 1, buffer_.data(),
            static_cast<LONG>(buffer_.size()), AVIIF_KEYFRAME, nullptr, nullptr);
        CheckAviResult(result, L"Writing AVI video frame");
        ++frameIndex_;
    }

    void Finish() override {
        FinishResources();
    }

private:
    void EnsureMatchingFrame(const NDIlib_video_frame_v2_t& frame) const {
        ValidateFrame(frame);
        if (frame.xres != width_ || frame.yres != height_) {
            throw std::runtime_error("The NDI video size changed during recording.");
        }
    }

    void FinishResources() {
        if (compressedStream_) {
            AVIStreamRelease(compressedStream_);
            compressedStream_ = nullptr;
        }
        if (stream_) {
            AVIStreamRelease(stream_);
            stream_ = nullptr;
        }
        if (file_) {
            AVIFileRelease(file_);
            file_ = nullptr;
        }
        if (initialized_) {
            AVIFileExit();
            initialized_ = false;
        }
    }

    int width_;
    int height_;
    size_t rowBytes_;
    size_t outputRowBytes_ = 0;
    PAVIFILE file_ = nullptr;
    PAVISTREAM stream_ = nullptr;
    PAVISTREAM compressedStream_ = nullptr;
    BITMAPINFOHEADER bitmap_{};
    std::vector<uint8_t> buffer_;
    std::vector<uint8_t> codecState_;
    bool monochrome_ = false;
    LONG frameIndex_ = 0;
    bool initialized_ = false;
};

class Mp4Writer final : public VideoWriter {
public:
    Mp4Writer(const std::wstring& path, const NDIlib_video_frame_v2_t& frame, bool hevc = false,
              std::optional<UINT32> bitrate = std::nullopt)
        : width_(frame.xres), height_(frame.yres), rowBytes_(ValidateFrame(frame)) {
        if (static_cast<size_t>(height_) > std::numeric_limits<DWORD>::max() / rowBytes_) {
            throw std::runtime_error("The video frame is too large to write.");
        }

        frameRateN_ = static_cast<UINT32>(std::max(frame.frame_rate_N, 1));
        frameRateD_ = static_cast<UINT32>(std::max(frame.frame_rate_D, 1));

        CheckHResult(MFStartup(MF_VERSION), L"Starting Media Foundation");
        mfStarted_ = true;
        try {
            ComPtr<IMFAttributes> writerAttributes;
            if (hevc) {
                CheckHResult(MFCreateAttributes(&writerAttributes, 1),
                             L"Creating HEVC encoder settings");
                CheckHResult(writerAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE),
                             L"Enabling hardware encoding");
            }
            CheckHResult(MFCreateSinkWriterFromURL(path.c_str(), nullptr, writerAttributes.Get(), &sinkWriter_),
                         L"Creating MP4 writer");

            ComPtr<IMFMediaType> outputType;
            CheckHResult(MFCreateMediaType(&outputType), L"Creating MP4 output type");
            CheckHResult(outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video),
                         L"Setting MP4 media type");
            CheckHResult(outputType->SetGUID(MF_MT_SUBTYPE,
                                              hevc ? MFVideoFormat_HEVC : MFVideoFormat_H264),
                         L"Selecting MP4 compression");
            CheckHResult(outputType->SetUINT32(MF_MT_AVG_BITRATE, bitrate.value_or(Bitrate())),
                         L"Setting MP4 bitrate");
            CheckHResult(outputType->SetUINT32(MF_MT_INTERLACE_MODE,
                                               MFVideoInterlace_Progressive),
                         L"Setting MP4 scan mode");
            CheckHResult(MFSetAttributeSize(outputType.Get(), MF_MT_FRAME_SIZE,
                                            static_cast<UINT32>(width_),
                                            static_cast<UINT32>(height_)),
                         L"Setting MP4 frame size");
            CheckHResult(MFSetAttributeRatio(outputType.Get(), MF_MT_FRAME_RATE,
                                             frameRateN_, frameRateD_),
                         L"Setting MP4 frame rate");
            CheckHResult(MFSetAttributeRatio(outputType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1),
                         L"Setting MP4 pixel aspect ratio");
            CheckHResult(sinkWriter_->AddStream(outputType.Get(), &streamIndex_),
                         L"Adding MP4 video stream");

            ComPtr<IMFMediaType> inputType;
            CheckHResult(MFCreateMediaType(&inputType), L"Creating MP4 input type");
            CheckHResult(inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video),
                         L"Setting MP4 input type");
            CheckHResult(inputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32),
                         L"Selecting RGB video input");
            CheckHResult(inputType->SetUINT32(MF_MT_INTERLACE_MODE,
                                              MFVideoInterlace_Progressive),
                         L"Setting MP4 input scan mode");
            CheckHResult(MFSetAttributeSize(inputType.Get(), MF_MT_FRAME_SIZE,
                                            static_cast<UINT32>(width_),
                                            static_cast<UINT32>(height_)),
                         L"Setting MP4 input frame size");
            CheckHResult(MFSetAttributeRatio(inputType.Get(), MF_MT_FRAME_RATE,
                                             frameRateN_, frameRateD_),
                         L"Setting MP4 input frame rate");
            CheckHResult(MFSetAttributeRatio(inputType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1),
                         L"Setting MP4 input pixel aspect ratio");
            const HRESULT inputResult = sinkWriter_->SetInputMediaType(streamIndex_, inputType.Get(), nullptr);
            if (FAILED(inputResult) && hevc) {
                throw std::runtime_error(
                    "Could not configure H.265/HEVC encoding (HRESULT " +
                    std::to_string(static_cast<unsigned int>(inputResult)) +
                    "). Check that a compatible Windows HEVC encoder is installed.");
            }
            CheckHResult(inputResult, L"Configuring MP4 video input");
            const HRESULT beginResult = sinkWriter_->BeginWriting();
            if (FAILED(beginResult) && hevc) {
                throw std::runtime_error(
                    "Could not start H.265/HEVC encoding (HRESULT " +
                    std::to_string(static_cast<unsigned int>(beginResult)) +
                    "). Check that a compatible Windows HEVC encoder is installed.");
            }
            CheckHResult(beginResult, L"Starting MP4 recording");
            buffer_.resize(rowBytes_ * height_);
        } catch (...) {
            if (sinkWriter_) {
                sinkWriter_->Finalize();
            }
            sinkWriter_.Reset();
            MFShutdown();
            mfStarted_ = false;
            throw;
        }
    }

    ~Mp4Writer() override {
        if (sinkWriter_ && !finished_) {
            sinkWriter_->Finalize();
        }
        sinkWriter_.Reset();
        if (mfStarted_) {
            MFShutdown();
        }
    }

    void Write(const NDIlib_video_frame_v2_t& frame) override {
        ValidateFrame(frame);
        if (frame.xres != width_ || frame.yres != height_) {
            throw std::runtime_error("The NDI video size changed during recording.");
        }
        CopyBottomUp(frame, buffer_.data(), rowBytes_);

        ComPtr<IMFMediaBuffer> mediaBuffer;
        CheckHResult(MFCreateMemoryBuffer(static_cast<DWORD>(buffer_.size()), &mediaBuffer),
                     L"Allocating MP4 frame");
        BYTE* destination = nullptr;
        DWORD maxLength = 0;
        DWORD currentLength = 0;
        CheckHResult(mediaBuffer->Lock(&destination, &maxLength, &currentLength),
                     L"Locking MP4 frame");
        std::copy(buffer_.begin(), buffer_.end(), destination);
        const HRESULT unlockResult = mediaBuffer->Unlock();
        CheckHResult(unlockResult, L"Unlocking MP4 frame");
        CheckHResult(mediaBuffer->SetCurrentLength(static_cast<DWORD>(buffer_.size())),
                     L"Setting MP4 frame size");

        ComPtr<IMFSample> sample;
        CheckHResult(MFCreateSample(&sample), L"Creating MP4 sample");
        CheckHResult(sample->AddBuffer(mediaBuffer.Get()), L"Adding MP4 frame");
        const auto time = FrameTime(frameIndex_, frameRateN_, frameRateD_, 10'000'000);
        const auto nextTime = FrameTime(frameIndex_ + 1, frameRateN_, frameRateD_, 10'000'000);
        CheckHResult(sample->SetSampleTime(time),
                     L"Setting MP4 frame time");
        CheckHResult(sample->SetSampleDuration(nextTime - time), L"Setting MP4 frame duration");
        CheckHResult(sinkWriter_->WriteSample(streamIndex_, sample.Get()),
                     L"Writing MP4 video frame");
        ++frameIndex_;
    }

    void Finish() override {
        if (sinkWriter_ && !finished_) {
            CheckHResult(sinkWriter_->Finalize(), L"Finalizing MP4 file");
            finished_ = true;
        }
    }

private:
    UINT32 Bitrate() const {
        const uint64_t pixels = static_cast<uint64_t>(width_) * height_;
        const uint64_t fps = std::max<uint32_t>(frameRateN_ / frameRateD_, 1);
        const uint64_t suggested = pixels * fps / 5;
        return static_cast<UINT32>(std::clamp<uint64_t>(suggested, 2'000'000, 20'000'000));
    }

    int width_;
    int height_;
    size_t rowBytes_;
    UINT32 frameRateN_ = 30;
    UINT32 frameRateD_ = 1;
    DWORD streamIndex_ = 0;
    LONGLONG frameIndex_ = 0;
    bool mfStarted_ = false;
    bool finished_ = false;
    ComPtr<IMFSinkWriter> sinkWriter_;
    std::vector<uint8_t> buffer_;
};

// Encoders without 8-bit grayscale input receive neutral gray (B = G = R) frames.
class GreenToGrayWriter final : public VideoWriter {
public:
    explicit GreenToGrayWriter(std::unique_ptr<VideoWriter> writer) : writer_(std::move(writer)) {}

    void Write(const NDIlib_video_frame_v2_t& frame) override {
        const size_t rowBytes = ValidateFrame(frame);
        pixels_.resize(rowBytes * frame.yres);
        for (int y = 0; y < frame.yres; ++y) {
            const auto* source = frame.p_data + static_cast<size_t>(y) * VideoStride(frame);
            auto* destination = pixels_.data() + static_cast<size_t>(y) * rowBytes;
            for (int x = 0; x < frame.xres; ++x) {
                const uint8_t level = source[x * 4 + 1];
                destination[x * 4] = destination[x * 4 + 1] = destination[x * 4 + 2] = level;
                destination[x * 4 + 3] = 255;
            }
        }
        auto gray = frame;
        gray.FourCC = NDIlib_FourCC_video_type_BGRX;
        gray.line_stride_in_bytes = static_cast<int>(rowBytes);
        gray.p_data = pixels_.data();
        writer_->Write(gray);
    }

    void Finish() override {
        writer_->Finish();
    }

private:
    std::unique_ptr<VideoWriter> writer_;
    std::vector<uint8_t> pixels_;
};

class PrivacyMaskWriter final : public VideoWriter {
public:
    PrivacyMaskWriter(std::unique_ptr<VideoWriter> writer, std::vector<PrivacyMask> masks)
        : writer_(std::move(writer)), masks_(std::move(masks)) {}

    void Write(const NDIlib_video_frame_v2_t& frame) override {
        const size_t rowBytes = ValidateFrame(frame);
        if (static_cast<size_t>(frame.yres) >
            std::numeric_limits<size_t>::max() / rowBytes) {
            throw std::runtime_error("The video frame is too large to mask.");
        }
        pixels_.resize(rowBytes * static_cast<size_t>(frame.yres));
        for (int y = 0; y < frame.yres; ++y) {
            std::copy_n(frame.p_data + static_cast<size_t>(y) * VideoStride(frame),
                        rowBytes, pixels_.data() + static_cast<size_t>(y) * rowBytes);
        }

        for (const auto& mask : masks_) {
            const int left = std::clamp(static_cast<int>(std::floor(
                std::clamp(mask.left, 0.0, 1.0) * frame.xres)), 0, frame.xres);
            const int top = std::clamp(static_cast<int>(std::floor(
                std::clamp(mask.top, 0.0, 1.0) * frame.yres)), 0, frame.yres);
            const int right = std::clamp(static_cast<int>(std::ceil(
                std::clamp(mask.right, 0.0, 1.0) * frame.xres)), 0, frame.xres);
            const int bottom = std::clamp(static_cast<int>(std::ceil(
                std::clamp(mask.bottom, 0.0, 1.0) * frame.yres)), 0, frame.yres);
            for (int y = top; y < bottom; ++y) {
                for (int x = left; x < right; ++x) {
                    auto* pixel = pixels_.data() +
                        (static_cast<size_t>(y) * frame.xres + x) * 4;
                    pixel[0] = pixel[1] = pixel[2] = 128;
                    pixel[3] = 255;
                }
            }
        }

        auto masked = frame;
        masked.line_stride_in_bytes = static_cast<int>(rowBytes);
        masked.p_data = pixels_.data();
        writer_->Write(masked);
    }

    void Finish() override {
        writer_->Finish();
    }

private:
    std::unique_ptr<VideoWriter> writer_;
    std::vector<PrivacyMask> masks_;
    std::vector<uint8_t> pixels_;
};

std::unique_ptr<VideoWriter> CreateWriter(const RecordingOptions& options,
                                         const NDIlib_video_frame_v2_t& firstFrame) {
    auto outputFrame = firstFrame;
    if (options.frameRate) {
        outputFrame.frame_rate_N = options.frameRate->numerator;
        outputFrame.frame_rate_D = options.frameRate->denominator;
    }
    std::unique_ptr<VideoWriter> writer;
    if (options.format == RecordingFormat::UncompressedAvi) {
        writer = std::make_unique<AviWriter>(options.path, outputFrame, false,
                                             options.codecState, options.monochrome);
    } else if (options.format == RecordingFormat::LagarithAvi) {
        writer = std::make_unique<AviWriter>(options.path, outputFrame, true, options.codecState);
    } else {
        writer = std::make_unique<Mp4Writer>(
            options.path, outputFrame, options.format == RecordingFormat::HevcMp4, options.bitrate);
    }
    if (options.monochrome && options.format != RecordingFormat::UncompressedAvi) {
        writer = std::make_unique<GreenToGrayWriter>(std::move(writer));
    }
    if (!options.privacyMasks.empty()) {
        writer = std::make_unique<PrivacyMaskWriter>(std::move(writer), options.privacyMasks);
    }
    return writer;
}

class FrameRateConverter {
public:
    FrameRateConverter(VideoWriter& writer, FrameRate rate) : writer_(writer), rate_(rate) {
        if (rate.numerator <= 0 || rate.denominator <= 0) {
            throw std::runtime_error("The recording frame rate must be positive.");
        }
    }

    void Write(std::shared_ptr<CameraFrame> frame,
               std::chrono::steady_clock::time_point received) {
        ValidateFrame(frame->VideoFrame());
        if (!previous_) {
            first_ = received;
            previous_ = std::move(frame);
            WritePrevious();
        } else {
            if (frame->width != previous_->width || frame->height != previous_->height) {
                throw std::runtime_error("The video size changed during recording.");
            }
            if (received < last_) {
                throw std::runtime_error("Video capture timestamps moved backwards.");
            }
            Finish(received);
            previous_ = std::move(frame);
        }
        last_ = received;
    }

    void Finish(std::chrono::steady_clock::time_point end) {
        if (!previous_) return;
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - first_).count();
        while (NextFrameTime() < elapsed) WritePrevious();
    }

private:
    int64_t NextFrameTime() const {
        return FrameTime(frameIndex_, rate_.numerator, rate_.denominator, 1'000'000'000);
    }

    void WritePrevious() {
        auto frame = previous_->VideoFrame();
        frame.frame_rate_N = rate_.numerator;
        frame.frame_rate_D = rate_.denominator;
        writer_.Write(frame);
        ++frameIndex_;
    }

    VideoWriter& writer_;
    FrameRate rate_;
    std::shared_ptr<CameraFrame> previous_;
    std::chrono::steady_clock::time_point first_{};
    std::chrono::steady_clock::time_point last_{};
    int64_t frameIndex_ = 0;
};

std::shared_ptr<CameraFrame> CopyCameraFrame(const BYTE* top, ptrdiff_t stride,
                                             int width, int height, int bits,
                                             int rateN, int rateD);

void StopQueue(CameraQueue& queue, const std::string& error = {},
               std::chrono::steady_clock::time_point stopped = std::chrono::steady_clock::now()) {
    {
        std::lock_guard<std::mutex> lock(queue.mutex);
        if (queue.error.empty()) queue.error = error;
        if (!queue.stopped) queue.stopped = stopped;
        queue.recording = false;
    }
    queue.changed.notify_all();
}

bool QueueFrame(CameraQueue& queue, std::shared_ptr<CameraFrame> frame) {
    std::lock_guard<std::mutex> lock(queue.mutex);
    if (!queue.recording) return false;
    if (frame->pixels.size() > queue.maxBytes - queue.bytes) {
        queue.error = "Recording buffer is full; at least one frame was lost.";
        queue.stopped = std::chrono::steady_clock::now();
        queue.recording = false;
        queue.changed.notify_all();
        return false;
    }
    queue.frames.push_back({std::move(frame), std::chrono::steady_clock::now()});
    queue.bytes += queue.frames.back().frame->pixels.size();
    queue.changed.notify_one();
    return true;
}

bool QueueIsRecording(CameraQueue& queue) {
    std::lock_guard<std::mutex> lock(queue.mutex);
    return queue.recording;
}

bool WriteQueuedFrames(CameraQueue& queue, const RecordingOptions& options) {
    ComApartment apartment;
    std::unique_ptr<VideoWriter> writer;
    std::optional<FrameRateConverter> converter;
    std::chrono::steady_clock::time_point stopped;
    std::string captureError;
    for (;;) {
        QueuedCameraFrame queued;
        {
            std::unique_lock<std::mutex> lock(queue.mutex);
            queue.changed.wait(lock, [&queue] {
                return !queue.frames.empty() || !queue.recording;
            });
            if (queue.frames.empty()) {
                captureError = queue.error;
                stopped = queue.stopped.value_or(std::chrono::steady_clock::now());
                break;
            }
            queued = std::move(queue.frames.front());
            queue.frames.pop_front();
            queue.bytes -= queued.frame->pixels.size();
        }
        const auto& cameraFrame = queued.frame;
        if (cameraFrame->rateN <= 0 || cameraFrame->rateD <= 0) {
            throw std::runtime_error("The source did not report a valid frame rate.");
        }
        auto frame = cameraFrame->VideoFrame();
        if (!writer) {
            writer = CreateWriter(options, frame);
            if (options.frameRate) converter.emplace(*writer, *options.frameRate);
            PostStatus(L"Recording video...");
        }
        if (converter) converter->Write(cameraFrame, queued.received);
        else writer->Write(frame);
        if (!g_recordStart.load()) g_recordStart.store(GetTickCount64());
    }
    if (g_recordStart.load() && !g_recordEnd.load()) g_recordEnd.store(GetTickCount64());
    if (converter) converter->Finish(stopped);
    if (writer) writer->Finish();
    if (!captureError.empty()) throw std::runtime_error(captureError);
    return writer != nullptr;
}

void PostRecordingResult(const std::string& error, bool wroteFrames) {
    if (!error.empty()) {
        PostStatus(L"Recording failed: " + std::wstring(error.begin(), error.end()));
    } else if (wroteFrames) {
        PostStatus(L"Recording saved.");
    } else {
        PostStatus(L"Stopped before video arrived; no file was created.");
    }
}

void Record(RecordingOptions options) {
    CameraQueue queue;
    std::thread writerThread;
    std::string writerError;
    bool wroteFrames = false;
    try {
        NDIlib_recv_create_v3_t settings;
        settings.source_to_connect_to = NDIlib_source_t(
            options.source.ndiName.c_str(), options.source.address.c_str());
        settings.color_format = NDIlib_recv_color_format_BGRX_BGRA;
        settings.bandwidth = NDIlib_recv_bandwidth_highest;
        settings.allow_video_fields = false;
        settings.p_ndi_recv_name = APP_NAME_A;

        std::unique_ptr<NDIlib_recv_instance_type, ReceiverDeleter> receiver(
            NDIlib_recv_create_v3(&settings));
        if (!receiver) {
            throw std::runtime_error("Could not create the NDI receiver.");
        }

        NDIlib_recv_connect(receiver.get(), &settings.source_to_connect_to);
        queue.recording = true;
        writerThread = std::thread([&] {
            try {
                wroteFrames = WriteQueuedFrames(queue, options);
            } catch (const std::exception& error) {
                writerError = error.what();
                StopQueue(queue, writerError);
            } catch (...) {
                writerError = "Unexpected recording writer error.";
                StopQueue(queue, writerError);
            }
        });

        while (!g_stopRequested.load() && QueueIsRecording(queue)) {
            NDIlib_video_frame_v2_t frame{};
            const auto frameType = NDIlib_recv_capture_v3(
                receiver.get(), &frame, nullptr, nullptr, 250);
            if (frameType == NDIlib_frame_type_video) {
                VideoFrameGuard frameGuard{receiver.get(), &frame};
                if (g_stopRequested.load()) break;
                ValidateFrame(frame);
                QueueFrame(queue, CopyCameraFrame(frame.p_data, VideoStride(frame),
                    frame.xres, frame.yres, 32, frame.frame_rate_N, frame.frame_rate_D));
            } else if (frameType == NDIlib_frame_type_error) {
                throw std::runtime_error("The NDI connection was lost.");
            }
            NDIlib_recv_performance_t dropped{};
            NDIlib_recv_get_performance(receiver.get(), nullptr, &dropped);
            if (dropped.video_frames > 0) {
                throw std::runtime_error("NDI receiver dropped " +
                    std::to_string(dropped.video_frames) + " video frame(s).");
            }
        }
    } catch (const std::exception& error) {
        StopQueue(queue, error.what());
    } catch (...) {
        StopQueue(queue, "Unexpected NDI capture error.");
    }
    const int64_t requestedStop = g_requestedStopTime.load();
    StopQueue(queue, {}, g_stopRequested.load() && requestedStop
        ? std::chrono::steady_clock::time_point(std::chrono::nanoseconds(requestedStop))
        : std::chrono::steady_clock::now());
    if (writerThread.joinable()) writerThread.join();
    if (g_recordStart.load() && !g_recordEnd.load()) g_recordEnd.store(GetTickCount64());
    if (writerError.empty()) {
        std::lock_guard<std::mutex> lock(queue.mutex);
        writerError = queue.error;
    }
    PostRecordingResult(writerError, wroteFrames);
}

std::shared_ptr<PreviewFrame> MakePreviewFrame(const NDIlib_video_frame_v2_t& frame) {
    ValidateFrame(frame);
    const double scale = std::min({1.0, 480.0 / frame.xres, 270.0 / frame.yres});
    auto preview = std::make_shared<PreviewFrame>();
    preview->width = std::max(1, static_cast<int>(frame.xres * scale));
    preview->height = std::max(1, static_cast<int>(frame.yres * scale));
    preview->sourceWidth = frame.xres;
    preview->sourceHeight = frame.yres;
    preview->frameRateN = frame.frame_rate_N;
    preview->frameRateD = frame.frame_rate_D;
    preview->pixels.resize(static_cast<size_t>(preview->width) * preview->height * 4);
    preview->received = std::chrono::steady_clock::now();
    for (int y = 0; y < preview->height; ++y) {
        const int sourceY = static_cast<int>(
            static_cast<int64_t>(y) * frame.yres / preview->height);
        const auto* row = frame.p_data + static_cast<size_t>(sourceY) * VideoStride(frame);
        for (int x = 0; x < preview->width; ++x) {
            const int sourceX = static_cast<int>(
                static_cast<int64_t>(x) * frame.xres / preview->width);
            std::copy_n(row + static_cast<size_t>(sourceX) * 4, 4,
                        preview->pixels.data() +
                            (static_cast<size_t>(y) * preview->width + x) * 4);
        }
    }
    return preview;
}

std::shared_ptr<CameraFrame> CopyCameraFrame(const BYTE* top, ptrdiff_t stride,
                                             int width, int height, int bits,
                                             int rateN, int rateD) {
    if (!top || width <= 0 || height <= 0 || width > INT_MAX / 4 ||
        (bits != 24 && bits != 32) || stride == PTRDIFF_MIN ||
        static_cast<uint64_t>(width) * height * 4 > 128ULL * 1024 * 1024 ||
        static_cast<uint64_t>(stride < 0 ? -stride : stride) <
            static_cast<uint64_t>(width) * (bits / 8)) {
        throw std::runtime_error("The camera returned an unsupported video frame.");
    }
    auto result = std::make_shared<CameraFrame>();
    result->width = width;
    result->height = height;
    result->rateN = rateN;
    result->rateD = rateD;
    result->pixels.resize(static_cast<size_t>(width) * height * 4);
    for (int y = 0; y < height; ++y) {
        const BYTE* row = top + static_cast<ptrdiff_t>(y) * stride;
        BYTE* output = result->pixels.data() + static_cast<size_t>(y) * width * 4;
        if (bits == 32) {
            std::copy_n(row, static_cast<size_t>(width) * 4, output);
        } else {
            for (int x = 0; x < width; ++x) {
                std::copy_n(row + static_cast<size_t>(x) * 3, 3, output + static_cast<size_t>(x) * 4);
                output[static_cast<size_t>(x) * 4 + 3] = 255;
            }
        }
    }
    return result;
}

bool SessionIsRecording(const PreviewSession& session) {
    return session.queue && QueueIsRecording(*session.queue);
}

void PublishCameraFrame(std::shared_ptr<CameraFrame> frame, PreviewSession& session = g_recordPreview) {
    if (session.queue) QueueFrame(*session.queue, frame);
    const auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(session.mutex);
        const auto interval = std::chrono::milliseconds(session.queue ? 100 : 33);
        if (now - session.lastUpdate < interval) return;
        session.lastUpdate = now;
    }
    auto video = frame->VideoFrame();
    auto preview = MakePreviewFrame(video);
    preview->signalRateMilliHz = frame->signalRateMilliHz;
    preview->interlaced = frame->interlaced;
    {
        std::lock_guard<std::mutex> lock(session.mutex);
        session.frame = std::move(preview);
        session.error.clear();
    }
}

void CameraFailed(const std::string& message, PreviewSession& session = g_recordPreview) {
    if (session.queue) StopQueue(*session.queue, message);
    {
        std::lock_guard<std::mutex> lock(session.mutex);
        session.frame.reset();
        session.error = std::wstring(message.begin(), message.end());
    }
}

class CaptureRate {
public:
    int Add(std::chrono::steady_clock::time_point now) {
        samples_.push_back(now);
        while (samples_.size() > 2 && (samples_.size() > 4096 ||
               now - samples_.front() > std::chrono::seconds(2))) {
            samples_.pop_front();
        }
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            now - samples_.front()).count();
        if (elapsed < 1'000'000) return 0;
        const int64_t rate = static_cast<int64_t>(samples_.size() - 1) * 1'000'000'000 / elapsed;
        if (rate <= 0 || rate > INT_MAX) return 0;
        return static_cast<int>(rate);
    }
private:
    std::deque<std::chrono::steady_clock::time_point> samples_;
};

#ifdef HAVE_EPIPHAN_SDK
using EpiphanRate = CaptureRate;
struct EpiphanHandle {
    FrmGrabber* value = nullptr;
    ~EpiphanHandle() { if (value) FrmGrab_Close(value); }
    EpiphanHandle() = default;
    explicit EpiphanHandle(FrmGrabber* handle) : value(handle) {}
    EpiphanHandle(const EpiphanHandle&) = delete;
    EpiphanHandle& operator=(const EpiphanHandle&) = delete;
};

struct EpiphanFrameGuard {
    FrmGrabber* grabber;
    V2U_GrabFrame2* frame;
    ~EpiphanFrameGuard() { if (frame) FrmGrab_Release(grabber, frame); }
};

FrmGrabber* OpenEpiphan(const std::string& id) {
    const int count = FrmGrabLocal_Count();
    if (count < 0) throw std::runtime_error("Could not enumerate Epiphan devices.");
    for (int i = 0; i < count; ++i) {
        EpiphanHandle handle(FrmGrabLocal_OpenIndex(i));
        if (!handle.value) continue;
        const char* currentId = FrmGrab_GetId(handle.value);
        if (currentId && id == currentId) {
            FrmGrabber* result = handle.value;
            handle.value = nullptr;
            return result;
        }
    }
    throw std::runtime_error("Could not open the selected Epiphan device. Check its connection and close other capture applications.");
}

std::vector<NdiSource> EnumerateEpiphan() {
    const int count = FrmGrabLocal_Count();
    if (count < 0) throw std::runtime_error("Could not enumerate Epiphan devices.");
    std::vector<NdiSource> sources;
    for (int i = 0; i < count; ++i) {
        EpiphanHandle handle(FrmGrabLocal_OpenIndex(i));
        if (!handle.value) throw std::runtime_error("Could not open an Epiphan device during discovery.");
        const char* id = FrmGrab_GetId(handle.value);
        if (!id || !*id) throw std::runtime_error("An Epiphan device did not report its identifier.");
        NdiSource source;
        source.kind = SourceKind::Epiphan;
        source.ndiName = id;
        source.deviceId = Utf8ToWide(id);
        const char* description = FrmGrab_GetDescription(handle.value);
        source.name = L"Epiphan: " + Utf8ToWide(description && *description ? description : id) +
            L" [" + source.deviceId + L"]";
        sources.push_back(std::move(source));
    }
    return sources;
}

std::shared_ptr<CameraFrame> CopyEpiphanFrame(const V2U_GrabFrame2& frame, int rate) {
    const int width = frame.crop.width;
    const int height = frame.crop.height;
    if (frame.retcode != V2UERROR_OK || !frame.pixbuf || width <= 0 || height <= 0 ||
        width > INT_MAX / 3 ||
        (frame.palette & (V2U_GRABFRAME_FORMAT_MASK | V2U_GRABFRAME_BOTTOM_UP_FLAG)) !=
            V2U_GRABFRAME_FORMAT_BGR24 ||
        static_cast<uint64_t>(width) * height * 3 > frame.imagelen ||
        frame.imagelen > frame.pixbuflen) {
        throw std::runtime_error("Epiphan returned an invalid or unsupported BGR frame (SDK error " +
                                 std::to_string(frame.retcode) + ").");
    }
    auto result = CopyCameraFrame(static_cast<const BYTE*>(frame.pixbuf),
                                  static_cast<ptrdiff_t>(width) * 3, width, height, 24, rate, 1000);
    result->signalRateMilliHz = frame.mode.vfreq;
    return result;
}

void PreviewEpiphan(NdiSource source, PreviewSession* session) {
    try {
        EpiphanHandle handle(OpenEpiphan(source.ndiName));
        FrmGrab_Start(handle.value);
        struct StopGrab { FrmGrabber* value; ~StopGrab() { FrmGrab_Stop(value); } } stop{handle.value};
        EpiphanRate rate;
        int width = 0, height = 0, signalRate = 0;
        while (!session->stop.load()) {
            EpiphanFrameGuard frame{handle.value,
                FrmGrab_Frame(handle.value, V2U_GRABFRAME_FORMAT_BGR24, nullptr)};
            if (session->stop.load()) break;
            if (!frame.frame || frame.frame->retcode != V2UERROR_OK || !frame.frame->imagelen) {
                if (SessionIsRecording(*session)) {
                    throw std::runtime_error("Epiphan capture failed, the input signal was lost, or the device disconnected.");
                }
                {
                    std::lock_guard<std::mutex> lock(session->mutex);
                    session->frame.reset();
                    session->error = L"Epiphan capture unavailable: check the device and input signal.";
                }
                rate = EpiphanRate{};
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }
            const int currentSignalRate = frame.frame->mode.vfreq;
            const bool rateChanged = signalRate > 0 && currentSignalRate > 0 &&
                std::abs(static_cast<int64_t>(currentSignalRate) - signalRate) >
                    std::max<int64_t>(100, signalRate / 200);
            if (width != frame.frame->crop.width || height != frame.frame->crop.height || rateChanged) {
                width = frame.frame->crop.width;
                height = frame.frame->crop.height;
                signalRate = currentSignalRate;
                rate = EpiphanRate{};
                if (SessionIsRecording(*session))
                    throw std::runtime_error("Epiphan input mode changed during recording.");
            }
            if (signalRate <= 0 && currentSignalRate > 0) signalRate = currentSignalRate;
            const int measuredRate = rate.Add(std::chrono::steady_clock::now());
            PublishCameraFrame(CopyEpiphanFrame(*frame.frame, measuredRate), *session);
        }
    } catch (const std::exception& error) {
        if (!session->stop.load()) CameraFailed(error.what(), *session);
    }
}
#endif

#ifdef HAVE_DATAPATH_SDK
void CheckDatapath(unsigned long result, const char* operation) {
    if (result != 0)
        throw std::runtime_error(std::string(operation) + " failed (Datapath error " +
                                 std::to_string(result) + ").");
}

class DatapathApi {
public:
    DatapathApi() {
        wchar_t systemDirectory[MAX_PATH]{};
        const UINT length = GetSystemDirectoryW(systemDirectory, MAX_PATH);
        if (!length || length >= MAX_PATH)
            throw std::runtime_error("Could not locate the Windows system directory.");
        const std::wstring path = std::wstring(systemDirectory) + L"\\rgbeasy.dll";
        const DWORD attributes = GetFileAttributesW(path.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            const DWORD error = GetLastError();
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return;
            throw std::runtime_error("Could not check the Datapath driver runtime (Windows error " +
                                     std::to_string(error) + ").");
        }
        module_ = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!module_)
            throw std::runtime_error("Could not load the Datapath driver runtime (Windows error " +
                                     std::to_string(GetLastError()) + ").");
        try {
            load = Function<decltype(load)>("RGBLoad");
            free = Function<decltype(free)>("RGBFree");
            getCount = Function<decltype(getCount)>("RGBGetNumberOfInputs");
            getInfo = Function<decltype(getInfo)>("RGBGetInputInfoW");
            open = Function<decltype(open)>("RGBOpenInput");
            close = Function<decltype(close)>("RGBCloseInputs");
            setDropping = Function<decltype(setDropping)>("RGBSetFrameDropping");
            setDma = Function<decltype(setDma)>("RGBSetDMADirect");
            setPixelFormat = Function<decltype(setPixelFormat)>("RGBSetPixelFormat");
            setFrameCallback = Function<decltype(setFrameCallback)>("RGBSetFrameCapturedFnEx");
            setErrorCallback = Function<decltype(setErrorCallback)>("RGBSetErrorFn");
            start = Function<decltype(start)>("RGBStartCapture");
            stop = Function<decltype(stop)>("RGBStopCapture");
            getMode = Function<decltype(getMode)>("RGBGetModeInfo");
            getRate = Function<decltype(getRate)>("RGBGetFrameRate");
        } catch (...) {
            FreeLibrary(module_);
            module_ = nullptr;
            throw;
        }
    }
    ~DatapathApi() { if (module_ && !keepLoaded_) FreeLibrary(module_); }
    DatapathApi(const DatapathApi&) = delete;
    DatapathApi& operator=(const DatapathApi&) = delete;
    bool Available() const { return module_ != nullptr; }
    void KeepLoaded() { keepLoaded_ = true; }
    template <typename T> T OptionalFunction(const char* name) const {
        return reinterpret_cast<T>(GetProcAddress(module_, name));
    }

    decltype(&RGBLoad) load = nullptr;
    decltype(&RGBFree) free = nullptr;
    decltype(&RGBGetNumberOfInputs) getCount = nullptr;
    decltype(&RGBGetInputInfoW) getInfo = nullptr;
    decltype(&RGBOpenInput) open = nullptr;
    decltype(&RGBCloseInputs) close = nullptr;
    decltype(&RGBSetFrameDropping) setDropping = nullptr;
    decltype(&RGBSetDMADirect) setDma = nullptr;
    decltype(&RGBSetPixelFormat) setPixelFormat = nullptr;
    decltype(&RGBSetFrameCapturedFnEx) setFrameCallback = nullptr;
    decltype(&RGBSetErrorFn) setErrorCallback = nullptr;
    decltype(&RGBStartCapture) start = nullptr;
    decltype(&RGBStopCapture) stop = nullptr;
    decltype(&RGBGetModeInfo) getMode = nullptr;
    decltype(&RGBGetFrameRate) getRate = nullptr;
private:
    template <typename T> T Function(const char* name) {
        const FARPROC address = GetProcAddress(module_, name);
        if (!address)
            throw std::runtime_error(std::string("The Datapath runtime is missing ") + name + ".");
        return reinterpret_cast<T>(address);
    }
    HMODULE module_ = nullptr;
    bool keepLoaded_ = false;
};

struct DatapathSharedRuntime {
    std::mutex mutex;
    HRGBDLL library = 0;
    size_t users = 0;
    bool unsafe = false;

    template <typename Api>
    void Acquire(Api& api) {
        std::lock_guard<std::mutex> lock(mutex);
        if (unsafe)
            throw std::runtime_error("A Datapath input could not be closed safely. Restart the app before reopening it.");
        if (!users) CheckDatapath(api.load(&library), "Loading Datapath capture driver");
        ++users;
    }

    template <typename Api>
    std::string Release(Api& api, bool closeFailed) {
        std::lock_guard<std::mutex> lock(mutex);
        if (closeFailed) {
            unsafe = true;
            api.KeepLoaded();
        }
        --users;
        if (users || unsafe) return {};
        const auto result = api.free(library);
        library = 0;
        if (result) {
            unsafe = true;
            api.KeepLoaded();
            return "Releasing Datapath driver failed (error " + std::to_string(result) + ").";
        }
        return {};
    }
};

DatapathSharedRuntime g_datapathRuntime;

std::wstring DatapathId(const RGBINPUTINFOW& info, unsigned long index) {
    return std::to_wstring(info.Identifier[0]) + L":" + std::to_wstring(info.Identifier[1]) +
        L":" + std::to_wstring(info.Location.Bus) + L":" + std::to_wstring(info.Location.Device) +
        L":" + std::to_wstring(info.Location.Function) + L":" + std::to_wstring(index);
}

struct DatapathControlDescription {
    const wchar_t* name;
    const char* minimum;
    const char* maximum;
    const char* current;
    const char* defaultValue;
    const char* set;
    bool unsignedValue;
};

constexpr DatapathControlDescription kDatapathControls[] = {
    {L"Brightness", "RGBGetBrightnessMinimum", "RGBGetBrightnessMaximum", "RGBGetBrightness", "RGBGetBrightnessDefault", "RGBSetBrightness", false},
    {L"Contrast", "RGBGetContrastMinimum", "RGBGetContrastMaximum", "RGBGetContrast", "RGBGetContrastDefault", "RGBSetContrast", false},
    {L"Black level", "RGBGetBlackLevelMinimum", "RGBGetBlackLevelMaximum", "RGBGetBlackLevel", "RGBGetBlackLevelDefault", "RGBSetBlackLevel", false},
    {L"Saturation", "RGBGetSaturationMinimum", "RGBGetSaturationMaximum", "RGBGetSaturation", "RGBGetSaturationDefault", "RGBSetSaturation", false},
    {L"Hue", "RGBGetHueMinimum", "RGBGetHueMaximum", "RGBGetHue", "RGBGetHueDefault", "RGBSetHue", false},
    {L"Horizontal position", "RGBGetHorPositionMinimum", "RGBGetHorPositionMaximum", "RGBGetHorPosition", "RGBGetHorPositionDefault", "RGBSetHorPosition", false},
    {L"Vertical position", "RGBGetVerPositionMinimum", "RGBGetVerPositionMaximum", "RGBGetVerPosition", "RGBGetVerPositionDefault", "RGBSetVerPosition", false},
    {L"Phase", "RGBGetPhaseMinimum", "RGBGetPhaseMaximum", "RGBGetPhase", "RGBGetPhaseDefault", "RGBSetPhase", false},
    {L"Horizontal timing", "RGBGetHorScaleMinimum", "RGBGetHorScaleMaximum", "RGBGetHorScale", "RGBGetHorScaleDefault", "RGBSetHorScale", true},
    {L"Capture width", "RGBGetCaptureWidthMinimum", "RGBGetCaptureWidthMaximum", "RGBGetCaptureWidth", "RGBGetCaptureWidthDefault", "RGBSetCaptureWidth", true},
    {L"Capture height", "RGBGetCaptureHeightMinimum", "RGBGetCaptureHeightMaximum", "RGBGetCaptureHeight", "RGBGetCaptureHeightDefault", "RGBSetCaptureHeight", true}
};

template <typename Api>
int64_t ReadDatapathControl(Api& api, HRGB input,
                            const DatapathControlDescription& control, const char* function) {
    if (control.unsignedValue) {
        const auto get = api.template OptionalFunction<decltype(&RGBGetCaptureWidth)>(function);
        if (!get) throw std::runtime_error(std::string("The Datapath driver does not support ") + function + ".");
        unsigned long value = 0;
        CheckDatapath(get(input, &value), function);
        return value;
    }
    const auto get = api.template OptionalFunction<decltype(&RGBGetBrightness)>(function);
    if (!get) throw std::runtime_error(std::string("The Datapath driver does not support ") + function + ".");
    signed long value = 0;
    CheckDatapath(get(input, &value), function);
    return value;
}

template <typename Api>
void SetDatapathControl(Api& api, HRGB input, size_t index, int64_t value) {
    if (index >= std::size(kDatapathControls))
        throw std::runtime_error("Invalid Datapath video control.");
    const auto& control = kDatapathControls[index];
    const int64_t minimum = ReadDatapathControl(api, input, control, control.minimum);
    const int64_t maximum = ReadDatapathControl(api, input, control, control.maximum);
    if (minimum > maximum || value < minimum || value > maximum)
        throw std::runtime_error("The Datapath value is outside the driver-reported range.");
    if (control.unsignedValue) {
        const auto set = api.template OptionalFunction<decltype(&RGBSetCaptureWidth)>(control.set);
        if (!set) throw std::runtime_error(std::string("The Datapath driver does not support ") + control.set + ".");
        CheckDatapath(set(input, static_cast<unsigned long>(value)), control.set);
    } else {
        const auto set = api.template OptionalFunction<decltype(&RGBSetBrightness)>(control.set);
        if (!set) throw std::runtime_error(std::string("The Datapath driver does not support ") + control.set + ".");
        CheckDatapath(set(input, static_cast<signed long>(value)), control.set);
    }
}

template <typename Api>
void ApplyDatapathSettings(Api& api, HRGB input, const std::vector<DatapathSetting>& settings) {
    for (const auto& setting : settings) SetDatapathControl(api, input, setting.control, setting.value);
}

void RememberDatapathSetting(std::vector<DatapathSetting>& settings, size_t control, int64_t value) {
    settings.erase(std::remove_if(settings.begin(), settings.end(),
        [control](const DatapathSetting& setting) { return setting.control == control; }), settings.end());
    settings.push_back({control, value});
}

int64_t ParseDatapathSetting(const wchar_t* text) {
    errno = 0;
    wchar_t* end = nullptr;
    const long long value = std::wcstoll(text, &end, 10);
    if (end == text || *end != L'\0' || errno == ERANGE)
        throw std::runtime_error("Enter a whole number within the displayed range.");
    return value;
}

std::vector<NdiSource> EnumerateDatapath() {
    DatapathApi api;
    if (!api.Available()) return {};
    g_datapathRuntime.Acquire(api);
    bool acquired = true;
    try {
        unsigned long count = 0;
        CheckDatapath(api.getCount(&count), "Enumerating Datapath inputs");
        std::vector<NdiSource> sources;
        for (unsigned long index = 0; index < count; ++index) {
            RGBINPUTINFOW info{};
            info.Size = sizeof(info);
            CheckDatapath(api.getInfo(index, &info), "Reading Datapath input information");
            NdiSource source;
            source.kind = SourceKind::Datapath;
            source.driverIndex = index;
            source.deviceId = DatapathId(info, index);
            const size_t nameLength = wcsnlen_s(info.DeviceName, std::size(info.DeviceName));
            const std::wstring name(info.DeviceName, nameLength);
            source.name = L"Datapath: " + (name.empty() ? std::wstring(L"Vision input") : name) +
                L" [input " + std::to_wstring(index) + L"]";
            sources.push_back(std::move(source));
        }
        const auto releaseError = g_datapathRuntime.Release(api, false);
        acquired = false;
        if (!releaseError.empty()) throw std::runtime_error(releaseError);
        return sources;
    } catch (const std::exception& failure) {
        if (acquired) {
            const auto cleanupError = g_datapathRuntime.Release(api, false);
            if (!cleanupError.empty())
                throw std::runtime_error(std::string(failure.what()) + " " + cleanupError);
        }
        throw;
    } catch (...) {
        if (acquired) {
            const auto cleanupError = g_datapathRuntime.Release(api, false);
            if (!cleanupError.empty()) PostStatus(std::wstring(cleanupError.begin(), cleanupError.end()));
        }
        throw;
    }
}

std::shared_ptr<CameraFrame> CopyDatapathFrame(const BITMAPINFOHEADER& bitmap,
                                               const void* pixels, int rate) {
    if (!pixels || bitmap.biSize < sizeof(BITMAPINFOHEADER) || bitmap.biWidth <= 0 ||
        bitmap.biWidth > INT_MAX / 4 || bitmap.biHeight == 0 || bitmap.biHeight == LONG_MIN ||
        bitmap.biPlanes != 1 || (bitmap.biBitCount != 24 && bitmap.biBitCount != 32) ||
        (bitmap.biCompression != BI_RGB && bitmap.biCompression != BI_BITFIELDS))
        throw std::runtime_error("Datapath returned an unsupported bitmap format.");
    if (bitmap.biCompression == BI_BITFIELDS) {
        const auto* masks = reinterpret_cast<const DWORD*>(
            reinterpret_cast<const BYTE*>(&bitmap) + sizeof(BITMAPINFOHEADER));
        if (masks[0] != 0x00ff0000 || masks[1] != 0x0000ff00 || masks[2] != 0x000000ff)
            throw std::runtime_error("Datapath returned unsupported RGB color masks.");
    }
    const int height = bitmap.biHeight < 0 ? -bitmap.biHeight : bitmap.biHeight;
    const uint64_t rowBytes = (static_cast<uint64_t>(bitmap.biWidth) * bitmap.biBitCount + 31) / 32 * 4;
    const uint64_t imageBytes = rowBytes * height;
    if (rowBytes > PTRDIFF_MAX || imageBytes > 128ULL * 1024 * 1024 ||
        (bitmap.biSizeImage && bitmap.biSizeImage < imageBytes))
        throw std::runtime_error("Datapath returned an invalid bitmap size.");
    const auto* first = static_cast<const BYTE*>(pixels);
    const ptrdiff_t stride = static_cast<ptrdiff_t>(rowBytes);
    const BYTE* top = bitmap.biHeight > 0 ? first + static_cast<size_t>(height - 1) * stride : first;
    return CopyCameraFrame(top, bitmap.biHeight > 0 ? -stride : stride,
                           bitmap.biWidth, height, bitmap.biBitCount, rate, 1000);
}

struct DatapathCapture {
    PreviewSession* session = &g_recordPreview;
    std::mutex mutex;
    CaptureRate rate;
    std::string error;
    ULONGLONG previousTimestamp = 0;
    int width = 0;
    int height = 0;
    std::atomic<int> reportedRate{0};
    std::atomic<bool> interlaced{false};
    std::atomic<bool> active{true};
    std::atomic<unsigned long> driverError{0};
};

void RGBCBKAPI DatapathFrameCallback(HWND, HRGB, PRGBFRAMEDATA data, ULONG_PTR userData) {
    auto& context = *reinterpret_cast<DatapathCapture*>(userData);
    if (!context.active.load()) return;
    try {
        std::lock_guard<std::mutex> lock(context.mutex);
        if (!context.error.empty() || !context.active.load()) return;
        if (!data || data->Size < sizeof(RGBFRAMEDATA) || !data->PBitmapInfo || !data->PBitmapBits)
            throw std::runtime_error("Datapath returned an incomplete capture frame.");
        const auto& bitmap = *data->PBitmapInfo;
        const bool recording = SessionIsRecording(*context.session);
        if (recording && context.interlaced.load())
            throw std::runtime_error("Interlaced Datapath capture is not supported for recording.");
        if (recording && !data->TimeStamp)
            throw std::runtime_error("Datapath did not provide a frame timestamp; frame loss cannot be monitored.");
        if (recording && context.width &&
            (context.width != bitmap.biWidth ||
             context.height != (bitmap.biHeight < 0 ? -static_cast<int64_t>(bitmap.biHeight) : bitmap.biHeight)))
            throw std::runtime_error("The Datapath input size changed during recording.");
        const int expectedRate = context.reportedRate.load();
        if (recording && expectedRate <= 0)
            throw std::runtime_error("Datapath did not report a capture rate; frame gaps cannot be monitored.");
        if (recording && data->TimeStamp && context.previousTimestamp && expectedRate > 0) {
            if (data->TimeStamp <= context.previousTimestamp ||
                data->TimeStamp - context.previousTimestamp >
                    static_cast<ULONGLONG>(17'500'000) / static_cast<unsigned long>(expectedRate))
                throw std::runtime_error("Datapath frame timestamps show a gap or discontinuity; frames may have been lost.");
        }
        context.previousTimestamp = data->TimeStamp;
        const int rate = context.rate.Add(std::chrono::steady_clock::now());
        auto frame = CopyDatapathFrame(bitmap, data->PBitmapBits, rate);
        frame->interlaced = context.interlaced.load();
        frame->signalRateMilliHz = expectedRate * 1000;
        context.width = frame->width;
        context.height = frame->height;
        PublishCameraFrame(std::move(frame), *context.session);
    } catch (const std::exception& error) {
        {
            std::lock_guard<std::mutex> lock(context.mutex);
            if (context.error.empty()) context.error = error.what();
        }
        if (context.session->queue) StopQueue(*context.session->queue, error.what());
    } catch (...) {
        {
            std::lock_guard<std::mutex> lock(context.mutex);
            if (context.error.empty()) context.error = "Unexpected Datapath frame callback error.";
        }
        if (context.session->queue) StopQueue(*context.session->queue, "Unexpected Datapath frame callback error.");
    }
}

void RGBCBKAPI DatapathErrorCallback(HWND, HRGB, unsigned long error, ULONG_PTR userData,
                                     unsigned long*) {
    auto& context = *reinterpret_cast<DatapathCapture*>(userData);
    if (!context.active.load()) return;
    context.driverError.store(error ? error : 1);
    try {
        if (SessionIsRecording(*context.session))
            StopQueue(*context.session->queue, "The Datapath driver reported a non-recoverable capture error.");
    } catch (...) {
        // The worker observes driverError even if reporting from the callback fails.
    }
}

void PreviewDatapath(NdiSource source, std::vector<DatapathSetting> settings, PreviewSession* session) {
    try {
        DatapathApi api;
        if (!api.Available()) throw std::runtime_error("Install the 64-bit Datapath Vision driver and RGBEasy runtime.");
        HRGB input = 0;
        bool started = false;
        auto context = std::make_unique<DatapathCapture>();
        context->session = session;
        std::string error;
        g_datapathRuntime.Acquire(api);
        bool closeFailed = false;
        try {
            RGBINPUTINFOW info{};
            info.Size = sizeof(info);
            CheckDatapath(api.getInfo(source.driverIndex, &info), "Checking Datapath input");
            if (DatapathId(info, source.driverIndex) != source.deviceId)
                throw std::runtime_error("The selected Datapath input changed. Refresh the source list.");
            CheckDatapath(api.open(source.driverIndex, &input), "Opening Datapath input");
            ApplyDatapathSettings(api, input, settings);
            CheckDatapath(api.setDropping(input, 0), "Disabling Datapath intentional frame dropping");
            CheckDatapath(api.setDma(input, FALSE), "Selecting Datapath system-memory capture");
            CheckDatapath(api.setPixelFormat(input, RGB_PIXELFORMAT_888), "Selecting Datapath RGB capture");
            CheckDatapath(api.setFrameCallback(input, DatapathFrameCallback,
                          reinterpret_cast<ULONG_PTR>(context.get())), "Installing Datapath frame callback");
            CheckDatapath(api.setErrorCallback(input, DatapathErrorCallback,
                          reinterpret_cast<ULONG_PTR>(context.get())), "Installing Datapath error callback");
            CheckDatapath(api.start(input), "Starting Datapath capture");
            started = true;
            while (!session->stop.load()) {
                if (context->driverError.load())
                    throw std::runtime_error("Datapath capture driver reported error " +
                                             std::to_string(context->driverError.load()) + ".");
                {
                    std::lock_guard<std::mutex> lock(context->mutex);
                    if (!context->error.empty()) throw std::runtime_error(context->error);
                }
                RGBMODEINFO mode{};
                mode.Size = sizeof(mode);
                CheckDatapath(api.getMode(input, &mode), "Checking Datapath signal");
                if (mode.State == RGB_STATE_CAPTURING) {
                    context->interlaced.store(mode.BInterlaced != 0);
                    if (mode.BInterlaced && SessionIsRecording(*session))
                        throw std::runtime_error("Interlaced Datapath capture is not supported for recording.");
                    unsigned long rate = 0;
                    CheckDatapath(api.getRate(input, &rate), "Reading Datapath frame rate");
                    if (rate > 1000) throw std::runtime_error("Datapath returned an invalid frame rate.");
                    const int previousRate = context->reportedRate.exchange(static_cast<int>(rate));
                    if (SessionIsRecording(*session) && previousRate > 0 && rate > 0 &&
                        std::abs(previousRate - static_cast<int>(rate)) > 1)
                        throw std::runtime_error("The Datapath input frame rate changed during recording.");
                } else if (SessionIsRecording(*session)) {
                    throw std::runtime_error("The Datapath signal was lost or capture stopped.");
                }
                if (SessionIsRecording(*session)) {
                    std::lock_guard<std::mutex> lock(session->mutex);
                    if (!session->frame ||
                        std::chrono::steady_clock::now() - session->frame->received >= std::chrono::seconds(3))
                        throw std::runtime_error("Datapath stopped delivering video frames.");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        } catch (const std::exception& failure) {
            error = failure.what();
        } catch (...) {
            error = "Unexpected Datapath capture error.";
        }
        context->active.store(false);
        const auto appendError = [&](const std::string& message) {
            if (!error.empty()) error += " ";
            error += message;
        };
        if (started) {
            const unsigned long result = api.stop(input);
            if (result) appendError("Stopping Datapath capture failed (error " + std::to_string(result) + ").");
        }
        if (input) {
            const unsigned long frameResult = api.setFrameCallback(input, nullptr, 0);
            if (frameResult) appendError("Removing the Datapath frame callback failed (error " + std::to_string(frameResult) + ").");
            const unsigned long errorResult = api.setErrorCallback(input, nullptr, 0);
            if (errorResult) appendError("Removing the Datapath error callback failed (error " + std::to_string(errorResult) + ").");
            const unsigned long result = api.close(&input, 1);
            if (result) {
                appendError("Closing Datapath input failed (error " + std::to_string(result) + ").");
                closeFailed = true;
                context.release();
            }
        }
        const auto releaseError = g_datapathRuntime.Release(api, closeFailed);
        if (!releaseError.empty()) appendError(releaseError);
        if (!error.empty()) {
            if (session->stop.load()) PostStatus(L"Datapath shutdown failed: " + std::wstring(error.begin(), error.end()));
            else CameraFailed(error, *session);
        }
    } catch (const std::exception& error) {
        if (!session->stop.load()) CameraFailed(error.what(), *session);
    }
}
#endif

void RecordCamera(RecordingOptions options) {
    std::string error;
    bool wroteFrames = false;
    try {
        wroteFrames = WriteQueuedFrames(g_camera, options);
    } catch (const std::exception& failure) {
        error = failure.what();
    } catch (...) {
        error = "Unexpected camera recording error.";
    }
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.recording = false;
        g_camera.frames.clear();
        g_camera.bytes = 0;
    }
    if (g_recordStart.load() && !g_recordEnd.load()) g_recordEnd.store(GetTickCount64());
    PostRecordingResult(error, wroteFrames);
}

class MediaFoundationScope {
public:
    MediaFoundationScope() { CheckHResult(MFStartup(MF_VERSION), L"Starting Media Foundation"); }
    ~MediaFoundationScope() { MFShutdown(); }
};

class CameraReaderCallback final :
    public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IMFSourceReaderCallback> {
public:
    STDMETHODIMP OnReadSample(HRESULT status, DWORD, DWORD flags, LONGLONG,
                              IMFSample* sample) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ready_ && FAILED(status_)) return S_OK;
        status_ = status;
        flags_ = flags;
        sample_ = sample;
        ready_ = true;
        changed_.notify_one();
        return S_OK;
    }

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent* event) override {
        HRESULT status = S_OK;
        if (event && SUCCEEDED(event->GetStatus(&status)) && FAILED(status)) {
            std::lock_guard<std::mutex> lock(mutex_);
            status_ = status;
            ready_ = true;
            changed_.notify_one();
        }
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD) override { return S_OK; }

    bool Wait(ComPtr<IMFSample>& sample, DWORD& flags, const std::atomic<bool>& stop) {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!ready_ && !stop.load()) {
            changed_.wait_for(lock, std::chrono::milliseconds(100));
        }
        if (stop.load()) return false;
        CheckHResult(status_, L"Reading USB camera");
        flags = flags_;
        sample = std::move(sample_);
        ready_ = false;
        return true;
    }

private:
    std::mutex mutex_;
    std::condition_variable changed_;
    ComPtr<IMFSample> sample_;
    HRESULT status_ = S_OK;
    DWORD flags_ = 0;
    bool ready_ = false;
};

std::shared_ptr<CameraFrame> CopyUsbSample(IMFSample* sample, IMFMediaType* type,
                                          int width, int height, int rateN, int rateD) {
    ComPtr<IMFMediaBuffer> buffer;
    CheckHResult(sample->ConvertToContiguousBuffer(&buffer), L"Getting USB camera frame");
    ComPtr<IMF2DBuffer> buffer2d;
    if (SUCCEEDED(buffer.As(&buffer2d))) {
        BYTE* top = nullptr;
        LONG stride = 0;
        CheckHResult(buffer2d->Lock2D(&top, &stride), L"Locking USB camera frame");
        std::shared_ptr<CameraFrame> result;
        try {
            result = CopyCameraFrame(top, stride, width, height, 32, rateN, rateD);
        } catch (...) {
            buffer2d->Unlock2D();
            throw;
        }
        CheckHResult(buffer2d->Unlock2D(), L"Unlocking USB camera frame");
        return result;
    }

    BYTE* data = nullptr;
    DWORD length = 0;
    CheckHResult(buffer->Lock(&data, nullptr, &length), L"Locking USB camera frame");
    std::shared_ptr<CameraFrame> result;
    try {
        UINT32 rawStride = 0;
        LONG stride = -width * 4;
        if (SUCCEEDED(type->GetUINT32(MF_MT_DEFAULT_STRIDE, &rawStride))) {
            stride = static_cast<LONG>(rawStride);
        }
        const uint64_t rowBytes = static_cast<uint64_t>(width) * 4;
        const uint64_t absStride = stride < 0 ? -static_cast<int64_t>(stride) : stride;
        const uint64_t required = absStride * static_cast<uint64_t>(height - 1) + rowBytes;
        if (absStride < rowBytes || required > length) {
            throw std::runtime_error("The USB camera returned an incomplete frame.");
        }
        const BYTE* top = stride < 0 ? data + absStride * (height - 1) : data;
        result = CopyCameraFrame(top, stride, width, height, 32, rateN, rateD);
    } catch (...) {
        buffer->Unlock();
        throw;
    }
    CheckHResult(buffer->Unlock(), L"Unlocking USB camera frame");
    return result;
}

ComPtr<IMFMediaSource> OpenUsbDevice(const std::wstring& deviceId) {
    ComPtr<IMFAttributes> attributes;
    CheckHResult(MFCreateAttributes(&attributes, 2), L"Creating camera settings");
    CheckHResult(attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                 MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID), L"Selecting video capture");
    CheckHResult(attributes->SetString(
                 MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, deviceId.c_str()),
                 L"Selecting USB camera");
    ComPtr<IMFMediaSource> device;
    CheckHResult(MFCreateDeviceSource(attributes.Get(), &device), L"Opening USB camera");
    return device;
}

bool SameUsbMode(const UsbMode& left, const UsbMode& right) {
    return left.width == right.width && left.height == right.height &&
        static_cast<uint64_t>(left.rateN) * right.rateD ==
            static_cast<uint64_t>(right.rateN) * left.rateD;
}

ComPtr<IMFMediaType> UsbRequestedType(const std::optional<UsbMode>& mode) {
    ComPtr<IMFMediaType> requested;
    CheckHResult(MFCreateMediaType(&requested), L"Creating camera video type");
    CheckHResult(requested->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), L"Setting camera video type");
    CheckHResult(requested->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32), L"Selecting camera RGB video");
    if (mode) {
        CheckHResult(MFSetAttributeSize(requested.Get(), MF_MT_FRAME_SIZE, mode->width, mode->height),
                     L"Selecting USB camera resolution");
        CheckHResult(MFSetAttributeRatio(requested.Get(), MF_MT_FRAME_RATE, mode->rateN, mode->rateD),
                     L"Selecting USB camera frame rate");
    }
    return requested;
}

void PreviewUsb(NdiSource source, std::optional<UsbMode> mode, PreviewSession* session) {
    try {
        ComApartment apartment;
        MediaFoundationScope mediaFoundation;
        auto device = OpenUsbDevice(source.deviceId);
        struct SourceShutdown {
            IMFMediaSource* source;
            ~SourceShutdown() { source->Shutdown(); }
        } shutdown{device.Get()};
        auto callback = Microsoft::WRL::Make<CameraReaderCallback>();
        if (!callback) throw std::runtime_error("Could not create the USB camera callback.");
        ComPtr<IMFAttributes> readerAttributes;
        CheckHResult(MFCreateAttributes(&readerAttributes, 2), L"Creating camera reader settings");
        CheckHResult(readerAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callback.Get()),
                     L"Setting camera callback");
        CheckHResult(readerAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE),
                     L"Enabling camera video conversion");
        ComPtr<IMFSourceReader> reader;
        CheckHResult(MFCreateSourceReaderFromMediaSource(device.Get(), readerAttributes.Get(), &reader),
                     L"Creating USB camera reader");
        auto requested = UsbRequestedType(mode);
        const DWORD stream = static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
        CheckHResult(reader->SetCurrentMediaType(stream, nullptr, requested.Get()),
                     L"Configuring USB camera RGB video");
        ComPtr<IMFMediaType> actual;
        CheckHResult(reader->GetCurrentMediaType(stream, &actual), L"Reading camera video format");
        UINT32 width = 0, height = 0, rateN = 0, rateD = 0;
        CheckHResult(MFGetAttributeSize(actual.Get(), MF_MT_FRAME_SIZE, &width, &height),
                     L"Reading camera resolution");
        MFGetAttributeRatio(actual.Get(), MF_MT_FRAME_RATE, &rateN, &rateD);
        if (!width || !height || width > INT_MAX / 4 || height > INT_MAX ||
            rateN > INT_MAX || rateD > INT_MAX) {
            throw std::runtime_error("The USB camera reported an unsupported video format.");
        }
        if (mode && (!rateN || !rateD || !SameUsbMode(*mode, {width, height, rateN, rateD}))) {
            throw std::runtime_error("The USB camera did not accept the selected resolution and frame rate.");
        }

        while (!session->stop.load()) {
            CheckHResult(reader->ReadSample(stream, 0, nullptr, nullptr, nullptr, nullptr),
                         L"Requesting USB camera frame");
            ComPtr<IMFSample> sample;
            DWORD flags = 0;
            if (!callback->Wait(sample, flags, session->stop)) break;
            if (flags & (MF_SOURCE_READERF_ERROR | MF_SOURCE_READERF_ENDOFSTREAM)) {
                throw std::runtime_error("The USB camera stream stopped or was disconnected.");
            }
            if (flags & (MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED |
                         MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)) {
                throw std::runtime_error("The USB camera video format changed; refresh the source.");
            }
            if (sample) {
                PublishCameraFrame(CopyUsbSample(sample.Get(), actual.Get(),
                    static_cast<int>(width), static_cast<int>(height),
                    static_cast<int>(rateN), static_cast<int>(rateD)), *session);
            }
        }
    } catch (const std::exception& error) {
        if (!session->stop.load()) CameraFailed(error.what(), *session);
    }
}

struct VfwCapture {
    PreviewSession* session = &g_recordPreview;
    HWND window = nullptr;
    BITMAPINFOHEADER bitmap{};
    std::string error;
    bool connected = false;

    ~VfwCapture() {
        if (window) {
            capPreview(window, FALSE);
            capSetCallbackOnFrame(window, nullptr);
            if (connected) capDriverDisconnect(window);
            DestroyWindow(window);
        }
    }

    static LRESULT CALLBACK OnFrame(HWND window, LPVIDEOHDR header) {
        auto* capture = reinterpret_cast<VfwCapture*>(capGetUserData(window));
        if (!capture || !header || !header->lpData) return FALSE;
        try {
            const auto& bitmap = capture->bitmap;
            const int height = bitmap.biHeight < 0 ? -bitmap.biHeight : bitmap.biHeight;
            const uint64_t stride = ((static_cast<uint64_t>(bitmap.biWidth) *
                                      bitmap.biBitCount + 31) / 32) * 4;
            if (stride * height > header->dwBytesUsed) {
                throw std::runtime_error("The VFW driver returned an incomplete frame.");
            }
            const auto* data = reinterpret_cast<const BYTE*>(header->lpData);
            const BYTE* top = bitmap.biHeight > 0 ? data + stride * (height - 1) : data;
            const ptrdiff_t pitch = bitmap.biHeight > 0
                ? -static_cast<ptrdiff_t>(stride) : static_cast<ptrdiff_t>(stride);
            PublishCameraFrame(CopyCameraFrame(top, pitch, bitmap.biWidth, height,
                                              bitmap.biBitCount, 1000, 33), *capture->session);
            return TRUE;
        } catch (const std::exception& error) {
            capture->error = error.what();
            return FALSE;
        }
    }
};

void PreviewVfw(NdiSource source, PreviewSession* session) {
    try {
        VfwCapture capture;
        capture.session = session;
        capture.window = capCreateCaptureWindowW(
            L"VFW camera", WS_CHILD, 0, 0, 1, 1, HWND_MESSAGE, 0);
        if (!capture.window) throw std::runtime_error("Could not create the VFW capture window.");
        if (!capDriverConnect(capture.window, source.driverIndex)) {
            throw std::runtime_error("Could not connect to the VFW camera driver.");
        }
        capture.connected = true;
        const LRESULT size = capGetVideoFormatSize(capture.window);
        if (size < static_cast<LRESULT>(sizeof(BITMAPINFOHEADER)) || size > 1024 * 1024) {
            throw std::runtime_error("The VFW driver returned an invalid video format.");
        }
        std::vector<BYTE> format(static_cast<size_t>(size));
        if (!capGetVideoFormat(capture.window, format.data(), size)) {
            throw std::runtime_error("Could not read the VFW camera format.");
        }
        auto* bitmap = reinterpret_cast<BITMAPINFOHEADER*>(format.data());
        if (bitmap->biCompression != BI_RGB ||
            (bitmap->biBitCount != 24 && bitmap->biBitCount != 32)) {
            BITMAPINFOHEADER requested = *bitmap;
            requested.biSize = sizeof(requested);
            requested.biCompression = BI_RGB;
            requested.biBitCount = 24;
            requested.biPlanes = 1;
            requested.biSizeImage = 0;
            requested.biClrUsed = 0;
            requested.biClrImportant = 0;
            if (!capSetVideoFormat(capture.window, &requested, sizeof(requested))) {
                throw std::runtime_error("The VFW driver does not support 24-bit or 32-bit RGB video.");
            }
            capture.bitmap = requested;
        } else {
            capture.bitmap = *bitmap;
        }
        if (capture.bitmap.biWidth <= 0 || capture.bitmap.biWidth > INT_MAX / 4 ||
            capture.bitmap.biHeight == 0 || capture.bitmap.biHeight == LONG_MIN ||
            capture.bitmap.biPlanes != 1) {
            throw std::runtime_error("The VFW driver reported invalid video dimensions.");
        }
        capSetUserData(capture.window, &capture);
        if (!capSetCallbackOnFrame(capture.window, VfwCapture::OnFrame) ||
            !capPreviewRate(capture.window, 33) || !capPreview(capture.window, TRUE)) {
            throw std::runtime_error("Could not start VFW camera preview.");
        }
        while (!session->stop.load() && capture.error.empty()) {
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 100, QS_ALLINPUT);
            MSG message{};
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
        }
        if (!capture.error.empty()) throw std::runtime_error(capture.error);
    } catch (const std::exception& error) {
        if (!session->stop.load()) CameraFailed(error.what(), *session);
    }
}

void Preview(NdiSource source, PreviewSession* session) {
    try {
        NDIlib_recv_create_v3_t settings;
        settings.source_to_connect_to = NDIlib_source_t(
            source.ndiName.c_str(), source.address.c_str());
        settings.color_format = NDIlib_recv_color_format_BGRX_BGRA;
        settings.bandwidth = NDIlib_recv_bandwidth_highest;
        settings.allow_video_fields = false;
        settings.p_ndi_recv_name = APP_NAME_A " Preview";

        std::unique_ptr<NDIlib_recv_instance_type, ReceiverDeleter> receiver(
            NDIlib_recv_create_v3(&settings));
        if (!receiver) {
            throw std::runtime_error("Could not create the preview receiver.");
        }

        auto lastUpdate = std::chrono::steady_clock::time_point{};
        while (!session->stop.load()) {
            NDIlib_video_frame_v2_t frame{};
            const auto frameType = NDIlib_recv_capture_v3(
                receiver.get(), &frame, nullptr, nullptr, 250);
            if (frameType == NDIlib_frame_type_video) {
                VideoFrameGuard frameGuard{receiver.get(), &frame};
                const auto now = std::chrono::steady_clock::now();
                const auto interval = std::chrono::milliseconds(session->queue ? 100 : 33);
                if (now - lastUpdate < interval) {
                    continue;
                }
                auto preview = MakePreviewFrame(frame);
                {
                    std::lock_guard<std::mutex> lock(session->mutex);
                    session->frame = std::move(preview);
                    session->error.clear();
                }
                lastUpdate = now;
            } else if (frameType == NDIlib_frame_type_error) {
                throw std::runtime_error("The preview connection was lost.");
            }
        }
    } catch (const std::exception& error) {
        const std::string message = error.what();
        std::lock_guard<std::mutex> lock(session->mutex);
        session->frame.reset();
        session->error = std::wstring(message.begin(), message.end());
    } catch (...) {
        std::lock_guard<std::mutex> lock(session->mutex);
        session->frame.reset();
        session->error = L"Preview failed with an unexpected error.";
    }
}

void StopPreviewSession(PreviewSession& session) {
    session.stop.store(true);
    if (session.worker.joinable()) {
        session.worker.join();
    }
    {
        std::lock_guard<std::mutex> lock(session.mutex);
        session.frame.reset();
        session.error.clear();
        session.lastUpdate = {};
    }
}

void StartPreviewSession(PreviewSession& session, const NdiSource& source) {
    session.stop.store(false);
    try {
        if (source.kind == SourceKind::Usb) {
            const auto saved = g_usbModes.find(source.deviceId);
            const std::optional<UsbMode> mode = saved == g_usbModes.end()
                ? std::nullopt : std::optional<UsbMode>(saved->second);
            session.worker = std::thread(PreviewUsb, source, mode, &session);
        } else if (source.kind == SourceKind::Vfw) {
            session.worker = std::thread(PreviewVfw, source, &session);
#ifdef HAVE_EPIPHAN_SDK
        } else if (source.kind == SourceKind::Epiphan) {
            session.worker = std::thread(PreviewEpiphan, source, &session);
#endif
#ifdef HAVE_DATAPATH_SDK
        } else if (source.kind == SourceKind::Datapath) {
            const auto saved = g_datapathSettings.find(source.deviceId);
            const std::vector<DatapathSetting> settings = saved == g_datapathSettings.end()
                ? std::vector<DatapathSetting>{} : saved->second;
            session.worker = std::thread(PreviewDatapath, source, settings, &session);
#endif
        } else {
            session.worker = std::thread(Preview, source, &session);
        }
    } catch (const std::exception& error) {
        const std::string message = error.what();
        std::lock_guard<std::mutex> lock(session.mutex);
        session.error = L"Could not start preview: " + std::wstring(message.begin(), message.end());
    }
}

void StopPreview() {
    StopPreviewSession(g_recordPreview);
    {
        std::lock_guard<std::mutex> lock(g_camera.mutex);
        g_camera.frames.clear();
        g_camera.bytes = 0;
        g_camera.recording = false;
        g_camera.error.clear();
    }
    g_previewName.clear();
    g_previewAddress.clear();
    g_previewDeviceId.clear();
    g_previewKind = SourceKind::Ndi;
    if (g_previewControl) {
        InvalidateRect(g_previewControl, nullptr, FALSE);
    }
}

void UpdatePreviewSource(bool force = false) {
    if (g_viewOnly) return;
    const LRESULT selection = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
    if (g_usbSettingsControl) {
        EnableWindow(g_usbSettingsControl, !g_recording && selection != CB_ERR &&
            static_cast<size_t>(selection) < g_sources.size() &&
            (g_sources[static_cast<size_t>(selection)].kind == SourceKind::Usb ||
             g_sources[static_cast<size_t>(selection)].kind == SourceKind::Epiphan ||
             g_sources[static_cast<size_t>(selection)].kind == SourceKind::Datapath));
        SetWindowTextW(g_usbSettingsControl,
            selection != CB_ERR && static_cast<size_t>(selection) < g_sources.size() &&
            (g_sources[static_cast<size_t>(selection)].kind == SourceKind::Epiphan ||
             g_sources[static_cast<size_t>(selection)].kind == SourceKind::Datapath)
                ? L"Device settings..." : L"USB settings...");
    }
    if (selection == CB_ERR || static_cast<size_t>(selection) >= g_sources.size()) {
        const bool hadMasks = !g_privacyMasks.empty() || g_drawingPrivacyMask;
        ClearPrivacyMasks();
        if (hadMasks) SetStatus(L"Privacy masks cleared because no video source is selected.");
        StopPreview();
        return;
    }
    const NdiSource source = g_sources[static_cast<size_t>(selection)];
    if ((!g_privacyMasks.empty() || g_drawingPrivacyMask) &&
        (!g_privacyMaskSource || !SamePrivacyMaskSource(*g_privacyMaskSource, source))) {
        ClearPrivacyMasks();
        SetStatus(L"Video source changed; privacy masks were cleared.");
    }
    if (!force && g_previewKind == source.kind && g_previewDeviceId == source.deviceId &&
        g_previewName == (source.kind == SourceKind::Ndi ? source.ndiName : "camera") &&
        g_previewAddress == source.address &&
        g_previewWorker.joinable()) {
        return;
    }
    StopPreview();
    g_previewName = source.kind == SourceKind::Ndi ? source.ndiName : "camera";
    g_previewAddress = source.address;
    g_previewDeviceId = source.deviceId;
    g_previewKind = source.kind;
    StartPreviewSession(g_recordPreview, source);
}

std::wstring FormatRecordingTime(ULONGLONG start, ULONGLONG end, ULONGLONG now) {
    const ULONGLONG seconds = start == 0 ? 0 : ((end ? end : now) - start) / 1000;
    wchar_t text[64]{};
    swprintf_s(text, L"Recording time: %02llu:%02llu:%02llu",
               seconds / 3600, (seconds / 60) % 60, seconds % 60);
    return text;
}

void UpdateRecordingTimer() {
    const std::wstring text = FormatRecordingTime(
        g_recordStart.load(), g_recordEnd.load(), GetTickCount64());
    SetWindowTextW(g_timerControl, text.c_str());
}

void RefreshCameraSources() {
#ifdef NDI_ONLY
    g_cameraSources.clear();
#else
    try {
        MediaFoundationScope mediaFoundation;
        ComPtr<IMFAttributes> attributes;
        CheckHResult(MFCreateAttributes(&attributes, 1), L"Creating camera discovery settings");
        CheckHResult(attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                     MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID), L"Selecting video devices");
        struct DeviceList {
            IMFActivate** devices = nullptr;
            UINT32 count = 0;
            ~DeviceList() {
                for (UINT32 i = 0; i < count; ++i) devices[i]->Release();
                CoTaskMemFree(devices);
            }
        } devices;
        CheckHResult(MFEnumDeviceSources(attributes.Get(), &devices.devices, &devices.count),
                     L"Enumerating USB cameras");
        std::vector<NdiSource> cameras;
        for (UINT32 i = 0; i < devices.count; ++i) {
            WCHAR* name = nullptr;
            WCHAR* id = nullptr;
            UINT32 length = 0;
            CheckHResult(devices.devices[i]->GetAllocatedString(
                         MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &length),
                         L"Reading camera name");
            const HRESULT result = devices.devices[i]->GetAllocatedString(
                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &id, &length);
            if (FAILED(result)) {
                CoTaskMemFree(name);
                CheckHResult(result, L"Reading camera identifier");
            }
            NdiSource camera;
            camera.name = L"USB: " + std::wstring(name);
            camera.deviceId = id;
            camera.kind = SourceKind::Usb;
            CoTaskMemFree(name);
            CoTaskMemFree(id);
            cameras.push_back(std::move(camera));
        }
        for (unsigned int index = 0; index < 10; ++index) {
            wchar_t name[256]{};
            wchar_t version[256]{};
            if (capGetDriverDescriptionW(index, name, 256, version, 256)) {
                NdiSource camera;
                camera.name = L"VFW: " + std::wstring(name);
                camera.deviceId = std::to_wstring(index);
                camera.driverIndex = index;
                camera.kind = SourceKind::Vfw;
                cameras.push_back(std::move(camera));
            }
        }
#ifdef HAVE_EPIPHAN_SDK
        try {
            auto epiphan = EnumerateEpiphan();
            cameras.insert(cameras.end(), epiphan.begin(), epiphan.end());
        } catch (const std::exception& error) {
            const std::string message = error.what();
            MessageBoxW(g_window, std::wstring(message.begin(), message.end()).c_str(),
                        L"Epiphan discovery", MB_OK | MB_ICONERROR);
        }
#endif
#ifdef HAVE_DATAPATH_SDK
        try {
            auto datapath = EnumerateDatapath();
            cameras.insert(cameras.end(), datapath.begin(), datapath.end());
        } catch (const std::exception& error) {
            const std::string message = error.what();
            MessageBoxW(g_window, std::wstring(message.begin(), message.end()).c_str(),
                        L"Datapath discovery", MB_OK | MB_ICONERROR);
        }
#endif
        g_cameraSources = std::move(cameras);
    } catch (const std::exception& error) {
        const std::string message = error.what();
        SetStatus(L"Camera discovery failed: " + std::wstring(message.begin(), message.end()));
    }
#endif
}

void UpdateStreamInformation() {
    std::shared_ptr<const PreviewFrame> frame;
    {
        std::lock_guard<std::mutex> lock(g_previewMutex);
        frame = g_previewFrame;
    }
    std::wstring stream = L"Video: unavailable";
    if (frame && std::chrono::steady_clock::now() - frame->received < std::chrono::seconds(3)) {
        stream = L"Video: " + std::to_wstring(frame->sourceWidth) + L" x " +
            std::to_wstring(frame->sourceHeight) +
            ((g_previewKind == SourceKind::Epiphan || g_previewKind == SourceKind::Datapath)
                ? L" | Capture: " : L" | Frame rate: ");
        if (frame->frameRateN > 0 && frame->frameRateD > 0) {
            wchar_t rate[32]{};
            swprintf_s(rate, L"%.2f fps",
                       static_cast<double>(frame->frameRateN) / frame->frameRateD);
            stream += rate;
        } else {
            stream += L"N/A";
        }
        if (g_previewKind == SourceKind::Epiphan && frame->signalRateMilliHz > 0) {
            wchar_t rate[40]{};
            swprintf_s(rate, L" | Input: %.2f Hz", frame->signalRateMilliHz / 1000.0);
            stream += rate;
        } else if (g_previewKind == SourceKind::Datapath && frame->signalRateMilliHz > 0) {
            wchar_t rate[40]{};
            swprintf_s(rate, L" | Driver: %.2f fps", frame->signalRateMilliHz / 1000.0);
            stream += rate;
        }
    }
    SetWindowTextW(g_streamInfoControl, stream.c_str());
    const std::wstring address = NumericSourceAddress(g_previewAddress);
    SetWindowTextW(g_addressInfoControl,
                   (L"Source IP: " + (address.empty() ? std::wstring(L"unavailable") : address)).c_str());
}

void UpdatePrivacyMaskControls() {
    if (!g_privacyMaskControl) return;
    const std::wstring label = L"Draw mask (" + std::to_wstring(g_privacyMasks.size()) + L"/2)";
    SetWindowTextW(g_privacyMaskControl, label.c_str());
    EnableWindow(g_privacyMaskControl, !g_recording &&
        (g_drawingPrivacyMask || g_privacyMasks.size() < kMaxPrivacyMasks));
    EnableWindow(g_clearPrivacyMasksControl, !g_recording && !g_privacyMasks.empty());
}

void ClearPrivacyMasks() {
    g_privacyMasks.clear();
    g_privacyMaskSource.reset();
    g_dragPrivacyMask.reset();
    g_drawingPrivacyMask = false;
    UpdatePrivacyMaskControls();
    if (g_previewControl) InvalidateRect(g_previewControl, nullptr, FALSE);
}

RECT PreviewImageRect(const RECT& area, int frameWidth, int frameHeight) {
    if (frameWidth <= 0 || frameHeight <= 0) return {};
    const double scale = std::min(static_cast<double>(area.right) / frameWidth,
                                  static_cast<double>(area.bottom) / frameHeight);
    const int width = std::max(1, static_cast<int>(frameWidth * scale));
    const int height = std::max(1, static_cast<int>(frameHeight * scale));
    const int x = (area.right - width) / 2;
    const int y = (area.bottom - height) / 2;
    return {x, y, x + width, y + height};
}

std::optional<std::pair<double, double>> PreviewCoordinates(
    const RECT& image, POINT point, bool clampToImage) {
    if (image.right <= image.left || image.bottom <= image.top) return std::nullopt;
    if (!clampToImage &&
        (point.x < image.left || point.x >= image.right ||
         point.y < image.top || point.y >= image.bottom)) {
        return std::nullopt;
    }
    const double x = std::clamp(static_cast<double>(point.x - image.left) /
                                (image.right - image.left), 0.0, 1.0);
    const double y = std::clamp(static_cast<double>(point.y - image.top) /
                                (image.bottom - image.top), 0.0, 1.0);
    return std::make_pair(x, y);
}

RECT PrivacyMaskRect(const PrivacyMask& mask, const RECT& image) {
    const int width = image.right - image.left;
    const int height = image.bottom - image.top;
    return {
        image.left + static_cast<int>(std::floor(mask.left * width)),
        image.top + static_cast<int>(std::floor(mask.top * height)),
        image.left + static_cast<int>(std::ceil(mask.right * width)),
        image.top + static_cast<int>(std::ceil(mask.bottom * height))
    };
}

LRESULT CALLBACK PreviewProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
    }
    auto* session = reinterpret_cast<PreviewSession*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (!session) session = &g_recordPreview;
    if (message == WM_ERASEBKGND) {
        return 1;
    }
    if (message == WM_LBUTTONDOWN && session == &g_recordPreview &&
        g_drawingPrivacyMask && !g_recording &&
        g_privacyMasks.size() < kMaxPrivacyMasks) {
        std::shared_ptr<const PreviewFrame> frame;
        {
            std::lock_guard<std::mutex> lock(session->mutex);
            frame = session->frame;
        }
        RECT area{};
        GetClientRect(window, &area);
        const RECT image = frame ? PreviewImageRect(area, frame->width, frame->height) : RECT{};
        POINT point{static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam))};
        const auto coordinates = frame &&
            std::chrono::steady_clock::now() - frame->received < std::chrono::seconds(3)
            ? PreviewCoordinates(image, point, false) : std::nullopt;
        if (!coordinates) {
            SetStatus(L"Wait for a live preview, then drag inside the video to draw a mask.");
            return 0;
        }
        g_dragPrivacyStartX = coordinates->first;
        g_dragPrivacyStartY = coordinates->second;
        g_dragPrivacyMask = PrivacyMask{
            g_dragPrivacyStartX, g_dragPrivacyStartY,
            g_dragPrivacyStartX, g_dragPrivacyStartY
        };
        SetCapture(window);
        return 0;
    }
    if (message == WM_MOUSEMOVE && session == &g_recordPreview &&
        g_dragPrivacyMask && GetCapture() == window) {
        std::shared_ptr<const PreviewFrame> frame;
        {
            std::lock_guard<std::mutex> lock(session->mutex);
            frame = session->frame;
        }
        if (frame) {
            RECT area{};
            GetClientRect(window, &area);
            const RECT image = PreviewImageRect(area, frame->width, frame->height);
            POINT point{static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam))};
            const auto coordinates = PreviewCoordinates(image, point, true);
            if (coordinates) {
                g_dragPrivacyMask = PrivacyMask{
                    std::min(g_dragPrivacyStartX, coordinates->first),
                    std::min(g_dragPrivacyStartY, coordinates->second),
                    std::max(g_dragPrivacyStartX, coordinates->first),
                    std::max(g_dragPrivacyStartY, coordinates->second)
                };
                InvalidateRect(window, nullptr, FALSE);
            }
        }
        return 0;
    }
    if (message == WM_LBUTTONUP && session == &g_recordPreview &&
        g_dragPrivacyMask && GetCapture() == window) {
        std::shared_ptr<const PreviewFrame> frame;
        {
            std::lock_guard<std::mutex> lock(session->mutex);
            frame = session->frame;
        }
        if (frame) {
            RECT area{};
            GetClientRect(window, &area);
            const RECT image = PreviewImageRect(area, frame->width, frame->height);
            POINT point{static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam))};
            const auto coordinates = PreviewCoordinates(image, point, true);
            if (coordinates) {
                const PrivacyMask mask{
                    std::min(g_dragPrivacyStartX, coordinates->first),
                    std::min(g_dragPrivacyStartY, coordinates->second),
                    std::max(g_dragPrivacyStartX, coordinates->first),
                    std::max(g_dragPrivacyStartY, coordinates->second)
                };
                if ((mask.right - mask.left) * frame->width >= 4 &&
                    (mask.bottom - mask.top) * frame->height >= 4) {
                    g_privacyMasks.push_back(mask);
                    g_drawingPrivacyMask = g_privacyMasks.size() < kMaxPrivacyMasks;
                    SetStatus(L"Gray mask added. It will cover the full recording.");
                } else {
                    SetStatus(L"Mask is too small. Drag a larger rectangle over the private area.");
                }
            }
        }
        g_dragPrivacyMask.reset();
        ReleaseCapture();
        UpdatePrivacyMaskControls();
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    }
    if (message == WM_CAPTURECHANGED && session == &g_recordPreview && g_dragPrivacyMask) {
        g_dragPrivacyMask.reset();
        InvalidateRect(window, nullptr, FALSE);
    }
    if (message == WM_PAINT) {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT area{};
        GetClientRect(window, &area);

        std::shared_ptr<const PreviewFrame> frame;
        std::wstring error;
        {
            std::lock_guard<std::mutex> lock(session->mutex);
            frame = session->frame;
            error = session->error;
        }
        const bool fresh = frame &&
            std::chrono::steady_clock::now() - frame->received < std::chrono::seconds(3);
        if (fresh && area.right > 0 && area.bottom > 0) {
            const RECT image = PreviewImageRect(area, frame->width, frame->height);
            // Clear only the letterbox bars so a black frame is never painted over live video.
            const RECT bars[] = {
                {0, 0, area.right, image.top},
                {0, image.bottom, area.right, area.bottom},
                {0, image.top, image.left, image.bottom},
                {image.right, image.top, area.right, image.bottom}
            };
            for (const auto& bar : bars)
                FillRect(dc, &bar, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
            BITMAPINFO bitmap{};
            bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
            bitmap.bmiHeader.biWidth = frame->width;
            bitmap.bmiHeader.biHeight = -frame->height;
            bitmap.bmiHeader.biPlanes = 1;
            bitmap.bmiHeader.biBitCount = 32;
            bitmap.bmiHeader.biCompression = BI_RGB;
            const uint8_t* pixels = frame->pixels.data();
            std::vector<uint8_t> gray;
            if (session == &g_recordPreview && g_monochromePreview) {
                gray.resize(frame->pixels.size());
                for (size_t i = 0; i + 3 < gray.size(); i += 4)
                    gray[i] = gray[i + 1] = gray[i + 2] = frame->pixels[i + 1];
                pixels = gray.data();
            }
            SetStretchBltMode(dc, COLORONCOLOR);
            StretchDIBits(dc, image.left, image.top,
                          image.right - image.left, image.bottom - image.top,
                          0, 0, frame->width, frame->height,
                          pixels, &bitmap, DIB_RGB_COLORS, SRCCOPY);
            if (session == &g_recordPreview) {
                const HBRUSH maskBrush = CreateSolidBrush(RGB(128, 128, 128));
                if (maskBrush) {
                    for (const auto& mask : g_privacyMasks) {
                        const RECT rectangle = PrivacyMaskRect(mask, image);
                        FillRect(dc, &rectangle, maskBrush);
                    }
                    if (g_dragPrivacyMask) {
                        const RECT rectangle = PrivacyMaskRect(*g_dragPrivacyMask, image);
                        FrameRect(dc, &rectangle, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
                    }
                    DeleteObject(maskBrush);
                }
            }
        } else {
            FillRect(dc, &area, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
            const wchar_t* text = !error.empty() ? error.c_str() :
                ((session == &g_recordPreview ? g_previewName.empty() : !session->worker.joinable())
                    ? L"Select a video source" : L"Waiting for video...");
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, RGB(255, 255, 255));
            DrawTextW(dc, text, -1, &area, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        }
        EndPaint(window, &paint);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

void EnableRecordingControls(bool enabled) {
    EnableWindow(g_viewButton, enabled);
    EnableWindow(g_sourceControl, enabled);
    EnableWindow(GetDlgItem(g_window, kRefreshControl), enabled);
    EnableWindow(g_discoveryIpControl, enabled);
    EnableWindow(GetDlgItem(g_window, kApplyIpControl), enabled);
    EnableWindow(g_ndiSettingsControl, enabled);
    EnableWindow(g_pathControl, enabled);
    EnableWindow(GetDlgItem(g_window, kBrowseControl), enabled);
    EnableWindow(g_formatControl, enabled);
    EnableWindow(g_bitrateControl, enabled);
    EnableWindow(g_frameRateControl, enabled);
    EnableWindow(g_customFrameRateControl, enabled &&
        SendMessageW(g_frameRateControl, CB_GETCURSEL, 0, 0) == kCustomFrameRate);
    EnableWindow(g_codecSettingsControl, enabled);
    EnableWindow(g_colorControl, enabled);
    UpdatePrivacyMaskControls();
    const LRESULT selection = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
    EnableWindow(g_usbSettingsControl, enabled && selection != CB_ERR &&
        static_cast<size_t>(selection) < g_sources.size() &&
        (g_sources[static_cast<size_t>(selection)].kind == SourceKind::Usb ||
         g_sources[static_cast<size_t>(selection)].kind == SourceKind::Epiphan ||
         g_sources[static_cast<size_t>(selection)].kind == SourceKind::Datapath));
}

std::vector<UsbMode> EnumerateUsbModes(const std::wstring& deviceId) {
    MediaFoundationScope mediaFoundation;
    auto device = OpenUsbDevice(deviceId);
    struct SourceShutdown {
        IMFMediaSource* source;
        ~SourceShutdown() { source->Shutdown(); }
    } shutdown{device.Get()};
    ComPtr<IMFSourceReader> reader;
    CheckHResult(MFCreateSourceReaderFromMediaSource(device.Get(), nullptr, &reader),
                 L"Opening USB format list");
    std::vector<UsbMode> modes;
    const DWORD stream = static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
    for (DWORD index = 0;; ++index) {
        ComPtr<IMFMediaType> type;
        const HRESULT result = reader->GetNativeMediaType(stream, index, &type);
        if (result == MF_E_NO_MORE_TYPES) break;
        CheckHResult(result, L"Reading USB capture format");
        UsbMode mode{};
        if (FAILED(MFGetAttributeSize(type.Get(), MF_MT_FRAME_SIZE, &mode.width, &mode.height)) ||
            !mode.width || !mode.height) continue;
        const auto addRate = [&](REFGUID attribute) {
            if (FAILED(MFGetAttributeRatio(type.Get(), attribute, &mode.rateN, &mode.rateD)) ||
                !mode.rateN || !mode.rateD) return;
            if (std::none_of(modes.begin(), modes.end(), [&](const UsbMode& existing) {
                return SameUsbMode(existing, mode);
            })) modes.push_back(mode);
        };
        addRate(MF_MT_FRAME_RATE);
        addRate(MF_MT_FRAME_RATE_RANGE_MAX);
    }
    if (modes.empty()) {
        throw std::runtime_error("The USB device did not report any resolution and frame-rate combinations.");
    }
    return modes;
}

ComPtr<IBaseFilter> OpenUsbControlFilter(const std::wstring& deviceId) {
    ComPtr<ICreateDevEnum> devices;
    CheckHResult(CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&devices)), L"Opening camera control discovery");
    ComPtr<IEnumMoniker> enumerator;
    const HRESULT enumeration = devices->CreateClassEnumerator(
        CLSID_VideoInputDeviceCategory, &enumerator, 0);
    if (enumeration == S_FALSE) throw std::runtime_error("No DirectShow camera controls are available.");
    CheckHResult(enumeration, L"Enumerating camera controls");
    ComPtr<IMoniker> moniker;
    while (enumerator->Next(1, &moniker, nullptr) == S_OK) {
        ComPtr<IPropertyBag> properties;
        if (SUCCEEDED(moniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&properties)))) {
            VARIANT path;
            VariantInit(&path);
            const HRESULT result = properties->Read(L"DevicePath", &path, nullptr);
            const bool match = SUCCEEDED(result) && path.vt == VT_BSTR && path.bstrVal &&
                _wcsicmp(path.bstrVal, deviceId.c_str()) == 0;
            VariantClear(&path);
            if (match) {
                ComPtr<IBaseFilter> filter;
                CheckHResult(moniker->BindToObject(nullptr, nullptr, IID_PPV_ARGS(&filter)),
                             L"Opening USB driver controls");
                return filter;
            }
        }
        moniker.Reset();
    }
    throw std::runtime_error("The USB device does not expose a matching DirectShow control interface.");
}

struct UsbProperty {
    const wchar_t* name;
    bool camera;
    LONG id;
};

constexpr UsbProperty kUsbProperties[] = {
    {L"Brightness", false, VideoProcAmp_Brightness},
    {L"Contrast", false, VideoProcAmp_Contrast},
    {L"Hue", false, VideoProcAmp_Hue},
    {L"Saturation", false, VideoProcAmp_Saturation},
    {L"Sharpness", false, VideoProcAmp_Sharpness},
    {L"Gamma", false, VideoProcAmp_Gamma},
    {L"White balance", false, VideoProcAmp_WhiteBalance},
    {L"Backlight compensation", false, VideoProcAmp_BacklightCompensation},
    {L"Gain", false, VideoProcAmp_Gain},
    {L"Exposure", true, CameraControl_Exposure},
    {L"Focus", true, CameraControl_Focus},
    {L"Zoom", true, CameraControl_Zoom},
    {L"Pan", true, CameraControl_Pan},
    {L"Tilt", true, CameraControl_Tilt},
    {L"Roll", true, CameraControl_Roll},
    {L"Iris", true, CameraControl_Iris},
};

struct UsbSettingsContext {
    std::vector<UsbMode> modes;
    std::optional<UsbMode> selected;
    std::string formatsError;
    ComPtr<IBaseFilter> filter;
    ComPtr<IAMCameraControl> camera;
    ComPtr<IAMVideoProcAmp> video;
    std::vector<UsbProperty> properties;
    std::string controlsError;
};

constexpr int kUsbModeChoice = 2101;
constexpr int kUsbPropertyChoice = 2102;
constexpr int kUsbPropertyValue = 2103;
constexpr int kUsbPropertyAuto = 2104;
constexpr int kUsbPropertyRange = 2105;
constexpr int kUsbPropertyApply = 2106;
constexpr int kUsbDriverSettings = 2107;

HRESULT UsbPropertyRange(UsbSettingsContext& context, const UsbProperty& property,
                         LONG* minimum, LONG* maximum, LONG* step, LONG* defaultValue,
                         LONG* flags) {
    if (property.camera) {
        return context.camera ? context.camera->GetRange(property.id, minimum, maximum, step, defaultValue, flags)
                              : E_NOINTERFACE;
    }
    return context.video ? context.video->GetRange(property.id, minimum, maximum, step, defaultValue, flags)
                         : E_NOINTERFACE;
}

void UpdateUsbProperty(HWND dialog, UsbSettingsContext& context) {
    EnableWindow(GetDlgItem(dialog, kUsbPropertyApply), FALSE);
    const LRESULT selection = SendDlgItemMessageW(dialog, kUsbPropertyChoice, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= context.properties.size()) return;
    const auto& property = context.properties[static_cast<size_t>(selection)];
    LONG minimum = 0, maximum = 0, step = 0, defaultValue = 0, capabilities = 0;
    CheckHResult(UsbPropertyRange(context, property, &minimum, &maximum, &step, &defaultValue, &capabilities),
                 L"Reading USB control range");
    LONG value = 0, flags = 0;
    const HRESULT result = property.camera ? context.camera->Get(property.id, &value, &flags)
                                           : context.video->Get(property.id, &value, &flags);
    CheckHResult(result, L"Reading USB control value");
    SetDlgItemInt(dialog, kUsbPropertyValue, static_cast<UINT>(value), TRUE);
    const std::wstring range = L"Range: " + std::to_wstring(minimum) + L" to " +
        std::to_wstring(maximum) + L", step " + std::to_wstring(step);
    SetDlgItemTextW(dialog, kUsbPropertyRange, range.c_str());
    CheckDlgButton(dialog, kUsbPropertyAuto, (flags & CameraControl_Flags_Auto) ? BST_CHECKED : BST_UNCHECKED);
    EnableWindow(GetDlgItem(dialog, kUsbPropertyAuto), (capabilities & CameraControl_Flags_Auto) != 0);
    EnableWindow(GetDlgItem(dialog, kUsbPropertyValue), (capabilities & CameraControl_Flags_Manual) != 0);
    EnableWindow(GetDlgItem(dialog, kUsbPropertyApply), TRUE);
}

bool IsValidUsbManualValue(LONG value, LONG minimum, LONG maximum, LONG step) {
    return step > 0 && value >= minimum && value <= maximum &&
        (static_cast<int64_t>(value) - minimum) % step == 0;
}

void ApplyUsbProperty(HWND dialog, UsbSettingsContext& context) {
    const LRESULT selection = SendDlgItemMessageW(dialog, kUsbPropertyChoice, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= context.properties.size()) {
        throw std::runtime_error("Select an available USB camera control.");
    }
    const auto& property = context.properties[static_cast<size_t>(selection)];
    LONG minimum = 0, maximum = 0, step = 0, defaultValue = 0, capabilities = 0;
    CheckHResult(UsbPropertyRange(context, property, &minimum, &maximum, &step, &defaultValue, &capabilities),
                 L"Reading USB control range");
    const bool automatic = IsDlgButtonChecked(dialog, kUsbPropertyAuto) == BST_CHECKED;
    BOOL valid = FALSE;
    const LONG value = static_cast<LONG>(GetDlgItemInt(dialog, kUsbPropertyValue, &valid, TRUE));
    if (!valid || (!automatic && !IsValidUsbManualValue(value, minimum, maximum, step))) {
        throw std::runtime_error("Enter a value within the displayed range and step.");
    }
    const LONG flags = automatic ? CameraControl_Flags_Auto : CameraControl_Flags_Manual;
    if (!(capabilities & flags)) throw std::runtime_error("The selected control mode is not supported.");
    const HRESULT result = property.camera ? context.camera->Set(property.id, value, flags)
                                           : context.video->Set(property.id, value, flags);
    CheckHResult(result, L"Applying USB camera control");
    UpdateUsbProperty(dialog, context);
}

void ShowUsbDriverSettings(HWND dialog, UsbSettingsContext& context) {
    if (!context.filter) throw std::runtime_error(context.controlsError);
    ComPtr<ISpecifyPropertyPages> pages;
    CheckHResult(context.filter.As(&pages), L"Opening USB driver settings");
    CAUUID ids{};
    CheckHResult(pages->GetPages(&ids), L"Reading USB driver settings pages");
    struct FreePages { CAUUID& ids; ~FreePages() { CoTaskMemFree(ids.pElems); } } freePages{ids};
    if (!ids.cElems) throw std::runtime_error("The USB driver does not provide a settings dialog.");
    IUnknown* object = context.filter.Get();
    CheckHResult(OleCreatePropertyFrame(dialog, 0, 0, L"USB driver settings", 1, &object,
                                        ids.cElems, ids.pElems, 0, 0, nullptr),
                 L"Showing USB driver settings");
}

INT_PTR CALLBACK UsbSettingsDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* context = reinterpret_cast<UsbSettingsContext*>(GetWindowLongPtrW(dialog, DWLP_USER));
    if (message == WM_INITDIALOG) {
        context = reinterpret_cast<UsbSettingsContext*>(lParam);
        SetWindowLongPtrW(dialog, DWLP_USER, reinterpret_cast<LONG_PTR>(context));
        SetWindowTextW(dialog, L"USB capture settings");
        const auto control = [&](const wchar_t* type, const wchar_t* text, DWORD style,
                                 int x, int y, int width, int height, int id) {
            return CreateWindowW(type, text, WS_CHILD | WS_VISIBLE | style, x, y, width, height,
                dialog, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
        };
        control(L"STATIC", L"Resolution and frame rate:", 0, 15, 15, 200, 22, 0);
        HWND modes = control(WC_COMBOBOXW, L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                             15, 40, 390, 240, kUsbModeChoice);
        SendMessageW(modes, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Device default"));
        LRESULT selected = 0;
        for (size_t i = 0; i < context->modes.size(); ++i) {
            const auto& mode = context->modes[i];
            wchar_t label[128]{};
            swprintf_s(label, L"%u x %u  -  %.3f fps", mode.width, mode.height,
                       static_cast<double>(mode.rateN) / mode.rateD);
            SendMessageW(modes, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label));
            if (context->selected && SameUsbMode(*context->selected, mode)) selected = static_cast<LRESULT>(i + 1);
        }
        SendMessageW(modes, CB_SETCURSEL, selected, 0);
        if (!context->formatsError.empty()) EnableWindow(modes, FALSE);
        control(L"STATIC", L"Camera control:", 0, 15, 85, 140, 22, 0);
        HWND properties = control(WC_COMBOBOXW, L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                                  15, 110, 200, 240, kUsbPropertyChoice);
        for (const auto& property : context->properties) {
            SendMessageW(properties, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(property.name));
        }
        control(L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
                225, 110, 80, 24, kUsbPropertyValue);
        control(L"BUTTON", L"Auto", WS_TABSTOP | BS_AUTOCHECKBOX,
                315, 110, 70, 24, kUsbPropertyAuto);
        control(L"STATIC", L"", 0, 15, 145, 300, 22, kUsbPropertyRange);
        control(L"BUTTON", L"Apply control", WS_TABSTOP | BS_PUSHBUTTON,
                15, 175, 130, 30, kUsbPropertyApply);
        control(L"BUTTON", L"Driver settings...", WS_TABSTOP | BS_PUSHBUTTON,
                160, 175, 145, 30, kUsbDriverSettings);
        control(L"STATIC", L"Control and driver changes apply immediately.", 0, 15, 215, 390, 22, 0);
        control(L"BUTTON", L"OK", WS_TABSTOP | BS_DEFPUSHBUTTON, 225, 250, 85, 30, IDOK);
        control(L"BUTTON", L"Cancel", WS_TABSTOP | BS_PUSHBUTTON, 320, 250, 85, 30, IDCANCEL);
        SetWindowPos(dialog, nullptr, 0, 0, 440, 335, SWP_NOMOVE | SWP_NOZORDER);
        if (context->properties.empty()) {
            SetDlgItemTextW(dialog, kUsbPropertyRange, L"No standard camera controls available.");
            EnableWindow(properties, FALSE);
            EnableWindow(GetDlgItem(dialog, kUsbPropertyValue), FALSE);
            EnableWindow(GetDlgItem(dialog, kUsbPropertyAuto), FALSE);
            EnableWindow(GetDlgItem(dialog, kUsbPropertyApply), FALSE);
        } else {
            SendMessageW(properties, CB_SETCURSEL, 0, 0);
            try { UpdateUsbProperty(dialog, *context); }
            catch (const std::exception& error) {
                const std::string text = error.what();
                MessageBoxW(dialog, std::wstring(text.begin(), text.end()).c_str(), L"USB settings", MB_OK | MB_ICONERROR);
            }
        }
        if (!context->formatsError.empty()) {
            MessageBoxW(dialog, std::wstring(context->formatsError.begin(), context->formatsError.end()).c_str(),
                        L"USB format list unavailable", MB_OK | MB_ICONERROR);
        }
        return TRUE;
    }
    if (message == WM_CLOSE) { EndDialog(dialog, IDCANCEL); return TRUE; }
    if (message != WM_COMMAND || !context) return FALSE;
    try {
        switch (LOWORD(wParam)) {
        case IDOK: {
            if (!context->formatsError.empty()) {
                EndDialog(dialog, IDOK);
                return TRUE;
            }
            const LRESULT selected = SendDlgItemMessageW(dialog, kUsbModeChoice, CB_GETCURSEL, 0, 0);
            if (selected == CB_ERR || static_cast<size_t>(selected) > context->modes.size()) {
                throw std::runtime_error("Select a USB capture format.");
            }
            context->selected = selected == 0 ? std::nullopt :
                std::optional<UsbMode>(context->modes[static_cast<size_t>(selected - 1)]);
            EndDialog(dialog, IDOK);
            return TRUE;
        }
        case IDCANCEL: EndDialog(dialog, IDCANCEL); return TRUE;
        case kUsbPropertyChoice:
            if (HIWORD(wParam) == CBN_SELCHANGE) UpdateUsbProperty(dialog, *context);
            return TRUE;
        case kUsbPropertyApply: ApplyUsbProperty(dialog, *context); return TRUE;
        case kUsbDriverSettings: ShowUsbDriverSettings(dialog, *context); return TRUE;
        }
    } catch (const std::exception& error) {
        const std::string text = error.what();
        MessageBoxW(dialog, std::wstring(text.begin(), text.end()).c_str(), L"USB settings", MB_OK | MB_ICONERROR);
        return TRUE;
    }
    return FALSE;
}

struct alignas(DWORD) UsbDialogTemplate {
    DLGTEMPLATE base{};
    WORD menu = 0;
    WORD windowClass = 0;
    WORD title = 0;

    UsbDialogTemplate() {
        base.style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME;
        base.cx = 280;
        base.cy = 200;
    }
};
static_assert(offsetof(UsbDialogTemplate, menu) == sizeof(DLGTEMPLATE), "Unexpected dialog layout.");

#ifdef HAVE_EPIPHAN_SDK
enum class EpiphanAdjustment { Horizontal, Vertical, Phase };

struct EpiphanSettingsContext {
    FrmGrabber* grabber = nullptr;
    std::wstring signal;
    std::vector<EpiphanAdjustment> adjustments;
};

struct EpiphanAdjustmentInfo {
    const wchar_t* name;
    int minimum;
    int maximum;
    int value;
    V2U_UINT32 flag;
};

EpiphanAdjustmentInfo EpiphanAdjustmentDetails(EpiphanAdjustment kind,
        const V2U_GrabParameters& params, const V2UAdjRange& range) {
    switch (kind) {
    case EpiphanAdjustment::Horizontal:
        return {L"Horizontal shift", range.hshift_min, range.hshift_max, params.hshift, V2U_FLAG_VALID_HSHIFT};
    case EpiphanAdjustment::Vertical:
        return {L"Vertical shift", range.vshift_min, range.vshift_max, params.vshift, V2U_FLAG_VALID_VSHIFT};
    case EpiphanAdjustment::Phase:
        return {L"Phase", std::max<int>(0, range.phase_min),
                std::min<int>(255, range.phase_max), params.phase, V2U_FLAG_VALID_PHASE};
    }
    throw std::runtime_error("Unknown Epiphan adjustment.");
}

void ReadEpiphanParameters(FrmGrabber* grabber, V2U_GrabParameters& params, V2UAdjRange& range) {
    params = {};
    range = {};
    if (!FrmGrab_GetGrabParams2(grabber, &params, &range))
        throw std::runtime_error("Could not read Epiphan adjustment values and ranges.");
}

void UpdateEpiphanAdjustment(HWND dialog, EpiphanSettingsContext& context) {
    const LRESULT selection = SendDlgItemMessageW(dialog, kUsbPropertyChoice, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= context.adjustments.size())
        throw std::runtime_error("Select an Epiphan adjustment.");
    V2U_GrabParameters params{};
    V2UAdjRange range{};
    ReadEpiphanParameters(context.grabber, params, range);
    const auto info = EpiphanAdjustmentDetails(context.adjustments[static_cast<size_t>(selection)], params, range);
    if (info.minimum >= info.maximum) throw std::runtime_error("This adjustment has no usable range for the current signal.");
    SetDlgItemInt(dialog, kUsbPropertyValue, static_cast<UINT>(info.value), TRUE);
    CheckDlgButton(dialog, kUsbPropertyAuto,
                   (params.flags & info.flag) ? BST_UNCHECKED : BST_CHECKED);
    const std::wstring text = L"Range: " + std::to_wstring(info.minimum) + L" to " + std::to_wstring(info.maximum);
    SetDlgItemTextW(dialog, kUsbPropertyRange, text.c_str());
}

void ApplyEpiphanAdjustment(HWND dialog, EpiphanSettingsContext& context) {
    const LRESULT selection = SendDlgItemMessageW(dialog, kUsbPropertyChoice, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= context.adjustments.size())
        throw std::runtime_error("Select an Epiphan adjustment.");
    const auto kind = context.adjustments[static_cast<size_t>(selection)];
    V2U_GrabParameters params{};
    V2UAdjRange range{};
    ReadEpiphanParameters(context.grabber, params, range);
    const auto info = EpiphanAdjustmentDetails(kind, params, range);
    if (info.minimum >= info.maximum) throw std::runtime_error("This adjustment is no longer available.");
    if (IsDlgButtonChecked(dialog, kUsbPropertyAuto) == BST_CHECKED) {
        params.flags &= ~info.flag;
    } else {
        BOOL valid = FALSE;
        const int value = static_cast<int>(GetDlgItemInt(dialog, kUsbPropertyValue, &valid, TRUE));
        if (!valid || value < info.minimum || value > info.maximum)
            throw std::runtime_error("Enter a value within the displayed adjustment range.");
        params.flags |= info.flag;
        if (kind == EpiphanAdjustment::Horizontal) params.hshift = value;
        else if (kind == EpiphanAdjustment::Vertical) params.vshift = value;
        else params.phase = static_cast<V2U_UINT8>(value);
    }
    if (!FrmGrab_SetGrabParams(context.grabber, &params))
        throw std::runtime_error("The Epiphan driver rejected the adjustment.");
    UpdateEpiphanAdjustment(dialog, context);
}

INT_PTR CALLBACK EpiphanSettingsDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* context = reinterpret_cast<EpiphanSettingsContext*>(GetWindowLongPtrW(dialog, DWLP_USER));
    if (message == WM_INITDIALOG) {
        context = reinterpret_cast<EpiphanSettingsContext*>(lParam);
        SetWindowLongPtrW(dialog, DWLP_USER, reinterpret_cast<LONG_PTR>(context));
        SetWindowTextW(dialog, L"Epiphan device settings");
        const auto control = [&](const wchar_t* type, const wchar_t* text, DWORD style,
                                 int x, int y, int width, int height, int id) {
            return CreateWindowW(type, text, WS_CHILD | WS_VISIBLE | style, x, y, width, height,
                dialog, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
        };
        control(L"STATIC", context->signal.c_str(), 0, 15, 15, 395, 65, 0);
        control(L"STATIC", L"Analog input adjustment:", 0, 15, 90, 200, 22, 0);
        HWND choice = control(WC_COMBOBOXW, L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                              15, 115, 200, 200, kUsbPropertyChoice);
        for (auto adjustment : context->adjustments) {
            V2U_GrabParameters unused{};
            V2UAdjRange range{};
            const auto info = EpiphanAdjustmentDetails(adjustment, unused, range);
            SendMessageW(choice, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(info.name));
        }
        control(L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 225, 115, 75, 24, kUsbPropertyValue);
        control(L"BUTTON", L"Auto", WS_TABSTOP | BS_AUTOCHECKBOX, 310, 115, 75, 24, kUsbPropertyAuto);
        control(L"STATIC", L"Adjustments unavailable for this signal.", 0, 15, 150, 390, 22, kUsbPropertyRange);
        control(L"BUTTON", L"Apply adjustment", WS_TABSTOP | BS_PUSHBUTTON, 15, 180, 145, 30, kUsbPropertyApply);
        control(L"STATIC", L"Changes apply immediately. Signal mode is set by the source.", 0, 15, 220, 395, 22, 0);
        control(L"BUTTON", L"Close", WS_TABSTOP | BS_DEFPUSHBUTTON, 320, 255, 85, 30, IDOK);
        SetWindowPos(dialog, nullptr, 0, 0, 440, 335, SWP_NOMOVE | SWP_NOZORDER);
        if (context->adjustments.empty()) {
            EnableWindow(choice, FALSE);
            EnableWindow(GetDlgItem(dialog, kUsbPropertyValue), FALSE);
            EnableWindow(GetDlgItem(dialog, kUsbPropertyAuto), FALSE);
            EnableWindow(GetDlgItem(dialog, kUsbPropertyApply), FALSE);
        } else {
            SendMessageW(choice, CB_SETCURSEL, 0, 0);
            try { UpdateEpiphanAdjustment(dialog, *context); }
            catch (const std::exception& error) {
                const std::string text = error.what();
                MessageBoxW(dialog, std::wstring(text.begin(), text.end()).c_str(), L"Epiphan settings", MB_OK | MB_ICONERROR);
                EnableWindow(GetDlgItem(dialog, kUsbPropertyApply), FALSE);
            }
        }
        return TRUE;
    }
    if (message == WM_CLOSE) { EndDialog(dialog, IDCANCEL); return TRUE; }
    if (message != WM_COMMAND || !context) return FALSE;
    try {
        switch (LOWORD(wParam)) {
        case IDOK: case IDCANCEL: EndDialog(dialog, IDOK); return TRUE;
        case kUsbPropertyChoice:
            if (HIWORD(wParam) == CBN_SELCHANGE) UpdateEpiphanAdjustment(dialog, *context);
            return TRUE;
        case kUsbPropertyApply: ApplyEpiphanAdjustment(dialog, *context); return TRUE;
        }
    } catch (const std::exception& error) {
        const std::string text = error.what();
        MessageBoxW(dialog, std::wstring(text.begin(), text.end()).c_str(), L"Epiphan settings", MB_OK | MB_ICONERROR);
        return TRUE;
    }
    return FALSE;
}

void ConfigureEpiphanDevice(const NdiSource& source) {
    g_configuringUsb = true;
    StopPreview();
    try {
        EpiphanHandle handle(OpenEpiphan(source.ndiName));
        EpiphanSettingsContext context;
        context.grabber = handle.value;
        V2U_VideoMode mode{};
        FrmGrab_GetVideoMode(handle.value, &mode);
        V2U_Property input{};
        input.key = V2UKey_InputSignalType;
        const bool hasInput = FrmGrab_GetProperty(handle.value, &input) != V2U_FALSE;
        context.signal = L"Input: ";
        if (!hasInput) context.signal += L"unavailable";
        else if (input.value.int32 & V2U_INPUT_ENCRYPTED) context.signal += L"encrypted";
        else if (input.value.int32 & V2U_INPUT_DIGITAL) context.signal += L"digital";
        else if (input.value.int32 & (V2U_INPUT_ANALOG | V2U_INPUT_SOG | V2U_INPUT_SOG3))
            context.signal += L"analog";
        else context.signal += L"no signal or unsupported type";
        if (mode.width > 0 && mode.height > 0 && mode.vfreq > 0) {
            wchar_t details[100]{};
            swprintf_s(details, L"\nSignal: %d x %d at %.2f Hz", mode.width, mode.height, mode.vfreq / 1000.0);
            context.signal += details;
        } else {
            context.signal += L"\nSignal mode unavailable";
        }
        if (hasInput && (input.value.int32 & (V2U_INPUT_ANALOG | V2U_INPUT_SOG | V2U_INPUT_SOG3))) {
            V2U_GrabParameters params{};
            V2UAdjRange range{};
            ReadEpiphanParameters(handle.value, params, range);
            for (auto adjustment : {EpiphanAdjustment::Horizontal, EpiphanAdjustment::Vertical, EpiphanAdjustment::Phase}) {
                const auto info = EpiphanAdjustmentDetails(adjustment, params, range);
                if (info.minimum < info.maximum) context.adjustments.push_back(adjustment);
            }
        }
        UsbDialogTemplate definition;
        if (DialogBoxIndirectParamW(GetModuleHandleW(nullptr), &definition.base, g_window,
                                    EpiphanSettingsDialogProc, reinterpret_cast<LPARAM>(&context)) == -1)
            throw std::runtime_error("Could not open Epiphan settings.");
    } catch (const std::exception& error) {
        const std::string text = error.what();
        MessageBoxW(g_window, std::wstring(text.begin(), text.end()).c_str(), L"Epiphan settings", MB_OK | MB_ICONERROR);
    }
    g_configuringUsb = false;
    UpdatePreviewSource(true);
}
#endif

#ifdef HAVE_DATAPATH_SDK
constexpr int kDatapathDefault = 2201;
constexpr int kDatapathForget = 2202;

struct DatapathSettingsContext {
    DatapathApi* api = nullptr;
    HRGB input = 0;
    std::wstring signal;
    std::vector<DatapathSetting>* settings = nullptr;
};

size_t SelectedDatapathControl(HWND dialog) {
    const LRESULT selection = SendDlgItemMessageW(dialog, kUsbPropertyChoice, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= std::size(kDatapathControls))
        throw std::runtime_error("Select a Datapath control.");
    return static_cast<size_t>(selection);
}

void UpdateDatapathControl(HWND dialog, DatapathSettingsContext& context) {
    const auto& control = kDatapathControls[SelectedDatapathControl(dialog)];
    EnableWindow(GetDlgItem(dialog, kUsbPropertyApply), FALSE);
    EnableWindow(GetDlgItem(dialog, kDatapathDefault), FALSE);
    EnableWindow(GetDlgItem(dialog, kUsbPropertyValue), FALSE);
    SetDlgItemTextW(dialog, kUsbPropertyValue, L"");
    try {
        const auto minimum = ReadDatapathControl(*context.api, context.input, control, control.minimum);
        const auto maximum = ReadDatapathControl(*context.api, context.input, control, control.maximum);
        const auto current = ReadDatapathControl(*context.api, context.input, control, control.current);
        if (minimum > maximum || current < minimum || current > maximum)
            throw std::runtime_error("The Datapath driver returned an invalid range or value.");
        const auto range = L"Range: " + std::to_wstring(minimum) + L" to " + std::to_wstring(maximum);
        SetDlgItemTextW(dialog, kUsbPropertyRange, range.c_str());
        SetDlgItemTextW(dialog, kUsbPropertyValue, std::to_wstring(current).c_str());
        EnableWindow(GetDlgItem(dialog, kUsbPropertyValue), TRUE);
        EnableWindow(GetDlgItem(dialog, kUsbPropertyApply), TRUE);
        EnableWindow(GetDlgItem(dialog, kDatapathDefault), TRUE);
    } catch (const std::exception& error) {
        const std::string text = error.what();
        const std::wstring message = L"Unavailable: " + std::wstring(text.begin(), text.end());
        SetDlgItemTextW(dialog, kUsbPropertyRange, message.c_str());
    }
}

INT_PTR CALLBACK DatapathSettingsDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* context = reinterpret_cast<DatapathSettingsContext*>(GetWindowLongPtrW(dialog, DWLP_USER));
    if (message == WM_INITDIALOG) {
        context = reinterpret_cast<DatapathSettingsContext*>(lParam);
        SetWindowLongPtrW(dialog, DWLP_USER, reinterpret_cast<LONG_PTR>(context));
        SetWindowTextW(dialog, L"Datapath video settings");
        const auto control = [&](const wchar_t* type, const wchar_t* text, DWORD style,
                                 int x, int y, int width, int height, int id) {
            return CreateWindowW(type, text, WS_CHILD | WS_VISIBLE | style, x, y, width, height,
                dialog, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
        };
        control(L"STATIC", context->signal.c_str(), 0, 15, 15, 400, 60, 0);
        control(L"STATIC", L"Video control:", 0, 15, 85, 200, 22, 0);
        HWND choice = control(WC_COMBOBOXW, L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                              15, 110, 200, 250, kUsbPropertyChoice);
        for (const auto& item : kDatapathControls)
            SendMessageW(choice, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.name));
        control(L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 225, 110, 90, 24, kUsbPropertyValue);
        SendDlgItemMessageW(dialog, kUsbPropertyValue, EM_SETLIMITTEXT, 20, 0);
        control(L"BUTTON", L"Default", WS_TABSTOP | BS_PUSHBUTTON, 325, 110, 80, 24, kDatapathDefault);
        control(L"STATIC", L"", 0, 15, 145, 395, 45, kUsbPropertyRange);
        control(L"BUTTON", L"Apply", WS_TABSTOP | BS_PUSHBUTTON, 15, 195, 110, 30, kUsbPropertyApply);
        control(L"BUTTON", L"Forget saved settings", WS_TABSTOP | BS_PUSHBUTTON, 135, 195, 160, 30, kDatapathForget);
        control(L"STATIC", L"Apply changes one control. Settings are reapplied during this app session.\n"
                L"Capture size is not the source signal mode. Forgetting settings does not reset the driver.",
                0, 15, 235, 395, 55, 0);
        control(L"BUTTON", L"Close", WS_TABSTOP | BS_DEFPUSHBUTTON, 320, 295, 85, 30, IDOK);
        SetWindowPos(dialog, nullptr, 0, 0, 440, 375, SWP_NOMOVE | SWP_NOZORDER);
        SendMessageW(choice, CB_SETCURSEL, 0, 0);
        UpdateDatapathControl(dialog, *context);
        return TRUE;
    }
    if (message == WM_CLOSE) { EndDialog(dialog, IDCANCEL); return TRUE; }
    if (message != WM_COMMAND || !context) return FALSE;
    try {
        switch (LOWORD(wParam)) {
        case IDOK: case IDCANCEL: EndDialog(dialog, IDOK); return TRUE;
        case kUsbPropertyChoice:
            if (HIWORD(wParam) == CBN_SELCHANGE) UpdateDatapathControl(dialog, *context);
            return TRUE;
        case kDatapathDefault: {
            const auto& item = kDatapathControls[SelectedDatapathControl(dialog)];
            const auto value = ReadDatapathControl(*context->api, context->input, item, item.defaultValue);
            SetDlgItemTextW(dialog, kUsbPropertyValue, std::to_wstring(value).c_str());
            return TRUE;
        }
        case kUsbPropertyApply: {
            const size_t index = SelectedDatapathControl(dialog);
            wchar_t text[21]{};
            GetDlgItemTextW(dialog, kUsbPropertyValue, text, static_cast<int>(std::size(text)));
            const int64_t value = ParseDatapathSetting(text);
            SetDatapathControl(*context->api, context->input, index, value);
            RememberDatapathSetting(*context->settings, index, value);
            const auto& item = kDatapathControls[index];
            const auto actual = ReadDatapathControl(*context->api, context->input, item, item.current);
            if (actual != value) {
                RememberDatapathSetting(*context->settings, index, actual);
                const auto warning = L"The driver applied " + std::to_wstring(actual) +
                    L" instead of the requested value.";
                MessageBoxW(dialog, warning.c_str(), L"Datapath settings", MB_OK | MB_ICONWARNING);
            }
            UpdateDatapathControl(dialog, *context);
            return TRUE;
        }
        case kDatapathForget:
            context->settings->clear();
            return TRUE;
        }
    } catch (const std::exception& error) {
        const std::string text = error.what();
        MessageBoxW(dialog, std::wstring(text.begin(), text.end()).c_str(), L"Datapath settings", MB_OK | MB_ICONERROR);
        return TRUE;
    }
    return FALSE;
}

void ConfigureDatapathDevice(const NdiSource& source) {
    g_configuringUsb = true;
    StopPreview();
    try {
        DatapathApi api;
        if (!api.Available()) throw std::runtime_error("Install the 64-bit Datapath Vision driver and RGBEasy runtime.");
        HRGB input = 0;
        std::string error;
        g_datapathRuntime.Acquire(api);
        bool closeFailed = false;
        try {
            RGBINPUTINFOW info{};
            info.Size = sizeof(info);
            CheckDatapath(api.getInfo(source.driverIndex, &info), "Checking Datapath input");
            if (DatapathId(info, source.driverIndex) != source.deviceId)
                throw std::runtime_error("The selected Datapath input changed. Refresh the source list.");
            CheckDatapath(api.open(source.driverIndex, &input), "Opening Datapath input");
            auto& settings = g_datapathSettings[source.deviceId];
            try {
                ApplyDatapathSettings(api, input, settings);
            } catch (const std::exception& failure) {
                const std::string text = failure.what();
                const std::wstring warning = L"Could not restore a saved setting: " +
                    std::wstring(text.begin(), text.end()) +
                    L"\nUse Forget saved settings if the signal or driver no longer supports it.";
                MessageBoxW(g_window, warning.c_str(), L"Datapath settings", MB_OK | MB_ICONWARNING);
            }
            DatapathSettingsContext context{&api, input, L"Signal information unavailable.", &settings};
            const auto signal = api.OptionalFunction<decltype(&RGBGetInputSignalType)>("RGBGetInputSignalType");
            if (signal) {
                SIGNALTYPE type = RGB_SIGNALTYPE_NOSIGNAL;
                unsigned long width = 0, height = 0, refresh = 0;
                const auto result = signal(source.driverIndex, &type, &width, &height, &refresh);
                if (result) {
                    context.signal = L"Signal information unavailable (Datapath error " +
                        std::to_wstring(result) + L").";
                } else if (type == RGB_SIGNALTYPE_NOSIGNAL || type == RGB_SIGNALTYPE_OUTOFRANGE) {
                    context.signal = type == RGB_SIGNALTYPE_NOSIGNAL ? L"No input signal detected." : L"Input signal is out of range.";
                } else {
                    const wchar_t* typeName = L"unknown";
                    switch (type) {
                    case RGB_SIGNALTYPE_VGA: typeName = L"VGA"; break;
                    case RGB_SIGNALTYPE_DVI: typeName = L"DVI"; break;
                    case RGB_SIGNALTYPE_YPRPB: typeName = L"component"; break;
                    case RGB_SIGNALTYPE_COMPOSITE: typeName = L"composite"; break;
                    case RGB_SIGNALTYPE_SVIDEO: typeName = L"S-video"; break;
                    case RGB_SIGNALTYPE_SDI: typeName = L"SDI"; break;
                    case RGB_SIGNALTYPE_DLDVI: typeName = L"dual-link DVI"; break;
                    case RGB_SIGNALTYPE_DISPLAYPORT: typeName = L"DisplayPort"; break;
                    default: break;
                    }
                    context.signal = L"Detected " + std::wstring(typeName) + L" input: " + std::to_wstring(width) + L" x " +
                        std::to_wstring(height) + L"\nThe source determines the signal resolution and refresh rate.";
                }
            } else {
                context.signal = L"Signal information is not supported by this Datapath driver.";
            }
            UsbDialogTemplate definition;
            if (DialogBoxIndirectParamW(GetModuleHandleW(nullptr), &definition.base, g_window,
                                       DatapathSettingsDialogProc, reinterpret_cast<LPARAM>(&context)) == -1)
                throw std::runtime_error("Could not open Datapath video settings.");
        } catch (const std::exception& failure) {
            error = failure.what();
        }
        if (input) {
            const auto result = api.close(&input, 1);
            if (result) {
                if (!error.empty()) error += " ";
                error += "Closing Datapath input failed (error " + std::to_string(result) + ").";
                closeFailed = true;
            }
        }
        const auto releaseError = g_datapathRuntime.Release(api, closeFailed);
        if (!releaseError.empty()) {
            if (!error.empty()) error += " ";
            error += releaseError;
        }
        if (!error.empty()) throw std::runtime_error(error);
    } catch (const std::exception& error) {
        const std::string text = error.what();
        MessageBoxW(g_window, std::wstring(text.begin(), text.end()).c_str(), L"Datapath settings", MB_OK | MB_ICONERROR);
    }
    g_configuringUsb = false;
    UpdatePreviewSource(true);
}
#endif

void ConfigureUsbDevice() {
    const LRESULT selection = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= g_sources.size()) return;
    const NdiSource source = g_sources[static_cast<size_t>(selection)];
#ifdef HAVE_EPIPHAN_SDK
    if (source.kind == SourceKind::Epiphan) {
        ConfigureEpiphanDevice(source);
        return;
    }
#endif
#ifdef HAVE_DATAPATH_SDK
    if (source.kind == SourceKind::Datapath) {
        ConfigureDatapathDevice(source);
        return;
    }
#endif
    if (source.kind != SourceKind::Usb) return;
    g_configuringUsb = true;
    StopPreview();
    try {
        UsbSettingsContext context;
        const auto previous = g_usbModes.find(source.deviceId);
        if (previous != g_usbModes.end()) context.selected = previous->second;
        try {
            context.modes = EnumerateUsbModes(source.deviceId);
        } catch (const std::exception& error) {
            context.formatsError = error.what();
        }
        if (context.selected && context.formatsError.empty()) {
            if (std::none_of(context.modes.begin(), context.modes.end(),
                [&](const UsbMode& mode) { return SameUsbMode(mode, *context.selected); })) {
                MessageBoxW(g_window, L"The previously selected USB format is no longer available. "
                            L"Choose a new format or the device default.",
                            L"USB settings", MB_OK | MB_ICONWARNING);
                context.selected.reset();
            }
        }
        try {
            context.filter = OpenUsbControlFilter(source.deviceId);
            context.filter.As(&context.camera);
            context.filter.As(&context.video);
            for (const auto& property : kUsbProperties) {
                LONG minimum = 0, maximum = 0, step = 0, defaultValue = 0, flags = 0;
                if (SUCCEEDED(UsbPropertyRange(context, property, &minimum, &maximum, &step, &defaultValue, &flags))) {
                    context.properties.push_back(property);
                }
            }
        } catch (const std::exception& error) {
            context.controlsError = error.what();
        }
        UsbDialogTemplate definition;
        const INT_PTR result = DialogBoxIndirectParamW(GetModuleHandleW(nullptr), &definition.base,
            g_window, UsbSettingsDialogProc, reinterpret_cast<LPARAM>(&context));
        if (result == -1) throw std::runtime_error("Could not open the USB settings window.");
        if (result == IDOK) {
            if (context.selected) g_usbModes.insert_or_assign(source.deviceId, *context.selected);
            else g_usbModes.erase(source.deviceId);
        }
    } catch (const std::exception& error) {
        const std::string text = error.what();
        MessageBoxW(g_window, std::wstring(text.begin(), text.end()).c_str(), L"USB settings", MB_OK | MB_ICONERROR);
    }
    g_configuringUsb = false;
    UpdatePreviewSource(true);
}

std::wstring UserGuidePath() {
    std::wstring path(MAX_PATH, L'\0');
    for (;;) {
        const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (length == 0) return {};
        if (length < path.size()) {
            path.resize(length);
            break;
        }
        path.resize(path.size() * 2);
    }
    const auto slash = path.find_last_of(L"\\/");
    path.resize(slash == std::wstring::npos ? 0 : slash + 1);
    return path + L"user-guide\\index.html";
}

void OpenUserGuide() {
    const HWND owner = g_viewOnly && g_viewPanel ? g_viewPanel : g_window;
    const std::wstring path = UserGuidePath();
    if (path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(owner, (L"The user guide was not found:\n" + path +
                            L"\n\nKeep the user-guide folder next to the application.").c_str(),
                    kAppName, MB_OK | MB_ICONERROR);
        return;
    }
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(owner, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
    if (result <= 32) {
        MessageBoxW(owner, (L"Could not open the user guide in a web browser:\n" + path).c_str(),
                    kAppName, MB_OK | MB_ICONERROR);
    }
}

std::string WideToUtf8(const std::wstring& value) {
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
                                         nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
                        result.data(), size, nullptr, nullptr);
    return result;
}

std::wstring TrimText(std::wstring text) {
    while (!text.empty() && iswspace(text.back())) text.pop_back();
    size_t first = 0;
    while (first < text.size() && iswspace(text[first])) ++first;
    return text.substr(first);
}

std::wstring ControlText(HWND control) {
    const int length = GetWindowTextLengthW(control);
    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(control, text.data(), length + 1);
    text.resize(static_cast<size_t>(length));
    return text;
}

std::wstring AppSettingsPath() {
    wchar_t appData[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return {};
    return std::wstring(appData) + L"\\" APP_NAME_W L"\\settings.ini";
}

void LoadAppSettings(const std::wstring& path, NdiSettings& settings) {
    settings = {};
    if (path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) return;
    wchar_t buffer[128]{};
    GetPrivateProfileStringW(L"NDI", L"DeviceIp", L"", buffer, 128, path.c_str());
    if (IsNumericIp(buffer)) settings.deviceIp = buffer;
}

void SaveAppSettings(const std::wstring& path, const NdiSettings& settings) {
    if (path.empty()) throw std::runtime_error("Could not find the user's application data folder.");
    const size_t slash = path.find_last_of(L'\\');
    if (slash != std::wstring::npos && !CreateDirectoryW(path.substr(0, slash).c_str(), nullptr) &&
        GetLastError() != ERROR_ALREADY_EXISTS) {
        throw std::runtime_error("Could not create the settings folder.");
    }
    if (!WritePrivateProfileStringW(L"NDI", L"DeviceIp", settings.deviceIp.c_str(), path.c_str()))
        throw std::runtime_error("Could not save the settings file.");
}

NDIlib_find_instance_t CreateNdiFinder(const std::wstring& extraIps) {
    const std::string ips = WideToUtf8(extraIps);
    NDIlib_find_create_t settings;
    settings.p_extra_ips = ips.empty() ? nullptr : ips.c_str();
    return NDIlib_find_create_v2(&settings);
}

bool RestartNdiDiscovery(const std::wstring& deviceIp) {
    NDIlib_find_instance_t finder = CreateNdiFinder(deviceIp);
    if (!finder) {
        MessageBoxW(g_window, L"Could not update NDI discovery. The previous settings are still active.",
                    kAppName, MB_OK | MB_ICONERROR);
        return false;
    }
    NDIlib_find_destroy(g_finder);
    g_finder = finder;
    StopPreview();
    g_sources.clear();
    SendMessageW(g_sourceControl, CB_RESETCONTENT, 0, 0);
    SetStatus(L"Searching for NDI sources. Discovery may take a few seconds.");
    return true;
}

void ApplyDiscoveryIp() {
    const std::wstring address = TrimText(ControlText(g_discoveryIpControl));
    if (!address.empty() && !IsNumericIp(address)) {
        MessageBoxW(g_window, L"Enter a numeric IPv4 or IPv6 address, or leave it blank.",
                    kAppName, MB_OK | MB_ICONERROR);
        return;
    }
    if (!RestartNdiDiscovery(address)) return;
    g_ndiSettings.deviceIp = address;
    try {
        SaveAppSettings(AppSettingsPath(), g_ndiSettings);
    } catch (const std::exception& error) {
        const std::string message = error.what();
        MessageBoxW(g_window, (L"The IP is active but could not be saved for next time:\n" +
                               std::wstring(message.begin(), message.end())).c_str(),
                    kAppName, MB_OK | MB_ICONWARNING);
    }
}

// NDI devices such as converters serve their settings on their own web page.
std::wstring DeviceWebPageUrl(const std::string& sourceAddress) {
    const std::wstring ip = NumericSourceAddress(sourceAddress);
    if (ip.empty()) return {};
    return ip.find(L':') != std::wstring::npos ? L"http://[" + ip + L"]/" : L"http://" + ip + L"/";
}

const NdiSource* SelectedSource() {
    const LRESULT selection = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= g_sources.size()) return nullptr;
    return &g_sources[static_cast<size_t>(selection)];
}

void OpenNdiDeviceWebPage() {
    const NdiSource* source = SelectedSource();
    if (!source || source->kind != SourceKind::Ndi) {
        MessageBoxW(g_window, L"Select an NDI source first.", kAppName, MB_OK | MB_ICONINFORMATION);
        return;
    }
    const std::wstring url = DeviceWebPageUrl(source->address);
    if (url.empty()) {
        MessageBoxW(g_window, L"The IP address of this NDI source is not known yet. Wait for the preview "
                              L"to start and try again.", kAppName, MB_OK | MB_ICONINFORMATION);
        return;
    }
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(g_window, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
    if (result <= 32) {
        MessageBoxW(g_window, (L"Could not open the device settings page in a web browser:\n" + url).c_str(),
                    kAppName, MB_OK | MB_ICONERROR);
    }
}
void UpdateViewSources();

void RefreshSources() {
    if (!g_finder || g_recording || g_configuringUsb) {
        return;
    }

    const LRESULT oldSelection = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
    std::optional<NdiSource> selected;
    if (oldSelection != CB_ERR && static_cast<size_t>(oldSelection) < g_sources.size()) {
        selected = g_sources[static_cast<size_t>(oldSelection)];
    }

    uint32_t count = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(g_finder, &count);
    if (!sources) {
        count = 0;
    }
    std::vector<NdiSource> currentSources;
    currentSources.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        NdiSource source;
        source.ndiName = sources[i].p_ndi_name ? sources[i].p_ndi_name : "";
        source.address = sources[i].p_url_address ? sources[i].p_url_address : "";
        source.name = Utf8ToWide(source.ndiName.c_str());
        currentSources.push_back(std::move(source));
    }
    for (const auto& ipFinder : g_ipFinders) {
        uint32_t ipCount = 0;
        const NDIlib_source_t* ipSources = NDIlib_find_get_current_sources(ipFinder.second, &ipCount);
        for (uint32_t i = 0; ipSources && i < ipCount; ++i) {
            const std::string ndiName = ipSources[i].p_ndi_name ? ipSources[i].p_ndi_name : "";
            if (std::any_of(currentSources.begin(), currentSources.end(),
                            [&](const NdiSource& known) { return known.ndiName == ndiName; }))
                continue;
            NdiSource source;
            source.ndiName = ndiName;
            source.address = ipSources[i].p_url_address ? ipSources[i].p_url_address : "";
            source.name = Utf8ToWide(source.ndiName.c_str());
            currentSources.push_back(std::move(source));
        }
    }
    currentSources.insert(currentSources.end(), g_cameraSources.begin(), g_cameraSources.end());

    bool changed = currentSources.size() != g_sources.size();
    if (!changed) {
        for (size_t i = 0; i < currentSources.size(); ++i) {
            if (currentSources[i].kind != g_sources[i].kind ||
                currentSources[i].ndiName != g_sources[i].ndiName ||
                currentSources[i].address != g_sources[i].address ||
                currentSources[i].deviceId != g_sources[i].deviceId ||
                currentSources[i].name != g_sources[i].name) {
                changed = true;
                break;
            }
        }
    }
    if (!changed) {
        return;
    }

    g_sources = std::move(currentSources);
    SendMessageW(g_sourceControl, CB_RESETCONTENT, 0, 0);
    LRESULT restoredSelection = CB_ERR;
    for (size_t i = 0; i < g_sources.size(); ++i) {
        const LRESULT item = SendMessageW(
            g_sourceControl, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(g_sources[i].name.c_str()));
        if (selected && g_sources[i].kind == selected->kind &&
            g_sources[i].ndiName == selected->ndiName &&
            g_sources[i].deviceId == selected->deviceId) {
            restoredSelection = item;
        }
    }
    if (!g_sources.empty()) {
        if (restoredSelection == CB_ERR) {
            for (size_t i = 0; i < g_sources.size(); ++i) {
                if (g_sources[i].kind == SourceKind::Ndi) {
                    restoredSelection = static_cast<LRESULT>(i);
                    break;
                }
            }
        }
        SendMessageW(g_sourceControl, CB_SETCURSEL, restoredSelection, 0);
        SetStatus(L"Select a source and press Start recording. Video only.");
    } else {
        SetStatus(kNdiOnly ? L"No NDI sources found. Check the network or enter an NDI device IP, then refresh."
                           : L"No video sources found. Connect a camera or NDI source, then refresh.");
    }
    if (g_viewOnly) UpdateViewSources();
    else UpdatePreviewSource();
}

bool SameViewSource(const NdiSource& left, const NdiSource& right) {
    if (left.kind != right.kind) return false;
    return left.kind == SourceKind::Ndi ? left.ndiName == right.ndiName
                                       : left.deviceId == right.deviceId;
}

void UpdateViewSources() {
    for (auto& slot : g_viewSlots) {
        SendMessageW(slot.choice, CB_RESETCONTENT, 0, 0);
        SendMessageW(slot.choice, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Select a source"));
        LRESULT selection = 0;
        std::optional<NdiSource> current;
        for (size_t i = 0; i < g_sources.size(); ++i) {
            SendMessageW(slot.choice, CB_ADDSTRING, 0,
                         reinterpret_cast<LPARAM>(g_sources[i].name.c_str()));
            if (slot.selected && SameViewSource(*slot.selected, g_sources[i])) {
                selection = static_cast<LRESULT>(i + 1);
                current = g_sources[i];
            }
        }
        SendMessageW(slot.choice, CB_SETCURSEL, selection, 0);
        if (slot.selected && !current) {
            StopPreviewSession(slot.session);
            slot.selected.reset();
            std::lock_guard<std::mutex> lock(slot.session.mutex);
            slot.session.error = L"Source no longer available. Select another source.";
        } else if (current && slot.selected && current->address != slot.selected->address) {
            StopPreviewSession(slot.session);
            slot.selected = current;
            StartPreviewSession(slot.session, *current);
        } else if (current) {
            slot.selected = current;
        }
        InvalidateRect(slot.preview, nullptr, FALSE);
    }
}

void SelectViewSource(size_t index) {
    auto& slot = g_viewSlots[index];
    const LRESULT selection = SendMessageW(slot.choice, CB_GETCURSEL, 0, 0);
    std::optional<NdiSource> source;
    if (selection > 0 && static_cast<size_t>(selection - 1) < g_sources.size())
        source = g_sources[static_cast<size_t>(selection - 1)];
    if (source) {
        for (size_t i = 0; i < static_cast<size_t>(g_viewCount); ++i) {
            if (i != index && g_viewSlots[i].selected && SameViewSource(*source, *g_viewSlots[i].selected)) {
                MessageBoxW(g_viewPanel, L"Choose a different source for each view.", L"View only",
                            MB_OK | MB_ICONINFORMATION);
                UpdateViewSources();
                return;
            }
        }
    }
    StopPreviewSession(slot.session);
    slot.pendingIp.clear();
    slot.selected = source;
    if (source) StartPreviewSession(slot.session, *source);
    InvalidateRect(slot.preview, nullptr, FALSE);
}

void StopViewSessions() {
    for (auto& slot : g_viewSlots) slot.session.stop.store(true);
    for (auto& slot : g_viewSlots) StopPreviewSession(slot.session);
}

void SetViewMessage(ViewSlot& slot, const std::wstring& message) {
    {
        std::lock_guard<std::mutex> lock(slot.session.mutex);
        slot.session.error = message;
    }
    InvalidateRect(slot.preview, nullptr, FALSE);
}

void ResolvePendingViewIps() {
    for (size_t index = 0; index < static_cast<size_t>(g_viewCount); ++index) {
        auto& slot = g_viewSlots[index];
        if (slot.pendingIp.empty()) continue;
        bool foundAny = false;
        std::optional<NdiSource> match;
        for (const auto& source : g_sources) {
            if (source.kind != SourceKind::Ndi || NumericSourceAddress(source.address) != slot.pendingIp)
                continue;
            foundAny = true;
            bool used = false;
            for (size_t other = 0; other < static_cast<size_t>(g_viewCount); ++other) {
                if (other != index && g_viewSlots[other].selected &&
                    SameViewSource(source, *g_viewSlots[other].selected))
                    used = true;
            }
            if (!used) {
                match = source;
                break;
            }
        }
        if (match) {
            slot.pendingIp.clear();
            StopPreviewSession(slot.session);
            slot.selected = match;
            StartPreviewSession(slot.session, *match);
            UpdateViewSources();
        } else if (std::chrono::steady_clock::now() >= slot.pendingDeadline) {
            const std::wstring ip = slot.pendingIp;
            slot.pendingIp.clear();
            SetViewMessage(slot, foundAny
                ? L"Every NDI source at " + ip + L" is already shown in another view."
                : L"No NDI source found at " + ip +
                  L". Check the address and network, then press Connect again.");
        } else {
            SetViewMessage(slot, L"Searching for NDI sources at " + slot.pendingIp + L"...");
        }
    }
}

void ConnectViewIp(size_t index) {
    auto& slot = g_viewSlots[index];
    const int length = GetWindowTextLengthW(slot.ip);
    std::wstring address(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(slot.ip, address.data(), length + 1);
    address.resize(static_cast<size_t>(length));
    while (!address.empty() && iswspace(address.back())) address.pop_back();
    while (!address.empty() && iswspace(address.front())) address.erase(address.begin());
    if (!IsNumericIp(address)) {
        MessageBoxW(g_viewPanel, L"Enter the numeric IPv4 or IPv6 address of the NDI device.",
                    L"View only", MB_OK | MB_ICONERROR);
        SetFocus(slot.ip);
        return;
    }
    const bool known = std::any_of(g_ipFinders.begin(), g_ipFinders.end(),
                                   [&](const auto& item) { return item.first == address; });
    if (!known) {
        NDIlib_find_instance_t finder = CreateNdiFinder(address);
        if (!finder) {
            MessageBoxW(g_viewPanel, L"Could not start NDI discovery for this address.",
                        L"View only", MB_OK | MB_ICONERROR);
            return;
        }
        g_ipFinders.emplace_back(address, finder);
    }
    StopPreviewSession(slot.session);
    slot.selected.reset();
    UpdateViewSources();
    slot.pendingIp = address;
    slot.pendingDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    ResolvePendingViewIps();
}

void DestroyIpFinders() {
    for (auto& item : g_ipFinders) NDIlib_find_destroy(item.second);
    g_ipFinders.clear();
}

bool IsAviFormatIndex(LRESULT format) {
    return format == kFormatAvi || format == kFormatLagarith;
}

std::optional<UINT32> ParseBitrate(const std::wstring& text) {
    if (text.empty()) return std::nullopt;
    if (text.size() > 4) {
        throw std::runtime_error("Enter an MP4 bitrate from 1 to 1000 Mbps, or leave it blank for Auto.");
    }
    UINT32 value = 0;
    for (wchar_t character : text) {
        if (character < L'0' || character > L'9' || value > 1000) {
            throw std::runtime_error("Enter an MP4 bitrate from 1 to 1000 Mbps, or leave it blank for Auto.");
        }
        value = value * 10 + static_cast<UINT32>(character - L'0');
    }
    if (value == 0 || value > 1000) {
        throw std::runtime_error("Enter an MP4 bitrate from 1 to 1000 Mbps, or leave it blank for Auto.");
    }
    return value * 1'000'000;
}

void UpdateCompressionControls() {
    const LRESULT format = SendMessageW(g_formatControl, CB_GETCURSEL, 0, 0);
    const bool mp4 = !IsAviFormatIndex(format);
    ShowWindow(g_bitrateLabel, mp4 ? SW_SHOW : SW_HIDE);
    ShowWindow(g_bitrateControl, mp4 ? SW_SHOW : SW_HIDE);
    ShowWindow(g_codecSettingsControl, format == kFormatLagarith ? SW_SHOW : SW_HIDE);
}

FrameRate ParseFrameRate(const std::wstring& text) {
    const char* error = "Enter a recording frame rate from 1 to 85 FPS, with up to 3 decimal places.";
    int numerator = 0;
    int denominator = 1;
    bool decimal = false;
    bool digit = false;
    for (const wchar_t character : text) {
        if (character == L'.' && !decimal && digit) {
            decimal = true;
            digit = false;
        } else {
            if (character < L'0' || character > L'9' || numerator > 85'000 ||
                (decimal && denominator == 1000)) {
                throw std::runtime_error(error);
            }
            numerator = numerator * 10 + static_cast<int>(character - L'0');
            if (decimal) denominator *= 10;
            digit = true;
        }
    }
    if (!digit || numerator < denominator || numerator > 85 * denominator) {
        throw std::runtime_error(error);
    }
    return {numerator, denominator};
}

void UpdateFrameRateControls() {
    const bool custom = SendMessageW(g_frameRateControl, CB_GETCURSEL, 0, 0) == kCustomFrameRate;
    ShowWindow(g_customFrameRateControl, custom ? SW_SHOW : SW_HIDE);
    EnableWindow(g_customFrameRateControl, custom && !g_recording);
}

std::optional<FrameRate> SelectedFrameRate() {
    switch (SendMessageW(g_frameRateControl, CB_GETCURSEL, 0, 0)) {
    case 0: return std::nullopt;
    case 1: return FrameRate{30, 1};
    case 2: return FrameRate{15, 1};
    case 3: return FrameRate{15, 2};
    case kCustomFrameRate: {
        const int length = GetWindowTextLengthW(g_customFrameRateControl);
        std::wstring text(static_cast<size_t>(length) + 1, L'\0');
        GetWindowTextW(g_customFrameRateControl, text.data(), length + 1);
        text.resize(static_cast<size_t>(length));
        return ParseFrameRate(text);
    }
    default: throw std::runtime_error("Choose a recording frame rate.");
    }
}

void ConfigureLagarith() {
    HIC codec = ICOpen(ICTYPE_VIDEO, kLagarithFourCC, ICMODE_COMPRESS);
    if (!codec) {
        MessageBoxW(g_window, L"Install the 64-bit Lagarith VFW codec to configure it.",
                    L"Lagarith unavailable", MB_OK | MB_ICONERROR);
        return;
    }
    struct CodecHandle {
        HIC value;
        ~CodecHandle() { ICClose(value); }
    } handle{codec};
    try {
        if (!g_lagarithState.empty()) {
            const LRESULT result = ICSetState(
                codec, g_lagarithState.data(), static_cast<DWORD>(g_lagarithState.size()));
            std::vector<uint8_t> restored(g_lagarithState.size());
            if (result < 0 ||
                ICGetState(codec, restored.data(), static_cast<DWORD>(restored.size())) < 0 ||
                restored != g_lagarithState) {
                throw std::runtime_error("Could not restore the Lagarith settings.");
            }
        }
        if (!ICQueryConfigure(codec)) {
            throw std::runtime_error("The Lagarith codec does not provide a settings dialog.");
        }
        const LRESULT result = ICConfigure(codec, g_window);
        if (result != ICERR_OK) {
            if (result == ICERR_ABORT) return;
            throw std::runtime_error("Could not open the Lagarith settings dialog.");
        }
        const LRESULT size = ICGetStateSize(codec);
        if (size <= 0 || size > 1024 * 1024) {
            throw std::runtime_error("Could not read the Lagarith settings size.");
        }
        std::vector<uint8_t> state(static_cast<size_t>(size));
        if (ICGetState(codec, state.data(), static_cast<DWORD>(state.size())) < 0) {
            throw std::runtime_error("Could not read the Lagarith settings.");
        }
        g_lagarithState = std::move(state);
    } catch (const std::exception& error) {
        const std::string message = error.what();
        MessageBoxW(g_window, std::wstring(message.begin(), message.end()).c_str(),
                    L"Lagarith settings", MB_OK | MB_ICONERROR);
    }
}

void UpdatePathExtension() {
    const int format = static_cast<int>(SendMessageW(g_formatControl, CB_GETCURSEL, 0, 0));
    const bool mp4 = !IsAviFormatIndex(format);
    const int length = GetWindowTextLengthW(g_pathControl);
    if (length <= 0) {
        return;
    }

    std::wstring path(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(g_pathControl, path.data(), length + 1);
    path.resize(static_cast<size_t>(length));
    const auto dot = path.find_last_of(L'.');
    const auto slash = path.find_last_of(L"\\/");
    if (dot != std::wstring::npos &&
        (slash == std::wstring::npos || dot > slash) &&
        (path.substr(dot) == L".mp4" || path.substr(dot) == L".avi")) {
        path.resize(dot);
    }
    path += mp4 ? L".mp4" : L".avi";
    SetWindowTextW(g_pathControl, path.c_str());
}

bool BrowseForOutputPath() {
    wchar_t path[MAX_PATH]{};
    GetWindowTextW(g_pathControl, path, MAX_PATH);

    const bool mp4 = !IsAviFormatIndex(SendMessageW(g_formatControl, CB_GETCURSEL, 0, 0));
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = g_window;
    dialog.lpstrFilter = L"MP4 video (*.mp4)\0*.mp4\0AVI video (*.avi)\0*.avi\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = path;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = mp4 ? L"mp4" : L"avi";
    dialog.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetSaveFileNameW(&dialog)) {
        const DWORD error = CommDlgExtendedError();
        if (error) {
            const auto message = L"Could not open the output file dialog (Windows error " +
                std::to_wstring(error) + L").";
            MessageBoxW(g_window, message.c_str(), L"Output file", MB_OK | MB_ICONERROR);
        }
        return false;
    }

    const std::wstring expectedExtension = mp4 ? L".mp4" : L".avi";
    const auto dot = std::wstring(path).find_last_of(L'.');
    const auto slash = std::wstring(path).find_last_of(L"\\/");
    if (dot == std::wstring::npos || (slash != std::wstring::npos && dot < slash)) {
        wcscat_s(path, expectedExtension.c_str());
    } else {
        wcscpy_s(path + dot, MAX_PATH - dot, expectedExtension.c_str());
    }
    SetWindowTextW(g_pathControl, path);
    return true;
}

constexpr int kOverwriteOutput = 2301;
constexpr int kNumberedOutput = 2302;
constexpr int kChooseOutput = 2303;

std::optional<DWORD> OutputPathAttributes(const std::wstring& path) {
    const DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES) return attributes;
    const DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return std::nullopt;
    throw std::runtime_error("Could not check the output path (Windows error " +
                             std::to_string(error) + ").");
}

std::wstring NumberedOutputPath(const std::wstring& path) {
    const auto dot = path.find_last_of(L'.');
    const auto slash = path.find_last_of(L"\\/");
    const bool hasExtension = dot != std::wstring::npos &&
        (slash == std::wstring::npos || dot > slash);
    const std::wstring stem = hasExtension ? path.substr(0, dot) : path;
    const std::wstring extension = hasExtension ? path.substr(dot) : L"";
    for (unsigned int suffix = 1; suffix != 0; ++suffix) {
        const auto candidate = stem + L"_" + std::to_wstring(suffix) + extension;
        if (!OutputPathAttributes(candidate)) return candidate;
    }
    throw std::runtime_error("No unused numbered output filename is available.");
}

INT_PTR CALLBACK ExistingOutputDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_INITDIALOG) {
        const auto& path = *reinterpret_cast<const std::wstring*>(lParam);
        SetWindowTextW(dialog, L"Output file already exists");
        const auto control = [&](const wchar_t* type, const wchar_t* text, DWORD style,
                                 int y, int height, int id) {
            return CreateWindowW(type, text, WS_CHILD | WS_VISIBLE | style, 15, y, 470, height,
                dialog, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
        };
        if (!control(L"STATIC", L"A recording already exists at this path:", 0, 15, 22, 0) ||
            !control(L"STATIC", path.c_str(), SS_PATHELLIPSIS, 42, 24, 0) ||
            !control(L"BUTTON", L"Overwrite existing file (replace its contents)",
                     WS_TABSTOP | BS_PUSHBUTTON, 82, 34, kOverwriteOutput) ||
            !control(L"BUTTON", L"Use numbered filename (keep existing file)",
                     WS_TABSTOP | BS_PUSHBUTTON, 124, 34, kNumberedOutput) ||
            !control(L"BUTTON", L"Choose another filename...",
                     WS_TABSTOP | BS_PUSHBUTTON, 166, 34, kChooseOutput) ||
            !control(L"BUTTON", L"Cancel",
                     WS_TABSTOP | BS_DEFPUSHBUTTON, 208, 34, IDCANCEL)) {
            EndDialog(dialog, -1);
            return TRUE;
        }
        SetWindowPos(dialog, nullptr, 0, 0, 520, 300, SWP_NOMOVE | SWP_NOZORDER);
        SendMessageW(dialog, DM_SETDEFID, IDCANCEL, 0);
        SetFocus(GetDlgItem(dialog, IDCANCEL));
        return FALSE;
    }
    if (message == WM_CLOSE) {
        EndDialog(dialog, IDCANCEL);
        return TRUE;
    }
    if (message == WM_COMMAND) {
        switch (LOWORD(wParam)) {
        case kOverwriteOutput:
        case kNumberedOutput:
        case kChooseOutput:
        case IDCANCEL:
            EndDialog(dialog, LOWORD(wParam));
            return TRUE;
        }
    }
    return FALSE;
}

INT_PTR ChooseExistingOutputAction(const std::wstring& path) {
    UsbDialogTemplate definition;
    definition.base.style |= DS_CENTER;
    const INT_PTR result = DialogBoxIndirectParamW(GetModuleHandleW(nullptr), &definition.base,
        g_window, ExistingOutputDialogProc, reinterpret_cast<LPARAM>(&path));
    if (result == -1) throw std::runtime_error("Could not open the existing-file options dialog.");
    return result;
}

void StartRecording() {
    if (g_viewOnly) return;
    if (g_drawingPrivacyMask) {
        MessageBoxW(g_window, L"Finish drawing the privacy mask or cancel mask drawing before recording.",
                    L"Privacy masks", MB_OK | MB_ICONINFORMATION);
        return;
    }
    const LRESULT selection = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<size_t>(selection) >= g_sources.size()) {
        MessageBoxW(g_window, L"Choose an available video source first.", kAppName,
                    MB_OK | MB_ICONINFORMATION);
        return;
    }

    const int pathLength = GetWindowTextLengthW(g_pathControl);
    if (pathLength <= 0) {
        MessageBoxW(g_window, L"Choose an output file first.", kAppName,
                    MB_OK | MB_ICONINFORMATION);
        return;
    }
    std::wstring path(static_cast<size_t>(pathLength) + 1, L'\0');
    GetWindowTextW(g_pathControl, path.data(), pathLength + 1);
    path.resize(static_cast<size_t>(pathLength));

    RecordingOptions options;
    options.source = g_sources[static_cast<size_t>(selection)];
    options.privacyMasks = g_privacyMasks;
    try {
        options.frameRate = SelectedFrameRate();
    } catch (const std::exception& error) {
        const std::string message = error.what();
        MessageBoxW(g_window, std::wstring(message.begin(), message.end()).c_str(),
                    L"Invalid frame rate", MB_OK | MB_ICONERROR);
        return;
    }
    options.monochrome = SendMessageW(g_colorControl, CB_GETCURSEL, 0, 0) == kColorMonochrome;
#ifdef HAVE_EPIPHAN_SDK
    if (options.source.kind == SourceKind::Epiphan) {
        std::shared_ptr<const PreviewFrame> frame;
        {
            std::lock_guard<std::mutex> lock(g_previewMutex);
            frame = g_previewFrame;
        }
        if (!frame || frame->frameRateN <= 0 ||
            std::chrono::steady_clock::now() - frame->received >= std::chrono::seconds(3)) {
            MessageBoxW(g_window, L"Wait for a stable Epiphan preview and measured capture rate before recording.",
                        L"Epiphan capture", MB_OK | MB_ICONWARNING);
            return;
        }
        wchar_t rates[160]{};
        if (frame->signalRateMilliHz > 0) {
            swprintf_s(rates, L"Input refresh: %.2f Hz; measured SDK capture: %.2f fps.\n\n",
                       frame->signalRateMilliHz / 1000.0,
                       static_cast<double>(frame->frameRateN) / frame->frameRateD);
        } else {
            swprintf_s(rates, L"Input refresh: unavailable; measured SDK capture: %.2f fps.\n\n",
                       static_cast<double>(frame->frameRateN) / frame->frameRateD);
        }
        const std::wstring warning = std::wstring(rates) +
            L"A capture rate below the input refresh can indicate missed frames. The SDK provides "
            L"no source sequence number or timestamp, so upstream loss and repeated frames cannot "
            L"be reliably detected. " +
            std::wstring(options.frameRate
                ? L"The selected recording FPS skips or repeats received frames to preserve timing; "
                  L"it cannot restore missing source frames. Continue?"
                : L"The file uses the measured preview capture rate; playback timing "
                  L"can be inaccurate if capture speed changes. Continue?");
        if (MessageBoxW(g_window, warning.c_str(),
            L"Epiphan capture limitations",
            MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) return;
    }
#endif
#ifdef HAVE_DATAPATH_SDK
    if (options.source.kind == SourceKind::Datapath) {
        std::shared_ptr<const PreviewFrame> frame;
        {
            std::lock_guard<std::mutex> lock(g_previewMutex);
            frame = g_previewFrame;
        }
        if (!frame || frame->frameRateN <= 0 || frame->signalRateMilliHz <= 0 ||
            std::chrono::steady_clock::now() - frame->received >= std::chrono::seconds(3)) {
            MessageBoxW(g_window, L"Wait for a stable Datapath preview and measured and driver-reported capture rates before recording.",
                        L"Datapath capture", MB_OK | MB_ICONWARNING);
            return;
        }
        if (frame->interlaced) {
            MessageBoxW(g_window, L"Interlaced Datapath input is not supported for recording.",
                        L"Datapath capture", MB_OK | MB_ICONWARNING);
            return;
        }
        wchar_t rate[100]{};
        swprintf_s(rate, L"Measured capture: %.2f fps; driver reports: %.2f fps.\n\n",
                   static_cast<double>(frame->frameRateN) / frame->frameRateD,
                   frame->signalRateMilliHz / 1000.0);
        const std::wstring warning = std::wstring(rate) +
            L"Timestamp gaps can indicate lost frames, but not every upstream loss is detectable. "
            L"Interlaced input is not supported. " +
            std::wstring(options.frameRate
                ? L"The selected recording FPS skips or repeats received frames to preserve timing; "
                  L"it cannot restore missing source frames. Continue?"
                : L"This file uses a fixed rate measured from preview; playback timing can be inaccurate "
                  L"if capture speed changes. Continue?");
        if (MessageBoxW(g_window, warning.c_str(), L"Datapath capture limitations",
                        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) return;
    }
#endif
    if (options.source.kind == SourceKind::Vfw &&
        MessageBoxW(g_window,
                    L"VFW recording uses a preview interval of 33 ms and can miss "
                    L"frames, especially from a 60 fps source. Continue?",
                    L"VFW frame loss warning",
                    MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) {
        return;
    }
    const LRESULT selectedFormat = SendMessageW(g_formatControl, CB_GETCURSEL, 0, 0);
    options.format = selectedFormat == kFormatAvi ? RecordingFormat::UncompressedAvi :
        (selectedFormat == kFormatHevc ? RecordingFormat::HevcMp4 :
        (selectedFormat == kFormatLagarith ? RecordingFormat::LagarithAvi : RecordingFormat::H264Mp4));
    if (selectedFormat == kFormatLagarith) {
        HIC codec = ICOpen(ICTYPE_VIDEO, kLagarithFourCC, ICMODE_COMPRESS);
        if (!codec) {
            MessageBoxW(g_window, L"Install the 64-bit Lagarith VFW codec before recording.",
                        L"Lagarith unavailable", MB_OK | MB_ICONERROR);
            return;
        }
        ICClose(codec);
        options.codecState = g_lagarithState;
    } else if (!IsAviFormatIndex(selectedFormat)) {
        const int length = GetWindowTextLengthW(g_bitrateControl);
        std::wstring bitrate(static_cast<size_t>(length) + 1, L'\0');
        GetWindowTextW(g_bitrateControl, bitrate.data(), length + 1);
        bitrate.resize(static_cast<size_t>(length));
        try {
            options.bitrate = ParseBitrate(bitrate);
        } catch (const std::exception& error) {
            const std::string message = error.what();
            MessageBoxW(g_window, std::wstring(message.begin(), message.end()).c_str(),
                        L"Invalid bitrate", MB_OK | MB_ICONERROR);
            return;
        }
    }
    const std::wstring extension = IsAviFormatIndex(selectedFormat) ? L".avi" : L".mp4";
    const auto dot = path.find_last_of(L'.');
    const auto slash = path.find_last_of(L"\\/");
    if (dot == std::wstring::npos || (slash != std::wstring::npos && dot < slash)) {
        path += extension;
    } else {
        path.resize(dot);
        path += extension;
    }
    options.path = std::move(path);
    SetWindowTextW(g_pathControl, options.path.c_str());

    try {
        while (const auto attributes = OutputPathAttributes(options.path)) {
            if (*attributes & FILE_ATTRIBUTE_DIRECTORY) {
                throw std::runtime_error("The output path is a directory.");
            }
            const INT_PTR action = ChooseExistingOutputAction(options.path);
            if (action == IDCANCEL) return;
            if (action == kOverwriteOutput) break;
            if (action == kNumberedOutput) {
                options.path = NumberedOutputPath(options.path);
                break;
            }
            if (action != kChooseOutput) {
                throw std::runtime_error("No valid existing-file action was selected.");
            }
            if (!BrowseForOutputPath()) return;
            const int length = GetWindowTextLengthW(g_pathControl);
            options.path.resize(static_cast<size_t>(length) + 1);
            GetWindowTextW(g_pathControl, options.path.data(), length + 1);
            options.path.resize(static_cast<size_t>(length));
        }
        SetWindowTextW(g_pathControl, options.path.c_str());
    } catch (const std::exception& error) {
        const std::string message = error.what();
        MessageBoxW(g_window, std::wstring(message.begin(), message.end()).c_str(),
                    L"Output file", MB_OK | MB_ICONERROR);
        return;
    }

    if (g_worker.joinable()) {
        g_worker.join();
    }
    g_stopRequested.store(false);
    g_requestedStopTime.store(0);
    g_recordStart.store(0);
    g_recordEnd.store(0);
    UpdateRecordingTimer();
    g_recording = true;
    EnableRecordingControls(false);
    SetWindowTextW(g_recordControl, L"Stop recording");
    SetStatus(options.source.kind == SourceKind::Ndi
        ? L"Connecting to NDI source..." : L"Waiting for camera video...");
    try {
        if (options.source.kind == SourceKind::Ndi) {
            g_worker = std::thread(Record, std::move(options));
        } else {
            {
                std::lock_guard<std::mutex> lock(g_camera.mutex);
                g_camera.frames.clear();
                g_camera.bytes = 0;
                g_camera.stopped.reset();
                g_camera.recording = g_camera.error.empty();
            }
            g_worker = std::thread(RecordCamera, std::move(options));
        }
    } catch (const std::exception& error) {
        {
            std::lock_guard<std::mutex> lock(g_camera.mutex);
            g_camera.recording = false;
        }
        g_recording = false;
        UpdateRecordingTimer();
        EnableRecordingControls(true);
        SetWindowTextW(g_recordControl, L"Start recording");
        const std::string message = error.what();
        const std::wstring wide(message.begin(), message.end());
        SetStatus(L"Could not start recording: " + wide);
    }
}

void StopRecording() {
    if (!g_recording) {
        return;
    }
    g_requestedStopTime.store(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    g_stopRequested.store(true);
    if (g_recordStart.load()) g_recordEnd.store(GetTickCount64());
    StopQueue(g_camera);
    SetStatus(L"Finalizing buffered frames...");
    EnableWindow(g_recordControl, FALSE);
    if (g_closing.load() && g_worker.joinable()) {
        g_worker.join();
    }
}

void CreateControls(HWND window) {
    CreateWindowW(L"STATIC", L"Video source:", WS_CHILD | WS_VISIBLE,
                  20, 22, 100, 24, window, nullptr, nullptr, nullptr);
    g_sourceControl = CreateWindowW(
        WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        125, 18, 455, 240, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSourceControl)), nullptr, nullptr);
    g_usbSettingsControl = CreateWindowW(
        L"BUTTON", L"USB settings...",
        WS_CHILD | (kNdiOnly ? 0 : WS_VISIBLE) | WS_TABSTOP | BS_PUSHBUTTON | WS_DISABLED,
        475, 16, 105, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUsbSettingsControl)), nullptr, nullptr);
    g_ndiSettingsControl = CreateWindowW(
        L"BUTTON", L"NDI settings...", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        365, 16, 105, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kNdiSettingsControl)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"Refresh", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                  475, 52, 105, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRefreshControl)), nullptr, nullptr);
    CreateWindowW(L"STATIC", L"NDI device IP:", WS_CHILD | WS_VISIBLE,
                  20, 58, 105, 24, window, nullptr, nullptr, nullptr);
    g_discoveryIpControl = CreateWindowW(
        L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
        125, 54, 230, 26, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDiscoveryIpControl)), nullptr, nullptr);
    SendMessageW(g_discoveryIpControl, EM_SETLIMITTEXT, 45, 0);
    SetWindowTextW(g_discoveryIpControl, g_ndiSettings.deviceIp.c_str());
    CreateWindowW(L"BUTTON", L"Apply IP", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                  360, 52, 105, 30, window,
                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApplyIpControl)), nullptr, nullptr);
    CreateWindowW(L"STATIC", L"Save to:", WS_CHILD | WS_VISIBLE,
                  20, 100, 100, 24, window, nullptr, nullptr, nullptr);
    g_pathControl = CreateWindowW(
        L"EDIT", L"ndi-recording.mp4",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
        125, 96, 345, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPathControl)), nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"Browse...", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                  475, 94, 105, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBrowseControl)), nullptr, nullptr);

    CreateWindowW(L"STATIC", L"Format:", WS_CHILD | WS_VISIBLE,
                  20, 148, 100, 24, window, nullptr, nullptr, nullptr);
    g_formatControl = CreateWindowW(
        WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        125, 144, 180, 120, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFormatControl)), nullptr, nullptr);
    SendMessageW(g_formatControl, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"MP4 (H.264)"));
    SendMessageW(g_formatControl, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"AVI (uncompressed)"));
    SendMessageW(g_formatControl, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"MP4 (H.265/HEVC)"));
    SendMessageW(g_formatControl, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"AVI (Lagarith)"));
    SendMessageW(g_formatControl, CB_SETCURSEL, 0, 0);
    g_bitrateLabel = CreateWindowW(L"STATIC", L"Bitrate (Mbps):", WS_CHILD | WS_VISIBLE,
                                  320, 148, 105, 24, window, nullptr, nullptr, nullptr);
    g_bitrateControl = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
        430, 144, 90, 26, window, nullptr, nullptr, nullptr);
    SendMessageW(g_bitrateControl, EM_SETCUEBANNER, FALSE, reinterpret_cast<LPARAM>(L"Auto"));
    g_codecSettingsControl = CreateWindowW(L"BUTTON", L"Codec settings...",
        WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 320, 142, 160, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCodecSettingsControl)), nullptr, nullptr);
    UpdateCompressionControls();

    CreateWindowW(L"STATIC", L"Record FPS:", WS_CHILD | WS_VISIBLE,
                  20, 193, 100, 24, window, nullptr, nullptr, nullptr);
    g_frameRateControl = CreateWindowW(
        WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        125, 189, 180, 160, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFrameRateControl)), nullptr, nullptr);
    for (const wchar_t* choice : {L"Same as source", L"30 FPS", L"15 FPS", L"7.5 FPS", L"Custom FPS"}) {
        SendMessageW(g_frameRateControl, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(choice));
    }
    SendMessageW(g_frameRateControl, CB_SETCURSEL, 0, 0);
    g_customFrameRateControl = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
        320, 189, 200, 26, window, nullptr, nullptr, nullptr);
    SendMessageW(g_customFrameRateControl, EM_SETLIMITTEXT, 16, 0);
    SendMessageW(g_customFrameRateControl, EM_SETCUEBANNER, FALSE,
                 reinterpret_cast<LPARAM>(L"1-85 FPS (e.g. 29.97)"));
    UpdateFrameRateControls();

    CreateWindowW(L"STATIC", L"Color:", WS_CHILD | WS_VISIBLE,
                  20, 238, 100, 24, window, nullptr, nullptr, nullptr);
    g_colorControl = CreateWindowW(
        WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        125, 234, 260, 120, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kColorControl)), nullptr, nullptr);
    SendMessageW(g_colorControl, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Source color"));
    SendMessageW(g_colorControl, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(L"Monochrome 8-bit (green channel)"));
    SendMessageW(g_colorControl, CB_SETCURSEL, 0, 0);
    CreateWindowW(L"BUTTON", L"User guide", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                  0, 232, 105, 30, window,
                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(kHelpControl)), nullptr, nullptr);

    g_recordControl = CreateWindowW(
        L"BUTTON", L"Start recording", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        125, 286, 180, 38, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRecordControl)), nullptr, nullptr);
    g_viewButton = CreateWindowW(
        L"BUTTON", L"View only", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        20, 286, 95, 38, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewOnlyControl)), nullptr, nullptr);
    g_timerControl = CreateWindowW(
        L"STATIC", L"Recording time: 00:00:00", WS_CHILD | WS_VISIBLE,
        325, 297, 240, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTimerControl)), nullptr, nullptr);
    g_privacyMaskControl = CreateWindowW(
        L"BUTTON", L"Draw mask (0/2)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        20, 346, 145, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPrivacyMaskControl)), nullptr, nullptr);
    g_clearPrivacyMasksControl = CreateWindowW(
        L"BUTTON", L"Clear masks", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | WS_DISABLED,
        175, 346, 105, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kClearPrivacyMasksControl)), nullptr, nullptr);
    g_statusControl = CreateWindowW(
        L"STATIC", L"Searching for NDI sources...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 380, 560, 34, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStatusControl)), nullptr, nullptr);
    g_previewControl = CreateWindowExW(
        WS_EX_CLIENTEDGE, kPreviewClass, L"", WS_CHILD | WS_VISIBLE,
        20, 424, 560, 226, window, nullptr, GetModuleHandleW(nullptr), nullptr);
    g_streamInfoControl = CreateWindowW(
        L"STATIC", L"Video: unavailable", WS_CHILD | WS_VISIBLE,
        20, 0, 560, 20, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStreamInfoControl)), nullptr, nullptr);
    g_addressInfoControl = CreateWindowW(
        L"STATIC", L"Source IP: unavailable", WS_CHILD | WS_VISIBLE,
        20, 0, 560, 20, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAddressInfoControl)), nullptr, nullptr);
    g_performanceInfoControl = CreateWindowW(
        L"STATIC", L"System: CPU N/A | Memory N/A | Disk active N/A | GPU busiest engine N/A",
        WS_CHILD | WS_VISIBLE, 20, 0, 560, 20, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPerformanceInfoControl)), nullptr, nullptr);
}

void LayoutViewOnly(HWND panel) {
    RECT client{};
    GetClientRect(panel, &client);
    const int width = static_cast<int>(client.right);
    const int height = static_cast<int>(client.bottom);
    MoveWindow(GetDlgItem(panel, kViewRefreshControl), std::max(width - 230, 215), 12, 100, 30, TRUE);
    MoveWindow(GetDlgItem(panel, kViewBackControl), std::max(width - 120, 320), 12, 105, 30, TRUE);
    MoveWindow(GetDlgItem(panel, kViewHelpControl), std::max(width - 340, 210), 12, 100, 30, TRUE);
    constexpr int margin = 15;
    constexpr int gap = 10;
    constexpr int top = 55;
    constexpr int choiceHeight = 26;
    constexpr int ipRowHeight = 26;
    constexpr int labelWidth = 24;
    constexpr int connectWidth = 80;
    const int columnWidth = std::max((width - 2 * margin - gap) / 2, 1);
    const int rows = g_viewCount == 4 ? 2 : 1;
    const int headerHeight = choiceHeight + 4 + ipRowHeight + 4;
    const int rowHeight = std::max((height - top - margin - (rows - 1) * gap) / rows, headerHeight + 1);
    for (size_t i = 0; i < g_viewSlots.size(); ++i) {
        const bool visible = i < static_cast<size_t>(g_viewCount);
        auto& slot = g_viewSlots[i];
        for (HWND control : {slot.choice, slot.ipLabel, slot.ip, slot.connect, slot.preview})
            ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
        if (!visible) continue;
        const int x = margin + static_cast<int>(i % 2) * (columnWidth + gap);
        const int y = top + static_cast<int>(i / 2) * (rowHeight + gap);
        const int ipY = y + choiceHeight + 4;
        MoveWindow(slot.choice, x, y, columnWidth, 240, TRUE);
        MoveWindow(slot.ipLabel, x, ipY + 4, labelWidth, 20, TRUE);
        MoveWindow(slot.ip, x + labelWidth, ipY,
                   std::max(columnWidth - labelWidth - connectWidth - 4, 1), ipRowHeight, TRUE);
        MoveWindow(slot.connect, x + std::max(columnWidth - connectWidth, labelWidth), ipY,
                   connectWidth, ipRowHeight, TRUE);
        MoveWindow(slot.preview, x, y + headerHeight, columnWidth,
                   std::max(rowHeight - headerHeight, 1), TRUE);
    }
}

void EnterViewOnly() {
    if (g_recording || g_viewOnly) return;
    std::optional<NdiSource> selected;
    const LRESULT index = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
    if (index != CB_ERR && static_cast<size_t>(index) < g_sources.size())
        selected = g_sources[static_cast<size_t>(index)];
    StopPreview();
    g_viewOnly = true;
    g_viewSlots[0].selected = selected;
    UpdateViewSources();
    if (selected) StartPreviewSession(g_viewSlots[0].session, *selected);
    for (HWND control = GetWindow(g_window, GW_CHILD); control;
         control = GetWindow(control, GW_HWNDNEXT)) {
        if (control != g_viewPanel && (GetWindowLongW(control, GWL_STYLE) & WS_VISIBLE)) {
            g_hiddenRecordingControls.push_back(control);
            ShowWindow(control, SW_HIDE);
        }
    }
    ShowWindow(g_viewPanel, SW_SHOW);
    g_viewFastTimer = SetTimer(g_window, 3, 33, nullptr) != 0;
    if (!g_viewFastTimer)
        MessageBoxW(g_viewPanel, L"Could not start the fast preview timer. Views will update more slowly.",
                    L"View only", MB_OK | MB_ICONWARNING);
    SetFocus(g_viewSlots[0].choice);
}

void ExitViewOnly() {
    if (!g_viewOnly) return;
    KillTimer(g_window, 3);
    g_viewFastTimer = false;
    StopViewSessions();
    for (auto& slot : g_viewSlots) {
        slot.selected.reset();
        slot.pendingIp.clear();
    }
    ShowWindow(g_viewPanel, SW_HIDE);
    for (HWND control : g_hiddenRecordingControls) ShowWindow(control, SW_SHOW);
    g_hiddenRecordingControls.clear();
    g_viewOnly = false;
    RedrawWindow(g_window, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    UpdatePreviewSource(true);
    SetFocus(g_viewButton);
}

LRESULT CALLBACK ViewOnlyProcedure(HWND panel, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        CreateWindowW(L"STATIC", L"View only:", WS_CHILD | WS_VISIBLE, 15, 18, 75, 22,
                      panel, nullptr, nullptr, nullptr);
        g_viewLayout = CreateWindowW(WC_COMBOBOXW, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 90, 14, 110, 100,
            panel, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewLayoutControl)), nullptr, nullptr);
        SendMessageW(g_viewLayout, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"2 views"));
        SendMessageW(g_viewLayout, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"4 views"));
        SendMessageW(g_viewLayout, CB_SETCURSEL, g_viewCount == 4 ? 1 : 0, 0);
        CreateWindowW(L"BUTTON", L"Refresh", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                      0, 0, 100, 30, panel,
                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewRefreshControl)), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Back to record", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                      0, 0, 105, 30, panel,
                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewBackControl)), nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"User guide", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                      0, 0, 100, 30, panel,
                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewHelpControl)), nullptr, nullptr);
        for (size_t i = 0; i < g_viewSlots.size(); ++i) {
            auto& slot = g_viewSlots[i];
            slot.choice = CreateWindowW(WC_COMBOBOXW, L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                0, 0, 100, 240, panel,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewSourceFirst + i)), nullptr, nullptr);
            slot.preview = CreateWindowExW(WS_EX_CLIENTEDGE, kPreviewClass, L"", WS_CHILD | WS_VISIBLE,
                0, 0, 100, 100, panel, nullptr, GetModuleHandleW(nullptr), &slot.session);
            slot.ipLabel = CreateWindowW(L"STATIC", L"IP:", WS_CHILD | WS_VISIBLE,
                0, 0, 24, 20, panel, nullptr, nullptr, nullptr);
            slot.ip = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 100, 26, panel,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewIpFirst + i)), nullptr, nullptr);
            slot.connect = CreateWindowW(L"BUTTON", L"Connect",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 80, 26, panel,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kViewConnectFirst + i)), nullptr, nullptr);
            if (!slot.choice || !slot.preview || !slot.ipLabel || !slot.ip || !slot.connect) return -1;
            SendMessageW(slot.ip, EM_SETCUEBANNER, FALSE,
                         reinterpret_cast<LPARAM>(L"NDI device IP, e.g. 172.22.2.2"));
        }
        LayoutViewOnly(panel);
        return 0;
    }
    case WM_SIZE:
        LayoutViewOnly(panel);
        return 0;
    case WM_COMMAND: {
        const int id = LOWORD(wParam);
        if (id == kViewBackControl) {
            ExitViewOnly();
            return 0;
        }
        if (id == kViewHelpControl) {
            OpenUserGuide();
            return 0;
        }
        if (id == kViewLayoutControl && HIWORD(wParam) == CBN_SELCHANGE) {
            const int count = SendMessageW(g_viewLayout, CB_GETCURSEL, 0, 0) == 1 ? 4 : 2;
            if (count < g_viewCount) {
                for (int i = count; i < g_viewCount; ++i) {
                    StopPreviewSession(g_viewSlots[static_cast<size_t>(i)].session);
                    g_viewSlots[static_cast<size_t>(i)].selected.reset();
                    g_viewSlots[static_cast<size_t>(i)].pendingIp.clear();
                }
                UpdateViewSources();
            }
            g_viewCount = count;
            LayoutViewOnly(panel);
            return 0;
        }
        if (id == kViewRefreshControl) {
            StopViewSessions();
            NDIlib_find_wait_for_sources(g_finder, 250);
            RefreshCameraSources();
            RefreshSources();
            UpdateViewSources();
            for (int i = 0; i < g_viewCount; ++i) {
                auto& slot = g_viewSlots[static_cast<size_t>(i)];
                if (slot.selected && !slot.session.worker.joinable())
                    StartPreviewSession(slot.session, *slot.selected);
            }
            return 0;
        }
        if (id >= kViewSourceFirst && id < kViewSourceFirst + static_cast<int>(g_viewSlots.size()) &&
            HIWORD(wParam) == CBN_SELCHANGE) {
            SelectViewSource(static_cast<size_t>(id - kViewSourceFirst));
            return 0;
        }
        if (id >= kViewConnectFirst && id < kViewConnectFirst + static_cast<int>(g_viewSlots.size())) {
            ConnectViewIp(static_cast<size_t>(id - kViewConnectFirst));
            return 0;
        }
        if (id == IDOK) {
            // Enter in a view's IP field connects that view.
            const HWND focus = GetFocus();
            for (size_t i = 0; i < static_cast<size_t>(g_viewCount); ++i) {
                if (focus == g_viewSlots[i].ip) ConnectViewIp(i);
            }
            return 0;
        }
        break;
    }
    }
    return DefWindowProcW(panel, message, wParam, lParam);
}

void LayoutControls(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    const int width = std::max(client.right - 20, 251L);
    MoveWindow(g_sourceControl, 125, 18, width - (kNdiOnly ? 240 : 350), 240, TRUE);
    MoveWindow(g_usbSettingsControl, width - 105, 16, 105, 30, TRUE);
    MoveWindow(g_ndiSettingsControl, width - (kNdiOnly ? 105 : 215), 16, 105, 30, TRUE);
    MoveWindow(GetDlgItem(window, kRefreshControl), width - 105, 52, 105, 30, TRUE);
    MoveWindow(GetDlgItem(window, kApplyIpControl), width - 215, 52, 105, 30, TRUE);
    MoveWindow(g_discoveryIpControl, 125, 54, std::max(width - 345, 1), 26, TRUE);
    MoveWindow(g_pathControl, 125, 96, width - 250, 26, TRUE);
    MoveWindow(GetDlgItem(window, kBrowseControl), width - 105, 94, 105, 30, TRUE);
    MoveWindow(GetDlgItem(window, kHelpControl), width - 105, 232, 105, 30, TRUE);
    const int infoTop = std::max(static_cast<int>(client.bottom) - 78, 391);
    MoveWindow(g_privacyMaskControl, 20, 346, 145, 30, TRUE);
    MoveWindow(g_clearPrivacyMasksControl, 175, 346, 105, 30, TRUE);
    MoveWindow(g_statusControl, 20, 380, width - 20, 34, TRUE);
    MoveWindow(g_previewControl, 20, 424, width - 20, std::max(infoTop - 432, 1), TRUE);
    MoveWindow(g_streamInfoControl, 20, infoTop, width - 20, 20, TRUE);
    MoveWindow(g_addressInfoControl, 20, infoTop + 22, width - 20, 20, TRUE);
    MoveWindow(g_performanceInfoControl, 20, infoTop + 44, width - 20, 20, TRUE);
    if (g_viewPanel) MoveWindow(g_viewPanel, 0, 0, client.right, client.bottom, TRUE);
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        g_window = window;
        CreateControls(window);
        g_viewPanel = CreateWindowW(kViewClass, L"", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                    0, 0, 1, 1, window, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!g_viewPanel) return -1;
        LayoutControls(window);
        g_performanceMonitor = std::make_unique<PerformanceMonitor>();
        RefreshCameraSources();
        SetTimer(window, 1, 1500, nullptr);
        SetTimer(window, 2, 250, nullptr);
        RefreshSources();
        return 0;
    case WM_GETMINMAXINFO: {
        auto* limits = reinterpret_cast<MINMAXINFO*>(lParam);
        limits->ptMinTrackSize.x = 620;
        limits->ptMinTrackSize.y = 740;
        return 0;
    }
    case WM_SIZE:
        LayoutControls(window);
        return 0;
    case WM_TIMER:
        if (wParam == 1) {
            RefreshSources();
            if (g_viewOnly) ResolvePendingViewIps();
            if (g_performanceMonitor) {
                SetWindowTextW(g_performanceInfoControl, g_performanceMonitor->Sample().c_str());
            }
        } else if (wParam == 2) {
            UpdateRecordingTimer();
            UpdateStreamInformation();
            if (g_viewOnly && !g_viewFastTimer) {
                for (int i = 0; i < g_viewCount; ++i)
                    InvalidateRect(g_viewSlots[static_cast<size_t>(i)].preview, nullptr, FALSE);
            } else if (!g_viewOnly) {
                InvalidateRect(g_previewControl, nullptr, FALSE);
            }
        } else if (wParam == 3 && g_viewOnly) {
            for (int i = 0; i < g_viewCount; ++i)
                InvalidateRect(g_viewSlots[static_cast<size_t>(i)].preview, nullptr, FALSE);
        }
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kRefreshControl:
            NDIlib_find_wait_for_sources(g_finder, 250);
            if (g_previewKind != SourceKind::Ndi) StopPreview();
            RefreshCameraSources();
            RefreshSources();
            {
                bool failed = false;
                {
                    std::lock_guard<std::mutex> lock(g_previewMutex);
                    failed = !g_previewError.empty();
                }
                if (failed || !g_previewWorker.joinable()) {
                    UpdatePreviewSource(true);
                }
            }
            return 0;
        case kApplyIpControl:
            ApplyDiscoveryIp();
            return 0;
        case kNdiSettingsControl:
            OpenNdiDeviceWebPage();
            return 0;
        case kBrowseControl:
            BrowseForOutputPath();
            return 0;
        case kFormatControl:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                UpdatePathExtension();
                UpdateCompressionControls();
            }
            return 0;
        case kCodecSettingsControl:
            ConfigureLagarith();
            return 0;
        case kFrameRateControl:
            if (HIWORD(wParam) == CBN_SELCHANGE) UpdateFrameRateControls();
            return 0;
        case kColorControl:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                g_monochromePreview =
                    SendMessageW(g_colorControl, CB_GETCURSEL, 0, 0) == kColorMonochrome;
                InvalidateRect(g_previewControl, nullptr, FALSE);
            }
            return 0;
        case kUsbSettingsControl:
            ConfigureUsbDevice();
            return 0;
        case kPrivacyMaskControl:
            if (g_drawingPrivacyMask) {
                g_drawingPrivacyMask = false;
                g_dragPrivacyMask.reset();
                if (GetCapture() == g_previewControl) ReleaseCapture();
                SetStatus(L"Mask drawing canceled.");
            } else if (g_privacyMasks.size() < kMaxPrivacyMasks) {
                const LRESULT selection = SendMessageW(g_sourceControl, CB_GETCURSEL, 0, 0);
                if (selection == CB_ERR || static_cast<size_t>(selection) >= g_sources.size()) {
                    MessageBoxW(g_window, L"Choose a video source before drawing privacy masks.",
                                L"Privacy masks", MB_OK | MB_ICONINFORMATION);
                    return 0;
                }
                g_privacyMaskSource = g_sources[static_cast<size_t>(selection)];
                g_drawingPrivacyMask = true;
                SetStatus(L"Drag a rectangle over a private area in the preview. Up to two masks.");
            }
            UpdatePrivacyMaskControls();
            InvalidateRect(g_previewControl, nullptr, FALSE);
            return 0;
        case kClearPrivacyMasksControl:
            ClearPrivacyMasks();
            SetStatus(L"Privacy masks cleared.");
            return 0;
        case kViewOnlyControl:
            EnterViewOnly();
            return 0;
        case kHelpControl:
            OpenUserGuide();
            return 0;
        case kSourceControl:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                UpdatePreviewSource();
            }
            return 0;
        case kRecordControl:
            if (g_recording) {
                StopRecording();
            } else {
                StartRecording();
            }
            return 0;
        }
        break;
    case kStatusMessage: {
        std::unique_ptr<std::wstring> status(reinterpret_cast<std::wstring*>(lParam));
        if (*status != L"Recording video..." || !g_stopRequested.load()) {
            SetStatus(*status);
        }
        if (g_viewOnly && status->find(L"Datapath shutdown failed:") == 0)
            MessageBoxW(g_viewPanel, status->c_str(), L"View only", MB_OK | MB_ICONERROR);
        if (status->find(L"Recording failed:") == 0 ||
            *status == L"Recording failed with an unexpected error." ||
            *status == L"Recording saved." ||
            *status == L"Stopped before video arrived; no file was created.") {
            g_recording = false;
            UpdateRecordingTimer();
            EnableRecordingControls(true);
            EnableWindow(g_recordControl, TRUE);
            SetWindowTextW(g_recordControl, L"Start recording");
        }
        return 0;
    }
    case WM_CLOSE:
        g_closing.store(true);
        StopRecording();
        StopViewSessions();
        StopPreview();
        {
            MSG pending{};
            while (PeekMessageW(&pending, window, kStatusMessage, kStatusMessage, PM_REMOVE)) {
                delete reinterpret_cast<std::wstring*>(pending.lParam);
            }
        }
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        KillTimer(window, 1);
        KillTimer(window, 2);
        KillTimer(window, 3);
        g_performanceMonitor.reset();
        StopViewSessions();
        StopPreview();
        if (g_worker.joinable()) {
            g_stopRequested.store(true);
            g_worker.join();
        }
        DestroyIpFinders();
        if (g_finder) {
            NDIlib_find_destroy(g_finder);
            g_finder = nullptr;
        }
        NDIlib_destroy();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);

    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(comResult)) {
        MessageBoxW(nullptr, HResultMessage(L"Initializing COM", comResult).c_str(),
                    kAppName, MB_OK | MB_ICONERROR);
        return 1;
    }

#ifdef HAVE_EPIPHAN_SDK
    FrmGrab_Init();
    struct EpiphanLibrary { ~EpiphanLibrary() { FrmGrab_Deinit(); } } epiphanLibrary;
#endif
    if (!NDIlib_initialize()) {
        MessageBoxW(nullptr, L"NDI initialization failed. Install the NDI runtime and retry.",
                    kAppName, MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }
    LoadAppSettings(AppSettingsPath(), g_ndiSettings);
    g_finder = CreateNdiFinder(g_ndiSettings.deviceIp);
    if (!g_finder) {
        MessageBoxW(nullptr, L"Could not start NDI source discovery.",
                    kAppName, MB_OK | MB_ICONERROR);
        NDIlib_destroy();
        CoUninitialize();
        return 1;
    }

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&windowClass)) {
        MessageBoxW(nullptr, L"Could not register the application window.",
                    kAppName, MB_OK | MB_ICONERROR);
        NDIlib_find_destroy(g_finder);
        NDIlib_destroy();
        CoUninitialize();
        return 1;
    }

    WNDCLASSEXW previewClass{};
    previewClass.cbSize = sizeof(previewClass);
    previewClass.lpfnWndProc = PreviewProcedure;
    previewClass.hInstance = instance;
    previewClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    previewClass.lpszClassName = kPreviewClass;
    if (!RegisterClassExW(&previewClass)) {
        MessageBoxW(nullptr, L"Could not register the preview window.",
                    kAppName, MB_OK | MB_ICONERROR);
        NDIlib_find_destroy(g_finder);
        NDIlib_destroy();
        CoUninitialize();
        return 1;
    }

    WNDCLASSEXW viewClass{};
    viewClass.cbSize = sizeof(viewClass);
    viewClass.lpfnWndProc = ViewOnlyProcedure;
    viewClass.hInstance = instance;
    viewClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    viewClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    viewClass.lpszClassName = kViewClass;
    if (!RegisterClassExW(&viewClass)) {
        MessageBoxW(nullptr, L"Could not register the view-only window.",
                    kAppName, MB_OK | MB_ICONERROR);
        NDIlib_find_destroy(g_finder);
        NDIlib_destroy();
        CoUninitialize();
        return 1;
    }

    HWND window = CreateWindowExW(
        0, kWindowClass, kWindowTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 795, nullptr, nullptr, instance, nullptr);
    if (!window) {
        MessageBoxW(nullptr, L"Could not create the application window.",
                    kAppName, MB_OK | MB_ICONERROR);
        NDIlib_find_destroy(g_finder);
        NDIlib_destroy();
        CoUninitialize();
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (message.message == WM_KEYDOWN && message.wParam == VK_F1 &&
            (message.hwnd == window || IsChild(window, message.hwnd))) {
            OpenUserGuide();
            continue;
        }
        if (g_viewOnly && IsDialogMessageW(g_viewPanel, &message)) continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    CoUninitialize();
    return static_cast<int>(message.wParam);
}
