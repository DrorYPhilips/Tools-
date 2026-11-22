using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

namespace MaskVideo
{
    /// <summary>Helpers that run ffmpeg.exe. All video reading, filtering and writing is done by ffmpeg.</summary>
    internal static class Ff
    {
        public static string ExePath;

        private static readonly CultureInfo IC = CultureInfo.InvariantCulture;

        /// <summary>Number to text for ffmpeg (always a dot as decimal separator).</summary>
        public static string N(double v) => v.ToString("0.######", IC);

        public static string Q(string path) => "\"" + path + "\"";

        /// <summary>Looks for ffmpeg.exe next to this exe, in a "ffmpeg" sub folder, then on PATH.</summary>
        public static string Find()
        {
            string dir = AppDomain.CurrentDomain.BaseDirectory;
            string[] local =
            {
                Path.Combine(dir, "ffmpeg.exe"),
                Path.Combine(dir, "ffmpeg", "ffmpeg.exe"),
                Path.Combine(dir, "ffmpeg", "bin", "ffmpeg.exe"),
            };
            foreach (string p in local)
                if (File.Exists(p)) return p;

            foreach (string d in (Environment.GetEnvironmentVariable("PATH") ?? "").Split(';'))
            {
                try
                {
                    string dd = d.Trim().Trim('"');
                    if (dd.Length == 0) continue;
                    string p = Path.Combine(dd, "ffmpeg.exe");
                    if (File.Exists(p)) return p;
                }
                catch { /* bad PATH entry */ }
            }
            return null;
        }

        private static readonly Dictionary<string, bool> _encoders = new Dictionary<string, bool>();

        /// <summary>True when this ffmpeg build has the encoder (for example "libx265"). The answer is cached.</summary>
        public static bool HasEncoder(string name)
        {
            lock (_encoders)
            {
                if (_encoders.TryGetValue(name, out bool has)) return has;
                string list = "";
                try { list = Encoding.UTF8.GetString(Run("-hide_banner -nostdin -encoders").Stdout); } catch { /* no list: assume missing */ }
                has = Regex.IsMatch(list, @"\s" + Regex.Escape(name) + @"\s");
                _encoders[name] = has;
                return has;
            }
        }

        public static Process Start(string args)
        {
            var psi = new ProcessStartInfo(ExePath, args)
            {
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                StandardErrorEncoding = Encoding.UTF8,
            };
            return Process.Start(psi);
        }

        public sealed class RunResult
        {
            public int ExitCode;
            public byte[] Stdout;
            public string Stderr;
        }

        /// <summary>Runs ffmpeg to the end and collects stdout (binary) and stderr (text).</summary>
        public static RunResult Run(string args)
        {
            using (var p = Start(args))
            {
                var errTask = p.StandardError.ReadToEndAsync();
                var ms = new MemoryStream();
                p.StandardOutput.BaseStream.CopyTo(ms);
                p.WaitForExit();
                return new RunResult { ExitCode = p.ExitCode, Stdout = ms.ToArray(), Stderr = errTask.Result };
            }
        }

