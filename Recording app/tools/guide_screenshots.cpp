// Renders the user-guide screenshots from the application's own window code.
// Usage: guide-screenshots-<edition> <sample-frame.png> <output-directory>
// Writes main-screen.png, masking.png, settings.png, recording.png,
// view-only-2.png and view-only-4.png.
#include "../src/main.cpp"

#include <dwmapi.h>
#include <iostream>

using std::max;
using std::min;
#include <gdiplus.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

namespace {

constexpr wchar_t kShotClass[] = L"GuideScreenshotWindow";
bool g_shotReady = false;

LRESULT CALLBACK ShotProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_SIZE && g_shotReady) {
        LayoutControls(window);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

void Pump(int milliseconds) {
    const ULONGLONG end = GetTickCount64() + milliseconds;
    MSG message{};
    while (GetTickCount64() < end) {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        Sleep(10);
    }
}

CLSID PngEncoder() {
    UINT count = 0, bytes = 0;
    Gdiplus::GetImageEncodersSize(&count, &bytes);
    std::vector<uint8_t> buffer(bytes);
    auto* encoders = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    Gdiplus::GetImageEncoders(count, bytes, encoders);
    for (UINT i = 0; i < count; ++i) {
        if (std::wstring(encoders[i].MimeType) == L"image/png") return encoders[i].Clsid;
    }
    throw std::runtime_error("The PNG encoder is unavailable.");
}

std::shared_ptr<PreviewFrame> LoadFrame(const std::wstring& path) {
    Gdiplus::Bitmap bitmap(path.c_str());
    if (bitmap.GetLastStatus() != Gdiplus::Ok) throw std::runtime_error("Could not open the sample frame.");
    const int width = static_cast<int>(bitmap.GetWidth());
    const int height = static_cast<int>(bitmap.GetHeight());
    Gdiplus::BitmapData data{};
    const Gdiplus::Rect area(0, 0, width, height);
    if (bitmap.LockBits(&area, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok) {
        throw std::runtime_error("Could not read the sample frame.");
    }
    auto frame = std::make_shared<PreviewFrame>();
    frame->width = width;
    frame->height = height;
    frame->sourceWidth = 1920;
    frame->sourceHeight = 1080;
    frame->frameRateN = 60;
    frame->frameRateD = 1;
    frame->pixels.resize(static_cast<size_t>(width) * height * 4);
    for (int y = 0; y < height; ++y) {
        std::copy_n(static_cast<const uint8_t*>(data.Scan0) + static_cast<ptrdiff_t>(y) * data.Stride,
                    static_cast<size_t>(width) * 4,
                    frame->pixels.data() + static_cast<size_t>(y) * width * 4);
    }
    bitmap.UnlockBits(&data);
    return frame;
}

void ShowFrame(const std::shared_ptr<PreviewFrame>& source) {
    auto frame = std::make_shared<PreviewFrame>(*source);
    frame->received = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(g_previewMutex);
        g_previewFrame = frame;
        g_previewError.clear();
    }
    UpdateStreamInformation();
    SetWindowTextW(g_performanceInfoControl,
                   L"System: CPU 22% | Memory 35% | Disk active 1% | GPU busiest engine 2%");
    InvalidateRect(g_previewControl, nullptr, FALSE);
}

void SaveWindow(HWND window, const std::wstring& path, const CLSID& png) {
    RECT outer{}, visible{};
    GetWindowRect(window, &outer);
    if (FAILED(DwmGetWindowAttribute(window, DWMWA_EXTENDED_FRAME_BOUNDS, &visible, sizeof(visible)))) {
        visible = outer;
    }
    const int outerWidth = outer.right - outer.left;
    const int outerHeight = outer.bottom - outer.top;
    HDC screen = GetDC(nullptr);
    HDC memory = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateCompatibleBitmap(screen, outerWidth, outerHeight);
    HGDIOBJ previous = SelectObject(memory, bitmap);
    const BOOL printed = PrintWindow(window, memory, PW_RENDERFULLCONTENT);
    SelectObject(memory, previous);
    DeleteDC(memory);
    ReleaseDC(nullptr, screen);
    if (!printed) {
        DeleteObject(bitmap);
        throw std::runtime_error("Could not capture the application window.");
    }
    {
        Gdiplus::Bitmap full(bitmap, nullptr);
        std::unique_ptr<Gdiplus::Bitmap> cropped(full.Clone(
            Gdiplus::Rect(visible.left - outer.left, visible.top - outer.top,
                          visible.right - visible.left, visible.bottom - visible.top),
            PixelFormat24bppRGB));
        if (!cropped || cropped->Save(path.c_str(), &png, nullptr) != Gdiplus::Ok) {
            DeleteObject(bitmap);
            throw std::runtime_error("Could not save a screenshot.");
        }
    }
    DeleteObject(bitmap);
    std::wcout << L"Saved " << path << L'\n';
}

void Capture(HWND window, const std::shared_ptr<PreviewFrame>& frame,
             const std::wstring& path, const CLSID& png) {
    ShowFrame(frame);
    Pump(400);
    ShowFrame(frame);
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    Pump(300);
    SaveWindow(window, path, png);
}

void ShowViewFrame(const std::shared_ptr<PreviewFrame>& source) {
    auto frame = std::make_shared<PreviewFrame>(*source);
    frame->received = std::chrono::steady_clock::now();
    auto& session = g_viewSlots[0].session;
    std::lock_guard<std::mutex> lock(session.mutex);
    session.frame = frame;
}

void CaptureViewOnly(HWND window, const std::shared_ptr<PreviewFrame>& frame,
                     const std::wstring& path, const CLSID& png) {
    ShowViewFrame(frame);
    Pump(400);
    ShowViewFrame(frame);
    for (auto& slot : g_viewSlots) InvalidateRect(slot.preview, nullptr, FALSE);
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    Pump(300);
    SaveWindow(window, path, png);
}

void SetCombo(HWND combo, const wchar_t* text, int selection) {
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text));
    SendMessageW(combo, CB_SETCURSEL, selection, 0);
}

