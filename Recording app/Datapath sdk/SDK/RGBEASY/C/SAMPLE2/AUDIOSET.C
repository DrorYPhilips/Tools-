/*******************************************************************************

Copyright Datapath Ltd. 2013.

File:    AudioSet.c

Purpose: Implements the audio dialogue.

History:
         20 FEB 13    RL   Created.

*******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <commctrl.h>

#include <audiocap.h>
#include <audio.h>
#include <audioAPI.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

#include "sample2.h"
#include "resource.h"

/******************************************************************************/

static void
SetAudioValueFromSlider (
   HWND        hWnd,
   HWND        hCtl,
   ULONG       input,
   SCROLLINFO  *pSi )
{
   ULONG scale;

   switch ( GetDlgCtrlID(hCtl) )
   {
      case IDC_UNBALANCEDGAIN:
      {
         if(RGBAudioSetAnalogueUnbalancedGain(input, pSi->nPos)== RGBERROR_NO_ERROR)
         {
            SetScrollInfo( hCtl, SB_CTL, pSi, TRUE );
            if (RGBAudioGetAnalogueUnbalancedGainScale(input, &scale)==RGBERROR_NO_ERROR)
            {
               TCHAR buffer[32];
               
               StringCchPrintf(buffer, 32, TEXT("%+.3f dB"), 
                  ((float)pSi->nPos * scale) / 1000.0);
               SetWindowText(GetDlgItem(hWnd, IDC_UNBALANCEDGAINVAL), buffer);
            }
            else
               SetDlgItemInt ( hWnd, IDC_UNBALANCEDGAINVAL, pSi->nPos, TRUE );
         }
         break;
      }
      case IDC_BALANCEDGAIN:
      {
         if(RGBAudioSetAnalogueBalancedGain(input, pSi->nPos)== RGBERROR_NO_ERROR)
         {
            SetScrollInfo( hCtl, SB_CTL, pSi, TRUE );
            if (RGBAudioGetAnalogueBalancedGainScale(input, &scale)==RGBERROR_NO_ERROR)
            {
               TCHAR buffer[32];
               StringCchPrintf(buffer, 32, TEXT("%+.3f dB"), 
                  ((float)pSi->nPos * scale) / 1000.0);
               SetWindowText(GetDlgItem(hWnd, IDC_BALANCEDGAINVAL), buffer);
            }
            else
               SetDlgItemInt ( hWnd, IDC_BALANCEDGAINVAL, pSi->nPos, TRUE );
         }
         break;
      }
      case IDC_LINEOUTVOLUME:
      {
         if(RGBAudioSetLineOutGain(input, pSi->nPos)== RGBERROR_NO_ERROR)
         {
            SetScrollInfo( hCtl, SB_CTL, pSi, TRUE );

            if (RGBAudioGetLineOutGainScale(input, &scale)==RGBERROR_NO_ERROR)
            {
               TCHAR buffer[32];
               StringCchPrintf(buffer, 32, TEXT("%+.3f dB"), 
                  ((float)pSi->nPos * scale) / 1000.0);
               SetWindowText(GetDlgItem(hWnd, IDC_LINEOUTVOLUMEVAL), buffer);
            }
            else
               SetDlgItemInt ( hWnd, IDC_LINEOUTVOLUMEVAL, pSi->nPos, TRUE );
         }
         break;
      }
      case IDC_OUTPUTMIXERVOLUME:
      {
         if(RGBAudioSetADCGain(input, pSi->nPos)== RGBERROR_NO_ERROR)
         {
            SetScrollInfo( hCtl, SB_CTL, pSi, TRUE );

            if (RGBAudioGetADCGainScale(input, &scale)==RGBERROR_NO_ERROR)
            {
               TCHAR buffer[32];
               StringCchPrintf(buffer, 32, TEXT("%+.3f dB"), 
                  ((float)pSi->nPos * scale) / 1000.0);
               SetWindowText(GetDlgItem(hWnd, IDC_OUTPUTMIXERVOLUMEVAL), buffer);
            }
            else
               SetDlgItemInt ( hWnd, IDC_OUTPUTMIXERVOLUMEVAL, pSi->nPos, TRUE );
         }
         break;
      }
   }
}

