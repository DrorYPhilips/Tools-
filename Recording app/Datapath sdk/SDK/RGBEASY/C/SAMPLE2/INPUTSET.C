/*******************************************************************************

Copyright Datapath Ltd. 2007.

File:    InputSet.c

Purpose: Implements the Input Settings dialog.

History:
         20 DEC 07    SB   Created.

*******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <commctrl.h>

#include <audio.h>
#include <audioAPI.h>
#include <audiocap.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

#include "sample2.h"
#include "resource.h"

/******************************************************************************/

#define BLANK_MODE_IMAGE   0
#define CURRENT_MODE_IMAGE 1

#define PROP_SETFOCUS         ( WM_USER + 1 )
#define WM_MODECHANGEUPDATE   ( WM_USER + 2 )
#define WM_NOSIGNALUPDATE     ( WM_USER + 3 )
#define WM_INVALIDSIGUPDATE   ( WM_USER + 4 )
#define WM_VALUECHANGEUPDATE  ( WM_USER + 5 )


typedef enum 
{
   VALIDSIGSTATE     = RGB_STATE_CAPTURING,
   NOSIGNALSTATE     = RGB_STATE_NOSIGNAL,
   INVALIDSIGSTATE   = RGB_STATE_INVALIDSIGNAL,

   CAPTURENOTRUNNING = RGB_STATE_PAUSED,
   ERRORSTATE        = RGB_STATE_ERROR,
}  SIGNALSTATE;


RGBMODECHANGEDFN  ModeChangeFn;
RGBNOSIGNALFN     NoSignalFn;
RGBINVALIDSIGNALFN   InvalidSignalFn;
RGBVALUECHANGEDFN    ValueChangeFn;

/******************************************************************************/

BOOL
GetDlgItemIntInRange (
   HWND  hDlg,
   int   iEditId,
   int   iMin,
   int   iMax,
   BOOL  bSigned,
   int   *pValue )
{
   BOOL  bTranslated;

   *pValue = GetDlgItemInt ( hDlg, iEditId, &bTranslated, bSigned );
   if ( bTranslated )
   {
      if ( bSigned )
      {
         if ( ( (int)*pValue < iMin ) || ( (int)*pValue > iMax ) )
         {
            return FALSE;
         }
      }
      else
      {
         if ( ( (unsigned int)*pValue < (unsigned int)iMin ) ||
              ( (unsigned int)*pValue > (unsigned int)iMax ) )
         {
            return FALSE;
         }
      }
      return TRUE;
   }
   return FALSE;
}

/******************************************************************************/

#define USE_SCROLL_BARS 1

void
SetTrackBarRange (
   HWND  hCtl,
   LONG  min,
   LONG  max,
   BOOL  bRedraw )
{
#if USE_SCROLL_BARS
   SetScrollRange ( hCtl, SB_CTL, (int)min, (int)max, bRedraw );
#else
   SendMessage ( hCtl, TBM_SETRANGEMIN, 0, min );
   SendMessage ( hCtl, TBM_SETRANGEMAX, 0, max );
#endif
}

/******************************************************************************/

void
GetTrackBarRange (
   HWND     hCtl,
   LPLONG   min,
   LPLONG   max )
{
#if USE_SCROLL_BARS
   {
      int   iMin, iMax;

      GetScrollRange ( hCtl, SB_CTL, &iMin, &iMax );
      *min = (long)iMin;
      *max = (long)iMax;
   }
#else
   *min = SendMessage( hCtl, TBM_GETRANGEMIN, 0, 0 );
   *max = SendMessage( hCtl, TBM_GETRANGEMAX, 0, 0 );
#endif
}

/******************************************************************************/

void
SetTrackBarPos (
   HWND  hCtl,
   LONG  pos,
   BOOL  bRedraw )
{
#if USE_SCROLL_BARS
   SetScrollPos ( hCtl, SB_CTL, (int)pos, bRedraw );
#else
   SendMessage ( hCtl, TBM_SETPOS, bRedraw, pos );
#endif
}

/******************************************************************************/

LONG
GetTrackBarPos (
   HWND  hCtl )
{
#if USE_SCROLL_BARS
   return GetScrollPos ( hCtl, SB_CTL );
#else
   return SendMessage ( hCtl, TBM_GETPOS, 0, 0 );
#endif
}

/******************************************************************************/

BOOL
Cropping (
   HWND           hDlg,
   PWINDOW        pWindow,
   unsigned long  hCurrentMode,
   BOOL           bMessage )
{
   static const struct
   {
      int   EditId;
      int   SpinId;
      BOOL  BSigned;
   } ctl[] =
   {
      { IDC_TOP,      IDC_TOPSPIN,      TRUE },
      { IDC_LEFT,     IDC_LEFTSPIN,     TRUE },
      { IDC_WIDTH,    IDC_WIDTHSPIN,    FALSE },
      { IDC_HEIGHT,   IDC_HEIGHTSPIN,   FALSE },
   };
   long           top, left;
   unsigned long  width, height;
   int   *cropping[] =
   {
      { &top },
      { &left },
      { &width },
      { &height },
   };
   int            i, iMin, iMax;
   unsigned long  error;
   BOOL           bEnabled;

   for ( i = 0; i < sizeof ( ctl ) / sizeof ( ctl[0] ); i++ )
   {
      iMin = iMax = 0;
      SendMessage ( GetDlgItem ( hDlg, ctl[i].SpinId ), UDM_GETRANGE32, 
            (WPARAM)&iMin, (LPARAM)&iMax );

      if ( GetDlgItemIntInRange ( hDlg, ctl[i].EditId, iMin, iMax,
            ctl[i].BSigned, cropping[i] ) == FALSE )
      {
         if ( bMessage )
         {
            PostMessage ( hDlg, PROP_SETFOCUS, 0, ctl[i].EditId );
         }
         return FALSE;
      }
   }
   bEnabled = IsDlgButtonChecked ( hDlg, IDC_CROPPINGON ) == BST_CHECKED;
      
   /* TODO: Get a more meaningful error number back. */
   error = RGBSetCropping ( pWindow->HRGB, top, left, width, height );
   if ( error == 0 )
   {
      error = RGBEnableCropping ( pWindow->HRGB, bEnabled );
      if ( error )
      {
         return FALSE;
      }
   }

   return TRUE;
}

