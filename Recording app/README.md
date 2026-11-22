# Multi-Recorder

A small Windows desktop application that records NDI sources, USB cameras,
native Epiphan and Datapath Vision frame grabbers, and legacy Video for Windows (VFW) devices to H.264 or H.265/HEVC MP4, or
uncompressed or Lagarith-compressed AVI files.

## Requirements

- 64-bit Windows
- Visual Studio 2022 with the Desktop development with C++ workload
- CMake 3.20 or newer
- The NDI 6 SDK in the bundled `NDI 6 SDK` folder
- For native Epiphan capture, the supplied Epiphan C++ SDK and its installed
  64-bit device driver
- For native Datapath capture, the supplied RGBEasy SDK headers and the
  installed 64-bit Datapath Vision driver and RGBEasy runtime

The NDI runtime and SDK are subject to the license terms supplied with the
bundled SDK.

## Build

From the project directory, run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Every build creates two editions from the same source code:

| Edition | Output | Video inputs |
| --- | --- | --- |
| Multi-Recorder | `build\Release\Multi-Recorder\Multi-Recorder.exe` | NDI, USB cameras, Epiphan, Datapath, VFW |
| NDI-Recorder | `build\Release\NDI-Recorder\NDI-Recorder.exe` | NDI only |

Both editions have the same recording, view-only, frame rate, color, and
file-exists features and share one build number. NDI-Recorder is compiled
with `NDI_ONLY`: it lists only NDI sources, hides **USB settings**, and does
not link or ship the Epiphan or Datapath SDKs. Copy the whole edition folder
to install it on another PC.

CMake copies the NDI runtime DLL into each edition folder. When the Epiphan
SDK is present, CMake also links its x64 library into Multi-Recorder and
copies `frmgrab.dll` beside it. The SDK location can
be set with `EPIPHAN_SDK_ROOT`; set `ENABLE_EPIPHAN` to `OFF` to build without
it. If its files are missing, native Epiphan support is disabled at build
time. The SDK and runtime are subject to Epiphan's terms; check those terms
before redistributing them.

The window title shows the app version and a build number, for example
`Multi-Recorder v1.0 (Build 1)`. The build number increases by one whenever
the app's source files, CMake configuration, or SDK enable/disable options
change and the app is rebuilt; rebuilding unchanged sources keeps the same
number. The current number is stored in `build-number.txt` next to
`CMakeLists.txt`, so keep that file with the sources. It does not identify
installed drivers or SDKs.

For Datapath, `DATAPATH_SDK_ROOT` identifies the RGBEasy C SDK directory and
`ENABLE_DATAPATH=OFF` disables the integration. If the headers are missing,
the app builds without Datapath support. The app loads `rgbeasy.dll` from the
Windows system directory at runtime, so the Datapath driver is only required
on PCs using Datapath devices. Install a compatible 64-bit driver rather than
copying a DLL from the SDK or driver store. The SDK and driver are subject to
Datapath's licensing terms and any separate permission you have.

## Use

Click **User guide** (on the recording screen or in view-only mode) or press
**F1** to open the illustrated HTML user guide in the default browser. The
guide source is in `docs\user-guide`; each build copies it to a `user-guide`
folder next to each edition's exe. Text wrapped in `<!--multi-->...<!--/multi-->`
or `<!--ndi-->...<!--/ndi-->` appears only in that edition's guide, and
screenshots in `images\ndi` replace the same-named images in the NDI-Recorder
guide. Keep the `user-guide` folder with the application when you copy it to
another PC.

Choose a discovered NDI source, USB camera, or VFW device, output path, and format, then click
**Start recording**. Click **Stop recording** to finalize the file. MP4 uses
Windows Media Foundation encoders; HEVC requires a compatible installed
encoder. AVI supports uncompressed video or the separately installed 64-bit
Lagarith VFW codec.

If the output file already exists, choose **Overwrite existing file**,
**Use numbered filename** (for example, `video_1.mp4`, skipping names already in
use), **Choose another filename...**, or **Cancel**. Cancel is the default.
Choosing another existing filename shows the same options again. The final
filename is shown in **Save to** before recording starts.

Click **View only** to show two or four sources in one window. Choose **2 views**
or **4 views**, then select a different source above each tile. **Refresh**
rediscovers devices, and **Back to record** returns to the single-source
recording screen with its controls and settings restored. Recording controls
are hidden in view-only mode. View-only mode does not record; switching from four
views to two closes and clears the lower two views. Previously selected USB and Datapath
settings are used during the current app session. Device drivers and system
resources must support the selected sources simultaneously; a tile reports an
error if its source cannot be opened.
To show an NDI device that is not discovered automatically, type its numeric IP
address in the view's **IP** field and click **Connect** (or press Enter). The
view searches that address for up to 15 seconds and shows the first NDI source
there that no other view is using. Other views keep playing while it searches,
and sources found this way also appear in every source list for the rest of the
app session.
Views update at up to about 30 frames per second when sources and the PC can
keep up; frames may be skipped for display.