/******************************************************************************/

unsigned long
GetMinimumAudioValueForSlider (
   ULONG          input,
   int            ctlID,
   signed long    *pValue )
{
   unsigned long  error;
   
   switch ( ctlID )
   {
      case IDC_UNBALANCEDGAIN:
         error = RGBAudioGetAnalogueUnbalancedGainMinimum ( input, pValue );
         break;
      case IDC_BALANCEDGAIN:
         error = RGBAudioGetAnalogueBalancedGainMinimum ( input, pValue );
         break;
      case IDC_LINEOUTVOLUME:
         error = RGBAudioGetLineOutGainMinimum ( input, pValue );
         break;
      case IDC_OUTPUTMIXERVOLUME:
         error = RGBAudioGetADCGainMinimum ( input, pValue );
         break;
   }

   return error;
}

/******************************************************************************/

unsigned long
GetCurrentAudioValueForSlider (
   ULONG          input,
   int            ctlID,
   signed long    *pValue )
{
   unsigned long  error;

   switch ( ctlID )
   {
      case IDC_UNBALANCEDGAIN:
         error = RGBAudioGetAnalogueUnbalancedGain ( input, pValue );
         break;
      case IDC_BALANCEDGAIN:
         error = RGBAudioGetAnalogueBalancedGain ( input, pValue );
         break;
      case IDC_LINEOUTVOLUME:
         error = RGBAudioGetLineOutGain ( input, pValue );
         break;
      case IDC_OUTPUTMIXERVOLUME:
         error = RGBAudioGetADCGain ( input, pValue );
         break;
   }

   return error;
}

/******************************************************************************/

unsigned long
GetMaximumAudioValueForSlider (
   ULONG          input,
   int            ctlID,
   signed long    *pValue )
{
   unsigned long  error;

   switch ( ctlID )
   {
      case IDC_UNBALANCEDGAIN:
         error = RGBAudioGetAnalogueUnbalancedGainMaximum ( input, pValue );
         break;
      case IDC_BALANCEDGAIN:
         error = RGBAudioGetAnalogueBalancedGainMaximum ( input, pValue );
         break;
      case IDC_LINEOUTVOLUME:
         error = RGBAudioGetLineOutGainMaximum ( input, pValue );
         break;
      case IDC_OUTPUTMIXERVOLUME:
         error = RGBAudioGetADCGainMaximum ( input, pValue );
         break;
   }

   return error;
}

/******************************************************************************/

typedef struct
{
    TCHAR szName[64];

} COMBODATA;

const COMBODATA ChannelPairs[] = {

   TEXT("Channel 0 - 1\0"),
   TEXT("Channel 2 - 3\0"),
   TEXT("Channel 4 - 5\0"),
   TEXT("Channel 6 - 7\0"),
};

void
UpdateStereoPair (
   HWND        hDlg,
   ULONG       input )
{
   ULONG channelPair;
   HWND  hCtl = GetDlgItem ( hDlg, IDC_COMBO_CHANNELSELECT );
   int count = sizeof ( ChannelPairs ) / sizeof ( ChannelPairs[0] );
   int i;

   for ( i = count-1; i > -1; i-- )
   {
      /* Add the string to the combo */
      if (  SendMessage ( hCtl, CB_FINDSTRINGEXACT, -1, 
            (LPARAM)ChannelPairs[i].szName ) == CB_ERR )
      {
         SendMessage ( hCtl, CB_INSERTSTRING, 0, 
               (LPARAM)ChannelPairs[i].szName );
      }
   }

   if ( RGBAudioGetDigitalChannelPair ( input, &channelPair ) == RGBERROR_NO_ERROR )
   {
      EnableWindow ( GetDlgItem ( hDlg, IDC_COMBO_CHANNELSELECT ), TRUE );
      SendMessage ( hCtl, CB_SETCURSEL, channelPair, 0 );
   }
}