void Run(const std::wstring& framePath, const std::wstring& outputDirectory) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    const HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSW previewClass{};
    previewClass.hInstance = instance;
    previewClass.lpfnWndProc = PreviewProcedure;
    previewClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    previewClass.lpszClassName = kPreviewClass;
    WNDCLASSW shotClass{};
    shotClass.hInstance = instance;
    shotClass.lpfnWndProc = ShotProcedure;
    shotClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    shotClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    shotClass.lpszClassName = kShotClass;
    WNDCLASSW viewClass{};
    viewClass.hInstance = instance;
    viewClass.lpfnWndProc = ViewOnlyProcedure;
    viewClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    viewClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    viewClass.lpszClassName = kViewClass;
    if (!RegisterClassW(&previewClass) || !RegisterClassW(&shotClass) || !RegisterClassW(&viewClass)) {
        throw std::runtime_error("Could not register the screenshot window classes.");
    }

    HWND window = CreateWindowExW(0, kShotClass, kWindowTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                  100, 60, 700, 795, nullptr, nullptr, instance, nullptr);
    if (!window) throw std::runtime_error("Could not create the screenshot window.");
    g_window = window;
    CreateControls(window);
    g_shotReady = true;
    LayoutControls(window);
    ShowWindow(window, SW_SHOWNOACTIVATE);

    const wchar_t* sourceName = L"PRO CONVERT (External NDI Frame Grabber)";
    NdiSource source;
    source.name = sourceName;
    source.ndiName = "PRO CONVERT (External NDI Frame Grabber)";
    source.address = "172.22.2.2:5961";
    g_sources.push_back(source);
    SetCombo(g_sourceControl, sourceName, 0);
    g_previewName = source.ndiName;
    g_previewAddress = source.address;
    g_previewKind = SourceKind::Ndi;

    const auto frame = LoadFrame(framePath);
    const CLSID png = PngEncoder();
    const auto output = [&](const wchar_t* name) { return outputDirectory + L"\\" + name; };

    SetStatus(L"Select a source and press Start recording. Video only.");
    Capture(window, frame, output(L"main-screen.png"), png);

    // Two masks over the clock and the QR code.
    g_privacyMaskSource = source;
    g_privacyMasks = {{0.41, 0.15, 0.61, 0.38}, {0.92, 0.85, 0.99, 0.97}};
    UpdatePrivacyMaskControls();
    SetStatus(L"Gray mask added. It will cover the full recording.");
    Capture(window, frame, output(L"masking.png"), png);
    ClearPrivacyMasks();

    SetWindowTextW(g_pathControl, L"guide-demo.mp4");
    SetWindowTextW(g_bitrateControl, L"8");
    SendMessageW(g_frameRateControl, CB_SETCURSEL, kCustomFrameRate, 0);
    SetWindowTextW(g_customFrameRateControl, L"10");
    UpdateFrameRateControls();
    SendMessageW(g_colorControl, CB_SETCURSEL, kColorMonochrome, 0);
    g_monochromePreview = true;
    SetStatus(L"Select a source and press Start recording. Video only.");
    Capture(window, frame, output(L"settings.png"), png);

    g_recording = true;
    EnableRecordingControls(false);
    SetWindowTextW(g_recordControl, L"Stop recording");
    SetWindowTextW(g_timerControl, L"Recording time: 00:00:08");
    SetStatus(L"Recording video...");
    Capture(window, frame, output(L"recording.png"), png);

    g_recording = false;

    // View-only mode: hide the recording controls exactly like EnterViewOnly.
    g_viewPanel = CreateWindowW(kViewClass, L"", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                0, 0, 1, 1, window, nullptr, instance, nullptr);
    if (!g_viewPanel) throw std::runtime_error("Could not create the view-only panel.");
    for (HWND control = GetWindow(window, GW_CHILD); control; control = GetWindow(control, GW_HWNDNEXT)) {
        if (control != g_viewPanel) ShowWindow(control, SW_HIDE);
    }
    g_viewOnly = true;
    g_viewCount = 2;
    SendMessageW(g_viewLayout, CB_SETCURSEL, 0, 0);
    LayoutControls(window);
    ShowWindow(g_viewPanel, SW_SHOW);
    g_viewSlots[0].selected = source;
    UpdateViewSources();
    SetWindowTextW(g_viewSlots[1].ip, L"172.22.2.2");
    {
        std::lock_guard<std::mutex> lock(g_viewSlots[1].session.mutex);
        g_viewSlots[1].session.error = L"Searching for NDI sources at 172.22.2.2...";
    }
    CaptureViewOnly(window, frame, output(L"view-only-2.png"), png);

    g_viewCount = 4;
    SendMessageW(g_viewLayout, CB_SETCURSEL, 1, 0);
    LayoutViewOnly(g_viewPanel);
    {
        std::lock_guard<std::mutex> lock(g_viewSlots[1].session.mutex);
        g_viewSlots[1].session.error =
            L"Every NDI source at 172.22.2.2 is already shown in another view.";
    }
    CaptureViewOnly(window, frame, output(L"view-only-4.png"), png);
    DestroyWindow(window);
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: guide-screenshots <sample-frame.png> <output-directory>\n";
        return 2;
    }
    const std::string frame = argv[1];
    const std::string output = argv[2];
    Gdiplus::GdiplusStartupInput startup;
    ULONG_PTR token = 0;
    Gdiplus::GdiplusStartup(&token, &startup, nullptr);
    int result = 0;
    try {
        Run(std::wstring(frame.begin(), frame.end()), std::wstring(output.begin(), output.end()));
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        result = 1;
    }
    Gdiplus::GdiplusShutdown(token);
    return result;
}