/******************************************************************************/

unsigned long
IncreaseHorizontalScale (
   HWND           hDlg,
   unsigned long  width )
{
   unsigned long  horScale, horStart;
   BOOL           bTranslated;

   horScale = GetDlgItemInt ( hDlg, IDC_HORSCALEVALUE, &bTranslated, FALSE );
   if ( bTranslated )
   {
      horStart = GetDlgItemInt( hDlg, IDC_HOROFFSETVALUE, &bTranslated, FALSE );
      if ( bTranslated )
      {
         unsigned long scale;

         scale = horStart + width;
         if ( scale > horScale )
         {
            return scale;
         }
      }
   }
   return 0;
}

/******************************************************************************/

BOOL
AddrWidth ( 
   HWND     hDlg,
   PWINDOW  pWindow, 
   BOOL     bMessage )
{
   unsigned long  error, width;
   int            iMin, iMax;

   iMin = iMax = 0;
   SendMessage ( GetDlgItem ( hDlg, IDC_ADDRWIDTHSPIN ), UDM_GETRANGE32, 
         (WPARAM)&iMin, (LPARAM)&iMax );

   if ( GetDlgItemIntInRange ( hDlg, IDC_ADDRWIDTH, iMin, iMax,
         FALSE, &width ) == FALSE )
   {
      return FALSE;
   }
   {
      /* Increase the horizontal scale if needed. */
      unsigned long horizontalScale;

      horizontalScale = IncreaseHorizontalScale ( hDlg, width );

      if ( horizontalScale )
      {
         if ( RGBSetHorScale ( pWindow->HRGB, horizontalScale ) == 0 )
         {
            SetTrackBarPos ( GetDlgItem ( hDlg, IDC_HORSCALE ), 
                  horizontalScale, TRUE );
            SetDlgItemInt ( hDlg, IDC_HORSCALEVALUE, horizontalScale, TRUE );
         }
      }
   }
   error = RGBSetCaptureWidth ( pWindow->HRGB, width );

   if ( error )
   {
      return FALSE;
   }

   return TRUE;
}

/******************************************************************************/

BOOL
AddrHeight ( 
   HWND     hDlg,
   PWINDOW  pWindow,
   BOOL     bMessage )
{
   unsigned long  error, height;
   int            iMin, iMax;

   iMin = iMax = 0;
   SendMessage ( GetDlgItem ( hDlg, IDC_ADDRHEIGHTSPIN ), UDM_GETRANGE32, 
         (WPARAM)&iMin, (LPARAM)&iMax );

   if ( GetDlgItemIntInRange ( hDlg, IDC_ADDRHEIGHT, iMin, iMax,
         FALSE, &height ) == FALSE )
   {
      return FALSE;
   }

   error = RGBSetCaptureHeight ( pWindow->HRGB, height );

   if ( error )
   {
      return FALSE;
   }

   return TRUE;
}

/******************************************************************************/

static void
SetValueFromSlider (
   HWND           hDlg,
   PWINDOW        pWindow,
   int            ctlID,
   unsigned long  position )
{
   int nIDDlgItem;

   switch ( ctlID )
   {
      case IDC_BRIGHTNESS:
      {
         RGBSetBrightness ( pWindow->HRGB, position );
         nIDDlgItem = IDC_BRIGHTNESSVALUE;
         break;
      }

      case IDC_CONTRAST:
      {
         RGBSetContrast ( pWindow->HRGB, position );
         nIDDlgItem = IDC_CONTRASTVALUE;
         break;
      }

      case IDC_BLACKLEVEL:
      {
         RGBSetBlackLevel ( pWindow->HRGB, position );
         nIDDlgItem = IDC_BLACKLEVELVALUE;
         break;
      }

      case IDC_PHASE:
      {
         RGBSetPhase ( pWindow->HRGB, position );
         nIDDlgItem = IDC_PHASEVALUE;
         break;
      }

      case IDC_HOROFFSET:
      {
         RGBSetHorPosition ( pWindow->HRGB, position );
         nIDDlgItem = IDC_HOROFFSETVALUE;
         break;
      }

      case IDC_VEROFFSET:
      {
         RGBSetVerPosition ( pWindow->HRGB, position );
         nIDDlgItem = IDC_VEROFFSETVALUE;
         break;
      }

      case IDC_HORSCALE:
      {
         RGBSetHorScale ( pWindow->HRGB, position );
         nIDDlgItem = IDC_HORSCALEVALUE;
         break;
      }
      default:
         return;
   }

   SetDlgItemInt ( hDlg, nIDDlgItem, position, TRUE );
}

/******************************************************************************/