        /// <summary>
        /// Reads one frame at time t (seconds) as a bitmap.
        /// graph: optional filter_complex with output label [vout]. If null, the frame is only scaled to outW x outH
        /// (outW = 0 keeps the full size). Returns null when no frame was decoded (for example past the end).
        /// Fast seek ("-ss" before "-i") is exact in files with an index (MP4, MOV, MKV, AVI ...). In files without
        /// an index (.ts, .mts, .mpg, raw streams) it can give a LATER frame (the next keyframe) or no frame.
        /// maxLate > 0: the real time of the frame is read with "showinfo"; if it is more than maxLate seconds
        /// after t, null is returned, so the caller can use safeSeek.
        /// safeSeek: jump to 20 s before t, then decode up to t. Slower, but exact.
        /// </summary>
        public static Bitmap Grab(string file, double t, string graph, int outW, int outH, out string error,
                                  bool safeSeek = false, double maxLate = 0)
        {
            bool check = !safeSeek && t > 0 && maxLate > 0;
            var sb = new StringBuilder("-hide_banner -nostdin ");
            sb.Append(check ? "-loglevel info " : "-loglevel error ");
            if (safeSeek && t > 0)
            {
                double before = Math.Max(0, t - 20);
                if (before > 0) sb.Append("-ss ").Append(N(before)).Append(' ');
                sb.Append("-i ").Append(Q(file)).Append(" -ss ").Append(N(t - before));
            }
            else
            {
                if (t > 0) sb.Append("-ss ").Append(N(t)).Append(' ');
                sb.Append("-i ").Append(Q(file));
            }
            sb.Append(" -an -sn -dn -frames:v 1 ");
            if (graph != null)
            {
                if (check) graph = WithFirstFilter(graph, "showinfo");
                sb.Append("-filter_complex ").Append(Q(graph)).Append(" -map [vout] ");
            }
            else
            {
                var vf = new List<string>();
                if (check) vf.Add("showinfo");
                if (outW > 0) vf.Add($"scale={outW}:{outH}");
                if (vf.Count > 0) sb.Append("-vf ").Append(string.Join(",", vf)).Append(' ');
            }
            sb.Append("-f image2pipe -c:v bmp -pix_fmt bgr24 pipe:1");

            var r = Run(sb.ToString());
            error = r.Stderr;
            if (r.Stdout.Length < 54) return null; // 54 = BMP header size

            if (check)
            {
                double late = FirstPtsTime(r.Stderr);
                if (late > maxLate) return null; // ffmpeg gave a later frame than asked
            }

            using (var ms = new MemoryStream(r.Stdout))
            using (var tmp = new Bitmap(ms))
                return new Bitmap(tmp); // copy, so the bitmap does not depend on the stream
        }

        /// <summary>Puts a filter at the start of a graph that begins with [0:v].</summary>
        public static string WithFirstFilter(string graph, string filter) =>
            graph.StartsWith("[0:v]") ? "[0:v]" + filter + "," + graph.Substring(5) : graph;

        private static readonly Regex PtsTimeRx = new Regex(@"pts_time:\s*(-?[0-9.]+(?:e-?[0-9]+)?)", RegexOptions.Compiled);

        /// <summary>Time of a "showinfo" log line (seconds after the -ss point), or NaN.</summary>
        public static double PtsTime(string text)
        {
            if (text == null) return double.NaN;
            var m = PtsTimeRx.Match(text);
            return m.Success && double.TryParse(m.Groups[1].Value, NumberStyles.Float, IC, out double v) ? v : double.NaN;
        }

        /// <summary>Like PtsTime, but 0 when there is no value.</summary>
        private static double FirstPtsTime(string text)
        {
            double v = PtsTime(text);
            return double.IsNaN(v) ? 0 : v;
        }

        /// <summary>
        /// Builds the ffmpeg filter graph that hides all areas. Input is [0:v], output is [vout].
        /// timed = false: every window in the list is always on (used for a single preview frame).
        /// timed = true: each window gets enable='...' from its start/end time. shift is subtracted from
        /// the times, because "-ss X" before "-i" makes the filter time start at 0 at position X.
        /// tail: last filter (for example "scale=1280:720"), or null.
        /// </summary>
        public static string BuildGraph(IEnumerable<MaskWindow> windows, bool timed, double shift, double fps, string tail)
        {
            var sb = new StringBuilder();
            string cur = "[0:v]";
            int n = 0;
            // Small time margin, so a frame exactly on a start/end time is not lost to rounding.
            double margin = 0.25 / Math.Max(1.0, fps);

            foreach (var w in windows)
            {
                string en = timed ? EnableOption(w, shift, margin) : "";
                if (en == null) continue; // ends before the shown part of the video

                var r = w.Rect;
                string next = "[v" + n + "]";
                if (sb.Length > 0) sb.Append(';');

                if (w.Mode == MaskMode.Black)
                {
                    sb.Append(cur)
                      .Append($"drawbox=x={r.X}:y={r.Y}:w={r.Width}:h={r.Height}:color=black:t=fill{en}")
                      .Append(next);
                }
                else
                {
                    string effect;
                    if (w.Mode == MaskMode.Blur)
                    {
                        effect = $"gblur=sigma={Math.Max(1, w.Strength)}:steps=2";
                    }
                    else // Pixelate: shrink, then grow back with nearest-neighbour scaling
                    {
                        int block = Math.Max(2, w.Strength);
                        int pw = Math.Max(1, r.Width / block), ph = Math.Max(1, r.Height / block);
                        effect = $"scale={pw}:{ph}:flags=area,scale={r.Width}:{r.Height}:flags=neighbor";
                    }

                    sb.Append(cur).Append($"split[b{n}][c{n}];")
                      .Append($"[c{n}]crop={r.Width}:{r.Height}:{r.X}:{r.Y},{effect}[m{n}];")
                      .Append($"[b{n}][m{n}]overlay={r.X}:{r.Y}{en}")
                      .Append(next);
                }
                cur = next;
                n++;
            }

            if (sb.Length > 0) sb.Append(';');
            sb.Append(cur).Append(string.IsNullOrEmpty(tail) ? "null" : tail).Append("[vout]");
            return sb.ToString();
        }

