using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using Datapath.RGBEasy;

namespace Sample4
{
   /// <summary>
   /// Interaction logic for AudioProperties.xaml
   /// </summary>
   public partial class AudioProperties : Window
   {
      private uint _input;
      private uint _format;
      private IntPtr _session = IntPtr.Zero;

      private bool bAnalogue;
      private bool bDigital;
      private bool bLineOut;
      private AUDIOCAPTURETYPE captureType;
      private bool bInitialising = true;

      public AudioProperties(uint input, uint format, IntPtr audioSession)
      {
         _input = input;
         _format = format;
         _session = audioSession;
         InitializeComponent();
         this.Loaded += new RoutedEventHandler(Audio_Loaded);
      }

      /// <summary>
      /// Enable and disable UI contols based on the hardware configuration
      /// </summary>
      private void EnableDisableUI()
      {
         if (bDigital && bAnalogue)
         {
            // VisionLC cards are HDMI and SDI cards that support embedded audio
            // and, with an AM2 module fitted, external analogue audio
            if (captureType == AUDIOCAPTURETYPE.AUDIOCAPTURETYPE_EMBEDDED)
            {
               DigitalTab.IsEnabled = true;
               AnalogueTab.IsEnabled = false;
               MixerTab.IsEnabled = false;

               ChannelPair.IsEnabled = true;
               DigitalMute.IsEnabled = true;
            }
            else
            {
               DigitalTab.IsEnabled = false;
               AnalogueTab.IsEnabled = true;
               MixerTab.IsEnabled = true;

               ChannelPair.IsEnabled = false;
               DigitalMute.IsEnabled = false;
            }
         }
         else
         {
            DigitalTab.IsEnabled = bDigital;
            AnalogueTab.IsEnabled = bAnalogue;
            MixerTab.IsEnabled = bAnalogue;
            ChannelPair.IsEnabled = bDigital;
            DigitalMute.IsEnabled = bDigital;
         }
         LineOutTab.IsEnabled = bLineOut;
      }

      /// <summary>
      /// Initialise all of the values based on the hardware configuration
      /// </summary>
      private void Audio_Loaded(object sender, RoutedEventArgs e)
      {
         // Initialise the Audio Source
         RGBAUDIO.IsAnalogueSupported(_input, out bAnalogue);
         RGBAUDIO.IsHDMISupported(_input, out bool bHDMI);
         RGBAUDIO.IsSDISupported(_input, out bool bSDI);
         RGBAUDIO.IsLineOutSupported(_input, out bLineOut);
         RGBAUDIO.GetCaptureType(_input, out captureType);
         bDigital = bHDMI || bSDI;

         Initialise_FormatTab();
         Initialise_DigitalTab();
         Initialise_AnalogueTab();
         Initialise_MixerTab();
         Initialise_LineOutTab();
         EnableDisableUI();

         bInitialising = false;
      }

      /// <summary>
      /// Initialise the Audio Format tab controls
      /// </summary>
      private void Initialise_FormatTab()
      {
         // Is the embedded or external anlogue input selected for capture
         if (bDigital && bAnalogue)
         {
            if (captureType == AUDIOCAPTURETYPE.AUDIOCAPTURETYPE_EMBEDDED)
            {
               AudioSourceD.IsChecked = true;
            }
            else
            {
               AudioSourceD.IsChecked = false;
            }
         }
         else
         {
            if (bDigital)
            {
               AudioSourceD.IsChecked = true;
            }
            else
            {
               AudioSourceD.IsChecked = false;
            }
         }
         AudioSourceA.IsChecked = !AudioSourceD.IsChecked;

         // Sample Rate - must match the Speakers/Headphone sample rate for WASAPI.
         var caps = new AudioCaps();
         caps.Size = Convert.ToUInt32(Marshal.SizeOf(typeof(AudioCaps)));

         RGBAUDIO.GetCapabilities(_input, _format, ref caps);
         switch (caps.SamplesPerSec)
         {
            case AUDIOCAPTURESAMPLESPERSEC.AUDIOCAPTURESAMPLESPERSEC_32000:
               SampleFormat.SelectedIndex = 0;
               break;
            case AUDIOCAPTURESAMPLESPERSEC.AUDIOCAPTURESAMPLESPERSEC_44100:
               SampleFormat.SelectedIndex = 1;
               break;
            case AUDIOCAPTURESAMPLESPERSEC.AUDIOCAPTURESAMPLESPERSEC_48000:
               SampleFormat.SelectedIndex = 2;
               break;
            case AUDIOCAPTURESAMPLESPERSEC.AUDIOCAPTURESAMPLESPERSEC_96000:
               SampleFormat.SelectedIndex = 3;
               break;
            default:
               break;
         }
      }