**Color** selects **Source color** (default) or **Monochrome 8-bit (green
channel)**. Monochrome uses each pixel's green value as its gray level and
ignores red and blue, which suits sync-on-green (SOG) and other signals that
carry the image only on green. The recording preview shows the result; the
view-only tiles stay in source color. **AVI (uncompressed)** is saved as true
8-bit grayscale (a quarter of the color file size). MP4 and Lagarith encoders
don't accept 8-bit grayscale input, so those files store neutral gray in
their normal color format. MP4 encoding uses limited-range luma, and some
Lagarith color modes can also slightly change gray levels.

For MP4, enter a target bitrate in Mbps (a whole number from 1 to 1000), or
leave it blank for the existing automatic selection. The encoder's actual
bitrate can differ from the target. For Lagarith AVI, select **Codec
settings...** to open the codec's own configuration dialog. Settings selected
there are applied to subsequent Lagarith recordings during this app session.
Lagarith compresses losslessly in its chosen color format; choosing a
subsampled color mode can change RGB colors during conversion. Compression
also adds CPU load, so check that the system can keep up with the source.

On the development machine, the generated Lagarith AVI decoded correctly with
FFmpeg and by calling the VFW codec directly, but Windows'
`AVIStreamGetFrame` decoder path crashed. Verify playback with your intended
player; some VFW-based applications may be affected.

Click **Refresh** after connecting a camera. USB cameras appear with the
`USB:` prefix; legacy drivers appear as `VFW:`. Select a camera to start its
preview. Windows camera privacy settings and device permissions must allow
desktop apps to use it. A device can appear under both APIs. The 64-bit app
requires a compatible 64-bit VFW driver supporting uncompressed RGB video.

With the Epiphan SDK enabled, local devices also appear as `Epiphan:` sources.
Select that entry for native SDK capture; it shares one device connection
between preview and recording. Close other capture software if the device
cannot be opened. Wait for the measured capture rate to appear before starting
recording. The bottom line distinguishes the measured SDK capture rate from the
input signal's refresh rate.

With the Datapath SDK enabled and a compatible Vision driver installed, click
**Refresh** and select a `Datapath:` input. The SDK input is shared between
preview and recording. Wait for a measured capture rate before recording.
The information bar also shows the driver-reported capture rate; both rates
must be available before recording can start.
For a `Datapath:` source, **Device settings...** shows detected signal size and
driver video controls, including capture dimensions and input adjustments.
Availability and valid ranges depend on the input signal and driver. Select a
control, enter a value, and click **Apply**; **Default** fills in the driver's
default value without applying it. Preview pauses while settings are open, and
settings are unavailable during recording. Applied values are remembered for
the current app session and reapplied when preview reopens; if a saved value
becomes invalid, **Forget saved settings** removes the session overrides.
Forgetting them does not reset settings retained by the driver. Capture
dimensions do not change the source's signal resolution or refresh rate.
The app does not change EDID or intentionally limit the capture frame rate.
Interlaced Datapath inputs can be previewed but cannot be recorded by this path.

For an `Epiphan:` source, **Device settings...** shows the input type and
detected signal mode. When the SDK reports an analog input and usable ranges,
horizontal shift, vertical shift, and phase can be set manually or returned to
automatic mode. **Apply adjustment** changes the device immediately; closing
the dialog does not undo it. Available adjustments depend on the input and
driver, and are not offered for digital input. The source determines the input
resolution and refresh rate; this dialog does not change EDID or force a mode.
Preview pauses while the dialog is open, and settings are unavailable during
recording.

For a `USB:` source, click **USB settings...** before recording. Choose an
advertised resolution and frame rate, or **Device default**, and click **OK**.
The selection is kept for that device until the app closes. The app restarts
preview and reports an error if the requested output format cannot be applied;
the listed rates are device-reported and do not guarantee that every frame
arrives at that rate.

The settings window also lists supported standard camera controls such as
brightness, exposure, and focus. Select a control, enter a value within its
range and step (or select **Auto** if supported), then click **Apply control**.
**Driver settings...** opens the manufacturer's property pages when the driver
exposes them. Control and driver changes apply immediately, even if you cancel
the format selection. Controls and driver dialogs depend on the device and a
matching DirectShow driver; some devices provide neither. Preview pauses while
settings are open, and settings cannot be changed during recording.

