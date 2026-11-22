using System;
using System.Windows.Forms;
using System.Drawing;
using System.Runtime.InteropServices;
using Datapath.RGBEasy;

namespace Sample1
{
   public partial class Sample1 : Form
   {
      private RGBERROR loadError = RGBERROR.NO_ERROR;
      private IntPtr driver = IntPtr.Zero;
      private IntPtr rgb = IntPtr.Zero;
      private GCHandle gcHandle;
      private RGBMODECHANGEDFN m_ModeChangedFN;
      private RGBVALUECHANGEDFN m_ValueChangedFN;

      public Sample1(IntPtr hRGBDLL)
      {
         InitializeComponent();
         driver = hRGBDLL;
         if (driver != IntPtr.Zero)
         {
             loadError = RGB.OpenInput(0, out rgb);
             if (loadError == 0)
             {
               gcHandle = GCHandle.Alloc(this);
            }
         }
      }

      private void RgbSample_Load(object sender, EventArgs e)
      {
         if (rgb != IntPtr.Zero)
         {
            RGB.SetWindow(rgb, this.Handle);
            m_ModeChangedFN = new RGBMODECHANGEDFN(ModeChangedFN);
            m_ValueChangedFN = new RGBVALUECHANGEDFN(ValueChangedFN);
            RGB.SetModeChangedFn(rgb, m_ModeChangedFN, GCHandle.ToIntPtr(gcHandle));
            RGB.SetValueChangedFn(rgb, m_ValueChangedFN, GCHandle.ToIntPtr(gcHandle));
            RGB.DetectInput(rgb);
         }
      }

      public static void ModeChangedFN(IntPtr window, IntPtr capture, ref RGBMODECHANGEDINFO info, IntPtr userData)
      {
         Sample1 sample = (Sample1)((GCHandle.FromIntPtr((IntPtr)(userData))).Target);
         sample.ChangeWindowSize();
      }

      public static void ValueChangedFN(IntPtr window, IntPtr capture, ref RGBVALUECHANGEDINFO Info, IntPtr userData)
      {
         GCHandle gch = GCHandle.FromIntPtr((IntPtr)(userData));
         Sample1 sample = (Sample1)gch.Target;
         IAsyncResult ar = sample.BeginInvoke(new ChangeWindowSizeDelegate(sample.ChangeWindowSize));
      }

      delegate void ChangeWindowSizeDelegate();

      public void ChangeWindowSize()
      {
         uint width, height;

         RGB.GetCaptureWidth(rgb, out width);
         RGB.GetCaptureHeight(rgb, out height);
         this.SetClientSizeCore((int)width, (int)height);
      }

      private void Sample1_FormClosing(object sender, FormClosingEventArgs e)
      {
         if (driver != IntPtr.Zero)
         {
            if (rgb != IntPtr.Zero)
            {
               RGB.SetModeChangedFn(rgb, m_ModeChangedFN, IntPtr.Zero);
               RGB.SetValueChangedFn(rgb, m_ValueChangedFN, IntPtr.Zero);
               RGB.SetWindow(rgb, IntPtr.Zero);
               RGB.CloseInput(rgb);
            }
         }
      }
   }
}