      /// <summary>
      /// Initialise the Digital Audio tab controls
      /// </summary>
      private void Initialise_DigitalTab()
      {
         if (bDigital)
         {
            RGBAUDIO.GetDigitalChannelPair(_input, out uint pair);
            ChannelPair.SelectedIndex = (int)pair;

            RGBAUDIO.GetDigitalMute(_input, out bool mute);
            DigitalMute.IsChecked = mute;
         }
      }

      private double GainToDecibels(int gain, uint scale)
      {
         return (double)(gain * scale) / 1000.0;
      }

      /// <summary>
      /// Initialise the Analogue Audio tab controls
      /// </summary>
      private void Initialise_AnalogueTab()
      {
         if (bAnalogue)
         {
            // Unbalanced (RCA)
            RGBAUDIO.GetAnalogueUnbalancedMute(_input, out bool mute);
            RGBAUDIO.GetAnalogueUnbalancedGainMinimum(_input, out int min);
            RGBAUDIO.GetAnalogueUnbalancedGainMaximum(_input, out int max);
            RGBAUDIO.GetAnalogueUnbalancedGain(_input, out int val);
            RGBAUDIO.GetAnalogueUnbalancedGainScale(_input, out uint scale);
            UnbalancedMute.IsChecked = mute;
            UnbalancedGain.Minimum = min;
            UnbalancedGain.Maximum = max;
            UnbalancedGain.Value = val;
            UnbalancedDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale));