VFW preview requests a 33 ms frame interval; actual driver behavior can vary.
It is also the VFW recording source, so this mode does not provide reliable
60 fps capture; the app warns before starting a VFW recording.

To find an NDI device at a different IP address, enter its numeric IPv4 or
IPv6 address in **NDI device IP** and click **Apply IP**. Wait a few seconds
and select the discovered source. Clear the field and apply it to return to
default discovery. The applied IP is saved and restored the next time the app
opens; it does not change the device's network configuration. RTSP cameras are
not currently supported.

### NDI device settings

**NDI settings...** opens the web settings page of the selected NDI device
(`http://<source IP>/`) in the default browser. Select the NDI source and wait
for the preview so its IP is known. The device's own login, if any, applies.

The applied **NDI device IP** is saved per Windows user in
`%APPDATA%\<edition>\settings.ini` (`Multi-Recorder` or `NDI-Recorder`).

The preview shows the selected source before and during recording. It displays
"Waiting for video" when no recent frames are received. The recording timer
starts when the first frame is written and remains visible after stopping.
The bottom of the window shows the source resolution, frame rate and numeric
IP when available, plus system-wide CPU, memory, disk activity and the busiest
GPU engine. Disk activity can exceed 100% across multiple disks. Unavailable
measurements are shown as N/A.

This version records video only. AVI files can be large.

### Recording frame rate

**Record FPS** defaults to **Same as source**, preserving the existing behavior.
Alternatively, choose **30 FPS**, **15 FPS**, **7.5 FPS**, or **Custom FPS**.
Custom FPS accepts 1 through 85 FPS with up to three decimal places, using a
decimal point (for example, `12.5` or `29.97`). The selection is retained for
subsequent recordings until the app closes and cannot be changed while recording.

The selected FPS applies to every source and output format. It changes the saved
video, not the capture device settings or preview. Frames are intentionally
skipped or repeated using their capture-arrival times to keep real-time playback
duration, rounded up to the next output frame. A rate above the source rate does
not create additional motion detail. Conversion cannot recover frames lost
upstream; existing capture-loss checks and recording-buffer limits still apply.
All received frames pass through the recording queue before conversion.

With **Same as source**, the file still uses the source-reported rate (or the
measured preview rate for Epiphan and Datapath). The variable capture-speed
limitations described below still apply to that default mode.

## Frame buffering and limitations

NDI capture and camera recording use a queue of up to 4 GiB of frame data.
NDI capture runs separately from file encoding, and camera preview is updated
at most ten times per second. When capture ends, queued frames are written
before the output is finalized; this can take time after clicking Stop.

There is **no disk-backed spool yet**. If the memory queue fills, recording
stops and reports that at least one frame was lost, after attempting to save
the frames already queued. Insufficient memory, an encoder failure or a disk
write failure can also interrupt recording and prevent queued frames from
being saved. NDI receiver-reported video drops stop recording and are reported.
The application cannot reliably detect all losses inside USB or VFW drivers,
the source device, or the network. A 60 fps source requires a capture path,
encoder, disk and system capable of sustaining its data rate; no finite
buffer guarantees zero frame loss.

The Epiphan SDK frame interface used here does not provide a universal source
sequence number, timestamp, or drop counter. It may miss or repeat source
frames without the application being able to detect that. Capture runs as fast
as the SDK supplies frames; the 4 GiB queue only buffers frames already
received by the application. With **Same as source**, recordings use a fixed
frame rate estimated from the recent preview capture rate, not the input refresh
rate. If the capture rate changes during recording, playback duration and timing
can be inaccurate.
The app warns about these limits before starting an Epiphan recording. Signal
loss, a reported capture error, a substantial input mode change, or a full
queue stops recording and reports an error; already queued frames are drained
when possible. Hardware-specific capture behavior and sustained throughput
must be tested with the intended device and workload.

Datapath capture requests zero intentional frame dropping and copies SDK
callback frames into the same bounded queue. The SDK's single capture buffer,
callback processing, hardware and driver can still miss frames. During
recording, missing timestamps, timestamp discontinuities or sufficiently
large gaps relative to the driver-reported capture rate stop recording with an
error, but not every loss is detectable. With **Same as source**, the video file
uses a fixed frame rate estimated from preview; if capture speed changes,
playback timing may be inaccurate. A signal loss, unsupported interlacing, input mode change or queue
overflow also stops recording. Datapath capture requires testing with the
actual hardware and workload.
