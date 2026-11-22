using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;
using Datapath.RGBEasy;

/******************************************************************************/



namespace Sample2
{
   static class Program
   {
      /// <summary>
      /// The main entry point for the application.
      /// </summary>
      /// 
      [STAThread]
      static  void Main()
      {
         RGBERROR error = 0;
         IntPtr hRGBDLL = IntPtr.Zero;

         Application.EnableVisualStyles();
         Application.SetCompatibleTextRenderingDefault(false);

         
         {
            error = RGB.Load(out hRGBDLL);
            if (error == RGBERROR.NO_ERROR)
            {
               IntPtr hRGB;

               error = RGB.OpenInput(0, out hRGB);
               if (error == 0)
               {
                  Application.Run(new Sample2(hRGB));
                  RGB.CloseInput(hRGB);
               }
               else
               {
                  string msg = "OpenInput Failed: 0x" + error.ToString("X");
                  MessageBox.Show(msg, "RgbSample", MessageBoxButtons.OK);
               }
               RGB.Free(hRGBDLL);
            }
            else
            {
               string msg = "Load Failed: 0x" + error.ToString("X");
               MessageBox.Show(msg, "RgbSample", MessageBoxButtons.OK);
            }
         }
      }
   }
}