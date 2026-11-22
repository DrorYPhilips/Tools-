using System;
using System.Windows.Forms;
using Datapath.RGBEasy;

namespace Sample1
{
   static class Program
   {
      /// <summary>
      /// The main entry point for the application.
      /// </summary>
      /// 
       [STAThread]
       static void Main()
       {
           RGBERROR error = 0;
           IntPtr hRGBDLL = IntPtr.Zero;

           /* Load RGBEASY here rather than inside the Sample3 class itself. */
           error = RGB.Load(out hRGBDLL);
           if (error == RGBERROR.NO_ERROR)
           {
               Application.EnableVisualStyles();
               Application.SetCompatibleTextRenderingDefault(false);
               Application.Run(new Sample1(hRGBDLL));
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