unsigned long
GetMinimumValueForSlider (
   PWINDOW        pWindow,
   int            ctlID,
   unsigned long  *pValue )
{
   unsigned long  error;

   switch ( ctlID )
   {
      case IDC_BRIGHTNESS:
         error = RGBGetBrightnessMinimum ( pWindow->HRGB, pValue );
         break;
      case IDC_CONTRAST:
         error = RGBGetContrastMinimum ( pWindow->HRGB, pValue );
         break;
      case IDC_BLACKLEVEL:
         error = RGBGetBlackLevelMinimum ( pWindow->HRGB, pValue );
         break;
      case IDC_PHASE:
         error = RGBGetPhaseMinimum ( pWindow->HRGB, pValue );
         break;
      case IDC_HOROFFSET:
         error = RGBGetHorPositionMinimum ( pWindow->HRGB, pValue );
         break;
      case IDC_VEROFFSET:
         error = RGBGetVerPositionMinimum ( pWindow->HRGB, pValue );
         break;
      case IDC_HORSCALE:
         error = RGBGetHorScaleMinimum ( pWindow->HRGB, pValue );
         break;
   }

   return error;
}

/******************************************************************************/

unsigned long
GetCurrentValueForSlider (
   PWINDOW        pWindow,
   int            ctlID,
   unsigned long  *pValue )
{
   unsigned long  error;

   switch ( ctlID )
   {
      case IDC_BRIGHTNESS:
         error = RGBGetBrightness ( pWindow->HRGB, pValue );
         break;
      case IDC_CONTRAST:
         error = RGBGetContrast ( pWindow->HRGB, pValue );
         break;
      case IDC_BLACKLEVEL:
         error = RGBGetBlackLevel ( pWindow->HRGB, pValue );
         break;
      case IDC_PHASE:
         error = RGBGetPhase ( pWindow->HRGB, pValue );
         break;
      case IDC_HOROFFSET:
         error = RGBGetHorPosition ( pWindow->HRGB, pValue );
         break;
      case IDC_VEROFFSET:
         error = RGBGetVerPosition ( pWindow->HRGB, pValue );
         break;
      case IDC_HORSCALE:
         error = RGBGetHorScale ( pWindow->HRGB, pValue );
         break;
   }

   return error;
}

/******************************************************************************/

unsigned long
GetMaximumValueForSlider (
   PWINDOW        pWindow,
   int            ctlID,
   unsigned long  *pValue )
{
   unsigned long  error;

   switch ( ctlID )
   {
      case IDC_BRIGHTNESS:
         error = RGBGetBrightnessMaximum ( pWindow->HRGB, pValue );
         break;
      case IDC_CONTRAST:
         error = RGBGetContrastMaximum ( pWindow->HRGB, pValue );
         break;
      case IDC_BLACKLEVEL:
         error = RGBGetBlackLevelMaximum ( pWindow->HRGB, pValue );
         break;
      case IDC_PHASE:
         error = RGBGetPhaseMaximum ( pWindow->HRGB, pValue );
         break;
      case IDC_HOROFFSET:
         error = RGBGetHorPositionMaximum ( pWindow->HRGB, pValue );
         break;
      case IDC_VEROFFSET:
         error = RGBGetVerPositionMaximum ( pWindow->HRGB, pValue );
         break;
      case IDC_HORSCALE:
         error = RGBGetHorScaleMaximum ( pWindow->HRGB, pValue );
         break;
   }

   return error;
}

/******************************************************************************/

static unsigned long
InintialiseInputSource (
   HWND     hDlg,
   PWINDOW  pWindow )
{
   unsigned long  error, inputs;
   HWND           hCtl;

   hCtl = GetDlgItem ( hDlg, IDC_INPUTSOURCE );

   /* Find out how many input sources are available. */
   error = RGBGetNumberOfInputs ( &inputs );
   if ( error == 0 )
   {
      unsigned long i;
      LRESULT posI;

      /* Add numeric values to the control. */
      for ( i = 0; i < inputs; i++ )
      {
         TCHAR  text[32];

         StringCchPrintf ( text, 32, TEXT("%d"), i+1 );
         posI = SendMessage ( hCtl, CB_ADDSTRING, (WPARAM)0, (LPARAM)text );
         SendMessage ( hCtl, CB_SETITEMDATA , posI, (LPARAM)i );
      }

      /* Find the current input position. */
      for ( i = 0; i < inputs; i++ )
      {
         if (pWindow->Input == SendMessage ( hCtl, CB_GETITEMDATA , 
               (WPARAM)i, (LPARAM)0 ))
         {
             SendMessage ( hCtl, CB_SETCURSEL, (WPARAM)i, (LPARAM)0 );
             break;
         }
      }

      /* Select the current Input. */
     
   }

   return error;
}

/******************************************************************************/