/******************************************************************************/

void
UpdateAudioFormats (
   HWND        hDlg,
   HAUDIOINPUT handle,
   ULONG       input )
{
   ULONG error, formatCount, i;
   TCHAR name[64];
   PAUDIOCAPS pFormats;
   LRESULT index;
   HWND hCtl = GetDlgItem ( hDlg, IDC_COMBO_SAMPLESELECT );
   ULONG currentIndex;
   
   if ( GetAudioFormatIndex ( handle, &currentIndex ) != S_OK )
      return;

   error = RGBAudioGetCapabilitiesCount ( input, &formatCount );
   if ( error == RGBERROR_NO_ERROR )
   {
      pFormats = (PAUDIOCAPS) malloc ( formatCount * sizeof(AUDIOCAPS) );
      if ( pFormats )
      {
         /* Store RGBEasy supported audio formats. */
         for ( i = 0; i < formatCount; i++ )
         {
            pFormats[i].Size = sizeof(AUDIOCAPS);
            RGBAudioGetCapabilities ( input, i, &pFormats[i] );

            StringCchPrintf( name, 64, TEXT("%dHz - %dbpp"), 
                  pFormats[i].SamplesPerSec, pFormats[i].SampleDepth );

            if (  SendMessage ( hCtl, CB_FINDSTRINGEXACT, -1, (LPARAM)name ) == CB_ERR )
            {
               index = SendMessage( hCtl, CB_ADDSTRING, 0, (LPARAM)name );
               SendMessage( hCtl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
            }

            if ( i == currentIndex )
            {
               EnableWindow ( GetDlgItem ( hDlg, IDC_COMBO_SAMPLESELECT ), TRUE );
               SendMessage ( hCtl, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)name );
            }
         }
         free ( pFormats );
      }
   }
}

/******************************************************************************/

void
UpdateAudioSlider (
   HWND  hDlg,
   ULONG input,
   UINT  sliderID )
{
   HWND  hCtl;
   unsigned long  error;
   SCROLLINFO si;

   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
   si.nPage = 1;

   hCtl = GetDlgItem ( hDlg, sliderID );

   error = GetMinimumAudioValueForSlider ( input, sliderID, &si.nMin );
   if ( error == 0 )
   {
      GetMaximumAudioValueForSlider ( input, sliderID, &si.nMax );
      GetCurrentAudioValueForSlider ( input, sliderID, &si.nPos );
      SetAudioValueFromSlider ( hDlg, hCtl, input, &si );
   }
   else
      EnableWindow ( hCtl, FALSE );
}

/******************************************************************************/

