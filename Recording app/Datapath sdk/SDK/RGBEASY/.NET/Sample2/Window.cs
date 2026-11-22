using System;
using System.Windows.Forms;
using Datapath.RGBEasy;

namespace Sample2
{
   public partial class Sample2 : Form
   {
      private IntPtr rgb = IntPtr.Zero;

      public Sample2(IntPtr capture)
      {
         InitializeComponent();
         rgb = capture;
      }

      private void InputSettingsMenuItem_Click(object sender, EventArgs e)
      {
         new InputSettings(rgb);
      }

      private void ExitMenuItem_Click(object sender, EventArgs e)
      {
         RGB.SetWindow(rgb, IntPtr.Zero);
         this.Close();
      }

      private void RgbSample_Load(object sender, EventArgs e)
      {
         // Pass the window handle to RGBEASY
         RGB.SetWindow(rgb, this.Handle);
      }
   }
}