static unsigned long
SetInputSource (
   HWND     hDlg,
   PWINDOW  pWindow )
{
   ULONG          itemIndex, input;
   unsigned long  error = 0;
   HWND           hCtl;

   hCtl = GetDlgItem ( hDlg, IDC_INPUTSOURCE );

   itemIndex = (ULONG)SendMessage ( hCtl, CB_GETCURSEL, 0, 0 );
   input = (ULONG)SendMessage ( hCtl, CB_GETITEMDATA, (WPARAM)itemIndex, (LPARAM)0 );
   if ( input != pWindow->Input )
   {
      error = RGBSetInput ( pWindow->HRGB, (unsigned long)input );
      if ( error )
      {
         PostMessage ( hDlg, PROP_SETFOCUS, 0, IDC_INPUTSOURCE );
      }
      else
      {
         /* Audio */
         signed long audioSupported;

         pWindow->Input = input;
         
         if ( pWindow->HAudioInput )
            CloseSharedAudio ( pWindow->HAudioInput );

         pWindow->HAudioInput = 0;

         /* Does the input support audio capture. */
         if ( ( RGBAudioIsAudioSupported ( pWindow->Input, &audioSupported ) == 0 ) 
                && audioSupported )
         {
            HRESULT hr;

            hr = OpenSharedAudio ( pWindow->Input, &pWindow->HAudioInput );
            if ( FAILED(hr) )
            {
               TCHAR buffer[MAX_PATH];

               StringCchPrintf ( buffer, MAX_PATH, 
                     TEXT("Error opening audio input: 0x%08x"), hr );
               MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
            }
         }
      }
   }

   return error;
}

/******************************************************************************/

unsigned long
UpdateAddrWidth (
   HWND     hDlg,
   PWINDOW  pWindow )
{
   unsigned long  error;
   HWND           hCtl;
   unsigned long  maximum, minimum, current;

   if ( error = RGBGetCaptureWidthMaximum ( pWindow->HRGB, &maximum ) )
   {
      return error;
   }

   if ( error = RGBGetCaptureWidth ( pWindow->HRGB, &current ) )
   {
      return error;
   }

   if ( error = RGBGetCaptureWidthMinimum ( pWindow->HRGB, &minimum ) )
   {
      return error;
   }

   SetDlgItemInt ( hDlg, IDC_ADDRWIDTH, current, FALSE );

   hCtl = GetDlgItem ( hDlg, IDC_ADDRWIDTHSPIN );

   if ( hCtl )
   {
      SendMessage ( hCtl, UDM_SETRANGE32, minimum, maximum );
      SendMessage ( hCtl, UDM_SETPOS32, 0, current );
   }

   return error;
}

/******************************************************************************/

unsigned long
UpdateAddrHeight (
   HWND     hDlg,
   PWINDOW  pWindow )
{
   unsigned long  error;
   HWND           hCtl;
   unsigned long  maximum, minimum, current;

   if ( error = RGBGetCaptureHeightMaximum ( pWindow->HRGB, &maximum ) )
   {
      return error;
   }

   if ( error = RGBGetCaptureHeight ( pWindow->HRGB, &current ) )
   {
      return error;
   }

   if ( error = RGBGetCaptureHeightMinimum ( pWindow->HRGB, &minimum ) )
   {
      return error;
   }

   SetDlgItemInt ( hDlg, IDC_ADDRHEIGHT, current, FALSE );

   hCtl = GetDlgItem ( hDlg, IDC_ADDRHEIGHTSPIN );

   if ( hCtl )
   {
      SendMessage ( hCtl, UDM_SETRANGE32, minimum, maximum );
      SendMessage ( hCtl, UDM_SETPOS32, 0, current );
   }

   return error;
}

/******************************************************************************/

unsigned long
UpdateCroppingControls (
   HWND           hDlg,
   PWINDOW        pWindow )
{
   unsigned long  count;
   unsigned long  bEnable;

   struct Control
   {
      DWORD editCtrl;
      DWORD spinCtrl;
      long  minimum;
      long  current;
      long  maximum;
      BOOL  sign;
   }  controls[] = 
      {
         { IDC_TOP,     IDC_TOPSPIN,      0, 0, 0, TRUE },
         { IDC_LEFT,    IDC_LEFTSPIN,     0, 0, 0, TRUE },
         { IDC_WIDTH,   IDC_WIDTHSPIN,    0, 0, 0, FALSE },
         { IDC_HEIGHT,  IDC_HEIGHTSPIN,   0, 0, 0, FALSE },
      };

   /* TODO: error reporting. */
   RGBGetCroppingMaximum ( pWindow->HRGB,
         &controls[0].maximum, &controls[1].maximum, 
         &controls[2].maximum, &controls[3].maximum );

   RGBGetCroppingMinimum ( pWindow->HRGB,
         &controls[0].minimum, &controls[1].minimum, 
         &controls[2].minimum, &controls[3].minimum );

   /* Current comes last because we're interested in the current bEnable value.
    */
   RGBGetCropping (pWindow->HRGB,
         &controls[0].current, &controls[1].current, 
         &controls[2].current, &controls[3].current );

   RGBIsCroppingEnabled ( pWindow->HRGB, &bEnable );
   
   CheckRadioButton ( hDlg, IDC_CROPPINGON, IDC_CROPPINGOFF,
         ( bEnable ) ? IDC_CROPPINGON : IDC_CROPPINGOFF );

   for ( count = 0;
         count < sizeof ( controls ) / sizeof ( struct Control );
         count++ )
   {
      HWND hCtl;

      SetDlgItemInt ( hDlg, controls[count].editCtrl, 
            controls[count].current, 
            controls[count].sign );

      hCtl = GetDlgItem ( hDlg, controls[count].spinCtrl );

      if ( hCtl )
      {
         SendMessage ( hCtl, UDM_SETRANGE32, 
            controls[count].minimum, 
            controls[count].maximum );
      
         SendMessage ( hCtl, UDM_SETPOS32, 
            0, controls[count].current );
      }
   }

   return 0;
}

/******************************************************************************/