void
UpdateAudioControls (
   HWND     hDlg,
   PWINDOW  pWindow )
{
   ULONG mute = 0, boost = 0, source = 0;
   LONG supported;
  
   /* Initialise as no audio */
   EnableWindow ( GetDlgItem ( hDlg, IDC_MUTEUNBALANCEDCHECKED ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_UNBALANCEDGAIN ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_UNBALANCEDGAINVAL ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_MUTEBALANCEDCHECKED ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_BALANCEDGAIN ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_BALANCEDGAINVAL ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_COMBO_CHANNELSELECT ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_MUTECHECKED ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_ANALOGUERADIO ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_DIGITALRADIO ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_MUTELINEOUTCHECKED ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_LINEOUTVOLUME ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_LINEOUTVOLUMEVAL ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_BOOSTBALANCEDCHECKED ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_MUTEOUTPUTMIXERCHECKED ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_OUTPUTMIXERVOLUME ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_OUTPUTMIXERVOLUMEVAL ), FALSE );
   EnableWindow ( GetDlgItem ( hDlg, IDC_COMBO_SAMPLESELECT ), FALSE );

   if ( ( RGBAudioIsAudioSupported ( pWindow->Input, &supported ) == 
         RGBERROR_NO_ERROR ) && supported )
   {
      /* Initiailise format */
      UpdateAudioFormats( hDlg, pWindow->HAudioInput, pWindow->Input );

      /* Line out */
      if ( ( RGBAudioIsLineOutSupported ( pWindow->Input, &supported ) == 
            RGBERROR_NO_ERROR ) && supported )
      {
         EnableWindow ( GetDlgItem ( hDlg, IDC_LINEOUTVOLUME ), TRUE );
         EnableWindow ( GetDlgItem ( hDlg, IDC_LINEOUTVOLUMEVAL ), TRUE );

         UpdateAudioSlider ( hDlg, pWindow->Input, IDC_LINEOUTVOLUME );

         if ( RGBAudioGetLineOutMute( pWindow->Input, &mute ) == 
               RGBERROR_NO_ERROR )
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_MUTELINEOUTCHECKED ), TRUE );

            if ( mute )
               CheckDlgButton ( hDlg, IDC_MUTELINEOUTCHECKED, BST_CHECKED );
            else
               CheckDlgButton ( hDlg, IDC_MUTELINEOUTCHECKED, BST_UNCHECKED );
         }
   
         if ( RGBAudioGetLineOutSource( pWindow->Input, 
               &(AUDIOCAPTURESOURCE)source ) == RGBERROR_NO_ERROR )
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_ANALOGUERADIO ), TRUE );
            EnableWindow ( GetDlgItem ( hDlg, IDC_DIGITALRADIO ), TRUE );

            switch( source )
            {
               default:
               case AUDIOCAPTURESOURCE_ANALOGUE:
                  CheckDlgButton ( hDlg, IDC_ANALOGUERADIO, BST_CHECKED );
                  CheckDlgButton ( hDlg, IDC_DIGITALRADIO, BST_UNCHECKED );
                  break;

               case AUDIOCAPTURESOURCE_SDI:
               case AUDIOCAPTURESOURCE_HDMI:
                  CheckDlgButton ( hDlg, IDC_ANALOGUERADIO, BST_UNCHECKED );
                  CheckDlgButton ( hDlg, IDC_DIGITALRADIO, BST_CHECKED );
                  break;
            }
         }
      }
      /* Output mixer */
      UpdateAudioSlider ( hDlg, pWindow->Input, IDC_OUTPUTMIXERVOLUME );
      if ( RGBAudioGetADCMute( pWindow->Input, &source ) == RGBERROR_NO_ERROR )
      {
         EnableWindow ( GetDlgItem ( hDlg, IDC_MUTEOUTPUTMIXERCHECKED ), TRUE );
         EnableWindow ( GetDlgItem ( hDlg, IDC_OUTPUTMIXERVOLUME ), TRUE );
         EnableWindow ( GetDlgItem ( hDlg, IDC_OUTPUTMIXERVOLUMEVAL ), TRUE );

         if ( mute )
            CheckDlgButton ( hDlg, IDC_MUTEOUTPUTMIXERCHECKED, BST_CHECKED );
         else
            CheckDlgButton ( hDlg, IDC_MUTEOUTPUTMIXERCHECKED, BST_UNCHECKED );
      }
      /* Digital */
      if (  ( ( RGBAudioIsHDMISupported ( pWindow->Input, &supported ) == 
            RGBERROR_NO_ERROR ) && supported )   || 
            ( ( RGBAudioIsSDISupported ( pWindow->Input, &supported ) == 
            RGBERROR_NO_ERROR ) && supported ) )
      {
         /* Stereo pair */
         UpdateStereoPair( hDlg, pWindow->Input );
         
         if ( RGBAudioGetDigitalMute( pWindow->Input, &mute ) == 
               RGBERROR_NO_ERROR )
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_MUTECHECKED ), TRUE );

            if ( mute )
               CheckDlgButton ( hDlg, IDC_MUTECHECKED, BST_CHECKED );
            else
               CheckDlgButton ( hDlg, IDC_MUTECHECKED, BST_UNCHECKED );
         }
      }
      else if ( ( RGBAudioIsAnalogueSupported ( pWindow->Input, &supported ) == 
            RGBERROR_NO_ERROR ) && supported )
      {
         EnableWindow ( GetDlgItem ( hDlg, IDC_UNBALANCEDGAIN ), TRUE );
         EnableWindow ( GetDlgItem ( hDlg, IDC_UNBALANCEDGAINVAL ), TRUE );
         EnableWindow ( GetDlgItem ( hDlg, IDC_BALANCEDGAIN ), TRUE );
         EnableWindow ( GetDlgItem ( hDlg, IDC_BALANCEDGAINVAL ), TRUE );
 
         /* Unbalanced */
         UpdateAudioSlider ( hDlg, pWindow->Input, IDC_UNBALANCEDGAIN );
         if ( RGBAudioGetAnalogueUnbalancedMute( pWindow->Input, &mute ) == 
               RGBERROR_NO_ERROR )
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_MUTEUNBALANCEDCHECKED ), TRUE );

            if ( mute )
               CheckDlgButton ( hDlg, IDC_MUTEUNBALANCEDCHECKED, BST_CHECKED );
            else
               CheckDlgButton ( hDlg, IDC_MUTEUNBALANCEDCHECKED, BST_UNCHECKED );
         }
         /* Balanced */
         UpdateAudioSlider ( hDlg, pWindow->Input, IDC_BALANCEDGAIN );
         if ( RGBAudioGetAnalogueBalancedMute( pWindow->Input, &mute ) == 
               RGBERROR_NO_ERROR )
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_MUTEBALANCEDCHECKED ), TRUE );

            if ( mute )
               CheckDlgButton ( hDlg, IDC_MUTEBALANCEDCHECKED, BST_CHECKED );
            else
               CheckDlgButton ( hDlg, IDC_MUTEBALANCEDCHECKED, BST_UNCHECKED );
         }
         /* Balanced boost */
         if ( RGBAudioGetAnalogueBalancedGainBoost( pWindow->Input, &boost ) == 
               RGBERROR_NO_ERROR )
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_BOOSTBALANCEDCHECKED ), TRUE );

            if ( boost )
               CheckDlgButton ( hDlg, IDC_BOOSTBALANCEDCHECKED, BST_CHECKED );
            else
               CheckDlgButton ( hDlg, IDC_BOOSTBALANCEDCHECKED, BST_UNCHECKED );
         }
      }
   }
}

