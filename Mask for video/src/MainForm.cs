using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Threading;
using System.Windows.Forms;

namespace MaskVideo
{
    internal sealed class MainForm : Form
    {
        private const string DownloadPage = "https://www.gyan.dev/ffmpeg/builds/";

        // ---------- state ----------
        private VideoInfo _info;
        private readonly List<MaskWindow> _windows = new List<MaskWindow>();
        private double _time;
        private Size _preview;
        private readonly FrameFetcher _fetcher = new FrameFetcher();
        private readonly Player _player = new Player();
        private double _playSeek;         // the -ss value of the running playback
        private bool _ui;                 // true while code fills the controls; change events are ignored
        private MaskMode _defaultMode = MaskMode.Blur;
        private int _defaultStrength = 20;
        private readonly string _initialFile;
        private readonly float _s;        // DPI scale

        // Hand-off of frames from the player thread to the UI thread (only the newest frame is kept).
        private readonly object _frameLock = new object();
        private Bitmap _latestFrame;
        private int _latestIndex;
        private int _framePosted;

        // ---------- controls ----------
        private ToolStripButton _btnExport, _btnResult, _btnMute;
        private ToolStripComboBox _quality;
        private VideoPanel _panel;
        private Button _btnPlay;
        private TrackBar _track;
        private Label _timeLabel;
        private ListView _list;
        private GroupBox _props;
        private ComboBox _mode;
        private Label _strengthLabel;
        private NumericUpDown _strength, _x, _y, _w, _h, _start, _end;
        private ToolStripStatusLabel _statusText;
        private readonly ToolTip _tips = new ToolTip();

        public MainForm(string file)
        {
            _initialFile = file;
            _s = DeviceDpi / 96f;

            SuspendLayout();
            AutoScaleMode = AutoScaleMode.None; // sizes below are scaled by hand with D()
            Font = SystemFonts.MessageBoxFont;
            Text = "Video Mask";
            ClientSize = new Size(D(1220), D(780));
            MinimumSize = new Size(D(820), D(520));
            StartPosition = FormStartPosition.CenterScreen;
            AllowDrop = true;
            _ui = true; // building controls fires ValueChanged events; ignore them

            // The panel is made first: the change handlers of the other controls use it.
            _panel = new VideoPanel { Dock = DockStyle.Fill, Windows = _windows, NewWindow = MakeNewWindow, Font = Font };
            _panel.SelectionChanged += (s, e) => { SyncListSelection(); FillProps(); };
            _panel.AreaChanging += (s, e) => FillProps();
            _panel.AreaChanged += (s, e) => AreasEdited();
            _panel.ContextMenuStrip = BuildContextMenu();
            _panel.AllowDrop = true;

            var tool = BuildToolbar();
            var right = BuildRightPanel();
            var bottom = BuildBottomBar();
            var status = new StatusStrip { SizingGrip = true };
            _statusText = new ToolStripStatusLabel("No video") { Spring = true, TextAlign = ContentAlignment.MiddleLeft };
            status.Items.Add(_statusText);

            // Docking order: the control added last is docked first.
            Controls.Add(_panel);
            Controls.Add(bottom);
            Controls.Add(right);
            Controls.Add(status);
            Controls.Add(tool);

            ResumeLayout(false);
            PerformLayout();

            _fetcher.Done += OnFetcherDone;
            _player.Frame += OnPlayerFrame;
            _player.Finished += err => SafeInvoke(() => OnPlayerFinished(err));

            DragEnter += OnDragEnter;
            DragDrop += OnDragDrop;
            _panel.DragEnter += OnDragEnter;
            _panel.DragDrop += OnDragDrop;

            _ui = false;
            FillProps();
        }

        private int D(int v) => (int)Math.Round(v * _s);

        // =====================================================================
        // UI building
        // =====================================================================

        private ToolStrip BuildToolbar()
        {
            var t = new ToolStrip
            {
                GripStyle = ToolStripGripStyle.Hidden,
                Padding = new Padding(D(4), D(2), D(4), D(2)),
                Font = Font,
            };

            ToolStripButton Btn(string text, string tip, EventHandler click)
            {
                var b = new ToolStripButton(text) { DisplayStyle = ToolStripItemDisplayStyle.Text, ToolTipText = tip };
                b.Click += click;
                t.Items.Add(b);
                return b;
            }

            Btn("Open video...", "Open a video file (Ctrl+O)", (s, e) => OpenDialog());
            _btnExport = Btn("Export video...", "Write a new video with the areas hidden (Ctrl+E)", (s, e) => Export());
            _btnExport.Enabled = false;
            _btnExport.Font = new Font(Font, FontStyle.Bold);
            t.Items.Add(new ToolStripSeparator());
            Btn("Load areas...", "Load a saved list of areas", (s, e) => LoadAreas());
            Btn("Save areas...", "Save the list of areas, to use it again on another clip", (s, e) => SaveAreas());
            t.Items.Add(new ToolStripSeparator());

            _btnResult = Btn("Show result", "On: the preview shows the real effect made by ffmpeg.\nOff: faster preview with colored boxes.", (s, e) => { });
            _btnResult.CheckOnClick = true;
            _btnResult.Checked = true;
            _btnResult.CheckedChanged += (s, e) => OnResultToggled();

            t.Items.Add(new ToolStripSeparator());
            t.Items.Add(new ToolStripLabel("Export quality:"));
            _quality = new ToolStripComboBox { DropDownStyle = ComboBoxStyle.DropDownList, AutoSize = false, Width = D(135) };
            _quality.Items.AddRange(new object[] { "Same as original", "High", "Normal", "Small file" });
            _quality.SelectedIndex = 0;
            _quality.ToolTipText =
                "Same as original: the same codec (H.264 or HEVC), bit depth and bitrate as the original,\n" +
                "so the file has about the same size and quality.\n" +
                "High / Normal / Small file: H.264 at a fixed quality level.\n" +
                "The size and the frame rate never change. The sound is copied when possible.";
            t.Items.Add(_quality);

            t.Items.Add(new ToolStripSeparator());
            _btnMute = Btn("Mute", "On: the exported file has no sound.\nWith no areas, the video is copied without change (fast, same quality).", (s, e) => { });
            _btnMute.CheckOnClick = true;

            var help = new ToolStripButton("Help")
            {
                Alignment = ToolStripItemAlignment.Right,
                DisplayStyle = ToolStripItemDisplayStyle.Text,
                ToolTipText = "Open the user manual (F1)",
            };
            help.Click += (s, e) => OpenManual();
            t.Items.Add(help);
            return t;
        }

