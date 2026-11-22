using System;
using System.Threading;
using System.Windows.Forms;
using System.Drawing;
using System.Runtime.InteropServices;
using Datapath.RGBEasy;

namespace Sample3
{
   public partial class Sample3 : Form
   {
      private RGBERROR           loadError = RGBERROR.NO_ERROR;
      private IntPtr             driver = IntPtr.Zero;
      private IntPtr             rgb = IntPtr.Zero;
      private GCHandle           gcHandle;
      private RGBFRAMECAPTUREDFN m_FrameCapturedFN;
      private bool               bCapturedFrame = true;
      private Int16              FrameCount = 0;

      public Sample3(IntPtr hRGBDLL)
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
             /* Set up catpure. */
             RGB.SetFrameDropping(rgb, 0);
             RGB.SetDMADirect(rgb, 0);
             RGB.SetPixelformat(rgb, Datapath.RGBEasy.PIXELFORMAT.RGB888);

             /* Set frame captured callback function. */
             m_FrameCapturedFN = new RGBFRAMECAPTUREDFN(FrameCapturedFN);
             RGB.SetFrameCapturedFn(rgb, m_FrameCapturedFN, GCHandle.ToIntPtr(gcHandle));
             RGB.StartCapture (rgb);
         }
      }

      public static void FrameCapturedFN(IntPtr window, IntPtr capture, IntPtr Info, IntPtr bitmapBits, IntPtr userData)
      {          
          GCHandle gch = GCHandle.FromIntPtr((IntPtr)(userData));
          Sample3 sample = (Sample3)gch.Target;
          sample.Invoke(new FrameCapturedDelegate(sample.FrameCaptured), capture, Info, bitmapBits);
      }

      delegate void FrameCapturedDelegate(IntPtr hRGB, IntPtr bitmapInfo, IntPtr bitmapBits);

      public void FrameCaptured(IntPtr hRGB, IntPtr bitmapInfo, IntPtr bitmapBits)
      {
          unsafe
          {
              BITMAPINFOHEADER* pBitmapInfoHeader = (BITMAPINFOHEADER*)bitmapInfo;
              /* TODO: Directly access BITMAPINFOHEADER here if required. */
          }

          if (!bCapturedFrame)
          {
              bCapturedFrame = true;

              /* Construct bitmap filename using FrameCount. */
              string filename = "bitmap";
              filename += FrameCount.ToString();
              filename += ".bmp";

              /* Save the current frame to file. */
              RGB.SaveBitmap(hRGB, bitmapInfo, bitmapBits, filename);
          }
      }

      private void Sample3_FormClosing(object sender, FormClosingEventArgs e)
      {
         if (driver != IntPtr.Zero)
         {
            if (rgb != IntPtr.Zero)
            {
               RGB.SetFrameCapturedFn(rgb, m_FrameCapturedFN, IntPtr.Zero);
               RGB.StopCapture(rgb);
               RGB.CloseInput(rgb);
            }
         }
      }

      private void Sample3_ButtonClick(object sender, EventArgs e)
      {
          this.FrameCount++;
          this.bCapturedFrame = false;

      }
   }
}