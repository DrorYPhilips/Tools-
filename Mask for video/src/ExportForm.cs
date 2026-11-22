using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace MaskVideo
{
    internal sealed class ExportJob
    {
        public string Input, Output;
        public List<MaskWindow> Windows;
        public double Fps, Duration;
        public bool HasAudio;
        public int Quality; // 0 = same as original, 1 = high, 2 = normal, 3 = small file
        public bool Mute;   // true = the output has no sound
        public string SourceCodec = "", SourcePixFmt = ""; // from VideoInfo, for "same as original"
        public int SourceKbps;                             // video bitrate of the source, 0 if unknown
    }

    /// <summary>Runs the ffmpeg export and shows the progress.</summary>
    internal sealed class ExportForm : Form
    {
        private readonly ExportJob _job;
        private readonly ProgressBar _bar;
        private readonly Label _label;
        private readonly Button _cancel, _showFile;
        private readonly float _s;
        private Process _proc;
        private volatile bool _cancelled;
        private bool _finished;
        private readonly Stopwatch _clock = new Stopwatch();
        private long _lastUiTick;

        public ExportForm(ExportJob job)
        {
            _job = job;
            _s = DeviceDpi / 96f;

            AutoScaleMode = AutoScaleMode.None;
            Font = SystemFonts.MessageBoxFont;
            Text = "Export - " + Path.GetFileName(job.Output);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            ShowInTaskbar = false;
            StartPosition = FormStartPosition.CenterParent;
            ClientSize = new Size(D(480), D(150));

            _label = new Label
            {
                Text = "Starting ffmpeg ...",
                AutoSize = false,
                Location = new Point(D(14), D(12)),
                Size = new Size(D(452), D(42)),
            };
            _bar = new ProgressBar
            {
                Location = new Point(D(14), D(60)),
                Size = new Size(D(452), D(24)),
                Minimum = 0,
                Maximum = 1000,
                Style = job.Duration > 0 ? ProgressBarStyle.Continuous : ProgressBarStyle.Marquee,
            };
            _cancel = new Button
            {
                Text = "Cancel",
                Location = new Point(D(366), D(102)),
                Size = new Size(D(100), D(32)),
                UseVisualStyleBackColor = true,
            };
            _cancel.Click += (s, e) => Close();
            _showFile = new Button
            {
                Text = "Show file",
                Location = new Point(D(256), D(102)),
                Size = new Size(D(100), D(32)),
                Visible = false,
                UseVisualStyleBackColor = true,
            };
            _showFile.Click += (s, e) =>
            {
                try { Process.Start("explorer.exe", "/select,\"" + _job.Output + "\""); } catch { }
            };

            Controls.Add(_label);
            Controls.Add(_bar);
            Controls.Add(_showFile);
            Controls.Add(_cancel);
            CancelButton = _cancel;
        }

        private int D(int v) => (int)Math.Round(v * _s);

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            _clock.Start();
            new Thread(Work) { IsBackground = true, Name = "export" }.Start();
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (!_finished)
            {
                var r = MessageBox.Show(this, "Stop the export? The unfinished file will be deleted.", "Export",
                    MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (r != DialogResult.Yes || _finished)
                {
                    e.Cancel = !_finished;
                    return;
                }
                _cancelled = true;
                try { if (_proc != null && !_proc.HasExited) _proc.Kill(); } catch { }
            }
            base.OnFormClosing(e);
        }

        // ---------------------------------------------------------------------

        /// <summary>
        /// Video encoder settings chosen from the output file type and the quality.
        /// "Same as original" (quality 0): keep the codec family (H.264 or HEVC), for HEVC the bit depth,
        /// and the average bitrate of the original, so the file has about the same size and quality.
        /// (A CRF with a -maxrate cap was tested: x264 then used far fewer bits than allowed and lost quality.)
        /// </summary>
        private static string VideoArgs(ExportJob job, string ext, out string tail, out string extra)
        {
            bool same = job.Quality <= 0;
            int q = Math.Max(0, Math.Min(2, job.Quality - 1)); // High, Normal, Small file
            int[] crf = { 18, 22, 27 };   // x264
            int[] vpx = { 24, 31, 38 };   // VP9
            int[] qv = { 2, 4, 7 };       // older codecs (mpeg4, mpeg2, wmv ...)
            tail = null;
            extra = "";

            int kbps = same ? job.SourceKbps : 0;
            // Average bitrate of the original. Short peaks may go up to 2x, like in camera files.
            // Without a known bitrate, a high fixed quality is used instead.
            string rate = kbps > 0 ? $"-b:v {kbps}k -maxrate {2 * kbps}k -bufsize {4 * kbps}k" : null;
            bool tenBit = (job.SourcePixFmt ?? "").Contains("10");

            switch (ext)
            {
                case ".mp4":
                case ".m4v":
                case ".mov":
                case ".3gp":
                case ".3g2":
                    extra = "-movflags +faststart";
                    goto case ".mkv";
                case ".ts":
                case ".mts":
                case ".m2ts":
                    // TS has no index. A keyframe every 2 s keeps the file easy to seek in players.
                    extra = "-force_key_frames expr:gte(t,n_forced*2)";
                    goto case ".mkv";
                case ".mkv":
                case ".flv":
                case ".f4v":
                    // H.264 4:2:0 needs even width and height. For an odd size, cut off 1 pixel (no rescale).
                    tail = "crop=trunc(iw/2)*2:trunc(ih/2)*2:0:0";
                    bool hevcType = ext != ".flv" && ext != ".f4v" && ext != ".3gp" && ext != ".3g2";
                    if (same && job.SourceCodec == "hevc" && hevcType && Ff.HasEncoder("libx265"))
                    {
                        // HEVC stays HEVC, 10-bit stays 10-bit (HDR phone video). The "hvc1" tag lets
                        // Apple devices and Windows play HEVC in MP4 / MOV.
                        string tag = ext == ".mp4" || ext == ".m4v" || ext == ".mov" ? " -tag:v hvc1" : "";
                        return $"-c:v libx265 -preset medium {rate ?? "-crf 20"} -pix_fmt {(tenBit ? "yuv420p10le" : "yuv420p")}{tag} -x265-params log-level=error";
                    }
                    return $"-c:v libx264 -preset medium {(same ? rate ?? "-crf 18" : "-crf " + crf[q])} -pix_fmt yuv420p";
                case ".webm":
                    // VP9 with both -crf and -b:v: -b:v is the upper limit ("constrained quality").
                    return $"-c:v libvpx-vp9 -crf {(same ? 24 : vpx[q])} -b:v {(kbps > 0 ? kbps + "k" : "0")} -row-mt 1 -deadline good -cpu-used 4";
                case ".gif":
                    return "";
                default:
                    // Let ffmpeg choose the normal codec for this file type, with good quality.
                    // The codec is usually not the source codec here, so "same as original" uses the High level.
                    return $"-q:v {qv[same ? 0 : q]}";
            }
        }

        private void Work()
        {
            string ext = Path.GetExtension(_job.Output).ToLowerInvariant();
            string vargs = VideoArgs(_job, ext, out string tail, out string extra);
            string graph = Ff.BuildGraph(_job.Windows, true, 0, _job.Fps, tail);

            bool ok = false;
            string error = "";
            string audioNote = "";

            // Mute with no areas: only the sound is removed. Copy the video stream without change
            // (fast, no quality loss, same frames). If the file type cannot hold this video, encode it below.
            bool copied = false;
            if (_job.Mute && _job.Windows.Count == 0)
            {
                string copyExtra = extra.StartsWith("-movflags") ? extra : "";
                string args =
                    "-hide_banner -loglevel error -nostdin -y " +
                    "-i " + Ff.Q(_job.Input) + " -map 0:v:0 -c:v copy -an -map_metadata 0 " + copyExtra + " " +
                    "-progress pipe:1 -nostats " + Ff.Q(_job.Output);
                copied = ok = RunOnce(args, out _, out error) == 0;
            }

            // Sound: first try to copy it unchanged; if the file type does not accept it, encode it;
            // as a last try, write the video without sound.
            var audioModes = new List<string>();
            if (_job.HasAudio && !_job.Mute)
            {
                audioModes.Add("-map 0:a? -c:a copy");
                audioModes.Add("-map 0:a?");
            }
            audioModes.Add("-an");

            for (int i = 0; !ok && i < audioModes.Count && !_cancelled; i++)
            {
                string args =
                    "-hide_banner -loglevel error -nostdin -y " +
                    "-i " + Ff.Q(_job.Input) + " " +
                    "-filter_complex " + Ff.Q(graph) + " -map [vout] " +
                    audioModes[i] + " " + vargs + " -map_metadata 0 " + extra + " " +
                    "-progress pipe:1 -nostats " + Ff.Q(_job.Output);

                int code = RunOnce(args, out bool progressed, out error);
                if (_cancelled) break;
                if (code == 0)
                {
                    ok = true;
                    if (_job.HasAudio && i == 1) audioNote = "\r\nThe sound was re-encoded to fit the file type.";
                    if (_job.HasAudio && i == 2) audioNote = "\r\nThe sound could not be written to this file type, so the video has no sound.";
                    break;
                }
                // Failed in the middle: another sound option will not help. A failure while writing the
                // header (the file type does not accept this sound, for example AAC in WebM) also reports
                // some progress (ffmpeg 9 prints out_time_us > 0 in its last block), so then try the next one.
                if (progressed && !error.Contains("Could not write header")) break;
            }

            if (ok && _job.Mute && _job.HasAudio)
                audioNote = copied ? "\r\nSound removed. The video was copied without change." : "\r\nSound removed.";
            else if (ok && copied)
                audioNote = "\r\nThe video was copied without change.";

            if (!ok) TryDelete(_job.Output);
            UI(() => Finish(ok, error, audioNote));
        }

        private int RunOnce(string args, out bool progressed, out string error)
        {
            progressed = false;
            var err = new StringBuilder();
            using (var p = Ff.Start(args))
            {
                _proc = p;
                p.ErrorDataReceived += (s, e) => { if (e.Data != null) lock (err) err.AppendLine(e.Data); };
                p.BeginErrorReadLine();

                string line;
                while ((line = p.StandardOutput.ReadLine()) != null)
                {
                    // ffmpeg -progress writes "key=value" lines. out_time_us and out_time_ms are both microseconds.
                    if (line.StartsWith("out_time_us=") || line.StartsWith("out_time_ms="))
                    {
                        if (long.TryParse(line.Substring(12), out long us) && us > 0)
                        {
                            progressed = true;
                            Report(us / 1e6);
                        }
                    }
                }
                p.WaitForExit();
                _proc = null;
                lock (err) error = err.ToString();
                return p.ExitCode;
            }
        }

        private void Report(double outTime)
        {
            long now = _clock.ElapsedMilliseconds;
            if (now - Interlocked.Read(ref _lastUiTick) < 200) return;
            Interlocked.Exchange(ref _lastUiTick, now);

            UI(() =>
            {
                if (_finished) return;
                if (_job.Duration <= 0)
                {
                    _label.Text = "Encoding ...  " + Fmt.Time(outTime);
                    return;
                }
                double f = Math.Max(0, Math.Min(1, outTime / _job.Duration));
                _bar.Value = (int)(f * 1000);
                string left = "";
                if (f > 0.02)
                {
                    double sec = _clock.Elapsed.TotalSeconds * (1 - f) / f;
                    left = "   (about " + FmtDuration(sec) + " left)";
                }
                _label.Text = $"Encoding ...  {f * 100:0}%{left}\r\n{Fmt.Time(outTime)} of {Fmt.Time(_job.Duration)}";
            });
        }

        private void Finish(bool ok, string error, string audioNote)
        {
            _finished = true;
            if (_cancelled)
            {
                Close();
                return;
            }

            _bar.Style = ProgressBarStyle.Continuous;
            _cancel.Text = "Close";
            if (ok)
            {
                _bar.Value = _bar.Maximum;
                _label.Text = $"Done in {FmtDuration(_clock.Elapsed.TotalSeconds)}.  Saved: {Path.GetFileName(_job.Output)}{audioNote}";
                _showFile.Visible = true;
            }
            else
            {
                _bar.Value = 0;
                _label.Text = "Export failed.";
                MessageBox.Show(this,
                    "ffmpeg could not write the video.\r\n\r\n" + VideoInfo.LastLines(error, 10) +
                    "\r\n\r\nTip: try MP4 as the output type.",
                    "Export failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private static string FmtDuration(double sec)
        {
            if (sec < 60) return $"{Math.Max(1, (int)Math.Round(sec))} s";
            var ts = TimeSpan.FromSeconds(Math.Round(sec));
            return ts.TotalHours >= 1 ? $"{(int)ts.TotalHours} h {ts.Minutes} min" : $"{ts.Minutes} min {ts.Seconds} s";
        }

        private static void TryDelete(string path)
        {
            for (int i = 0; i < 5; i++)
            {
                try
                {
                    if (File.Exists(path)) File.Delete(path);
                    return;
                }
                catch { Thread.Sleep(200); } // ffmpeg may still hold the file for a moment
            }
        }

        private void UI(Action a)
        {
            try
            {
                if (!IsDisposed && IsHandleCreated) BeginInvoke(a);
            }
            catch (InvalidOperationException) { }
        }
    }
}