            // Balanced (XLR)
            RGBAUDIO.GetAnalogueBalancedMute(_input, out mute);
            RGBAUDIO.GetAnalogueBalancedGainBoost(_input, out bool boost);
            RGBAUDIO.GetAnalogueBalancedGainMinimum(_input, out min);
            RGBAUDIO.GetAnalogueBalancedGainMaximum(_input, out max);
            RGBAUDIO.GetAnalogueBalancedGain(_input, out val);
            RGBAUDIO.GetAnalogueBalancedGainScale(_input, out scale);
            BalancedMute.IsChecked = mute;
            BalancedBoost.IsChecked = boost;
            BalancedGain.Minimum = min;
            BalancedGain.Maximum = max;
            BalancedGain.Value = val;
            BalancedDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale) + (boost ? 20.0 : 0.0));
         }
      }

      /// <summary>
      /// Initialise the Mixer (ADC) Audio tab controls
      /// </summary>
      private void Initialise_MixerTab()
      {
         if (bAnalogue)
         {
            RGBAUDIO.GetADCMute(_input, out bool mute);
            RGBAUDIO.GetADCGainMinimum(_input, out int min);
            RGBAUDIO.GetADCGainMaximum(_input, out int max);
            RGBAUDIO.GetADCGain(_input, out int val);
            RGBAUDIO.GetADCGainScale(_input, out uint scale);
            MixerMute.IsChecked = mute;
            MixerGain.Minimum = min;
            MixerGain.Maximum = max;
            MixerGain.Value = val;
            MixerDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale));
         }
      }
      /// <summary>
      /// Initialise the LineOut Audio tab controls
      /// </summary>
      private void Initialise_LineOutTab()
      {
         if (bLineOut)
         {
            RGBAUDIO.GetLineOutSource(_input, out AUDIOCAPTURESOURCE source);
            if (source == AUDIOCAPTURESOURCE.AUDIOCAPTURESOURCE_ANALOGUE)
            {
               LineOutAnalogue.IsChecked = true;
            }
            else
            {
               LineOutDigital.IsChecked = true;
            }

            RGBAUDIO.GetLineOutMute(_input, out bool mute);
            RGBAUDIO.GetLineOutGainMinimum(_input, out int min);
            RGBAUDIO.GetLineOutGainMaximum(_input, out int max);
            RGBAUDIO.GetLineOutGain(_input, out int val);
            RGBAUDIO.GetLineOutGainScale(_input, out uint scale);
            LineOutMute.IsChecked = mute;
            LineOutGain.Minimum = min;
            LineOutGain.Maximum = max;
            LineOutGain.Value = val;
            LineOutDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale));
         }
      }

      /// <summary>
      /// Handler for the Audio Source controls that select between
      /// embedded audio and external analogue audio from the AM2 module.
      /// Audio canot be running
      /// </summary>
      private void AudioSource_Click(object sender, RoutedEventArgs e)
      {
         if (AudioSourceA.IsChecked == true)
         {
            captureType = AUDIOCAPTURETYPE.AUDIOCAPTURETYPE_EXTERNAL;
         }
         else
         {
            captureType = AUDIOCAPTURETYPE.AUDIOCAPTURETYPE_EMBEDDED;
         }
         RGBAUDIO.CloseSharedAudio(_session);
         _session = IntPtr.Zero;
         RGBAUDIO.SetCaptureType(_input, captureType);
         RGBAUDIO.OpenSharedAudio(_input, out _session);

         EnableDisableUI();
      }

      /// <summary>
      /// Handler for the digital channel pair checkbox
      /// </summary>
      private void ChannelPair_SelectionChanged(object sender, SelectionChangedEventArgs e)
      {
         RGBAUDIO.SetDigitalChannelPair(_input, (uint)ChannelPair.SelectedIndex);
      }

      /// <summary>
      /// Handler for the mute checkboxes.
      /// </summary>
      /// <param name="sender"></param>
      /// <param name="e"></param>
      private void Mute_Click(object sender, RoutedEventArgs e)
      {
         // TODO: Is there a better way of doing this?
         if (!bInitialising)
         {
            CheckBox checkBox = (CheckBox)e.OriginalSource;
            bool mute = checkBox.IsChecked ?? false;

            if (checkBox == DigitalMute)
            {
               RGBAUDIO.SetDigitalMute(_input, mute);
            }
            else if (checkBox == UnbalancedMute)
            {
               RGBAUDIO.SetAnalogueUnbalancedMute(_input, mute);
            }
            else if (checkBox == BalancedMute)
            {
               RGBAUDIO.SetAnalogueBalancedMute(_input, mute);
            }
            else if (checkBox == MixerMute)
            {
               RGBAUDIO.SetADCMute(_input, mute);
            }
            else if (checkBox == LineOutMute)
            {
               RGBAUDIO.SetLineOutMute(_input, mute);
            }
         }
      }

      /// <summary>
      /// Handler for the gain sliders.
      /// </summary>
      /// <param name="sender"></param>
      /// <param name="e"></param>
      private void Gain_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
      {
         // TODO: Is there a better way of doing this?
         if (!bInitialising)
         {
            Slider slider = (Slider)e.OriginalSource;
            int val = (int)slider.Value;

            if (slider == UnbalancedGain)
            {
               RGBAUDIO.SetAnalogueUnbalancedGain(_input, val);
               RGBAUDIO.GetAnalogueUnbalancedGainScale(_input, out uint scale);
               UnbalancedDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale));
            }
            else if (slider == BalancedGain)
            {
               RGBAUDIO.SetAnalogueBalancedGain(_input, val);
               RGBAUDIO.GetAnalogueBalancedGainScale(_input, out uint scale);
               RGBAUDIO.GetAnalogueBalancedGainBoost(_input, out bool boost);
               BalancedDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale) + (boost ? 20.0 : 0.0));
            }
            else if (slider == MixerGain)
            {
               RGBAUDIO.SetADCGain(_input, val);
               RGBAUDIO.GetADCGainScale(_input, out uint scale);
               MixerDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale));
            }
            else if (slider == LineOutGain)
            {
               RGBAUDIO.SetLineOutGain(_input, val);
               RGBAUDIO.GetLineOutGainScale(_input, out uint scale);
               LineOutDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale));
            }
         }
      }

      private void SampleFormat_SelectionChanged(object sender, SelectionChangedEventArgs e)
      {

      }

      private void Boost_Click(object sender, RoutedEventArgs e)
      {
         // TODO: Is there a better way of doing this?
         if (!bInitialising)
         {
            bool boost = BalancedBoost.IsChecked ?? false;
            RGBAUDIO.SetAnalogueBalancedGainBoost(_input, boost);
            RGBAUDIO.GetAnalogueBalancedGain(_input, out int val);
            RGBAUDIO.GetAnalogueBalancedGainScale(_input, out uint scale);
            BalancedDecibels.Text = String.Format("{0} dB", GainToDecibels(val, scale) + (boost ? 20.0 : 0.0));
         }
      }

      private void LineOut_Click(object sender, RoutedEventArgs e)
      {
         // TODO: Is there a better way of doing this?
         if (!bInitialising)
         {
            bool analogue = LineOutAnalogue.IsChecked ?? false;
            if (analogue)
            {
               RGBAUDIO.SetLineOutSource(_input, AUDIOCAPTURESOURCE.AUDIOCAPTURESOURCE_ANALOGUE);
            }
            else
            {
               // TODO: We don't know wether we are SDI or HDMI/DP so we have to set both.
               // One of these will return an error!!!!
               RGBAUDIO.SetLineOutSource(_input, AUDIOCAPTURESOURCE.AUDIOCAPTURESOURCE_SDI);
               RGBAUDIO.SetLineOutSource(_input, AUDIOCAPTURESOURCE.AUDIOCAPTURESOURCE_HDMI);
            }
         }
      }

      public IntPtr GetAudioSession()
      {
         return _session;
      }
   }
}