        /// <summary>"" = always on, ":enable='...'" = on for a time range, null = never on in this run.</summary>
        private static string EnableOption(MaskWindow w, double shift, double margin)
        {
            double s = w.Start - shift - margin;
            if (w.ToEnd)
                return s <= 0 ? "" : $":enable='gte(t,{N(s)})'";

            double e = w.End - shift + margin;
            if (e < 0) return null;
            if (s <= 0) return $":enable='lte(t,{N(e)})'";
            return $":enable='between(t,{N(s)},{N(e)})'";
        }
    }

    /// <summary>Basic facts about a video file, read with "ffmpeg -i" and one decoded frame.</summary>
    internal sealed class VideoInfo
    {
        public string Path;
        public int Width, Height;     // size of decoded frames (after auto-rotation)
        public double Duration;       // seconds, 0 if unknown
        public double Fps = 30;
        public double Sar = 1;        // sample aspect ratio (for non-square pixels)
        public string Codec = "";
        public string PixFmt = "";    // for example yuv420p or yuv420p10le
        public int VideoKbps;         // video bitrate in kbit/s, 0 if unknown
        public bool HasAudio;
        public Bitmap FirstFrame;     // full size

        public double FrameTime => 1.0 / Fps;

        /// <summary>Last time that still has a frame.</summary>
        public double MaxTime => Math.Max(0, Duration - FrameTime);

