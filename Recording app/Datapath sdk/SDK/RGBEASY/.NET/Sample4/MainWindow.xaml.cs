using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Datapath.RGBEasy;

namespace Sample4
{
   /// <summary>
   /// Interaction logic for MainWindow.xaml
   /// </summary>
   public partial class MainWindow : Window
   {
      private IntPtr dllHandle = IntPtr.Zero;
      private IntPtr rgbHandle = IntPtr.Zero;
      private IntPtr audioSession = IntPtr.Zero;
      private uint input = 0;

      public MainWindow()
      {
         InitializeComponent();
         this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
      }

      private void MainWindow_Loaded(object sender, RoutedEventArgs e)
      {
         RGBERROR error;

         error = RGB.Load(out dllHandle);
         if (error == RGBERROR.NO_ERROR)
         {
            error = RGB.OpenInput(input, out rgbHandle);
            if (error == RGBERROR.NO_ERROR)
            {
               error = RGB.SetWindow(rgbHandle, rgbControl.Handle);
               if (error == RGBERROR.NO_ERROR)
               {
                  error = RGBAUDIO.OpenSharedAudio(input, out audioSession);
               }
            }
         }
      }

      private void MainWindow_Closing(object sender, EventArgs e)
      {
         if (dllHandle != IntPtr.Zero)
         {
            if (dllHandle != IntPtr.Zero)
            {
               if (audioSession != IntPtr.Zero)
               {
                  RGBAUDIO.CloseSharedAudio(audioSession);
               }
               RGB.SetWindow(rgbHandle, IntPtr.Zero);
               RGB.CloseInput(rgbHandle);
            }
            RGB.Free(dllHandle);
         }
      }

      private void MenuItem_Exit(object sender, RoutedEventArgs e)
      {
         Application.Current.Shutdown();
      }
      private void MenuItem_Audio(object sender, RoutedEventArgs e)
      {
         RGBAUDIO.GetAudioFormatIndex(audioSession, out uint index);
         AudioProperties audioDialog = new AudioProperties(input, index, audioSession);
         audioDialog.ShowDialog();
         // the audio session may have changed
         audioSession = audioDialog.GetAudioSession();
      }
   }
}