        private Control BuildBottomBar()
        {
            var bar = new TableLayoutPanel
            {
                Dock = DockStyle.Bottom,
                Height = D(46),
                ColumnCount = 5,
                RowCount = 1,
                Padding = new Padding(D(4), D(4), D(4), D(2)),
            };
            bar.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            bar.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            bar.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            bar.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            bar.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            bar.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

            Button Btn(string text, string tip, EventHandler click, int minWidth)
            {
                var b = new Button
                {
                    Text = text,
                    AutoSize = true,
                    MinimumSize = new Size(D(minWidth), D(30)),
                    Anchor = AnchorStyles.Left,
                    UseVisualStyleBackColor = true,
                };
                b.Click += click;
                _tips.SetToolTip(b, tip);
                return b;
            }

            _btnPlay = Btn("Play", "Play / pause (Space). Plays without sound.", (s, e) => TogglePlay(), 80);
            var prev = Btn("◀", "Previous frame (Left arrow). Shift+Left = 1 second back.", (s, e) => Step(-1), 36);
            var next = Btn("▶", "Next frame (Right arrow). Shift+Right = 1 second forward.", (s, e) => Step(1), 36);

            _track = new TrackBar
            {
                Dock = DockStyle.Fill,
                AutoSize = false,
                TickStyle = TickStyle.None,
                Minimum = 0,
                Maximum = 1,
                Enabled = false,
            };
            _track.Scroll += (s, e) =>
            {
                if (_player.Running) StopPlay(false);
                SetTime(_track.Value / 1000.0, true);
            };

            _timeLabel = new Label
            {
                AutoSize = true,
                Anchor = AnchorStyles.Left,
                Font = new Font("Consolas", Font.SizeInPoints + 1),
                Text = "00:00.000 / 00:00.000",
                Margin = new Padding(D(6), 0, D(4), 0),
            };

            bar.Controls.Add(_btnPlay, 0, 0);
            bar.Controls.Add(prev, 1, 0);
            bar.Controls.Add(next, 2, 0);
            bar.Controls.Add(_track, 3, 0);
            bar.Controls.Add(_timeLabel, 4, 0);
            return bar;
        }

