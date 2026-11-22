using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

namespace MaskVideo
{
    /// <summary>
    /// Shows the current frame and the mask areas on top of it.
    /// Mouse: drag on empty video = new area, drag an area = move, drag a handle = resize.
    /// </summary>
    internal sealed class VideoPanel : Control
    {
        public Bitmap Image { get; private set; }
        public Size VideoSize;
        public double Sar = 1;
        public List<MaskWindow> Windows;
        public MaskWindow Selected { get; private set; }
        public double Time;

        /// <summary>True when the image already has the effects drawn by ffmpeg.</summary>
        public bool ResultShown;

        /// <summary>Makes a new area with the current default settings.</summary>
        public Func<MaskWindow> NewWindow;

        public event EventHandler SelectionChanged;
        public event EventHandler AreaChanging; // while the mouse drags
        public event EventHandler AreaChanged;  // when the mouse is released

        private enum DragMode { None, Create, Move, Resize }

        private DragMode _drag;
        private int _handle;
        private Point _downVideo, _downScreen;
        private Rectangle _orig;
        private MaskWindow _creating;

        public VideoPanel()
        {
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer |
                     ControlStyles.UserPaint | ControlStyles.ResizeRedraw | ControlStyles.Selectable, true);
            BackColor = Color.FromArgb(32, 32, 32);
            ForeColor = Color.Gainsboro;
        }

        private float S => DeviceDpi / 96f;

        public void SetImage(Bitmap bmp)
        {
            var old = Image;
            Image = bmp;
            if (old != null && !ReferenceEquals(old, bmp)) old.Dispose();
            Invalidate();
        }

        public void SelectArea(MaskWindow w, bool raise = true)
        {
            if (ReferenceEquals(Selected, w)) return;
            Selected = w;
            Invalidate();
            if (raise) SelectionChanged?.Invoke(this, EventArgs.Empty);
        }

        // ---------- coordinates ----------

        private RectangleF ImageRect()
        {
            if (VideoSize.Width <= 0 || VideoSize.Height <= 0) return RectangleF.Empty;
            float dw = (float)(VideoSize.Width * Sar), dh = VideoSize.Height;
            float pad = 6 * S;
            float cw = Width - 2 * pad, ch = Height - 2 * pad;
            if (cw < 2 || ch < 2) return RectangleF.Empty;
            float k = Math.Min(cw / dw, ch / dh);
            float w = dw * k, h = dh * k;
            return new RectangleF((Width - w) / 2f, (Height - h) / 2f, w, h);
        }

        private RectangleF ToScreen(Rectangle r)
        {
            var ir = ImageRect();
            float kx = ir.Width / VideoSize.Width, ky = ir.Height / VideoSize.Height;
            return new RectangleF(ir.X + r.X * kx, ir.Y + r.Y * ky, r.Width * kx, r.Height * ky);
        }

        private Point ToVideo(Point p)
        {
            var ir = ImageRect();
            if (ir.Width <= 0) return Point.Empty;
            int x = (int)Math.Round((p.X - ir.X) * VideoSize.Width / ir.Width);
            int y = (int)Math.Round((p.Y - ir.Y) * VideoSize.Height / ir.Height);
            return new Point(Math.Max(0, Math.Min(VideoSize.Width, x)), Math.Max(0, Math.Min(VideoSize.Height, y)));
        }

        private static PointF[] HandlePoints(RectangleF r)
        {
            float cx = r.X + r.Width / 2, cy = r.Y + r.Height / 2;
            // 0=top-left 1=top 2=top-right 3=right 4=bottom-right 5=bottom 6=bottom-left 7=left
            return new[]
            {
                new PointF(r.Left, r.Top), new PointF(cx, r.Top), new PointF(r.Right, r.Top), new PointF(r.Right, cy),
                new PointF(r.Right, r.Bottom), new PointF(cx, r.Bottom), new PointF(r.Left, r.Bottom), new PointF(r.Left, cy),
            };
        }

        private int HitHandle(MaskWindow w, Point p)
        {
            if (w == null) return -1;
            float hs = 7 * S;
            var pts = HandlePoints(ToScreen(w.Rect));
            for (int i = 0; i < pts.Length; i++)
                if (Math.Abs(p.X - pts[i].X) <= hs && Math.Abs(p.Y - pts[i].Y) <= hs) return i;
            return -1;
        }

