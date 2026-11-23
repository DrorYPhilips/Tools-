# Frame Check (C++ / Qt)

This is a native Windows desktop replacement for the Python scripts. It provides:

- Frame-number OCR, duplicate and missing-frame detection, plus a text report.
- A corrupted-image scan and report.
- Image folder or AVI/MP4/MOV video selection, interactive crop selection, a crop preview, and progress feedback.

The app reads PNG, JPG, JPEG, and BMP files without renaming or modifying them.
It can also decode AVI, MP4, and MOV videos through OpenCV's FFmpeg backend, subject
to the availability of the video's codec. Video files are read without modification.
Video reports compare adjacent frames using the expected counter increment of +1.
`10808, 10809, 10811` reports one dropped counter, 10810;
`10810, 10810, 10811` reports one duplicate. A backwards counter or an isolated
OCR spike is reported as unresolved, not silently repaired. Unreadable frames
also make totals provisional. Reports list missing numbers as ranges and
individually; very large gaps are kept as ranges to avoid excessive report size.
Image folders retain sorted-counter analysis.
Each report starts with a summary block (dropped and duplicated frames, processed
frames out of the expected total, OCR counts, and unresolved issues), followed by
the detailed sections. In the app, the dropped and duplicated lines are shown in red
when their total is more than 1. The saved `.txt` report is plain text.

## Dependencies

- CMake 3.21 or newer
- A C++17 compiler
- Qt 6.4 or newer (Widgets)
- OpenCV (core, imgcodecs, imgproc, videoio with FFmpeg)
- Tesseract OCR with English `eng.traineddata`
- OpenMP and libcurl development files (required by the installed Tesseract package)

The Python version uses EasyOCR; this native version uses Tesseract instead, so OCR
results can differ. The Tesseract English traineddata file must be available to
Tesseract at runtime.
For separated digits, the app recognizes each character individually; otherwise
it reads the whole cropped line. OCR may still misread numbers, so check unexpected
gaps against the original frames.

## Build

The installed toolchain is MSYS2 UCRT64, under
`%LOCALAPPDATA%\Programs\msys64`. It includes GCC 16.2.0, GNU Make 4.4.1,
CMake 4.5.0, Ninja, Qt 6.12.0, OpenCV 5.0.0, and Tesseract 5.5.4 with
English OCR data. Its `ucrt64\bin` directory has been added to the current
user's Windows PATH. Restart VS Code to pick up that change, or set PATH
in the current PowerShell session as shown below.
When `TESSDATA_PREFIX` is not set, the app looks for the English data in
`share\tessdata` next to the installed `tesseract.exe`'s parent directory.

From this folder run:

```powershell
$env:Path = "$env:LOCALAPPDATA\Programs\msys64\ucrt64\bin;$env:Path"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
.\build\FrameCheck.exe
```

For the currently installed updated build, double-click `Run FrameCheck.cmd`
in this folder. It opens `build\FrameCheck.exe`; the window title says
**Frame Check - Improved OCR**.

GNU Make is available as `mingw32-make` in PowerShell, or as `make` in the
MSYS2 terminal. The commands above use Ninja instead.

The executable is written to `build\FrameCheck.exe`. It uses the installed
toolchain's runtime DLLs and OCR data; it is not a standalone distribution.
Use the portable package below for another Windows PC.
Windows cannot overwrite an executable while it is running. Close the app before
rebuilding, or configure a separate build directory such as `build-report`.

MSYS2's certificate store includes the roots already trusted by Windows, so
package downloads on this network can use TLS verification. The primary
`repo.msys2.org` package server is preferred over the redirecting mirror.

To run the focused tests, including generated AVI, MP4, and MOV files:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFRAMECHECK_BUILD_TESTS=ON
cmake --build build --parallel 2
$env:QT_QPA_PLATFORM = "offscreen"
ctest --test-dir build --output-on-failure
```

### Portable package

To create a self-contained folder that runs without MSYS2 on PATH:

```powershell
cmake --install build --prefix dist\FrameCheck
```

The `dist\FrameCheck` folder contains `FrameCheck.exe`, the Qt, OpenCV,
Tesseract, and MinGW runtime DLLs, the Qt `platforms`, `styles`, and
`imageformats` plugins, and `tessdata\eng.traineddata` for OCR. Copy the whole
folder to another PC and run `FrameCheck.exe`. The `dist` folder is ignored by git.

## Use

See [USER_GUIDE.html](USER_GUIDE.html) for the illustrated, step-by-step user guide. You can also open it from the app with the **?** button next to the folder and video buttons. The window uses a card layout with a teal accent. Dark mode is on by default; turn it off with **View > Dark mode** (the choice is remembered): the three numbered steps (1 Choose input, 2 Select counter area, 3 Run analysis) are cards, and **Analyze frame numbers** is the primary action.

Choose an image folder or select an AVI, MP4, or MOV video. For frame analysis, click
**Select counter area...**, drag a rectangle around the counter on the first
frame, and click **OK**. The corresponding pixel coordinates and crop preview
are updated automatically. You can also enter or adjust the coordinates
manually and click **Preview crop**. Clear the **Show crop box** checkbox to hide the
"OCR crop box (pixels)" group and free up space; the coordinates are kept. Then select **Analyze frame numbers**.
The OCR reads
numbers visible within each frame; it does not use a video's internal frame
index as the detected number. Video frames in the report are indexed from 0,
so a 2,235-frame video has indices 0 through 2234.
The completed report is shown in the app as well as saved to disk. For a
missing video counter, the report estimates its time by interpolating between
the neighboring observed counters and gives the next observed frame and its
time for navigation. A missing frame is not actually present in the video, so
its exact video timestamp cannot be known. Suspect transitions are listed as
unresolved; missing timestamps are marked unavailable. When decoder timestamps are unavailable, frame rate
is used as an approximation; timing may differ for variable-frame-rate videos.
Use **Scan image health** for the separate corruption check of image folders
only. Reports are saved in the image folder or beside the video, with revision
suffixes so existing reports are not overwritten. The selected input and crop
coordinates are remembered by Qt settings. A warning is included when video
metadata reports more frames than the decoder could read; the metadata count
is only an estimate.

Uncompressed AVI files whose frames are stored bottom-up (positive height in the
stream header) are read by FrameCheck's built-in reader, because the bundled
OpenCV FFmpeg decoder crashes on them. The reader supports 24-bit and 32-bit
uncompressed (BI_RGB) video. Other AVI, MP4, and MOV files use OpenCV.
