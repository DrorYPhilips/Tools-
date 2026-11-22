using System;
using System.IO;
using System.Threading;
using System.Windows.Forms;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Runtime.InteropServices;
using Datapath.RGBEasy;

namespace Sample2
{
   public partial class InputSettings : Form
   {
      private IntPtr rgb;
      private GCHandle m_gcHandle;
      private RGBMODECHANGEDFN m_ModeChangedFN;
      private RGBNOSIGNALFN m_NoSignalFN;
      private RGBINVALIDSIGNALFN m_InvalidSignalFN;
      private RGBVALUECHANGEDFN m_ValueChangedFN;
      
       public InputSettings(IntPtr capture)
      {
         // Initialise Windows Forms stuff
         InitializeComponent();

         // Stow away our capture handle
         rgb = capture;

         // Initialise our delegates
         m_ModeChangedFN = new RGBMODECHANGEDFN(ModeChangedFN);
         m_NoSignalFN = new RGBNOSIGNALFN(NoSignalFN);
         m_InvalidSignalFN = new RGBINVALIDSIGNALFN(InvalidSignalFN);
         m_ValueChangedFN = new RGBVALUECHANGEDFN(ValueChangedFN);

         // Get the limits of the input source and sets its control
         {
            uint uMaximum, uCurrent;
            RGB.GetNumberOfInputs(out uMaximum);
            RGB.GetInput(rgb, out uCurrent);
            InputSource.Minimum = 0;
            InputSource.Maximum = uMaximum - 1;
            InputSource.Value = uCurrent;
         }

         // Initialise the remaining controls
         InitialiseDynamicControls();

         m_gcHandle = GCHandle.Alloc(this);

         RGB.SetModeChangedFn(rgb, m_ModeChangedFN, GCHandle.ToIntPtr(m_gcHandle));
         RGB.SetNoSignalFn(rgb, m_NoSignalFN, GCHandle.ToIntPtr(m_gcHandle));
         RGB.SetInvalidSignalFn(rgb, m_InvalidSignalFN, GCHandle.ToIntPtr(m_gcHandle));
         RGB.SetValueChangedFn(rgb, m_ValueChangedFN, GCHandle.ToIntPtr(m_gcHandle));

         ShowDialog();

         RGB.SetValueChangedFn(rgb, null, IntPtr.Zero);
         RGB.SetInvalidSignalFn(rgb, null, IntPtr.Zero);
         RGB.SetNoSignalFn(rgb, null, IntPtr.Zero);
         RGB.SetModeChangedFn(rgb, null, IntPtr.Zero);

         m_gcHandle.Free();

         this.Close();
      }
      /************************************************************************/

      public static  void
      ModeChangedFN(
         IntPtr window,
         IntPtr capture,
         ref RGBMODECHANGEDINFO info,
         IntPtr userData)
      {
         GCHandle gch = GCHandle.FromIntPtr((IntPtr)(userData));
         InputSettings ips = (InputSettings)gch.Target;

         IAsyncResult ar = ips.BeginInvoke(new InitialiseDynamicControlsDelegate(ips.InitialiseDynamicControls));
         ar.AsyncWaitHandle.WaitOne();
      }

      /************************************************************************/
      
      public static  void
      NoSignalFN(
         IntPtr window,
         IntPtr capture,
         IntPtr userData)
      {
         GCHandle gch = GCHandle.FromIntPtr((IntPtr)(userData));
         InputSettings ips = (InputSettings)gch.Target;

         RGB.NoSignal(capture);
         IAsyncResult ar = ips.BeginInvoke(new EnableControlsDelegate(ips.EnableControls),
               new object[] { false, false });
         ar.AsyncWaitHandle.WaitOne();
      }

      /************************************************************************/
      
      public static  void
      InvalidSignalFN(
         IntPtr window,
         IntPtr capture,
         uint horizontal,
         uint vertical,
         IntPtr userData)
      {
         GCHandle gch = GCHandle.FromIntPtr((IntPtr)(userData));
         InputSettings ips = (InputSettings)gch.Target;

         RGB.InvalidSignal(capture, horizontal, vertical);
         IAsyncResult ar = ips.BeginInvoke(new EnableControlsDelegate(ips.EnableControls),
               new object[] { false, false });
         ar.AsyncWaitHandle.WaitOne();
      }

      /************************************************************************/

      public  void
      ValueChangedFN(
         IntPtr window,
         IntPtr capture,
         ref RGBVALUECHANGEDINFO Info,
         IntPtr userData)
      {
         GCHandle gch = GCHandle.FromIntPtr(userData);
         InputSettings ips = (InputSettings)gch.Target;

         IAsyncResult ar = ips.BeginInvoke(new UpdateControlsDelegate(ips.UpdateControls),
               new object[] { capture, Info });
         ar.AsyncWaitHandle.WaitOne();
      }

      /************************************************************************/
      
       delegate void
      UpdateControlsDelegate(
         IntPtr capture,
         ref RGBVALUECHANGEDINFO info);