        private MaskWindow HitWindow(Point p)
        {
            if (Windows == null) return null;
            for (int i = Windows.Count - 1; i >= 0; i--) // top-most first
            {
                var r = ToScreen(Windows[i].Rect);
                r.Inflate(2, 2);
                if (r.Contains(p)) return Windows[i];
            }
            return null;
        }

        // ---------- mouse ----------

        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown(e);
            Focus();
            if (Windows == null || VideoSize.IsEmpty || Image == null) return;

            if (e.Button == MouseButtons.Right)
            {
                // The context menu works on the area under the mouse.
                SelectArea(HitWindow(e.Location));
                return;
            }
            if (e.Button != MouseButtons.Left) return;

            _downVideo = ToVideo(e.Location);
            _downScreen = e.Location;

            int h = HitHandle(Selected, e.Location);
            if (h >= 0)
            {
                _drag = DragMode.Resize;
                _handle = h;
                _orig = Selected.Rect;
                return;
            }

            var hit = HitWindow(e.Location);
            if (hit != null)
            {
                SelectArea(hit);
                _drag = DragMode.Move;
                _orig = hit.Rect;
                return;
            }

            SelectArea(null);
            if (ImageRect().Contains(e.Location))
            {
                _drag = DragMode.Create;
                _creating = null;
            }
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            if (_drag == DragMode.None)
            {
                UpdateCursor(e.Location);
                return;
            }

            Point v = ToVideo(e.Location);
            int dx = v.X - _downVideo.X, dy = v.Y - _downVideo.Y;