        public static VideoInfo Probe(string path)
        {
            var info = new VideoInfo { Path = path };
            string text = Ff.Run("-hide_banner -nostdin -i " + Ff.Q(path)).Stderr ?? "";

            var dm = Regex.Match(text, @"Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?)");
            if (dm.Success)
            {
                info.Duration = int.Parse(dm.Groups[1].Value) * 3600
                              + int.Parse(dm.Groups[2].Value) * 60
                              + double.Parse(dm.Groups[3].Value, CultureInfo.InvariantCulture);
            }

            info.HasAudio = Regex.IsMatch(text, @"Stream #\d+:\d+.*?: Audio:");

            int streamW = 0, streamH = 0;
            foreach (Match m in Regex.Matches(text, @"Stream #\d+:\d+[^\r\n]*?: Video: ([^\r\n]*)"))
            {
                string line = m.Groups[1].Value;
                if (line.Contains("attached pic")) continue; // cover image, not the real video

                info.Codec = line.Split(' ', ',')[0];

                var pf = Regex.Match(line, @"\b(yuvj?4[0-4][0-4]p(?:9|10|12|14|16)?(?:le|be)?|nv12|nv21|p010le|gray(?:10|12)?(?:le|be)?)\b");
                if (pf.Success) info.PixFmt = pf.Groups[1].Value;

                var br = Regex.Match(line, @"(\d+) kb/s");
                if (br.Success) info.VideoKbps = int.Parse(br.Groups[1].Value, CultureInfo.InvariantCulture);

                var sm = Regex.Match(line, @", (\d{2,5})x(\d{2,5})[ ,\]]");
                if (!sm.Success) sm = Regex.Match(line, @", (\d{2,5})x(\d{2,5})$");
                if (sm.Success)
                {
                    streamW = int.Parse(sm.Groups[1].Value);
                    streamH = int.Parse(sm.Groups[2].Value);
                }

                var sar = Regex.Match(line, @"SAR (\d+):(\d+)");
                if (sar.Success)
                {
                    double a = double.Parse(sar.Groups[1].Value), b = double.Parse(sar.Groups[2].Value);
                    if (a > 0 && b > 0) info.Sar = a / b;
                }

                var fm = Regex.Match(line, @"([\d.]+)(k?) fps");
                if (!fm.Success) fm = Regex.Match(line, @"([\d.]+)(k?) tbr");
                if (fm.Success && double.TryParse(fm.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out double fps))
                {
                    if (fm.Groups[2].Value == "k") fps *= 1000;
                    if (fps >= 1 && fps <= 1000) info.Fps = fps;
                }
                break;
            }

            if (streamW == 0 && !text.Contains("Video:"))
                throw new Exception("No video stream found in this file.\r\n\r\n" + LastLines(text, 6));

            // Some types (MKV, TS) give no video bitrate: use the total bitrate minus the sound.
            if (info.VideoKbps <= 0)
            {
                var tb = Regex.Match(text, @"Duration:[^\r\n]*?bitrate: (\d+) kb/s");
                if (tb.Success)
                {
                    int audio = 0;
                    foreach (Match am in Regex.Matches(text, @"Stream #\d+:\d+[^\r\n]*?: Audio: [^\r\n]*?(\d+) kb/s"))
                        audio += int.Parse(am.Groups[1].Value, CultureInfo.InvariantCulture);
                    info.VideoKbps = Math.Max(0, int.Parse(tb.Groups[1].Value, CultureInfo.InvariantCulture) - audio);
                }
            }

            // The decoded frame gives the true size (it includes the rotation of phone videos).
            Bitmap first = Ff.Grab(path, 0, null, 0, 0, out string err);
            if (first == null)
                throw new Exception("ffmpeg could not decode a frame from this file.\r\n\r\n" + LastLines(err + "\n" + text, 8));

            info.FirstFrame = first;
            info.Width = first.Width;
            info.Height = first.Height;

            // Apply SAR only when the frame is not rotated (rotated + anamorphic is very rare).
            if (!(streamW == info.Width && streamH == info.Height)) info.Sar = 1;
            if (info.Sar < 0.2 || info.Sar > 5) info.Sar = 1;

            return info;
        }

        public string Describe()
        {
            string s = $"{Width}x{Height}, {Fps:0.##} fps, {Fmt.Time(Duration)}";
            if (Codec.Length > 0) s += ", " + Codec;
            if (PixFmt.Contains("10")) s += " 10-bit";
            if (VideoKbps > 0) s += $", {VideoKbps} kb/s";
            s += HasAudio ? ", with audio" : ", no audio";
            return s;
        }

        public static string LastLines(string text, int count)
        {
            if (string.IsNullOrEmpty(text)) return "";
            string[] lines = text.Replace("\r", "").Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
            int from = Math.Max(0, lines.Length - count);
            return string.Join("\r\n", lines, from, lines.Length - from);
        }
    }

    internal static class Fmt
    {
        public static string Time(double t)
        {
            if (double.IsInfinity(t)) return "end";
            if (t < 0) t = 0;
            var ts = TimeSpan.FromMilliseconds(Math.Round(t * 1000));
            return ts.TotalHours >= 1
                ? $"{(int)ts.TotalHours}:{ts.Minutes:00}:{ts.Seconds:00}.{ts.Milliseconds:000}"
                : $"{ts.Minutes:00}:{ts.Seconds:00}.{ts.Milliseconds:000}";
        }
    }
}