void
UpdateControls (
   HWND           hDlg,
   PWINDOW        pWindow )
{
   pWindow->BProgram = FALSE;

   /* Initialise the addressable width and height. */
   UpdateAddrWidth ( hDlg, pWindow );
   UpdateAddrHeight ( hDlg, pWindow );

   /* Initialise cropping controls. */
   UpdateCroppingControls ( hDlg, pWindow );

   /* Initialise the slider bars. */
   {
      struct Control
      {
         int   ScrollID;
         int   ValueID;
         long  Current;
      }  control[] =
      {
         { IDC_HOROFFSET,  IDC_HOROFFSETVALUE,  0 },
         { IDC_HORSCALE,   IDC_HORSCALEVALUE,   0 },
         { IDC_PHASE,      IDC_PHASEVALUE,      0 },
         { IDC_VEROFFSET,  IDC_VEROFFSETVALUE,  0 },
         { IDC_BLACKLEVEL, IDC_BLACKLEVELVALUE, 0 },
         { IDC_BRIGHTNESS, IDC_BRIGHTNESSVALUE, 0 },
         { IDC_CONTRAST,   IDC_CONTRASTVALUE,   0 },
      };
      int i;

      for ( i = 0; i < ( sizeof ( control ) / sizeof ( control[0] )); i++ )
      {
         HWND  hCtl;
         unsigned long  error, minimum, maximum;

         hCtl = GetDlgItem ( hDlg, control[i].ScrollID );

         error = GetMinimumValueForSlider ( pWindow, control[i].ScrollID, 
               &minimum );
         if ( error == 0 )
         {
            GetMaximumValueForSlider ( pWindow, control[i].ScrollID, &maximum );
            GetCurrentValueForSlider ( pWindow, control[i].ScrollID, 
                  &control[i].Current );

            SetTrackBarRange ( hCtl, minimum, maximum, TRUE );
            SetTrackBarPos ( hCtl, control[i].Current, TRUE );

            SetDlgItemInt ( hDlg, control[i].ValueID, control[i].Current, 
                  TRUE );
         }
         else
         {
            EnableWindow ( hCtl, FALSE );
         }
      }
   }

   /* Initialise the Capture format. */
   pWindow->BProgram = TRUE;

   return;
}

/******************************************************************************/

unsigned long
UpdateCroppingState (
   HWND     hDlg,
   PWINDOW  pWindow,
   BOOL     bValidMode )
{
   static const struct
   {
      int   ScrollID;
      int   ValueID;
      int   StaticID;
   } control[] =
      {
         { IDC_TOPSPIN,    IDC_TOP,    IDC_TOPSTATIC },
         { IDC_LEFTSPIN,   IDC_LEFT,   IDC_LEFTSTATIC },
         { IDC_WIDTHSPIN,  IDC_WIDTH,  IDC_WIDTHSTATIC },
         { IDC_HEIGHTSPIN, IDC_HEIGHT, IDC_HEIGHTSTATIC },
      };
   HWND  hCtl;
   int   bEnable, i;

   bEnable = bValidMode ? TRUE : FALSE;
   hCtl = GetDlgItem ( hDlg, IDC_CROPPINGON );
   EnableWindow ( hCtl, bEnable );
   hCtl = GetDlgItem ( hDlg, IDC_CROPPINGOFF );
   EnableWindow ( hCtl, bEnable );
   
   bEnable = ( ( IsDlgButtonChecked ( hDlg, IDC_CROPPINGON ) == BST_CHECKED ) &&
         bValidMode );
   for ( i = 0; i < ( sizeof ( control ) / sizeof ( control[0] )); i++ )
   {
      hCtl = GetDlgItem ( hDlg, control[i].ScrollID );
      EnableWindow ( hCtl, bEnable );

      hCtl = GetDlgItem ( hDlg, control[i].ValueID );
      EnableWindow ( hCtl, bEnable );

      hCtl = GetDlgItem ( hDlg, control[i].StaticID );
      EnableWindow ( hCtl, bEnable );
   }

   return 0;
}

/******************************************************************************/

unsigned long
UpdateControlState (
   HWND     hDlg,
   PWINDOW  pWindow,
   BOOL     bValidMode )
{
   static const struct
   {
      int   ScrollID;
      int   ValueID;
      int   StaticID;

      BOOL  BHideValue;
   } control[] =
      {
         { IDC_ADDRWIDTHSPIN, IDC_ADDRWIDTH,       IDC_ADDRWIDTHSTATIC,  FALSE },
         { IDC_ADDRHEIGHTSPIN,IDC_ADDRHEIGHT,      IDC_ADDRHEIGHTSTATIC, FALSE },

         { IDC_HOROFFSET,     IDC_HOROFFSETVALUE,  IDC_HOROFFSETSTATIC,  TRUE },
         { IDC_HORSCALE,      IDC_HORSCALEVALUE,   IDC_HORSCALESTATIC,   TRUE },
         { IDC_PHASE,         IDC_PHASEVALUE,      IDC_PHASESTATIC,      TRUE },
         { IDC_VEROFFSET,     IDC_VEROFFSETVALUE,  IDC_VEROFFSETSTATIC,  TRUE },
         { IDC_BLACKLEVEL,    IDC_BLACKLEVELVALUE, IDC_BLACKLEVELSTATIC, TRUE },
         { IDC_BRIGHTNESS,    IDC_BRIGHTNESSVALUE, IDC_BRIGHTNESSSTATIC, TRUE },
         { IDC_CONTRAST,      IDC_CONTRASTVALUE,   IDC_CONTRASTSTATIC,   TRUE },
      };
   int   i;

   UpdateCroppingState ( hDlg, pWindow, bValidMode );

   for ( i = 0; i < ( sizeof ( control ) / sizeof ( control[0] )); i++ )
   {
      HWND  hCtl;

      hCtl = GetDlgItem ( hDlg, control[i].ScrollID );
      EnableWindow ( hCtl, ( bValidMode ) ? TRUE : FALSE );

      hCtl = GetDlgItem ( hDlg, control[i].StaticID );
      EnableWindow ( hCtl, ( bValidMode ) ? TRUE : FALSE );

      hCtl = GetDlgItem ( hDlg, control[i].ValueID );
      if ( control[i].BHideValue )
         ShowWindow ( hCtl, ( bValidMode ) ? SW_NORMAL : SW_HIDE );
      else
         EnableWindow ( hCtl, ( bValidMode ) ? TRUE : FALSE );
   }

   return 0;
}

