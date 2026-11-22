namespace Sample2
{
   partial class InputSettings
   {
      /// <summary>
      /// Required designer variable.
      /// </summary>
      private System.ComponentModel.IContainer components = null;

      /// <summary>
      /// Clean up any resources being used.
      /// </summary>
      /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
      protected override void Dispose(bool disposing)
      {
         if (disposing && (components != null))
         {
            components.Dispose();
         }
         base.Dispose(disposing);
      }

      #region Windows Form Designer generated code

      /// <summary>
      /// Required method for Designer support - do not modify
      /// the contents of this method with the code editor.
      /// </summary>
      private void InitializeComponent()
      {
         this.groupBox1 = new System.Windows.Forms.GroupBox();
         this.InputSource = new System.Windows.Forms.NumericUpDown();
         this.label1 = new System.Windows.Forms.Label();
         this.CaptureWidth = new System.Windows.Forms.NumericUpDown();
         this.CaptureHeight = new System.Windows.Forms.NumericUpDown();
         this.label2 = new System.Windows.Forms.Label();
         this.label3 = new System.Windows.Forms.Label();
         this.groupBox2 = new System.Windows.Forms.GroupBox();
         this.ContrastStatic = new System.Windows.Forms.Label();
         this.BrightnessStatic = new System.Windows.Forms.Label();
         this.BlackLevelStatic = new System.Windows.Forms.Label();
         this.VerticalPositionStatic = new System.Windows.Forms.Label();
         this.PhaseStatic = new System.Windows.Forms.Label();
         this.HorizontalSizeStatic = new System.Windows.Forms.Label();
         this.HorizontalPositionStatic = new System.Windows.Forms.Label();
         this.Contrast = new System.Windows.Forms.HScrollBar();
         this.VerticalPosition = new System.Windows.Forms.HScrollBar();
         this.BlackLevel = new System.Windows.Forms.HScrollBar();
         this.Brightness = new System.Windows.Forms.HScrollBar();
         this.HorizontalSize = new System.Windows.Forms.HScrollBar();
         this.Phase = new System.Windows.Forms.HScrollBar();
         this.label99 = new System.Windows.Forms.Label();
         this.HorizontalPosition = new System.Windows.Forms.HScrollBar();
         this.label9 = new System.Windows.Forms.Label();
         this.label7 = new System.Windows.Forms.Label();
         this.label = new System.Windows.Forms.Label();
         this.label5 = new System.Windows.Forms.Label();
         this.label6 = new System.Windows.Forms.Label();
         this.label4 = new System.Windows.Forms.Label();
         this.CloseDialog = new System.Windows.Forms.Button();
         this.groupBox1.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.InputSource)).BeginInit();
         ((System.ComponentModel.ISupportInitialize)(this.CaptureWidth)).BeginInit();
         ((System.ComponentModel.ISupportInitialize)(this.CaptureHeight)).BeginInit();
         this.groupBox2.SuspendLayout();
         this.SuspendLayout();
         // 
         // groupBox1
         // 
         this.groupBox1.Controls.Add(this.InputSource);
         this.groupBox1.Controls.Add(this.label1);
         this.groupBox1.Location = new System.Drawing.Point(16, 3);
         this.groupBox1.Name = "groupBox1";
         this.groupBox1.Size = new System.Drawing.Size(310, 56);
         this.groupBox1.TabIndex = 1;
         this.groupBox1.TabStop = false;
         this.groupBox1.Text = "RGB Source";
         // 
         // InputSource
         // 
         this.InputSource.Location = new System.Drawing.Point(144, 23);
         this.InputSource.Name = "InputSource";
         this.InputSource.Size = new System.Drawing.Size(65, 20);
         this.InputSource.TabIndex = 3;
         this.InputSource.ValueChanged += new System.EventHandler(this.InputSource_ValueChanged);
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Location = new System.Drawing.Point(101, 25);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(31, 13);
         this.label1.TabIndex = 2;
         this.label1.Text = "Input";
         // 
         // CaptureWidth
         // 
         this.CaptureWidth.Location = new System.Drawing.Point(57, 26);
         this.CaptureWidth.Name = "CaptureWidth";
         this.CaptureWidth.Size = new System.Drawing.Size(65, 20);
         this.CaptureWidth.TabIndex = 2;
         this.CaptureWidth.ValueChanged += new System.EventHandler(this.CaptureWidth_ValueChanged);
         // 
         // CaptureHeight
         // 
         this.CaptureHeight.Location = new System.Drawing.Point(185, 26);
         this.CaptureHeight.Name = "CaptureHeight";
         this.CaptureHeight.Size = new System.Drawing.Size(65, 20);
         this.CaptureHeight.TabIndex = 2;
         this.CaptureHeight.ValueChanged += new System.EventHandler(this.CaptureHeight_ValueChanged);
         // 
         // label2
         // 
         this.label2.AutoSize = true;
         this.label2.Location = new System.Drawing.Point(16, 28);
         this.label2.Name = "label2";
         this.label2.Size = new System.Drawing.Size(35, 13);
         this.label2.TabIndex = 2;
         this.label2.Text = "Width";
         // 
         // label3
         // 
         this.label3.AutoSize = true;
         this.label3.Location = new System.Drawing.Point(141, 28);
         this.label3.Name = "label3";
         this.label3.Size = new System.Drawing.Size(38, 13);
         this.label3.TabIndex = 2;
         this.label3.Text = "Height";
         // 
         // groupBox2
         // 
         this.groupBox2.Controls.Add(this.ContrastStatic);
         this.groupBox2.Controls.Add(this.BrightnessStatic);
         this.groupBox2.Controls.Add(this.BlackLevelStatic);
         this.groupBox2.Controls.Add(this.VerticalPositionStatic);
         this.groupBox2.Controls.Add(this.PhaseStatic);
         this.groupBox2.Controls.Add(this.HorizontalSizeStatic);
         this.groupBox2.Controls.Add(this.HorizontalPositionStatic);
         this.groupBox2.Controls.Add(this.Contrast);
         this.groupBox2.Controls.Add(this.VerticalPosition);
         this.groupBox2.Controls.Add(this.BlackLevel);
         this.groupBox2.Controls.Add(this.Brightness);
         this.groupBox2.Controls.Add(this.HorizontalSize);
         this.groupBox2.Controls.Add(this.Phase);
         this.groupBox2.Controls.Add(this.label99);
         this.groupBox2.Controls.Add(this.HorizontalPosition);
         this.groupBox2.Controls.Add(this.label9);
         this.groupBox2.Controls.Add(this.label7);
         this.groupBox2.Controls.Add(this.label);
         this.groupBox2.Controls.Add(this.label5);
         this.groupBox2.Controls.Add(this.label6);
         this.groupBox2.Controls.Add(this.label4);
         this.groupBox2.Controls.Add(this.label2);
         this.groupBox2.Controls.Add(this.CaptureHeight);
         this.groupBox2.Controls.Add(this.label3);
         this.groupBox2.Controls.Add(this.CaptureWidth);
         this.groupBox2.Location = new System.Drawing.Point(16, 65);
         this.groupBox2.Name = "groupBox2";
         this.groupBox2.Size = new System.Drawing.Size(310, 223);
         this.groupBox2.TabIndex = 3;
         this.groupBox2.TabStop = false;
         this.groupBox2.Text = "Capture Settings";
         // 
         // ContrastStatic
         // 
         this.ContrastStatic.AutoSize = true;
         this.ContrastStatic.Location = new System.Drawing.Point(257, 195);
         this.ContrastStatic.Name = "ContrastStatic";
         this.ContrastStatic.Size = new System.Drawing.Size(35, 13);
         this.ContrastStatic.TabIndex = 4;
         this.ContrastStatic.Text = "label8";
         // 
         // BrightnessStatic
         // 
         this.BrightnessStatic.AutoSize = true;
         this.BrightnessStatic.Location = new System.Drawing.Point(257, 172);
         this.BrightnessStatic.Name = "BrightnessStatic";
         this.BrightnessStatic.Size = new System.Drawing.Size(35, 13);
         this.BrightnessStatic.TabIndex = 4;
         this.BrightnessStatic.Text = "label8";
         // 
         // BlackLevelStatic
         // 
         this.BlackLevelStatic.AutoSize = true;
         this.BlackLevelStatic.Location = new System.Drawing.Point(257, 149);
         this.BlackLevelStatic.Name = "BlackLevelStatic";
         this.BlackLevelStatic.Size = new System.Drawing.Size(35, 13);
         this.BlackLevelStatic.TabIndex = 4;
         this.BlackLevelStatic.Text = "label8";
         // 
         // VerticalPositionStatic
         // 
         this.VerticalPositionStatic.AutoSize = true;
         this.VerticalPositionStatic.Location = new System.Drawing.Point(257, 129);
         this.VerticalPositionStatic.Name = "VerticalPositionStatic";
         this.VerticalPositionStatic.Size = new System.Drawing.Size(35, 13);
         this.VerticalPositionStatic.TabIndex = 4;
         this.VerticalPositionStatic.Text = "label8";
         // 
         // PhaseStatic
         // 
         this.PhaseStatic.AutoSize = true;
         this.PhaseStatic.Location = new System.Drawing.Point(257, 103);
         this.PhaseStatic.Name = "PhaseStatic";
         this.PhaseStatic.Size = new System.Drawing.Size(35, 13);
         this.PhaseStatic.TabIndex = 4;
         this.PhaseStatic.Text = "label8";
         // 
         // HorizontalSizeStatic
         // 
         this.HorizontalSizeStatic.AutoSize = true;
         this.HorizontalSizeStatic.Location = new System.Drawing.Point(257, 80);
         this.HorizontalSizeStatic.Name = "HorizontalSizeStatic";
         this.HorizontalSizeStatic.Size = new System.Drawing.Size(35, 13);
         this.HorizontalSizeStatic.TabIndex = 4;
         this.HorizontalSizeStatic.Text = "label8";
         // 
         // HorizontalPositionStatic
         // 
         this.HorizontalPositionStatic.AutoSize = true;
         this.HorizontalPositionStatic.Location = new System.Drawing.Point(257, 57);
         this.HorizontalPositionStatic.Name = "HorizontalPositionStatic";
         this.HorizontalPositionStatic.Size = new System.Drawing.Size(35, 13);
         this.HorizontalPositionStatic.TabIndex = 4;
         this.HorizontalPositionStatic.Text = "label8";
         // 
         // Contrast
         // 
         this.Contrast.Location = new System.Drawing.Point(113, 195);
         this.Contrast.Name = "Contrast";
         this.Contrast.Size = new System.Drawing.Size(137, 16);
         this.Contrast.TabIndex = 3;
         this.Contrast.Scroll += new System.Windows.Forms.ScrollEventHandler(this.Contrast_Scroll);
         // 
         // VerticalPosition
         // 
         this.VerticalPosition.Location = new System.Drawing.Point(113, 126);
         this.VerticalPosition.Name = "VerticalPosition";
         this.VerticalPosition.Size = new System.Drawing.Size(137, 16);
         this.VerticalPosition.TabIndex = 3;
         this.VerticalPosition.Scroll += new System.Windows.Forms.ScrollEventHandler(this.VerticalPosition_Scroll);
         // 
         // BlackLevel
         // 
         this.BlackLevel.Location = new System.Drawing.Point(113, 149);
         this.BlackLevel.Name = "BlackLevel";
         this.BlackLevel.Size = new System.Drawing.Size(137, 16);
         this.BlackLevel.TabIndex = 3;
         this.BlackLevel.Scroll += new System.Windows.Forms.ScrollEventHandler(this.BlackLevel_Scroll);
         // 
         // Brightness
         // 
         this.Brightness.Location = new System.Drawing.Point(113, 172);
         this.Brightness.Name = "Brightness";
         this.Brightness.Size = new System.Drawing.Size(137, 16);
         this.Brightness.TabIndex = 3;
         this.Brightness.Scroll += new System.Windows.Forms.ScrollEventHandler(this.Brightness_Scroll);
         // 
         // HorizontalSize
         // 
         this.HorizontalSize.Location = new System.Drawing.Point(113, 80);
         this.HorizontalSize.Name = "HorizontalSize";
         this.HorizontalSize.Size = new System.Drawing.Size(137, 16);
         this.HorizontalSize.TabIndex = 3;
         this.HorizontalSize.Scroll += new System.Windows.Forms.ScrollEventHandler(this.HorizontalSize_Scroll);
         // 
         // Phase
         // 
         this.Phase.Location = new System.Drawing.Point(113, 103);
         this.Phase.Name = "Phase";
         this.Phase.Size = new System.Drawing.Size(137, 16);
         this.Phase.TabIndex = 3;
         this.Phase.Scroll += new System.Windows.Forms.ScrollEventHandler(this.Phase_Scroll);
         // 
         // label99
         // 
         this.label99.AutoSize = true;
         this.label99.Location = new System.Drawing.Point(16, 195);
         this.label99.Name = "label99";
         this.label99.Size = new System.Drawing.Size(46, 13);
         this.label99.TabIndex = 2;
         this.label99.Text = "Contrast";
         // 
         // HorizontalPosition
         // 
         this.HorizontalPosition.Location = new System.Drawing.Point(113, 57);
         this.HorizontalPosition.Name = "HorizontalPosition";
         this.HorizontalPosition.Size = new System.Drawing.Size(137, 16);
         this.HorizontalPosition.TabIndex = 3;
         this.HorizontalPosition.Scroll += new System.Windows.Forms.ScrollEventHandler(this.HorizontalPosition_Scroll);
         // 
         // label9
         // 
         this.label9.AutoSize = true;
         this.label9.Location = new System.Drawing.Point(16, 149);
         this.label9.Name = "label9";
         this.label9.Size = new System.Drawing.Size(63, 13);
         this.label9.TabIndex = 2;
         this.label9.Text = "Black Level";
         // 
         // label7
         // 
         this.label7.AutoSize = true;
         this.label7.Location = new System.Drawing.Point(16, 126);
         this.label7.Name = "label7";
         this.label7.Size = new System.Drawing.Size(82, 13);
         this.label7.TabIndex = 2;
         this.label7.Text = "Vertical Position";
         // 
         // label
         // 
         this.label.AutoSize = true;
         this.label.Location = new System.Drawing.Point(16, 172);
         this.label.Name = "label";
         this.label.Size = new System.Drawing.Size(56, 13);
         this.label.TabIndex = 2;
         this.label.Text = "Brightness";
         // 
         // label5
         // 
         this.label5.AutoSize = true;
         this.label5.Location = new System.Drawing.Point(16, 80);
         this.label5.Name = "label5";
         this.label5.Size = new System.Drawing.Size(77, 13);
         this.label5.TabIndex = 2;
         this.label5.Text = "Horizontal Size";
         // 
         // label6
         // 
         this.label6.AutoSize = true;
         this.label6.Location = new System.Drawing.Point(16, 103);
         this.label6.Name = "label6";
         this.label6.Size = new System.Drawing.Size(37, 13);
         this.label6.TabIndex = 2;
         this.label6.Text = "Phase";
         // 
         // label4
         // 
         this.label4.AutoSize = true;
         this.label4.Location = new System.Drawing.Point(16, 57);
         this.label4.Name = "label4";
         this.label4.Size = new System.Drawing.Size(94, 13);
         this.label4.TabIndex = 2;
         this.label4.Text = "Horizontal Position";
         // 
         // CloseDialog
         // 
         this.CloseDialog.Location = new System.Drawing.Point(134, 294);
         this.CloseDialog.Name = "CloseDialog";
         this.CloseDialog.RightToLeft = System.Windows.Forms.RightToLeft.No;
         this.CloseDialog.Size = new System.Drawing.Size(75, 23);
         this.CloseDialog.TabIndex = 4;
         this.CloseDialog.Text = "Close";
         this.CloseDialog.UseVisualStyleBackColor = true;
         this.CloseDialog.Click += new System.EventHandler(this.CloseDialog_Click);
         // 
         // InputSettings
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(342, 328);
         this.Controls.Add(this.CloseDialog);
         this.Controls.Add(this.groupBox2);
         this.Controls.Add(this.groupBox1);
         this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
         this.MaximizeBox = false;
         this.MinimizeBox = false;
         this.Name = "InputSettings";
         this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
         this.Text = "Input Settings";
         this.Load += new System.EventHandler(this.InputSettings_Load);
         this.groupBox1.ResumeLayout(false);
         this.groupBox1.PerformLayout();
         ((System.ComponentModel.ISupportInitialize)(this.InputSource)).EndInit();
         ((System.ComponentModel.ISupportInitialize)(this.CaptureWidth)).EndInit();
         ((System.ComponentModel.ISupportInitialize)(this.CaptureHeight)).EndInit();
         this.groupBox2.ResumeLayout(false);
         this.groupBox2.PerformLayout();
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.GroupBox groupBox1;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.NumericUpDown CaptureWidth;
      private System.Windows.Forms.NumericUpDown CaptureHeight;
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.Label label3;
      private System.Windows.Forms.GroupBox groupBox2;
      private System.Windows.Forms.HScrollBar VerticalPosition;
      private System.Windows.Forms.HScrollBar HorizontalSize;
      private System.Windows.Forms.HScrollBar Phase;
      private System.Windows.Forms.HScrollBar HorizontalPosition;
      private System.Windows.Forms.Label label7;
      private System.Windows.Forms.Label label5;
      private System.Windows.Forms.Label label6;
      private System.Windows.Forms.Label label4;
      private System.Windows.Forms.HScrollBar Contrast;
      private System.Windows.Forms.HScrollBar BlackLevel;
      private System.Windows.Forms.HScrollBar Brightness;
      private System.Windows.Forms.Label label99;
      private System.Windows.Forms.Label label9;
      private System.Windows.Forms.Label label;
      private System.Windows.Forms.Button CloseDialog;
      private System.Windows.Forms.NumericUpDown InputSource;
      private System.Windows.Forms.Label ContrastStatic;
      private System.Windows.Forms.Label BrightnessStatic;
      private System.Windows.Forms.Label BlackLevelStatic;
      private System.Windows.Forms.Label VerticalPositionStatic;
      private System.Windows.Forms.Label PhaseStatic;
      private System.Windows.Forms.Label HorizontalSizeStatic;
      private System.Windows.Forms.Label HorizontalPositionStatic;
   }
}