      public  void
      UpdateControls(
         IntPtr capture,
         ref RGBVALUECHANGEDINFO info)
      {
         if (info.HorPosition.BChanged != 0)
         {
            HorizontalPosition.Value = info.HorPosition.Value;
            HorizontalPositionStatic.Text = HorizontalPosition.Value.ToString();
         }
         if (info.HorScale.BChanged != 0)
         {
            HorizontalSize.Value = (int)info.HorScale.Value;
            HorizontalSizeStatic.Text = HorizontalSize.Value.ToString();
         }
         if (info.VerPosition.BChanged != 0)
         {
            VerticalPosition.Value = (int)info.VerPosition.Value;
            VerticalPositionStatic.Text = VerticalPosition.Value.ToString();
         }
         if (info.CaptureWidth.BChanged != 0)
         {
            CaptureWidth.Value = info.CaptureWidth.Value;
         }
         if (info.CaptureHeight.BChanged != 0)
         {
            CaptureHeight.Value = (int)info.CaptureHeight.Value;
         }
         if (info.Brightness.BChanged != 0)
         {
            Brightness.Value = info.Brightness.Value;
            BrightnessStatic.Text = Brightness.Value.ToString();
         }
         if (info.Contrast.BChanged != 0)
         {
            Contrast.Value = info.Contrast.Value;
            ContrastStatic.Text = Contrast.Value.ToString();
         }
         if (info.BlackLevel.BChanged != 0)
         {
            BlackLevel.Value = info.BlackLevel.Value;
            BlackLevelStatic.Text = BlackLevel.Value.ToString();
         }
         if (info.Phase.BChanged != 0)
         {
            Phase.Value = info.Phase.Value;
            PhaseStatic.Text = Phase.Value.ToString();
         }
      }

      /************************************************************************/

      delegate void
      EnableControlsDelegate(
         bool valid,
         bool dvi);
 
      public void
      EnableControls(
         bool valid,
         bool dvi)
      {
         CaptureWidth.Enabled = valid && !dvi;
         CaptureHeight.Enabled = valid && !dvi; ;
         HorizontalPosition.Enabled = valid && !dvi; ;
         HorizontalSize.Enabled = valid && !dvi; ;
         Phase.Enabled = valid && !dvi; ;
         VerticalPosition.Enabled = valid && !dvi; ;
         BlackLevel.Enabled = valid && !dvi; ;
         Brightness.Enabled = valid;
         Contrast.Enabled = valid;
      }

      /************************************************************************/

      delegate void
      InitialiseDynamicControlsDelegate();