/******************************************************************************/

unsigned long
UpdateRedetectState (
   HWND           hDlg,
   unsigned long  signalState )
{
   HWND  hCtl;

   hCtl = GetDlgItem ( hDlg, IDC_RESET );
   switch ( signalState )
   {
      case VALIDSIGSTATE:
      case INVALIDSIGSTATE:
      {
         EnableWindow ( hCtl, TRUE );
         break;
      }
      case NOSIGNALSTATE:
      case CAPTURENOTRUNNING:
      {
         EnableWindow ( hCtl, FALSE );
         break;
      }

      /* TODO : Could we make the redetect try and restart the capture? */      
      case ERRORSTATE:
      {
         EnableWindow ( hCtl, FALSE );
         break;
      }

      default:
      {
         /* It's probably better to enable the window when we get a strange 
          * signal state. */
         EnableWindow ( hCtl, TRUE );
         return ERROR_INVALID_STATE;
      }
   }

   return 0;
}

/******************************************************************************/

unsigned long
UpdateInputState (
   HWND           hDlg,
   unsigned long  signalState )
{
   HWND  hCtl;

   hCtl = GetDlgItem ( hDlg, IDC_INPUTSOURCE );

   switch ( signalState )
   {
      case VALIDSIGSTATE:
      case INVALIDSIGSTATE:
      case NOSIGNALSTATE:
      {
         EnableWindow ( hCtl, TRUE );
         break;
      }
      
      case CAPTURENOTRUNNING:
      case ERRORSTATE:
      {
         EnableWindow ( hCtl, FALSE );
         break;
      }

      default:
      {
         /* It's probably better to enable the window when we get a strange 
          * signal state. */
         EnableWindow ( hCtl, TRUE );
         return ERROR_INVALID_STATE;
      }
   }

   return 0;
}

/******************************************************************************/

void RGBCBKAPI
ModeChangeFn (
   HWND                 hWnd,
   HRGB                 hRGB,
   PRGBMODECHANGEDINFO  pModeChangedInfo,
   ULONG_PTR        userData )
{
   HWND  hDlg = (HWND)userData;

   if ( hDlg )
   {
      SendMessage ( hDlg, WM_MODECHANGEUPDATE, (WPARAM) NULL, (LPARAM) NULL );
   }
}

/******************************************************************************/

void RGBCBKAPI
ValueChangeFn (
   HWND                 hWnd,
   HRGB                 hRGB,
   PRGBVALUECHANGEDINFO pValueChangedInfo,
   ULONG_PTR        userData )
{
   HWND  hDlg = (HWND)userData;

   if ( hDlg )
   {
      SendMessage ( hDlg, WM_VALUECHANGEUPDATE, (WPARAM) NULL, (LPARAM) NULL );
   }
}

/******************************************************************************/

void RGBCBKAPI
NoSignalFn (
   HWND           hWnd,
   HRGB           hRGB,
   ULONG_PTR  userData )
{
   HWND  hDlg = (HWND)userData;

   if ( hDlg )
   {
      SendMessage ( hDlg, WM_NOSIGNALUPDATE, (WPARAM) NULL, (LPARAM) NULL );
   }

   RGBNoSignal ( hRGB );
}

/******************************************************************************/

void RGBCBKAPI
InvalidSignalFn (
   HWND           hWnd,
   HRGB           hRGB,
   unsigned long  horClock,
   unsigned long  verClock,
   ULONG_PTR  userData )
{
   HWND  hDlg = (HWND)userData;

   if ( hDlg )
   {
      SendMessage ( hDlg, WM_INVALIDSIGUPDATE, (WPARAM) NULL, (LPARAM) NULL );
   }

   RGBInvalidSignal ( hRGB, horClock, verClock );
}

/******************************************************************************/

