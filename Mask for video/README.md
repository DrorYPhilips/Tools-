# Video Mask

A small portable Windows tool to hide areas of a video. Draw one or more
rectangles on the video. Each rectangle is blurred, pixelated or covered with
a black box, for the whole video or only for a time range. Then export a new
video file. The sound is kept, or removed with **Mute**.

## Files

The portable release folder (`dist\`) has everything. Nothing has to be
found on the web:

| File | Size | What it is |
|---|---|---|
| `MaskVideo.exe` | about 70 KB | The program (C# WinForms, .NET Framework 4.8) |
| `User Manual.html` | about 370 KB | The user manual, one file with the pictures inside (works offline) |
| `ffmpeg.exe` | about 100 MB | Reads and writes the video |
| `ffmpeg-LICENSE.txt` | | The ffmpeg license (GPL) |
| `READ ME.txt` | | Short start help |
| `.NET Framework 4.8 installer\ndp48-x86-x64-allos-enu.exe` | about 116 MB | Microsoft's offline installer, only for older Windows |

.NET Framework 4.8 is part of Windows 10 (version 1903 or newer) and
Windows 11, so nothing needs to be installed there. Copy the folder to a USB
stick or any PC and run `MaskVideo.exe`. On Windows 7 SP1, 8.1 or an old
Windows 10, run the .NET installer once first (it works without internet).

If `ffmpeg.exe` is missing, the program shows where to put it and can open
the download page (<https://www.gyan.dev/ffmpeg/builds/>, file
`ffmpeg-release-essentials.zip`, take `bin\ffmpeg.exe`). The program also
finds ffmpeg in a `ffmpeg\` or `ffmpeg\bin\` sub folder, or on `PATH`.

## Video formats

Input: every format ffmpeg can read (MP4, MOV, MKV, AVI, WMV, WebM, FLV,
MPEG, TS/MTS/M2TS, 3GP, VOB, OGV, MXF, GIF, and more). Phone videos that are
recorded rotated are shown and exported the correct way up.

Output: the same type as the source by default, or MP4 / MKV / MOV / AVI /
WebM, or any other extension that ffmpeg knows.

| Output type | Video codec | Sound |
|---|---|---|
| MP4, MOV, M4V, MKV, TS, MTS, M2TS, FLV, 3GP | H.264 (x264) | copied; re-encoded if the type does not accept it |
| WebM | VP9 | re-encoded to Opus if needed |
| other (AVI, WMV, MPG, ...) | ffmpeg's normal codec for the type, high quality | copied or re-encoded |

The "Export quality" box sets the video quality:

- **Same as original** (default): the same codec as the original (H.264
  stays H.264, HEVC stays HEVC), the same bit depth for HEVC (10-bit HDR
  phone video stays 10-bit, with its colour tags), and the same average
  bitrate. The file has about the same size and quality as the original.
  Tested: bitrate 96-100% of the original, SSIM 0.98-0.99 (1 = identical).
  For AVI, WMV and other old types it uses the High setting.
- **High** (x264 CRF 18), **Normal** (CRF 22), **Small file** (CRF 27):
  always H.264 (VP9 for WebM) at a fixed quality level. The file size
  depends on the picture, not on the original.

**Mute** on: the exported file has no sound. With Mute on and no areas, the
video is copied without change (fast, same quality, the same frames), so
this only removes the sound. If the chosen output type cannot hold the
source video (for example H.264 into WebM), the video is encoded instead.

Resolution and frame rate: the export never scales the video and never
changes the frame rate. Every frame keeps its time, also in phone videos
with a variable frame rate (tested with MP4, MOV, MKV, TS and WebM). The
only exceptions:

- An odd width or height (for example 641x361) into an H.264 type (MP4,
  MOV, MKV, TS, ...) loses 1 pixel column or row, because H.264 needs even
  sizes. It is cut, not scaled.
- A rotated phone video is written the correct way up (for example
  1080x1920 instead of 1920x1080 with a rotation flag). It looks the same.
- GIF stores times in steps of 1/100 s, so 29.97 or 30 fps cannot be kept
  exactly (25 fps can).
- AVI keeps all frames and the frame rate, but the video can start one
  frame late. Use MP4, MKV or MOV when exact timing matters.
- 10-bit H.264 is written as 8-bit. 10-bit HEVC stays 10-bit only with
  "Same as original"; High / Normal / Small file write 8-bit H.264.

## How to use

1. Open a video: **Open video...**, Ctrl+O, or drag the file into the window
   (or onto `MaskVideo.exe`).
2. Drag on the video to draw an area. Add as many areas as you need.
3. Drag an area to move it. Drag the white squares to resize it. The exact
   numbers can also be typed under **Selected area**.
4. Choose the effect: **Blur** (strength = blur size), **Pixelate**
   (strength = block size in pixels) or **Black box**.
5. To hide an area for only part of the video, go to the frame, select the
   area and press **Now** next to Start or End (or right-click the area).
6. **Export video...** writes a new file. The source file is never changed.
   Press **Mute** first to write the file without sound. To only remove the
   sound, open the video, press **Mute** and export, with no areas.

**Show result** on: the preview is made by ffmpeg with the real effect.
Off: faster preview with colored boxes.

**Save areas / Load areas** keep the list in a small `.masks` text file, so
the same areas can be used on other clips from the same camera.

Keys: Space = play/pause, Left/Right = one frame, Shift+Left/Right = one
second, Home/End = first/last frame, Delete = delete area, Ctrl+D =
duplicate area, Ctrl+S = save areas, Ctrl+E = export, F1 = user manual.

**Help** (or F1) opens `User Manual.html` from the program folder in the web
browser. Without that file it shows a short help text.

Note: for faces, number plates and text, Pixelate with a large block size or
Black box hide the content more safely than a light blur.

## Build

Needs the .NET SDK (any version that can build `net48`, for example 8.0).

```powershell
powershell -ExecutionPolicy Bypass -File build.ps1
```

This builds `bin\Release\net48\MaskVideo.exe` and makes the release folder
`dist\`. The user manual source is `docs\UserManual.html` with its pictures
in `docs\images\`; `build.ps1` puts the pictures inside the HTML and writes
`dist\User Manual.html`. The third-party files are not in git. Put them in `tools\` first,
and `build.ps1` copies them to `dist\`:

| File in `tools\` | Where to get it |
|---|---|
| `ffmpeg.exe` | <https://www.gyan.dev/ffmpeg/builds/> → `ffmpeg-release-essentials.zip` → `bin\ffmpeg.exe` |
| `ffmpeg-LICENSE.txt` | the `LICENSE` file from the same zip |
| `ndp48-x86-x64-allos-enu.exe` | .NET Framework 4.8 offline installer: <https://go.microsoft.com/fwlink/?linkid=2088631> |

Or only: `dotnet build -c Release`.

## Code

| File | What it does |
|---|---|
| `src/MainForm.cs` | Main window: toolbar, area list, properties, time bar, keys |
| `src/VideoPanel.cs` | Shows the frame; draw, move and resize areas with the mouse |
| `src/Ffmpeg.cs` | Finds and runs ffmpeg, reads video info, grabs frames, builds the filter graph |
| `src/Playback.cs` | Background frame grabbing and playback (raw frames through a pipe) |
| `src/ExportForm.cs` | Export with progress, codec choice, sound fallback |
| `src/MaskWindow.cs` | The area model and the `.masks` file format |

How the effect is made: one ffmpeg `-filter_complex` graph. For each area the
video is split, the area is cropped, blurred (`gblur`) or pixelated
(`scale` down and up), and laid back with `overlay`. A black box uses
`drawbox`. A time range adds `enable='between(t,start,end)'`.