/******************************************************************************/

INT_PTR
ScrollFunction ( 
   HWND     hWnd,
   PWINDOW  pWindow,
   UINT     uMsg, 
   WPARAM   wParam, 
   LPARAM   lParam )
{
   int delta = 0;
   HWND hCtl;
   long pos = -1;
   SCROLLINFO si;
   BOOL bTrack = FALSE;

   if ( !pWindow )
      return RGBERROR_INVALID_POINTER;

   hCtl = (HWND)lParam;
   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
   GetScrollInfo(hCtl, SB_CTL, &si);
   pos = si.nPos;
   switch (LOWORD(wParam))
   {
      case SB_THUMBTRACK:
      case SB_THUMBPOSITION:
         si.fMask = SIF_TRACKPOS;
         GetScrollInfo(hCtl, SB_CTL, &si);
         pos = si.nTrackPos;
         bTrack = TRUE;
         break;
      case SB_LINELEFT:
         delta = -1;
         break;
      case SB_LINERIGHT:
         delta = 1;
         break;
      case SB_PAGELEFT:
         delta = -(signed int)si.nPage;
         break;
      case SB_PAGERIGHT:
         delta = si.nPage;
         break;
      case SB_LEFT:
         delta = -((signed int)si.nPos);
         break;
      case SB_RIGHT:
         delta = si.nMax-si.nPos;
         break;
   }
   if (!bTrack)
      pos += delta;

   si.fMask = SIF_POS;
   
   if (pos < si.nMin) pos = si.nMin;
   if (pos > si.nMax/*-SCRLPAGESIZE+1*/) pos = si.nMax/*-SCRLPAGESIZE+1*/;

   si.nPos = pos;
   /* Now we know where the thumb is, we can use the data. */
   SetAudioValueFromSlider ( hWnd, hCtl, pWindow->Input, &si );
   return 0;
}