      public  void
      InitialiseDynamicControls()
      {
         RGBMODEINFO modeInfo;

         modeInfo.Size = (uint)Marshal.SizeOf(typeof(RGBMODEINFO));
         RGB.GetModeInfo(rgb, out modeInfo);

         if (modeInfo.State == CAPTURESTATE.CAPTURING)
         {
            int iMaximum, iMinimum, iCurrent;
            uint uMinimum, uMaximum, uCurrent;

            // CaptureWidth
            RGB.GetCaptureWidthMaximum(rgb, out uMaximum);
            RGB.GetCaptureWidthMinimum(rgb, out uMinimum);
            RGB.GetCaptureWidth(rgb, out uCurrent);
            CaptureWidth.Maximum = uMaximum;
            CaptureWidth.Minimum = uMinimum;
            CaptureWidth.Value = uCurrent;
            {
               RGBERROR error = RGBERROR.NO_ERROR;
               uint value = uCurrent;

               do
               {
                  value++;
                  error = RGB.TestCaptureWidth(rgb, value);
               } while ((error == RGBERROR.HORADDRTIME_NOT_WORD_DIVISIBLE) ||
                        (error == RGBERROR.HORADDRTIME_NOT_DWORD_DIVISIBLE) ||
                        (error == RGBERROR.HORADDRTIME_NOT_QWORD_DIVISIBLE));

               CaptureWidth.Increment = value - uCurrent;
            }

            // CaptureHeight
            RGB.GetCaptureHeightMaximum(rgb, out uMaximum);
            RGB.GetCaptureHeightMinimum(rgb, out uMinimum);
            RGB.GetCaptureHeight(rgb, out uCurrent);
            CaptureHeight.Maximum = uMaximum;
            CaptureHeight.Minimum = uMinimum;
            CaptureHeight.Value = uCurrent;

            // HorizontalPosition
            RGB.GetHorPositionMaximum(rgb, out iMaximum);
            RGB.GetHorPositionMinimum(rgb, out iMinimum);
            RGB.GetHorPosition(rgb, out iCurrent);
            HorizontalPosition.Maximum = iMaximum;
            HorizontalPosition.Minimum = iMinimum;
            HorizontalPosition.Value = iCurrent;
            HorizontalPositionStatic.Text = iCurrent.ToString();

            // HorizontalSize
            RGB.GetHorScaleMaximum(rgb, out uMaximum);
            RGB.GetHorScaleMinimum(rgb, out uMinimum);
            RGB.GetHorScale(rgb, out uCurrent);
            HorizontalSize.Maximum = (int)uMaximum;
            HorizontalSize.Minimum = (int)uMinimum;
            HorizontalSize.Value = (int)uCurrent;
            HorizontalSizeStatic.Text = uCurrent.ToString();

            // Phase
            RGB.GetPhaseMaximum(rgb, out iMaximum);
            RGB.GetPhaseMinimum(rgb, out iMinimum);
            RGB.GetPhase(rgb, out iCurrent);
            Phase.Maximum = iMaximum;
            Phase.Minimum = iMinimum;
            Phase.Value = iCurrent;
            PhaseStatic.Text = iCurrent.ToString();

            // VerticalPosition
            RGB.GetVerPositionMaximum(rgb, out iMaximum);
            RGB.GetVerPositionMinimum(rgb, out iMinimum);
            RGB.GetVerPosition(rgb, out iCurrent);
            VerticalPosition.Maximum = iMaximum;
            VerticalPosition.Minimum = iMinimum;
            VerticalPosition.Value = iCurrent;
            VerticalPositionStatic.Text = iCurrent.ToString();

            // BlackLevel
            RGB.GetBlackLevelMaximum(rgb, out iMaximum);
            RGB.GetBlackLevelMinimum(rgb, out iMinimum);
            RGB.GetBlackLevel(rgb, out iCurrent);
            BlackLevel.Maximum = iMaximum;
            BlackLevel.Minimum = iMinimum;
            BlackLevel.Value = iCurrent;
            BlackLevelStatic.Text = iCurrent.ToString();

            // Brightness      
            RGB.GetBrightnessMaximum(rgb, out iMaximum);
            RGB.GetBrightnessMinimum(rgb, out iMinimum);
            RGB.GetBrightness(rgb, out iCurrent);
            Brightness.Maximum = iMaximum;
            Brightness.Minimum = iMinimum;
            Brightness.Value = iCurrent;
            BrightnessStatic.Text = iCurrent.ToString();

            // Contrast
            RGB.GetContrastMaximum(rgb, out iMaximum);
            RGB.GetContrastMinimum(rgb, out iMinimum);
            RGB.GetContrast(rgb, out iCurrent);
            Contrast.Maximum = iMaximum;
            Contrast.Minimum = iMinimum;
            Contrast.Value = iCurrent;
            ContrastStatic.Text = iCurrent.ToString();
         }

         EnableControls(
               modeInfo.State == CAPTURESTATE.CAPTURING,
               modeInfo.BDVI == 1 );
      }

      /************************************************************************/


      private void InputSource_ValueChanged(object sender, EventArgs e)
      {
         RGB.SetInput(rgb, (uint)InputSource.Value);
      }

      private void CaptureWidth_ValueChanged(object sender, EventArgs e)
      {
         RGB.SetCaptureWidth(rgb, (uint)CaptureWidth.Value);
      }

      private void CaptureHeight_ValueChanged(object sender, EventArgs e)
      {
         RGB.SetCaptureHeight(rgb, (UInt32)CaptureHeight.Value);
      }

      private void HorizontalPosition_Scroll(object sender, ScrollEventArgs e)
      {
         RGB.SetHorPosition(rgb, HorizontalPosition.Value);
         HorizontalPositionStatic.Text = HorizontalPosition.Value.ToString();
      }

      private void HorizontalSize_Scroll(object sender, ScrollEventArgs e)
      {
         RGB.SetHorScale(rgb, (UInt32)HorizontalSize.Value);
         HorizontalSizeStatic.Text = HorizontalSize.Value.ToString();
      }

      private void Phase_Scroll(object sender, ScrollEventArgs e)
      {
         RGB.SetPhase(rgb, Phase.Value);
         PhaseStatic.Text = Phase.Value.ToString();
      }

      private void VerticalPosition_Scroll(object sender, ScrollEventArgs e)
      {
         RGB.SetVerPosition(rgb, VerticalPosition.Value);
         VerticalPositionStatic.Text = VerticalPosition.Value.ToString();
      }

      private void BlackLevel_Scroll(object sender, ScrollEventArgs e)
      {
         RGB.SetBlackLevel(rgb, BlackLevel.Value);
         BlackLevelStatic.Text = BlackLevel.Value.ToString();
      }

      private void Brightness_Scroll(object sender, ScrollEventArgs e)
      {
         RGB.SetBrightness(rgb, Brightness.Value);
         BrightnessStatic.Text = Brightness.Value.ToString();
      }

      private void Contrast_Scroll(object sender, ScrollEventArgs e)
      {
         RGB.SetContrast(rgb, Contrast.Value);
         ContrastStatic.Text = Contrast.Value.ToString();
      }

      private  void CloseDialog_Click(object sender, EventArgs e)
      {
         this.Close();
      }

      private void InputSettings_Load(object sender, EventArgs e)
      {

      }
   }
}