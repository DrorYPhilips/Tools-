using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace MaskVideo
{
    /// <summary>One request for a still preview frame.</summary>
    internal sealed class FrameJob
    {
        public string File;
        public double Time;
        public string Graph;   // null = no effects, only scale
        public int W, H;       // preview size
        public bool Result;    // "Show result" was on when the job was made
        public double MaxLate; // see Ff.Grab
    }

    /// <summary>
    /// Gets still frames on a background thread. When many requests come quickly
    /// (dragging the time slider), only the newest waiting request is done.
    /// </summary>
    internal sealed class FrameFetcher
    {
        private readonly object _lock = new object();
        private FrameJob _pending;
        private bool _busy;

        /// <summary>Called on the background thread. The receiver must marshal to the UI thread.</summary>
        public event Action<FrameJob, Bitmap> Done;

        public void Request(FrameJob job)
        {
            lock (_lock)
            {
                _pending = job;
                if (_busy) return;
                _busy = true;
            }
            Task.Run(Work);
        }

        private void Work()
        {
            while (true)
            {
                FrameJob job;
                lock (_lock)
                {
                    job = _pending;
                    _pending = null;
                    if (job == null) { _busy = false; return; }
                }

                Bitmap bmp = null;
                try
                {
                    bmp = Ff.Grab(job.File, job.Time, job.Graph, job.W, job.H, out _, false, job.MaxLate);
                    // Fast seek gave no frame or a wrong (later) frame: files without an index. Use the safe way.
                    if (bmp == null && job.Time > 0)
                        bmp = Ff.Grab(job.File, job.Time, job.Graph, job.W, job.H, out _, true);
                    // Near the end of some files there is no frame at the exact time. Step back a little.
                    for (int i = 0; bmp == null && i < 3 && job.Time > 0; i++)
                    {
                        job.Time = Math.Max(0, job.Time - 0.5);
                        bmp = Ff.Grab(job.File, job.Time, job.Graph, job.W, job.H, out _, false, job.MaxLate);
                    }
                }
                catch { /* show nothing new */ }

                try { Done?.Invoke(job, bmp); }
                catch { bmp?.Dispose(); }
            }
        }
    }

    /// <summary>
    /// Plays the video (no sound) at normal speed: ffmpeg decodes with -re and sends raw BGR frames through a pipe.
    /// </summary>
    internal sealed class Player
    {
        private Process _proc;
        private Thread _thread;
        private volatile bool _stopping;
        private volatile int _session; // changes on every Start, so an old read thread can tell it is old

        /// <summary>Background thread: bitmap and frame number since the start position.</summary>
        public event Action<Bitmap, int> Frame;

        /// <summary>Background thread: playback reached the end (or failed). Text is the error, or null.</summary>
        public event Action<string> Finished;

        public bool Running => _proc != null;

        private readonly object _offsetLock = new object();
        private double _offset;

        /// <summary>
        /// Real time of the first frame, in seconds after the seek point (from "showinfo").
        /// Near 0 normally; larger when the file has no index and the seek landed on a later keyframe.
        /// </summary>
        public double Offset { get { lock (_offsetLock) return _offset; } }

        public void Start(string file, double seek, string graph, int w, int h)
        {
            Stop();

            // showinfo logs the time of each frame (log level info). Only the first time is used.
            var sb = new StringBuilder("-hide_banner -loglevel info -nostdin -re ");
            if (seek > 0) sb.Append("-ss ").Append(Ff.N(seek)).Append(' ');
            sb.Append("-i ").Append(Ff.Q(file)).Append(" -an -sn -dn ");
            if (graph != null) sb.Append("-filter_complex ").Append(Ff.Q(Ff.WithFirstFilter(graph, "showinfo"))).Append(" -map [vout] ");
            else sb.Append("-vf showinfo,scale=").Append(w).Append(':').Append(h).Append(' ');
            sb.Append("-f rawvideo -pix_fmt bgr24 pipe:1");

            _stopping = false;
            lock (_offsetLock) _offset = 0;
            var proc = Ff.Start(sb.ToString());
            var err = new StringBuilder();
            bool gotFirst = false;
            proc.ErrorDataReceived += (s, e) =>
            {
                if (e.Data == null) return;
                if (e.Data.Contains("pts_time:"))
                {
                    if (!gotFirst)
                    {
                        gotFirst = true;
                        double v = Ff.PtsTime(e.Data);
                        if (!double.IsNaN(v) && v > 0) lock (_offsetLock) _offset = v;
                    }
                    return; // one line per frame: do not keep them
                }
                lock (err) if (err.Length < 20000) err.AppendLine(e.Data);
            };
            proc.BeginErrorReadLine();

            _proc = proc;
            int session = ++_session;
            _thread = new Thread(() => ReadLoop(proc, w, h, err, session)) { IsBackground = true, Name = "player" };
            _thread.Start();
        }

        private void ReadLoop(Process proc, int w, int h, StringBuilder err, int session)
        {
            int rowBytes = w * 3;
            int frameBytes = rowBytes * h;
            var buf = new byte[frameBytes];
            int index = 0;
            Stream s = proc.StandardOutput.BaseStream;
            try
            {
                while (!_stopping)
                {
                    int got = 0;
                    while (got < frameBytes)
                    {
                        int r = s.Read(buf, got, frameBytes - got);
                        if (r <= 0) break;
                        got += r;
                    }
                    if (got < frameBytes) break; // end of video

                    var bmp = new Bitmap(w, h, PixelFormat.Format24bppRgb);
                    var bd = bmp.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
                    for (int y = 0; y < h; y++)
                        Marshal.Copy(buf, y * rowBytes, bd.Scan0 + y * bd.Stride, rowBytes);
                    bmp.UnlockBits(bd);

                    if (_stopping || session != _session) { bmp.Dispose(); break; }
                    Frame?.Invoke(bmp, index++);
                }
            }
            catch { /* pipe closed by Stop() */ }

            if (!_stopping && session == _session)
            {
                try { proc.WaitForExit(2000); } catch { }
                string text;
                lock (err) text = err.ToString().Trim();
                bool failed = false;
                try { failed = index == 0 && proc.HasExited && proc.ExitCode != 0; } catch { }
                try { Finished?.Invoke(failed ? text : null); } catch { }
            }
        }

        public void Stop()
        {
            _stopping = true;
            var p = _proc;
            _proc = null;
            if (p != null)
            {
                try { if (!p.HasExited) p.Kill(); } catch { }
                try { p.Dispose(); } catch { }
            }
            _thread = null;
        }
    }
}