/******************************************************************************/

void
SetFormatSelection (
   HWND     hDlg,
   PWINDOW  pWindow )
{
   TCHAR buffer[MAX_PATH];
   ULONG currentIndex, currentFormat, formatCount, i;

   StringCchPrintf ( buffer, MAX_PATH, 
         TEXT("Use the 'Advanced' tab in speaker properties to set the prefered sample rate and bit depth when running in shared mixer mode.\n\nRestart the application to reflect any new settings.\n\nDo you want to launch the speaker properties dialogue?") );
   
   if ( MessageBox ( hDlg, buffer, Caption, MB_YESNO | MB_ICONINFORMATION ) == 
         IDYES )
      LaunchAdvancedDefaultSpeakerPropertiesDlg ( );
   
   if ( GetAudioFormatIndex ( pWindow->HAudioInput, &currentFormat ) == S_OK )
   {
      if ( RGBAudioGetCapabilitiesCount ( pWindow->Input, &formatCount ) == 
            RGBERROR_NO_ERROR )
      {
         HWND hCtl = GetDlgItem ( hDlg, IDC_COMBO_SAMPLESELECT );

         for ( i = 0; i < formatCount; i++ )
         {
            currentIndex = (ULONG)SendMessage ( hCtl, CB_GETITEMDATA, i, 0 );

            if ( currentIndex == currentFormat )
            {
               SendMessage ( hCtl, CB_SETCURSEL, i, 0 );
               break;
            }
         }
      }
   }
}

/******************************************************************************/

