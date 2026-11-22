# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Video Mask: a small portable Windows tool (C# WinForms, .NET Framework 4.8).
The user draws rectangles on a video. Each one is blurred, pixelated or
covered with a black box, for the whole video or for a time range. Then the
tool exports a new file. `README.md` is the user guide.

All video work is done by an external `ffmpeg.exe` that runs as a child
process. The app has no video library of its own.

The portable result is only two files: `MaskVideo.exe` (about 70 KB) and
`ffmpeg.exe`. Keep it that way: no NuGet packages, no extra DLLs. Only
`ffmpeg.exe` is shipped (no `ffprobe.exe`), so `VideoInfo.Probe()` reads the
video info by parsing the stderr text of `ffmpeg -i`.

This folder has no `.git` of its own. It belongs to the root `PlatformIO`
repo. Its own `.gitignore` covers `bin/`, `obj/`, `dist/` and `tools/*.exe`.

## Commands

- Build: `dotnet build -c Release` → `bin\Release\net48\MaskVideo.exe`.
  The .NET 8 SDK on this PC builds `net48`.
- Release folder: `powershell -ExecutionPolicy Bypass -File build.ps1`.
  It builds the exe and makes `dist\` with `ffmpeg.exe`, `ffmpeg-LICENSE.txt`,
  `READ ME.txt` and the .NET 4.8 offline installer. The third-party files
  come from `tools\` (gitignored; where to get them is in `build.ps1` and
  the README). The user wants the release folder to need no web search and
  no installation.
- Run: `bin\Release\net48\MaskVideo.exe [video file]`. A path on the command
  line is opened at start. `Ff.Find()` looks for `ffmpeg.exe` next to the exe,
  then in `ffmpeg\` and `ffmpeg\bin\` beside it, then on PATH. ffmpeg is not on
  PATH on this PC, so copy `ffmpeg.exe` into the output folder to run it.
- There are no tests in the project. Check a change by building it, then by
  opening a real video in the app (`dist\MaskVideo.exe` finds the
  `ffmpeg.exe` next to it). Shotcut's ffmpeg 9 at `C:\Program Files\Shotcut`
  also works when that folder is on PATH, but it needs the DLLs in its own
  folder, so it cannot be copied to `dist\`.

## Architecture

The UI is built in code (no designer files). `MainForm` holds all state. The
other classes each do one job; the "Code" table in `README.md` lists them.

### One filter graph for preview, playback and export

`Ff.BuildGraph()` in `src/Ffmpeg.cs` makes the `-filter_complex` string. The
input label is `[0:v]` and the output label is `[vout]`. For each area the
video is split, cropped, blurred (`gblur`) or pixelated (`scale` down and up),
and laid back with `overlay`. A black box uses `drawbox`. Three callers use it:

| Caller | `timed` | `shift` | `tail` |
|---|---|---|---|
| Still preview (`MainForm.RequestFrame`) | false; only the areas active at the current time are passed | 0 | `scale=` to preview size |
| Playback (`MainForm.StartPlay`) | true | seek time | `scale=` to preview size |
| Export (`ExportForm.Work`) | true | 0 | even-size `crop` for H.264 types, else null |

- `-ss` before `-i` makes the filter time `t` start at 0 at the seek point.
  So `shift` is subtracted from each area's start and end in `enable='...'`.
  A margin of 1/4 frame is added so a frame on the exact start/end is not lost.
- `Ff.WithFirstFilter()` puts `showinfo` right after `[0:v]`. It works only
  because the graph starts with `[0:v]`. Keep that.
- Area rectangles are always in full-size video pixels (the size after
  auto-rotation, taken from the first decoded frame). Preview frames are made
  smaller (max 1280 px) by the `tail`, after the effects.
- `MaskWindow.Fit()` keeps a rectangle inside the video and makes all values
  even (4:2:0 chroma). Pass every new or edited rectangle through it.
- `VideoPanel` uses the SAR (non-square pixels) only for display. The area
  coordinates do not change.

### Seeking

Fast seek (`-ss` before `-i`) can give a later frame (the next keyframe) in
files with no index (.ts, .mts, .mpg). `Ff.Grab()` with `maxLate > 0` reads
the real frame time from `showinfo` and returns null if the frame is too late.
`FrameFetcher` then tries again with `safeSeek` (seek to 20 s before, then
decode up to the time). If that also fails, it steps back 0.5 s, up to 3 times
(some files have no frame at the very end).

### Threads

- `FrameFetcher` (`src/Playback.cs`): a background task. When many requests
  come fast (slider drag), only the newest waiting request is done.
- `Player` (`src/Playback.cs`): ffmpeg with `-re` writes raw `bgr24` frames to
  stdout, and a thread reads `w*3*h` bytes per frame. The shown time is
  `seek + Offset + index * FrameTime`. `Offset` is the first `showinfo`
  pts_time. `_session` lets an old read thread know that it is out of date.
- `ExportForm`: its own thread. It reads the `-progress pipe:1` lines.
- The events of these classes fire on background threads. `MainForm` keeps
  only the newest playback frame and posts to the UI thread with
  `SafeInvoke()` / `BeginInvoke`. A bitmap that is not shown must be disposed.

### Export

- The user requires that export never changes the resolution or the frame
  rate. Do not add `scale`, `fps`, `-r` or `-fps_mode` to the export command.
  The only size change is the even-size `crop` tail for H.264 types. Tested:
  frame count and every frame time stay the same in MP4/MOV/MKV/TS/WebM,
  also for variable frame rate. The README lists the exceptions (odd size,
  GIF, AVI, 10-bit).
- `ExportJob.Mute`: only `-an` is tried. With Mute and no areas, `Work()`
  first tries `-c:v copy` (bit-exact video). If the output type cannot hold
  the source stream, it encodes as usual.
- `ExportForm.VideoArgs()` picks the encoder from the output extension and the
  quality. Default "Same as original" (`Quality = 0`): same codec family
  (`libx265` with `-tag:v hvc1` for an HEVC source, if `Ff.HasEncoder` finds
  it), same bit depth for HEVC, and ABR at the source video bitrate
  (`VideoInfo.VideoKbps`; for MKV/TS it is the total bitrate minus the
  sound). Do not switch it to CRF with `-maxrate`: tested, x264 then used
  far fewer bits than the cap and lost quality. High/Normal/Small file:
  x264 CRF 18/22/27, VP9 for WebM, `-q:v` for other types.
- Sound: first try `-c:a copy`, then re-encode, then `-an`. If ffmpeg already
  reported progress before it failed, stop there: another sound option will
  not help. Exception: "Could not write header" (the type does not accept
  this sound). ffmpeg 9 still prints `out_time_us > 0` in that case.
- The source file is never overwritten. A failed or cancelled export deletes
  the partial output file.

## Rules in the code

- Every number sent to ffmpeg goes through `Ff.N()` (invariant culture). The
  user's Windows may use a comma as the decimal separator.
- `MaskWindow.End = double.PositiveInfinity` means "until the end of the
  video". The UI shows this as End = video length. The `.masks` file writes
  `end`.
- `MainForm._ui` is true while code fills the controls, so the `ValueChanged`
  handlers do nothing. Set it around new code that fills controls.
- After any change to the areas, call `MainForm.AreasEdited()`. It refreshes
  the list, the properties, the drawing and the preview frame.
- DPI: `AutoScaleMode.None`. Every pixel size goes through `D()`
  (`DeviceDpi / 96`). `VideoPanel` uses `S` in the same way.
- `.masks` format (`MaskFile` in `src/MaskWindow.cs`): one area per line,
  `mode x y width height strength start end`, `#` for comments. Users keep
  these files, so old files must still load.
- The help text is in three places: `MainForm.ShowHelp()`, "How to use" in
  `README.md`, and the user manual `docs/UserManual.html`. The keys are in
  `MainForm.ProcessCmdKey()`. Change them together.
- User manual: `docs/UserManual.html` (simple English, for end users) with
  screenshots in `docs/images/`. `build.ps1` inlines the PNGs as base64 and
  writes `dist\User Manual.html`, so the release manual is one offline file
  (no external CSS, fonts or scripts). The screenshots are of the real app
  at 96 DPI; when the UI changes, retake them and update the text.
- `LangVersion` is `latest` but the target is .NET Framework 4.8. New C#
  syntax is fine; .NET Core-only APIs are not available.