INT_PTR CALLBACK
InputSettingsDlgProc (
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam )
{
   BOOL  bReturn = FALSE;

   switch ( message )
   {
      case PROP_SETFOCUS:
      {
         HWND hCtrl;

         hCtrl = GetDlgItem ( hDlg, (int)lParam );
         if ( hCtrl )
            SetFocus ( hCtrl );
         break;
      }
      case WM_HSCROLL:
      {
         int   nScrollCode;
         HWND  hCtl;
         long  minimum, maximum, position;

         nScrollCode = LOWORD ( wParam );
         if ( nScrollCode == SB_ENDSCROLL )
         {
            break;
         }

         hCtl = (HWND)lParam;
         GetTrackBarRange ( hCtl, &minimum, &maximum );
         position = GetTrackBarPos ( hCtl );

         switch ( nScrollCode )
         {
            case SB_LINELEFT:
               if ( position > minimum )
               {
                  position--;
               }
               break;

            case SB_LINERIGHT:
               if ( position < maximum )
               {
                  position++;
               }
               break;

            case SB_PAGELEFT:
               position -= 5;
               if ( position < minimum )
               {
                  position = minimum;
               }
               break;

            case SB_PAGERIGHT:
               position += 5;
               if ( position > maximum )
               {
                  position = maximum;
               }
               break;

            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
               position = (signed short)HIWORD ( wParam );
               break;

            case SB_LEFT:
               position = minimum;
               break;

            case SB_RIGHT:
               position = maximum;
               break;
         }

         {
            PWINDOW  pWindow;

            pWindow = GetProp ( hDlg, Property );
            if ( pWindow )
            {
               int   nScrollId;

               nScrollId = GetDlgCtrlID ( hCtl );

               SetValueFromSlider ( hDlg, pWindow, nScrollId, position );

               if ( ( nScrollId == IDC_HOROFFSET ) ||
                    ( nScrollId == IDC_VEROFFSET ) ||
                    ( nScrollId == IDC_HORSCALE ) )
               {
                  /* The cropping regions are related to the three capture 
                   * parameters above, update the cropping limits. */
                  UpdateCroppingControls ( hDlg, pWindow );
               }
            }
         }

         if ( nScrollCode != SB_THUMBTRACK )
         {
            SetTrackBarPos ( hCtl, position, TRUE );
         }

         bReturn = TRUE;
         break;
      }

      case WM_COMMAND:
      {
         PWINDOW     pWindow;

         /* WM_COMMAND messages are received before we have a chance to set the
          * property in response to the WM_INITDIALOG message. */
         pWindow = GetProp ( hDlg, Property );
         if ( ( pWindow == 0 ) || ( pWindow->BProgram == FALSE ) )
         {
            break;
         }

         switch ( LOWORD ( wParam ))
         {
            case IDC_TOP:
            case IDC_LEFT:
            case IDC_WIDTH:
            case IDC_HEIGHT:
               if ( HIWORD  ( wParam ) == EN_CHANGE )
               {
                  Cropping ( hDlg, pWindow, pWindow->HCurrentMode, FALSE );
               }
               bReturn = TRUE;
               break;

            case IDC_INPUTSOURCE:
            {
               HWND  hCtl;

               hCtl = (HWND)lParam;
               if ( HIWORD ( wParam ) == CBN_SELCHANGE )
               {
                  SetInputSource ( hDlg, pWindow );
               }
               bReturn = TRUE;
               break;
            }

            case IDC_ADDRWIDTH:
               if ( HIWORD  ( wParam ) == EN_CHANGE )
               {
                  AddrWidth ( hDlg, pWindow, FALSE );
               }
               bReturn = TRUE;
               break;

            case IDC_ADDRHEIGHT:
               if ( HIWORD  ( wParam ) == EN_CHANGE )
               {
                  AddrHeight ( hDlg, pWindow, FALSE );
               }
               bReturn = TRUE;
               break;

            case IDC_CROPPINGON:
            case IDC_CROPPINGOFF:
            {
               unsigned long  bEnable;
               
               bEnable = ( IsDlgButtonChecked ( hDlg, IDC_CROPPINGON ) == 
                     BST_CHECKED );

               RGBEnableCropping ( pWindow->HRGB, bEnable );
               
               UpdateCroppingState ( hDlg, pWindow, 
                     ( pWindow->HCurrentMode ) ? TRUE : FALSE );

               bReturn = TRUE;
               break;
            }

            case IDC_RESET:
            {
               RGBResetCapture ( pWindow->HRGB );
               break;
            }
         }
         break;
      }

      case WM_NOTIFY:
      {
         LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;

         switch ( lpPSHNotify->hdr.code )
         {
            case PSN_APPLY: /* The OK or Apply button was pressed. */
            {
               LONG     result = PSNRET_NOERROR;
               PWINDOW  pWindow;

               pWindow = GetProp ( hDlg, Property );
               if ( !pWindow )
               {
                  /* TODO: What should we do in these circumstances? */
                  break;
               }

               /* Check the input is valid. */
               {
                  unsigned long  error;

                  error = SetInputSource ( hDlg, pWindow );
                  if ( error )
                  {
                     PostMessage ( hDlg, PROP_SETFOCUS, 0, IDC_INPUTSOURCE );
                     result = PSNRET_INVALID;
                  }
               }

               /* These values cannot be set if there is no signal */
               if ( pWindow->HCurrentMode )
               {
                  if ( IsDlgButtonChecked ( hDlg, IDC_CROPPINGON ) == 
                        BST_CHECKED )
                  {
                     if (( result == PSNRET_NOERROR ) && 
                           !Cropping ( hDlg, pWindow, pWindow->HCurrentMode, 
                           TRUE ))
                     {
                        result = PSNRET_INVALID;
                     }
                  }

                  if (( result == PSNRET_NOERROR ) && 
                           !AddrWidth ( hDlg, pWindow, TRUE ))
                  {
                     result = PSNRET_INVALID;
                  }

                  if (( result == PSNRET_NOERROR ) && 
                           !AddrHeight ( hDlg, pWindow, TRUE ))
                  {
                     result = PSNRET_INVALID;
                  }
               }
               
               /* If no error occurred and the OK button was pressed, free the
                * resources allocated for this page. */
               if (( result == PSNRET_NOERROR ) && ( lpPSHNotify->lParam ))
               {
                  RGBSetModeChangedFn ( pWindow->HRGB, NULL, 0 );
                  RGBSetNoSignalFn ( pWindow->HRGB, NULL, 0 );
                  RGBSetInvalidSignalFn ( pWindow->HRGB, NULL, 0 );

                  RemoveProp ( hDlg, Property );
               }
               /* Indicate whether or not property sheet can be destroyed. */
               SetWindowLongPtr ( hDlg, DWLP_MSGRESULT, result );
               return TRUE;
            }

            case PSN_QUERYCANCEL: /* The Cancel button was pressed. */
               /* Tell Windows it can destroy the property sheet. */
               SetWindowLongPtr ( hDlg, DWLP_MSGRESULT, FALSE );
               return TRUE;

            case PSN_KILLACTIVE: /* Page is about to lose activation. */
               /* Set FALSE because want to allow page to lose activation. */
               SetWindowLongPtr ( hDlg, DWLP_MSGRESULT, FALSE );
               return TRUE;
         
            case UDN_DELTAPOS:
            {
               LPNMUPDOWN  lpUpDown = ( LPNMUPDOWN ) lParam;
               PWINDOW     pWindow;

               pWindow = GetProp ( hDlg, Property );

               switch ( lpUpDown->hdr.idFrom )
               {
                  /* We test the cropping width using the standard 
                   * TestAddrWidth. If it is successful it will be a legitimate
                   * cropping width. */
                  case IDC_ADDRWIDTHSPIN:
                  case IDC_WIDTHSPIN:
                  {
                     unsigned long  value;
                     unsigned long  result;

                     value = lpUpDown->iPos;

                     do
                     {
                        value += lpUpDown->iDelta;
                        result = RGBTestCaptureWidth ( pWindow->HRGB, value );
                     }
                     while ( ( result == RGBERROR_HORADDRTIME_NOT_WORD_DIVISIBLE ) ||
                             ( result == RGBERROR_HORADDRTIME_NOT_DWORD_DIVISIBLE ) ||
                             ( result == RGBERROR_HORADDRTIME_NOT_QWORD_DIVISIBLE ) );

                     if ( result == 0 )
                     {
                        /* Calculate the legal delta. */
                        lpUpDown->iDelta = value - lpUpDown->iPos;
                     }
                  }
                  break;
               }

               /* TODO : Do we need to do this for any of the other controls? */
            }

            default:
               return FALSE;
         }
         break;
      }  /* End of case WM_NOTIFY. */

      case WM_INITDIALOG:
      {
         LPPROPSHEETPAGE   pPage = (LPPROPSHEETPAGE)lParam;
         PWINDOW           pWindow = (PWINDOW)pPage->lParam;
         CAPTURESTATE     state;
         unsigned long     error;
         
         /* Save pWindow. */
         SetProp ( hDlg, Property, pWindow );

         /* Set our callbacks. */

         RGBSetValueChangedFn( pWindow->HRGB, ValueChangeFn, (unsigned long)hDlg );
         RGBSetModeChangedFn ( pWindow->HRGB, ModeChangeFn, (unsigned long)hDlg );
         RGBSetNoSignalFn ( pWindow->HRGB, NoSignalFn, (unsigned long)hDlg );
         RGBSetInvalidSignalFn ( pWindow->HRGB, InvalidSignalFn, (unsigned long)hDlg );

         InintialiseInputSource ( hDlg, pWindow );

         error = RGBGetCaptureState ( pWindow->HRGB, &state );
         if ( error == 0 )
         {
            if ( state == RGB_STATE_CAPTURING )
            {
               pWindow->HCurrentMode = TRUE;
               UpdateControls ( hDlg, pWindow );
               UpdateControlState ( hDlg, pWindow, TRUE );
            }
            else
            {
               pWindow->HCurrentMode = 0;
               UpdateControlState ( hDlg, pWindow, FALSE );
               pWindow->BProgram = TRUE;
            }

            UpdateInputState ( hDlg, state );
            UpdateRedetectState ( hDlg, state );
         }
         else
         {
            /* TODO : Error... */
         }

         break;
      }  /* End of case WM_INITDIALOG. */

      case WM_MODECHANGEUPDATE:
      case WM_VALUECHANGEUPDATE:
      {
         PWINDOW pWindow = GetProp ( hDlg, Property );

         pWindow->HCurrentMode = TRUE;

         UpdateControls ( hDlg, pWindow );
         UpdateControlState ( hDlg, pWindow, TRUE );
         UpdateRedetectState ( hDlg, VALIDSIGSTATE );
         break;
      }

      case WM_NOSIGNALUPDATE:
      {
         PWINDOW pWindow = GetProp ( hDlg, Property );

         pWindow->HCurrentMode = 0;

         UpdateControlState ( hDlg, pWindow, FALSE );
         UpdateRedetectState ( hDlg, NOSIGNALSTATE );
         break;
      }

      case WM_INVALIDSIGUPDATE:
      {
         PWINDOW pWindow = GetProp ( hDlg, Property );

         pWindow->HCurrentMode = 0;

         UpdateControlState ( hDlg, pWindow, FALSE );
         UpdateRedetectState ( hDlg, INVALIDSIGSTATE );
         break;
      }

      default:
         break;
   }

   return bReturn;
}

/******************************************************************************/