INT_PTR CALLBACK
AudioSettingsDlgProc (
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam )
{
   BOOL  bReturn = FALSE;

   switch ( message )
   {
      case WM_HSCROLL:
      {
         if (LOWORD(wParam) != SB_ENDSCROLL)
         {
            ScrollFunction ( hDlg, GetProp ( hDlg, Property ), 
                  message, wParam, lParam );
            return TRUE;
         }
         break;
      }

      case WM_COMMAND:
      {
         /* WM_COMMAND messages are received before we have a chance to set the
          * property in response to the WM_INITDIALOG message. */
         PWINDOW pWindow = GetProp ( hDlg, Property );
         if ( pWindow == 0 )
            break;

         switch ( LOWORD ( wParam ) )
         {
            unsigned long  bEnable;
              
            case IDC_MUTELINEOUTCHECKED:
               bEnable = ( IsDlgButtonChecked ( hDlg, IDC_MUTELINEOUTCHECKED ) == 
                     BST_CHECKED );
               RGBAudioSetLineOutMute ( pWindow->Input,  bEnable );
               return TRUE;
            case IDC_MUTECHECKED:
               bEnable = ( IsDlgButtonChecked ( hDlg, IDC_MUTECHECKED ) == 
                     BST_CHECKED );
               RGBAudioSetDigitalMute ( pWindow->Input,  bEnable );
               return TRUE;
            case IDC_MUTEUNBALANCEDCHECKED:
               bEnable = ( IsDlgButtonChecked ( hDlg, IDC_MUTEUNBALANCEDCHECKED ) == 
                     BST_CHECKED );
               RGBAudioSetAnalogueUnbalancedMute ( pWindow->Input,  bEnable );
               return TRUE;
            case IDC_MUTEBALANCEDCHECKED:
               bEnable = ( IsDlgButtonChecked ( hDlg, IDC_MUTEBALANCEDCHECKED ) == 
                     BST_CHECKED );
               RGBAudioSetAnalogueBalancedMute ( pWindow->Input,  bEnable );
               return TRUE;
            case IDC_BOOSTBALANCEDCHECKED:
               bEnable = ( IsDlgButtonChecked ( hDlg, IDC_BOOSTBALANCEDCHECKED ) == 
                     BST_CHECKED );
               RGBAudioSetAnalogueBalancedGainBoost ( pWindow->Input,  bEnable );
               return TRUE;
            case IDC_MUTEOUTPUTMIXERCHECKED:
               bEnable = ( IsDlgButtonChecked ( hDlg, IDC_MUTEOUTPUTMIXERCHECKED ) == 
                     BST_CHECKED );
               RGBAudioSetADCMute ( pWindow->Input,  bEnable );
               return TRUE;
            case IDC_COMBO_CHANNELSELECT:
               if ( HIWORD ( wParam ) == CBN_SELCHANGE )
               {
                  HWND  hCtl;
                  ULONG index;

                  hCtl = GetDlgItem ( hDlg, IDC_COMBO_CHANNELSELECT );
                  index = (ULONG)SendMessage ( hCtl, CB_GETCURSEL, 0, 0 );
                  RGBAudioSetDigitalChannelPair ( pWindow->Input, index );
                  return TRUE;
               }
            case IDC_COMBO_SAMPLESELECT:
               if ( HIWORD ( wParam ) == CBN_SELCHANGE )
               {
                  SetFormatSelection ( hDlg, pWindow );
                  return TRUE;
               }
            case IDC_ANALOGUERADIO:
               RGBAudioSetLineOutSource( pWindow->Input, AUDIOCAPTURESOURCE_ANALOGUE );
               return TRUE;
            case IDC_DIGITALRADIO:
               RGBAudioSetLineOutSource( pWindow->Input, AUDIOCAPTURESOURCE_SDI );
               RGBAudioSetLineOutSource( pWindow->Input, AUDIOCAPTURESOURCE_HDMI );
               return TRUE;
         }
         break;
      }

      case WM_NOTIFY:
      {
         LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;
         PWINDOW pWindow = GetProp ( hDlg, Property );
         if ( pWindow == 0 )
            break;

         switch ( lpPSHNotify->hdr.code )
         {
            case PSN_SETACTIVE: /* The dialogue is about to be activated */
               UpdateAudioControls ( hDlg,  pWindow );
               return TRUE;

            case PSN_APPLY: /* The OK or Apply button was pressed. */
               /* Indicate whether or not property sheet can be destroyed. */
               SetWindowLongPtr ( hDlg, DWLP_MSGRESULT, PSNRET_NOERROR );
               return TRUE;

            case PSN_QUERYCANCEL: /* The Cancel button was pressed. */
               /* Tell Windows it can destroy the property sheet. */
               SetWindowLongPtr ( hDlg, DWLP_MSGRESULT, FALSE );
               return TRUE;

            case PSN_KILLACTIVE: /* Page is about to lose activation. */
               /* Set FALSE because want to allow page to lose activation. */
               SetWindowLongPtr ( hDlg, DWLP_MSGRESULT, FALSE );
               return TRUE;
         }
         break;
      }  /* End of case WM_NOTIFY. */

      case WM_INITDIALOG:
      {
         LPPROPSHEETPAGE   pPage = (LPPROPSHEETPAGE)lParam;
         PWINDOW           pWindow = (PWINDOW)pPage->lParam;
        
         /* Save pWindow. */
         SetProp ( hDlg, Property, pWindow );
         return TRUE;
      }  /* End of case WM_INITDIALOG. */

      default:
         break;
   }

   return FALSE;
}

/******************************************************************************/