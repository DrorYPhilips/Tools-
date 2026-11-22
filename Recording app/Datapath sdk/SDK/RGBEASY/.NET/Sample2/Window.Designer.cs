namespace Sample2
{
   partial class Sample2
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
         this.components = new System.ComponentModel.Container();
         this.RGBContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
         this.FileMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.ExitMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.PropertiesMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.InputSettingsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.RGBContextMenu.SuspendLayout();
         this.SuspendLayout();
         // 
         // RGBContextMenu
         // 
         this.RGBContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.FileMenuItem,
            this.PropertiesMenuItem});
         this.RGBContextMenu.Name = "RGBContextMenu";
         this.RGBContextMenu.Size = new System.Drawing.Size(124, 48);
         // 
         // FileMenuItem
         // 
         this.FileMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ExitMenuItem});
         this.FileMenuItem.Name = "FileMenuItem";
         this.FileMenuItem.Size = new System.Drawing.Size(123, 22);
         this.FileMenuItem.Text = "&File";
         // 
         // ExitMenuItem
         // 
         this.ExitMenuItem.Name = "ExitMenuItem";
         this.ExitMenuItem.Size = new System.Drawing.Size(103, 22);
         this.ExitMenuItem.Text = "&Exit";
         this.ExitMenuItem.Click += new System.EventHandler(this.ExitMenuItem_Click);
         // 
         // PropertiesMenuItem
         // 
         this.PropertiesMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.InputSettingsMenuItem});
         this.PropertiesMenuItem.Name = "PropertiesMenuItem";
         this.PropertiesMenuItem.Size = new System.Drawing.Size(123, 22);
         this.PropertiesMenuItem.Text = "&Properties";
         // 
         // InputSettingsMenuItem
         // 
         this.InputSettingsMenuItem.Name = "InputSettingsMenuItem";
         this.InputSettingsMenuItem.Size = new System.Drawing.Size(165, 22);
         this.InputSettingsMenuItem.Text = "&Input Settings...";
         this.InputSettingsMenuItem.Click += new System.EventHandler(this.InputSettingsMenuItem_Click);
         // 
         // Sample2
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(851, 510);
         this.ContextMenuStrip = this.RGBContextMenu;
         this.Name = "Sample2";
         this.Text = "Sample2";
         this.Load += new System.EventHandler(this.RgbSample_Load);
         this.RGBContextMenu.ResumeLayout(false);
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.ContextMenuStrip RGBContextMenu;
      private System.Windows.Forms.ToolStripMenuItem FileMenuItem;
      private System.Windows.Forms.ToolStripMenuItem ExitMenuItem;
      private System.Windows.Forms.ToolStripMenuItem PropertiesMenuItem;
      private System.Windows.Forms.ToolStripMenuItem InputSettingsMenuItem;
   }
}

