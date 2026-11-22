using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Text;

namespace MaskVideo
{
    public enum MaskMode { Blur, Pixelate, Black }

    /// <summary>One area to hide. Coordinates are in video pixels.</summary>
    public sealed class MaskWindow
    {
        public Rectangle Rect;
        public MaskMode Mode = MaskMode.Blur;

        /// <summary>Blur: gaussian sigma. Pixelate: block size in pixels. Not used for Black.</summary>
        public int Strength = 20;

        /// <summary>Start time in seconds.</summary>
        public double Start;

        /// <summary>End time in seconds. PositiveInfinity means "until the end of the video".</summary>
        public double End = double.PositiveInfinity;

        public bool ToEnd => double.IsPositiveInfinity(End);

        public bool ActiveAt(double t) => t >= Start - 0.0005 && (ToEnd || t <= End + 0.0005);

        public MaskWindow Clone() => (MaskWindow)MemberwiseClone();

        public static string ModeName(MaskMode m) => m == MaskMode.Black ? "Black box" : m.ToString();

        public const int MinSize = 8;

        /// <summary>
        /// Keeps the rectangle inside the video and makes all values even.
        /// Even values avoid a 1-pixel shift with 4:2:0 chroma in ffmpeg crop/overlay.
        /// </summary>
        public static Rectangle Fit(Rectangle r, Size video)
        {
            int vw = video.Width, vh = video.Height;
            if (vw < MinSize || vh < MinSize) return r;

            int w = Math.Max(MinSize, Math.Min(r.Width, vw));
            int h = Math.Max(MinSize, Math.Min(r.Height, vh));
            w = (w + 1) & ~1;
            h = (h + 1) & ~1;
            if (w > vw) w = vw & ~1;
            if (h > vh) h = vh & ~1;

            int x = Math.Max(0, Math.Min(r.X, vw - w)) & ~1;
            int y = Math.Max(0, Math.Min(r.Y, vh - h)) & ~1;
            return new Rectangle(x, y, w, h);
        }
    }

    /// <summary>Saves and loads the area list as a small text file, so the same areas can be used on other clips.</summary>
    public static class MaskFile
    {
        private static readonly CultureInfo IC = CultureInfo.InvariantCulture;

        public static void Save(string path, IEnumerable<MaskWindow> list)
        {
            var sb = new StringBuilder();
            sb.AppendLine("# Video Mask areas");
            sb.AppendLine("# mode x y width height strength start end");
            sb.AppendLine("# times are in seconds; end = 'end' means until the end of the video");
            foreach (var w in list)
            {
                sb.Append(w.Mode).Append(' ')
                  .Append(w.Rect.X).Append(' ').Append(w.Rect.Y).Append(' ')
                  .Append(w.Rect.Width).Append(' ').Append(w.Rect.Height).Append(' ')
                  .Append(w.Strength).Append(' ')
                  .Append(w.Start.ToString("0.###", IC)).Append(' ')
                  .Append(w.ToEnd ? "end" : w.End.ToString("0.###", IC))
                  .AppendLine();
            }
            File.WriteAllText(path, sb.ToString());
        }

        public static List<MaskWindow> Load(string path)
        {
            var list = new List<MaskWindow>();
            int lineNo = 0;
            foreach (string raw in File.ReadAllLines(path))
            {
                lineNo++;
                string line = raw.Trim();
                if (line.Length == 0 || line.StartsWith("#")) continue;

                string[] p = line.Split(new[] { ' ', '\t', ',', ';' }, StringSplitOptions.RemoveEmptyEntries);
                if (p.Length < 8) throw new FormatException($"Line {lineNo}: expected 8 values, found {p.Length}.");

                try
                {
                    var w = new MaskWindow
                    {
                        Mode = (MaskMode)Enum.Parse(typeof(MaskMode), p[0], true),
                        Rect = new Rectangle(int.Parse(p[1], IC), int.Parse(p[2], IC), int.Parse(p[3], IC), int.Parse(p[4], IC)),
                        Strength = Math.Max(1, Math.Min(100, int.Parse(p[5], IC))),
                        Start = Math.Max(0, double.Parse(p[6], IC)),
                        End = p[7].Equals("end", StringComparison.OrdinalIgnoreCase)
                            ? double.PositiveInfinity
                            : double.Parse(p[7], IC),
                    };
                    list.Add(w);
                }
                catch (Exception ex)
                {
                    throw new FormatException($"Line {lineNo}: {ex.Message}");
                }
            }
            return list;
        }
    }
}