            switch (_drag)
            {
                case DragMode.Move:
                    Selected.Rect = MaskWindow.Fit(new Rectangle(_orig.X + dx, _orig.Y + dy, _orig.Width, _orig.Height), VideoSize);
                    break;

                case DragMode.Resize:
                    Selected.Rect = ResizeRect(_orig, _handle, dx, dy);
                    break;

                case DragMode.Create:
                    if (_creating == null)
                    {
                        // Wait for a real drag, so a simple click does not make a tiny area.
                        if (Math.Abs(e.X - _downScreen.X) < 5 * S && Math.Abs(e.Y - _downScreen.Y) < 5 * S) return;
                        _creating = NewWindow != null ? NewWindow() : new MaskWindow();
                        Windows.Add(_creating);
                        SelectArea(_creating);
                    }
                    _creating.Rect = MaskWindow.Fit(Rectangle.FromLTRB(
                        Math.Min(_downVideo.X, v.X), Math.Min(_downVideo.Y, v.Y),
                        Math.Max(_downVideo.X, v.X), Math.Max(_downVideo.Y, v.Y)), VideoSize);
                    break;
            }
            AreaChanging?.Invoke(this, EventArgs.Empty);
            Invalidate();
        }

        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp(e);
            if (_drag == DragMode.None) return;

            bool changed = _drag == DragMode.Create
                ? _creating != null
                : Selected != null && Selected.Rect != _orig;

            _drag = DragMode.None;
            _creating = null;
            UpdateCursor(e.Location);
            if (changed) AreaChanged?.Invoke(this, EventArgs.Empty);
        }

        private Rectangle ResizeRect(Rectangle o, int h, int dx, int dy)
        {
            int l = o.Left, t = o.Top, r = o.Right, b = o.Bottom;
            if (h == 0 || h == 6 || h == 7) l += dx;
            if (h == 2 || h == 3 || h == 4) r += dx;
            if (h == 0 || h == 1 || h == 2) t += dy;
            if (h == 4 || h == 5 || h == 6) b += dy;

            int W = VideoSize.Width, H = VideoSize.Height;
            l = Math.Max(0, Math.Min(W, l)); r = Math.Max(0, Math.Min(W, r));
            t = Math.Max(0, Math.Min(H, t)); b = Math.Max(0, Math.Min(H, b));

            return MaskWindow.Fit(Rectangle.FromLTRB(Math.Min(l, r), Math.Min(t, b), Math.Max(l, r), Math.Max(t, b)), VideoSize);
        }

        private void UpdateCursor(Point p)
        {
            Cursor c = Cursors.Default;
            if (Image != null && Windows != null)
            {
                int h = HitHandle(Selected, p);
                if (h == 0 || h == 4) c = Cursors.SizeNWSE;
                else if (h == 2 || h == 6) c = Cursors.SizeNESW;
                else if (h == 1 || h == 5) c = Cursors.SizeNS;
                else if (h == 3 || h == 7) c = Cursors.SizeWE;
                else if (HitWindow(p) != null) c = Cursors.SizeAll;
                else if (ImageRect().Contains(p)) c = Cursors.Cross;
            }
            if (Cursor != c) Cursor = c;
        }

        // ---------- drawing ----------

        public static Color ModeColor(MaskMode m)
        {
            switch (m)
            {
                case MaskMode.Pixelate: return Color.FromArgb(255, 200, 0);
                case MaskMode.Black: return Color.FromArgb(255, 90, 90);
                default: return Color.FromArgb(0, 190, 255);
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            var g = e.Graphics;
            g.Clear(BackColor);
            var ir = ImageRect();

            if (Image == null || ir.IsEmpty)
            {
                TextRenderer.DrawText(g, "Open a video file, or drag it here.", Font, ClientRectangle, Color.Gray,
                    TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter);
                return;
            }

            g.InterpolationMode = InterpolationMode.Bilinear;
            g.PixelOffsetMode = PixelOffsetMode.Half;
            g.DrawImage(Image, ir);
            g.PixelOffsetMode = PixelOffsetMode.Default;

            if (Windows == null) return;

            if (Windows.Count == 0)
            {
                DrawLabel(g, "Drag on the video to add an area to hide", new PointF(ir.X + 8 * S, ir.Y + 8 * S), Color.White);
                return;
            }

            for (int i = 0; i < Windows.Count; i++)
            {
                var w = Windows[i];
                var r = ToScreen(w.Rect);
                bool active = w.ActiveAt(Time);
                bool sel = ReferenceEquals(w, Selected);
                Color c = ModeColor(w.Mode);

                if (active)
                {
                    if (!ResultShown)
                    {
                        // Simple hint of the effect when the real result is not shown.
                        Color fill = w.Mode == MaskMode.Black ? Color.FromArgb(200, 0, 0, 0) : Color.FromArgb(70, c);
                        using (var br = new SolidBrush(fill)) g.FillRectangle(br, r);
                    }
                    using (var pen = new Pen(c, (sel ? 3f : 1.5f) * S))
                        g.DrawRectangle(pen, r.X, r.Y, r.Width, r.Height);
                }
                else
                {
                    using (var pen = new Pen(Color.FromArgb(200, 210, 210, 210), (sel ? 2f : 1f) * S) { DashStyle = DashStyle.Dash })
                        g.DrawRectangle(pen, r.X, r.Y, r.Width, r.Height);
                }

                string text = $"{i + 1} {MaskWindow.ModeName(w.Mode)}" + (active ? "" : " (off now)");
                DrawLabel(g, text, new PointF(r.X + 2 * S, r.Y + 2 * S), active ? c : Color.Silver);

                if (sel)
                {
                    float hs = 4 * S;
                    foreach (var p in HandlePoints(r))
                    {
                        var hr = new RectangleF(p.X - hs, p.Y - hs, hs * 2, hs * 2);
                        g.FillRectangle(Brushes.White, hr);
                        g.DrawRectangle(Pens.Black, hr.X, hr.Y, hr.Width, hr.Height);
                    }
                }
            }
        }

        private void DrawLabel(Graphics g, string text, PointF at, Color color)
        {
            Size sz = TextRenderer.MeasureText(g, text, Font, Size.Empty, TextFormatFlags.NoPadding);
            var box = new Rectangle((int)at.X, (int)at.Y, sz.Width + (int)(6 * S), sz.Height + (int)(2 * S));
            using (var br = new SolidBrush(Color.FromArgb(170, 0, 0, 0))) g.FillRectangle(br, box);
            TextRenderer.DrawText(g, text, Font, box, color,
                TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter | TextFormatFlags.NoPadding);
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing) Image?.Dispose();
            base.Dispose(disposing);
        }
    }
}