        private Control BuildRightPanel()
        {
            var outer = new Panel { Dock = DockStyle.Right, Width = D(370), Padding = new Padding(D(6)) };

            var layout = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 4 };
            layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));

            var title = new Label
            {
                Text = "Areas to hide",
                AutoSize = true,
                Font = new Font(Font, FontStyle.Bold),
                Margin = new Padding(0, 0, 0, D(4)),
            };

            _list = new ListView
            {
                Dock = DockStyle.Fill,
                View = View.Details,
                FullRowSelect = true,
                HideSelection = false,
                MultiSelect = false,
                GridLines = true,
                MinimumSize = new Size(0, D(120)),
            };
            _list.Columns.Add("#", D(30));
            _list.Columns.Add("Effect", D(90));
            _list.Columns.Add("Area (x,y w x h)", D(115));
            _list.Columns.Add("Time", D(105));
            _list.SelectedIndexChanged += (s, e) =>
            {
                if (_ui) return;
                var w = _list.SelectedItems.Count > 0 ? (MaskWindow)_list.SelectedItems[0].Tag : null;
                _panel.SelectArea(w, false);
                FillProps();
            };
            _list.DoubleClick += (s, e) =>
            {
                // Jump to the start of the area.
                var w = _panel.Selected;
                if (w != null) { StopPlay(false); SetTime(w.Start); }
            };
            _tips.SetToolTip(_list, "Double-click an area to go to its start time.");

            var buttons = new FlowLayoutPanel { AutoSize = true, Dock = DockStyle.Fill, Margin = new Padding(0, D(4), 0, D(4)), WrapContents = true };
            Button Btn(string text, EventHandler click)
            {
                var b = new Button { Text = text, AutoSize = true, MinimumSize = new Size(D(80), D(28)), UseVisualStyleBackColor = true };
                b.Click += click;
                buttons.Controls.Add(b);
                return b;
            }
            Btn("Duplicate", (s, e) => DuplicateSelected());
            Btn("Delete", (s, e) => DeleteSelected());
            Btn("Delete all", (s, e) => DeleteAll());

            layout.Controls.Add(title, 0, 0);
            layout.Controls.Add(_list, 0, 1);
            layout.Controls.Add(buttons, 0, 2);
            layout.Controls.Add(BuildProps(), 0, 3);
            outer.Controls.Add(layout);
            return outer;
        }

        private Control BuildProps()
        {
            _props = new GroupBox
            {
                Text = "Selected area",
                Dock = DockStyle.Fill,
                AutoSize = true,
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                Padding = new Padding(D(6), D(4), D(6), D(6)),
            };

            var t = new TableLayoutPanel { Dock = DockStyle.Top, AutoSize = true, ColumnCount = 3 };
            t.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            t.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            t.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));

            int row = 0;
            Label Lbl(string text) => new Label { Text = text, AutoSize = true, Anchor = AnchorStyles.Left, Margin = new Padding(0, D(6), D(6), D(4)) };

            void AddRow(Control label, Control field, Control extra = null)
            {
                t.RowStyles.Add(new RowStyle(SizeType.AutoSize));
                t.Controls.Add(label, 0, row);
                field.Anchor = AnchorStyles.Left | AnchorStyles.Right;
                t.Controls.Add(field, 1, row);
                if (extra != null) t.Controls.Add(extra, 2, row);
                else t.SetColumnSpan(field, 2);
                row++;
            }

            NumericUpDown Num(int decimals, decimal max, decimal inc, EventHandler changed)
            {
                var n = new NumericUpDown { DecimalPlaces = decimals, Minimum = 0, Maximum = max, Increment = inc, TextAlign = HorizontalAlignment.Right };
                n.ValueChanged += changed;
                return n;
            }

            Button SmallBtn(string text, string tip, EventHandler click)
            {
                var b = new Button { Text = text, AutoSize = true, MinimumSize = new Size(D(60), D(26)), Anchor = AnchorStyles.Left, UseVisualStyleBackColor = true };
                b.Click += click;
                _tips.SetToolTip(b, tip);
                return b;
            }

            _mode = new ComboBox { DropDownStyle = ComboBoxStyle.DropDownList };
            _mode.Items.AddRange(new object[] { "Blur", "Pixelate", "Black box" });
            _mode.SelectedIndexChanged += (s, e) => OnModeChanged();
            AddRow(Lbl("Effect"), _mode);

            _strengthLabel = Lbl("Blur strength");
            _strength = Num(0, 100, 1, (s, e) => OnStrengthChanged());
            _strength.Minimum = 1;
            AddRow(_strengthLabel, _strength);

            _x = Num(0, 10000, 2, (s, e) => OnRectChanged());
            _y = Num(0, 10000, 2, (s, e) => OnRectChanged());
            _w = Num(0, 10000, 2, (s, e) => OnRectChanged());
            _h = Num(0, 10000, 2, (s, e) => OnRectChanged());
            _w.Minimum = MaskWindow.MinSize;
            _h.Minimum = MaskWindow.MinSize;
            AddRow(Lbl("X (left)"), _x);
            AddRow(Lbl("Y (top)"), _y);
            AddRow(Lbl("Width"), _w);
            AddRow(Lbl("Height"), _h);

            _start = Num(3, 1000000, 0.1m, (s, e) => OnStartChanged());
            _end = Num(3, 1000000, 0.1m, (s, e) => OnEndChanged());
            AddRow(Lbl("Start (sec)"), _start, SmallBtn("Now", "Start at the current frame", (s, e) => SetStartNow()));
            AddRow(Lbl("End (sec)"), _end, SmallBtn("Now", "End at the current frame", (s, e) => SetEndNow()));

            var flow = new FlowLayoutPanel { AutoSize = true, Dock = DockStyle.Fill, Margin = new Padding(0, D(4), 0, 0) };
            flow.Controls.Add(SmallBtn("Whole video", "Show this area from the start to the end", (s, e) => SetWholeVideo()));
            flow.Controls.Add(SmallBtn("Go to start", "Move to the start time of this area", (s, e) => { if (_panel.Selected != null) { StopPlay(false); SetTime(_panel.Selected.Start); } }));
            flow.Controls.Add(SmallBtn("Go to end", "Move to the end time of this area", (s, e) => { if (_panel.Selected != null) { StopPlay(false); SetTime(_panel.Selected.ToEnd ? double.MaxValue : _panel.Selected.End); } }));
            t.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            t.Controls.Add(flow, 0, row);
            t.SetColumnSpan(flow, 3);
            row++;

            var note = new Label
            {
                Text = "End = video length means \"until the end\".",
                AutoSize = true,
                ForeColor = SystemColors.GrayText,
                Margin = new Padding(0, D(4), 0, 0),
            };
            t.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            t.Controls.Add(note, 0, row);
            t.SetColumnSpan(note, 3);

            _props.Controls.Add(t);
            return _props;
        }

        private ContextMenuStrip BuildContextMenu()
        {
            var m = new ContextMenuStrip();
            var dup = m.Items.Add("Duplicate", null, (s, e) => DuplicateSelected());
            var del = m.Items.Add("Delete", null, (s, e) => DeleteSelected());
            m.Items.Add(new ToolStripSeparator());
            var st = m.Items.Add("Start at current frame", null, (s, e) => SetStartNow());
            var en = m.Items.Add("End at current frame", null, (s, e) => SetEndNow());
            var wh = m.Items.Add("Whole video", null, (s, e) => SetWholeVideo());
            m.Opening += (s, e) =>
            {
                bool has = _panel.Selected != null;
                foreach (var it in new[] { dup, del, st, en, wh }) it.Enabled = has;
                if (!has) e.Cancel = true;
            };
            return m;
        }

        // =====================================================================
        // Open / ffmpeg
        // =====================================================================

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            Ff.ExePath = Ff.Find();
            if (Ff.ExePath == null) EnsureFfmpeg();
            else if (!string.IsNullOrEmpty(_initialFile)) OpenVideo(_initialFile);
        }

        private bool EnsureFfmpeg()
        {
            if (Ff.ExePath != null && File.Exists(Ff.ExePath)) return true;
            Ff.ExePath = Ff.Find();
            if (Ff.ExePath != null) return true;

            var r = MessageBox.Show(this,
                "ffmpeg.exe was not found.\r\n\r\n" +
                "This tool uses ffmpeg to read and write video. Put ffmpeg.exe in the same folder as MaskVideo.exe:\r\n" +
                AppDomain.CurrentDomain.BaseDirectory + "\r\n\r\n" +
                "Open the ffmpeg download page now?\r\n(Download \"ffmpeg-release-essentials.zip\" and copy bin\\ffmpeg.exe.)",
                "ffmpeg not found", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
            if (r == DialogResult.Yes)
            {
                try { Process.Start(DownloadPage); } catch { /* no browser */ }
            }
            return false;
        }

        private void OpenDialog()
        {
            using (var d = new OpenFileDialog
            {
                Title = "Open video",
                Filter = "Video files|*.mp4;*.m4v;*.mov;*.mkv;*.avi;*.wmv;*.webm;*.flv;*.mpg;*.mpeg;*.ts;*.mts;*.m2ts;*.3gp;*.3g2;*.vob;*.ogv;*.mxf;*.dv;*.gif;*.asf;*.rm;*.rmvb;*.divx;*.f4v;*.h264;*.h265;*.hevc|All files (*.*)|*.*",
            })
            {
                if (_info != null) d.InitialDirectory = Path.GetDirectoryName(_info.Path);
                if (d.ShowDialog(this) == DialogResult.OK) OpenVideo(d.FileName);
            }
        }

        private void OpenVideo(string path)
        {
            if (!EnsureFfmpeg()) return;
            if (!File.Exists(path))
            {
                MessageBox.Show(this, "File not found:\r\n" + path, "Open video", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            StopPlay(false);
            VideoInfo info;
            Cursor = Cursors.WaitCursor;
            _statusText.Text = "Reading " + Path.GetFileName(path) + " ...";
            Update();
            try
            {
                info = VideoInfo.Probe(path);
            }
            catch (Exception ex)
            {
                _statusText.Text = _info != null ? _info.Describe() : "No video";
                MessageBox.Show(this, "Could not open this video.\r\n\r\n" + ex.Message, "Open video", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            finally
            {
                Cursor = Cursors.Default;
            }

            var size = new Size(info.Width, info.Height);
            if (_windows.Count > 0)
            {
                var r = MessageBox.Show(this, $"Keep the {_windows.Count} current area(s) for the new video?",
                    "Open video", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (r == DialogResult.No) _windows.Clear();
                foreach (var w in _windows) w.Rect = MaskWindow.Fit(w.Rect, size);
            }

            _info = info;
            _preview = PreviewSize(info.Width, info.Height);
            _panel.SelectArea(null, false);
            _panel.VideoSize = size;
            _panel.Sar = info.Sar;
            _panel.ResultShown = false;
            _panel.SetImage(info.FirstFrame);
            info.FirstFrame = null; // the panel owns it now

            _track.Maximum = Math.Max(1, (int)Math.Round(info.MaxTime * 1000));
            _track.SmallChange = Math.Max(1, (int)Math.Round(info.FrameTime * 1000));
            _track.LargeChange = Math.Max(1, _track.Maximum / 20);
            _track.Value = 0;
            _track.Enabled = info.Duration > 0;
            _btnExport.Enabled = true;

            Text = "Video Mask - " + Path.GetFileName(path);
            _statusText.Text = Path.GetFileName(path) + "   |   " + info.Describe();

            RefreshList();
            FillProps();
            SetTime(0);
        }

        /// <summary>Preview frames are made smaller than the video to keep seeking fast.</summary>
        private static Size PreviewSize(int w, int h)
        {
            const int box = 1280;
            double k = Math.Min(1.0, (double)box / Math.Max(w, h));
            int pw = Math.Max(2, (int)Math.Round(w * k / 2) * 2);
            int ph = Math.Max(2, (int)Math.Round(h * k / 2) * 2);
            return new Size(pw, ph);
        }

        private void OnDragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop)) e.Effect = DragDropEffects.Copy;
        }

        private void OnDragDrop(object sender, DragEventArgs e)
        {
            if (!(e.Data.GetData(DataFormats.FileDrop) is string[] files) || files.Length == 0) return;
            string f = files[0];
            // Open after the drop finishes, so Explorer is not blocked while the video is read.
            BeginInvoke(new Action(() =>
            {
                if (Path.GetExtension(f).Equals(".masks", StringComparison.OrdinalIgnoreCase)) LoadAreasFrom(f);
                else OpenVideo(f);
            }));
        }

        // =====================================================================
        // Time, preview frames and playback
        // =====================================================================

        private double SeekTime(double t) => t <= 0 ? 0 : Math.Max(0, t - 0.25 * _info.FrameTime);

        private void SetTime(double t, bool fromTrack = false)
        {
            if (_info == null) return;
            double fps = _info.Fps;
            t = Math.Max(0, Math.Min(_info.MaxTime, t));
            t = Math.Round(t * fps) / fps; // snap to the frame grid
            if (t > _info.MaxTime) t = Math.Floor(_info.MaxTime * fps) / fps;

            _time = t;
            _panel.Time = t;
            UpdateTimeUi(fromTrack);
            _panel.Invalidate();
            RequestFrame();
        }

        private void UpdateTimeUi(bool fromTrack)
        {
            if (_info == null) return;
            if (!fromTrack)
            {
                int v = (int)Math.Round(_time * 1000);
                _track.Value = Math.Max(_track.Minimum, Math.Min(_track.Maximum, v));
            }
            _timeLabel.Text = Fmt.Time(_time) + " / " + Fmt.Time(_info.Duration);
        }

        private void Step(int frames)
        {
            if (_info == null) return;
            StopPlay(false);
            SetTime(_time + frames * _info.FrameTime);
        }

        private string PreviewTail => $"scale={_preview.Width}:{_preview.Height}";

        private void RequestFrame()
        {
            if (_info == null) return;
            var job = new FrameJob
            {
                File = _info.Path,
                Time = SeekTime(_time),
                W = _preview.Width,
                H = _preview.Height,
                Result = _btnResult.Checked,
                // The wanted frame is 0.25 frame after the seek point (see SeekTime), the next one 1.25.
                MaxLate = 0.75 * _info.FrameTime,
            };
            if (job.Result)
            {
                var active = _windows.Where(w => w.ActiveAt(_time)).ToList();
                if (active.Count > 0) job.Graph = Ff.BuildGraph(active, false, 0, _info.Fps, PreviewTail);
            }
            _fetcher.Request(job);
        }

        private void OnFetcherDone(FrameJob job, Bitmap bmp)
        {
            // Background thread.
            if (!SafeInvoke(() =>
            {
                if (bmp == null) return;
                if (_info == null || job.File != _info.Path || _player.Running)
                {
                    bmp.Dispose();
                    return;
                }
                _panel.ResultShown = job.Result;
                _panel.SetImage(bmp);
            }))
            {
                bmp?.Dispose();
            }
        }

        private void TogglePlay()
        {
            if (_player.Running) StopPlay(true);
            else StartPlay();
        }

        private void StartPlay()
        {
            if (_info == null || !EnsureFfmpeg()) return;
            if (_time >= _info.MaxTime - _info.FrameTime) SetTime(0);

            double seek = SeekTime(_time);
            _playSeek = seek;
            string graph = null;
            if (_btnResult.Checked && _windows.Count > 0)
                graph = Ff.BuildGraph(_windows, true, seek, _info.Fps, PreviewTail);

            _panel.ResultShown = _btnResult.Checked;
            try
            {
                _player.Start(_info.Path, seek, graph, _preview.Width, _preview.Height);
            }
            catch (Exception ex)
            {
                MessageBox.Show(this, "Could not start playback.\r\n\r\n" + ex.Message, "Play", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            _btnPlay.Text = "Pause";
        }

        private void StopPlay(bool showExactFrame)
        {
            if (!_player.Running) return;
            _player.Stop();
            _btnPlay.Text = "Play";
            lock (_frameLock)
            {
                _latestFrame?.Dispose();
                _latestFrame = null;
            }
            if (showExactFrame) RequestFrame();
        }

        private void OnPlayerFrame(Bitmap bmp, int index)
        {
            // Background thread. Keep only the newest frame; post one update at a time to the UI.
            Bitmap old;
            lock (_frameLock)
            {
                old = _latestFrame;
                _latestFrame = bmp;
                _latestIndex = index;
            }
            old?.Dispose();
            if (Interlocked.Exchange(ref _framePosted, 1) == 0)
                SafeInvoke(ShowLatestFrame);
        }

        private void ShowLatestFrame()
        {
            Interlocked.Exchange(ref _framePosted, 0);
            Bitmap bmp;
            int index;
            lock (_frameLock)
            {
                bmp = _latestFrame;
                _latestFrame = null;
                index = _latestIndex;
            }
            if (bmp == null) return;
            if (!_player.Running || _info == null)
            {
                bmp.Dispose();
                return;
            }
            // Seek point + real time of the first frame + frames since then, snapped to the frame grid.
            double fps = _info.Fps;
            double t = _playSeek + _player.Offset + index * _info.FrameTime;
            _time = Math.Min(_info.MaxTime, Math.Round(t * fps) / fps);
            _panel.Time = _time;
            _panel.SetImage(bmp);
            UpdateTimeUi(false);
        }

        private void OnPlayerFinished(string error)
        {
            if (!_player.Running) return;
            StopPlay(true);
            if (error != null)
                MessageBox.Show(this, "Playback stopped with an error:\r\n\r\n" + VideoInfo.LastLines(error, 8), "Play", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        private void OnResultToggled()
        {
            if (_info == null) { _panel.Invalidate(); return; }
            if (_player.Running)
            {
                StopPlay(false);
                StartPlay();
            }
            else
            {
                RequestFrame();
            }
        }

        /// <summary>Runs an action on the UI thread. Returns false if the form is already closed.</summary>
        private bool SafeInvoke(Action a)
        {
            try
            {
                if (IsDisposed || !IsHandleCreated) return false;
                BeginInvoke(a);
                return true;
            }
            catch (InvalidOperationException) { return false; }
        }

        // =====================================================================
        // Areas list and properties
        // =====================================================================

        private MaskWindow MakeNewWindow() => new MaskWindow { Mode = _defaultMode, Strength = _defaultStrength };

        private static string EffectText(MaskWindow w) =>
            w.Mode == MaskMode.Black ? "Black box" : $"{MaskWindow.ModeName(w.Mode)} {w.Strength}";

        private void RefreshList()
        {
            _ui = true;
            _list.BeginUpdate();
            _list.Items.Clear();
            for (int i = 0; i < _windows.Count; i++)
            {
                var w = _windows[i];
                var r = w.Rect;
                string time = (w.Start <= 0 && w.ToEnd) ? "whole video" : Fmt.Time(w.Start) + " - " + Fmt.Time(w.End);
                var it = new ListViewItem(new[] { (i + 1).ToString(), EffectText(w), $"{r.X},{r.Y}  {r.Width}x{r.Height}", time })
                {
                    Tag = w,
                };
                _list.Items.Add(it);
                if (ReferenceEquals(w, _panel.Selected))
                {
                    it.Selected = true;
                    it.EnsureVisible();
                }
            }
            _list.EndUpdate();
            _ui = false;
        }

        private void SyncListSelection()
        {
            _ui = true;
            foreach (ListViewItem it in _list.Items)
            {
                bool sel = ReferenceEquals(it.Tag, _panel.Selected);
                if (it.Selected != sel) it.Selected = sel;
                if (sel) it.EnsureVisible();
            }
            _ui = false;
        }

        private decimal TimeMax => _info != null && _info.Duration > 0 ? (decimal)Math.Round(_info.Duration, 3) : 1000000m;

        private static decimal Clamp(NumericUpDown n, decimal v) => Math.Max(n.Minimum, Math.Min(n.Maximum, v));

        private void FillProps()
        {
            var w = _panel.Selected;
            _ui = true;
            _props.Enabled = w != null;
            _props.Text = w != null ? $"Selected area: {_windows.IndexOf(w) + 1}" : "Selected area (none)";
            if (w != null)
            {
                _mode.SelectedIndex = (int)w.Mode;
                _strength.Enabled = w.Mode != MaskMode.Black;
                _strengthLabel.Text = w.Mode == MaskMode.Pixelate ? "Pixel size" : "Blur strength";
                _strength.Value = Clamp(_strength, w.Strength);

                if (_info != null)
                {
                    _x.Maximum = _info.Width; _w.Maximum = _info.Width;
                    _y.Maximum = _info.Height; _h.Maximum = _info.Height;
                }
                _x.Value = Clamp(_x, w.Rect.X);
                _y.Value = Clamp(_y, w.Rect.Y);
                _w.Value = Clamp(_w, w.Rect.Width);
                _h.Value = Clamp(_h, w.Rect.Height);

                _start.Maximum = TimeMax;
                _end.Maximum = TimeMax;
                _start.Value = Clamp(_start, (decimal)Math.Round(w.Start, 3));
                _end.Value = w.ToEnd ? _end.Maximum : Clamp(_end, (decimal)Math.Round(w.End, 3));
            }
            _ui = false;
        }

        /// <summary>Call after any change to the areas.</summary>
        private void AreasEdited()
        {
            RefreshList();
            FillProps();
            _panel.Invalidate();
            if (_btnResult.Checked && !_player.Running) RequestFrame();
        }

        private void OnModeChanged()
        {
            var w = _panel.Selected;
            if (_ui || w == null || _mode.SelectedIndex < 0) return;
            w.Mode = (MaskMode)_mode.SelectedIndex;
            _defaultMode = w.Mode;
            AreasEdited();
        }

        private void OnStrengthChanged()
        {
            var w = _panel.Selected;
            if (_ui || w == null) return;
            w.Strength = (int)_strength.Value;
            _defaultStrength = w.Strength;
            AreasEdited();
        }

        private void OnRectChanged()
        {
            var w = _panel.Selected;
            if (_ui || w == null || _info == null) return;
            w.Rect = MaskWindow.Fit(new Rectangle((int)_x.Value, (int)_y.Value, (int)_w.Value, (int)_h.Value), new Size(_info.Width, _info.Height));
            AreasEdited();
        }

        private void OnStartChanged()
        {
            var w = _panel.Selected;
            if (_ui || w == null) return;
            w.Start = (double)_start.Value;
            if (!w.ToEnd && w.End < w.Start) w.End = w.Start;
            AreasEdited();
        }

        private void OnEndChanged()
        {
            var w = _panel.Selected;
            if (_ui || w == null) return;
            decimal v = _end.Value;
            w.End = v >= _end.Maximum - 0.0005m && _info != null && _info.Duration > 0 ? double.PositiveInfinity : (double)v;
            if (w.End < w.Start) w.Start = w.End;
            AreasEdited();
        }

        private void SetStartNow()
        {
            var w = _panel.Selected;
            if (w == null || _info == null) return;
            StopPlay(true);
            w.Start = _time;
            if (!w.ToEnd && w.End < w.Start) w.End = double.PositiveInfinity;
            AreasEdited();
        }

        private void SetEndNow()
        {
            var w = _panel.Selected;
            if (w == null || _info == null) return;
            StopPlay(true);
            w.End = _time;
            if (w.Start > w.End) w.Start = 0;
            AreasEdited();
        }

        private void SetWholeVideo()
        {
            var w = _panel.Selected;
            if (w == null) return;
            w.Start = 0;
            w.End = double.PositiveInfinity;
            AreasEdited();
        }

        private void DuplicateSelected()
        {
            var w = _panel.Selected;
            if (w == null || _info == null) return;
            var c = w.Clone();
            c.Rect = MaskWindow.Fit(new Rectangle(w.Rect.X + 20, w.Rect.Y + 20, w.Rect.Width, w.Rect.Height), new Size(_info.Width, _info.Height));
            _windows.Add(c);
            _panel.SelectArea(c, false);
            AreasEdited();
        }

        private void DeleteSelected()
        {
            var w = _panel.Selected;
            if (w == null) return;
            int i = _windows.IndexOf(w);
            _windows.Remove(w);
            var next = _windows.Count == 0 ? null : _windows[Math.Min(i, _windows.Count - 1)];
            _panel.SelectArea(next, false);
            AreasEdited();
        }

        private void DeleteAll()
        {
            if (_windows.Count == 0) return;
            if (MessageBox.Show(this, $"Delete all {_windows.Count} areas?", "Delete all", MessageBoxButtons.YesNo, MessageBoxIcon.Question) != DialogResult.Yes)
                return;
            _windows.Clear();
            _panel.SelectArea(null, false);
            AreasEdited();
        }

        // =====================================================================
        // Save / load areas
        // =====================================================================

        private void SaveAreas()
        {
            if (_windows.Count == 0)
            {
                MessageBox.Show(this, "There are no areas to save.", "Save areas", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }
            using (var d = new SaveFileDialog { Title = "Save areas", Filter = "Area files (*.masks)|*.masks|All files (*.*)|*.*", DefaultExt = "masks" })
            {
                if (_info != null)
                {
                    d.InitialDirectory = Path.GetDirectoryName(_info.Path);
                    d.FileName = Path.GetFileNameWithoutExtension(_info.Path) + ".masks";
                }
                if (d.ShowDialog(this) != DialogResult.OK) return;
                try { MaskFile.Save(d.FileName, _windows); }
                catch (Exception ex) { MessageBox.Show(this, ex.Message, "Save areas", MessageBoxButtons.OK, MessageBoxIcon.Error); }
            }
        }

        private void LoadAreas()
        {
            using (var d = new OpenFileDialog { Title = "Load areas", Filter = "Area files (*.masks)|*.masks|All files (*.*)|*.*" })
            {
                if (_info != null) d.InitialDirectory = Path.GetDirectoryName(_info.Path);
                if (d.ShowDialog(this) == DialogResult.OK) LoadAreasFrom(d.FileName);
            }
        }

        private void LoadAreasFrom(string path)
        {
            List<MaskWindow> loaded;
            try { loaded = MaskFile.Load(path); }
            catch (Exception ex)
            {
                MessageBox.Show(this, "Could not read the file.\r\n\r\n" + ex.Message, "Load areas", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (_windows.Count > 0)
            {
                var r = MessageBox.Show(this,
                    "Replace the current areas?\r\n\r\nYes = replace, No = add to the current areas.",
                    "Load areas", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                if (r == DialogResult.Cancel) return;
                if (r == DialogResult.Yes) _windows.Clear();
            }

            if (_info != null)
                foreach (var w in loaded) w.Rect = MaskWindow.Fit(w.Rect, new Size(_info.Width, _info.Height));

            _windows.AddRange(loaded);
            _panel.SelectArea(null, false);
            AreasEdited();
        }

        // =====================================================================
        // Export
        // =====================================================================

        private static readonly string[] OutputTypes = { ".mp4", ".mkv", ".mov", ".avi", ".webm" };

        private void Export()
        {
            if (_info == null || !EnsureFfmpeg()) return;
            StopPlay(true);

            bool mute = _btnMute.Checked;
            // With Mute on, an export with no areas is a normal way to only remove the sound.
            if (_windows.Count == 0 && !mute &&
                MessageBox.Show(this, "There are no areas to hide. Export the video anyway?", "Export",
                    MessageBoxButtons.YesNo, MessageBoxIcon.Question) != DialogResult.Yes)
                return;

            string ext = Path.GetExtension(_info.Path).ToLowerInvariant();
            if (ext.Length < 2) ext = ".mp4";

            var filter = new List<string> { $"Same type as the source (*{ext})|*{ext}" };
            foreach (string t in OutputTypes)
                if (t != ext) filter.Add($"{t.Substring(1).ToUpperInvariant()} video (*{t})|*{t}");
            filter.Add("All files (*.*)|*.*");

            string output;
            using (var d = new SaveFileDialog
            {
                Title = "Export video",
                Filter = string.Join("|", filter),
                DefaultExt = ext.Substring(1),
                AddExtension = true,
                OverwritePrompt = true,
                InitialDirectory = Path.GetDirectoryName(_info.Path),
                FileName = Path.GetFileNameWithoutExtension(_info.Path) + "_masked" + ext,
            })
            {
                if (d.ShowDialog(this) != DialogResult.OK) return;
                output = d.FileName;
            }

            if (string.Equals(Path.GetFullPath(output), Path.GetFullPath(_info.Path), StringComparison.OrdinalIgnoreCase))
            {
                MessageBox.Show(this, "Choose a different file name. The source video cannot be overwritten.", "Export",
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            var job = new ExportJob
            {
                Input = _info.Path,
                Output = output,
                Windows = _windows.Select(w => w.Clone()).ToList(),
                Fps = _info.Fps,
                Duration = _info.Duration,
                HasAudio = _info.HasAudio,
                Quality = Math.Max(0, _quality.SelectedIndex),
                Mute = mute,
                SourceCodec = _info.Codec,
                SourcePixFmt = _info.PixFmt,
                SourceKbps = _info.VideoKbps,
            };
            using (var f = new ExportForm(job)) f.ShowDialog(this);
        }

        // =====================================================================
        // Keys, help, closing
        // =====================================================================

        private bool TextInputFocused()
        {
            Control c = ActiveControl;
            while (c is ContainerControl cc && cc.ActiveControl != null) c = cc.ActiveControl;
            return c is TextBoxBase || c is NumericUpDown || c is ComboBox || c?.Parent is NumericUpDown;
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Control | Keys.O: OpenDialog(); return true;
                case Keys.Control | Keys.E: Export(); return true;
                case Keys.Control | Keys.S: SaveAreas(); return true;
                case Keys.Control | Keys.D: DuplicateSelected(); return true;
                case Keys.F1: OpenManual(); return true;
            }

            if (!TextInputFocused() && _info != null)
            {
                int second = Math.Max(1, (int)Math.Round(_info.Fps));
                switch (keyData)
                {
                    case Keys.Space: TogglePlay(); return true;
                    case Keys.Left: Step(-1); return true;
                    case Keys.Right: Step(1); return true;
                    case Keys.Shift | Keys.Left: Step(-second); return true;
                    case Keys.Shift | Keys.Right: Step(second); return true;
                    case Keys.Home: StopPlay(false); SetTime(0); return true;
                    case Keys.End: StopPlay(false); SetTime(_info.MaxTime); return true;
                    case Keys.Delete: DeleteSelected(); return true;
                }
            }
            return base.ProcessCmdKey(ref msg, keyData);
        }

        private const string ManualFile = "User Manual.html";

        /// <summary>
        /// Opens the user manual (next to MaskVideo.exe) in the default web browser.
        /// Without the file (for example when run from bin\Release), shows the short help instead.
        /// </summary>
        private void OpenManual()
        {
            string path = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, ManualFile);
            if (File.Exists(path))
            {
                try
                {
                    Process.Start(path);
                    return;
                }
                catch (Exception) { /* no program opens .html files: show the short help */ }
            }
            ShowHelp();
        }

        private void ShowHelp()
        {
            MessageBox.Show(this,
                "How to use\r\n\r\n" +
                "1. Open a video: \"Open video...\", Ctrl+O, or drag the file into this window.\r\n" +
                "2. Drag on the video to draw an area to hide. Add as many areas as you need.\r\n" +
                "3. Drag an area to move it. Drag the white squares to change its size.\r\n" +
                "4. Under \"Selected area\" choose Blur, Pixelate or Black box, and the strength.\r\n" +
                "5. To hide an area only for part of the video: go to the frame, select the area, " +
                "and press \"Now\" next to Start or End (or right-click the area).\r\n" +
                "6. Press \"Export video...\" to write a new file. The sound is kept.\r\n" +
                "Mute on: the new file has no sound. With no areas, the video is copied without change " +
                "(fast, same quality), so this only removes the sound.\r\n\r\n" +
                "Keys\r\n" +
                "Space = play / pause    Left / Right = one frame    Shift+Left / Right = one second\r\n" +
                "Home / End = first / last frame    Delete = delete area    Ctrl+D = duplicate area\r\n\r\n" +
                "Tip: for faces, number plates and text, Pixelate (large pixel size) or Black box " +
                "hide the content more safely than a light blur.\r\n\r\n" +
                "Save areas / Load areas keep the list in a .masks file, so you can use the same areas on other clips " +
                "from the same camera.\r\n\r\n" +
                "The full user manual is the file \"" + ManualFile + "\" in the program folder.\r\n\r\n" +
                "ffmpeg: " + (Ff.ExePath ?? "not found"),
                "Video Mask - help", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            _player.Stop();
            base.OnFormClosing(e);
        }
    }
}
