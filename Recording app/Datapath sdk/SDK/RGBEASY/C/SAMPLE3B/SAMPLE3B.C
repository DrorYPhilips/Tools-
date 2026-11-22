/*******************************************************************************

Copyright Datapath Ltd. 2010, 2011.

File:    SAMPLE3B.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to capture
         RGB data into a user defined buffer at 1:1 when the input signal changes.

History:
         16 MAR 10    RL   Created from SAMPLE3A.
         20 MAY 11    RL   Demonstates the use of FrameCapturedFnEx.

*******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <commctrl.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

#include "resource.h"

#define NUM_BUFFERS  3

/* Static Constants ***********************************************************/

static const TCHAR
   Caption[]   = { TEXT("RGB Sample 3B") };

/* Global Variables ***********************************************************/

static HINSTANCE  gHInstance = NULL;
static HRGBDLL    gHRGBDLL = 0;
static HRGB       gHRGB = 0;

static unsigned long gForceClose = FALSE;
static TCHAR         gDirectory[MAX_PATH-32] = { 0, };

typedef struct {
   HBITMAP           HBitmap;
   BITMAPINFO        BitmapInfo;
   DWORD             Dummy[2];
   PVOID             PBitmapBits;
} CAPTURE, *PCAPTURE;

static CAPTURE Capture[NUM_BUFFERS] = { 0, };

static BOOL gBChainBuffer = TRUE;

typedef struct
{
   COLORREF Mask[3];
} COLOURMASK, PCOLOURMASK;

static COLOURMASK ColourMasks[] =
{
   { 0x00ff0000, 0x0000ff00, 0x000000ff, },  /* RGB888 */
};

static USHORT gBitCount = 32;
static LONG gWidth = 0;
static LONG gHeight = 0;

/******************************************************************************/

unsigned long
CreateBitmapAndBuffer (
   PCAPTURE    pCapture,
   int         width,
   int         height,
   USHORT      bitCount,
   PCOLOURMASK *pMask )
{
   /* Setup the bitmap header. */
   HDC hDC;
   
   pCapture->BitmapInfo.bmiHeader.biSize = sizeof ( BITMAPINFOHEADER );
   pCapture->BitmapInfo.bmiHeader.biPlanes = 1;
   pCapture->BitmapInfo.bmiHeader.biCompression = BI_BITFIELDS;
   pCapture->BitmapInfo.bmiHeader.biSizeImage = 0;
   pCapture->BitmapInfo.bmiHeader.biXPelsPerMeter = 3000;
   pCapture->BitmapInfo.bmiHeader.biYPelsPerMeter = 3000;
   pCapture->BitmapInfo.bmiHeader.biClrUsed = 0;
   pCapture->BitmapInfo.bmiHeader.biClrImportant = 0;

   pCapture->BitmapInfo.bmiHeader.biWidth = width;
   pCapture->BitmapInfo.bmiHeader.biHeight = -height;

   /* The bitmap format must match the pixel format
    * requested in RGBSetPixelFormat */
   pCapture->BitmapInfo.bmiHeader.biBitCount = bitCount;

   pCapture->BitmapInfo.bmiHeader.biSizeImage = 
         pCapture->BitmapInfo.bmiHeader.biWidth *
         abs(pCapture->BitmapInfo.bmiHeader.biHeight) *
         ( pCapture->BitmapInfo.bmiHeader.biBitCount / 8 );

   memcpy ( &pCapture->BitmapInfo.bmiColors, pMask, sizeof(COLOURMASK) );

   hDC = GetDC ( NULL );
   if ( hDC )
   {
      pCapture->HBitmap = CreateDIBSection ( hDC, &pCapture->BitmapInfo, 
            DIB_RGB_COLORS, &pCapture->PBitmapBits, NULL, 0 );
      
      ReleaseDC ( NULL, hDC );
      return 0;
   }
   return ERROR_DC_NOT_FOUND;
}

/******************************************************************************/

RGBFRAMECAPTUREDFNEX FrameCapturedFnEx;

void RGBCBKAPI FrameCapturedFnEx (
   HWND                 hWnd,
   HRGB                 hRGB,
   PRGBFRAMEDATA        pFrameData,
   ULONG_PTR            userData )
{
   if ( gBChainBuffer )
   {
      PCAPTURE pCapture = NULL;
      unsigned long i;
      
      /* Determine our buffer. */
      for ( i = 0; i < NUM_BUFFERS; i++ )
      {
         /* Isolate the capture structure associated with the buffer. */
         if ( &Capture[i].BitmapInfo.bmiHeader == pFrameData->PBitmapInfo )
         {
            pCapture = &Capture[i];
            break;
         }
      }
      if ( pCapture )
      {
         /* Has the input signal resolution changed? */
         if ( ( pFrameData->PBitmapInfo->biWidth == gWidth ) && 
              ( abs(pFrameData->PBitmapInfo->biHeight) == gHeight ) )
         {
            if ( pFrameData && pFrameData->TimeStamp )
            {
               TCHAR buffer[MAX_PATH];

               StringCchPrintf ( buffer, MAX_PATH, TEXT("%s\\RGB_%I64u.BMP"), 
                     gDirectory, pFrameData->TimeStamp );

               if ( ( pFrameData->PBitmapInfo ) && ( pFrameData->PBitmapBits ) )
                     RGBSaveBitmap ( hRGB, pFrameData->PBitmapInfo, 
                     pFrameData->PBitmapBits, buffer );
            }
            /* We own the buffer until it is passed back into the driver. Once we
             * chain it back in it will be reused in another capture. */
            RGBChainOutputBuffer ( gHRGB, &pCapture->BitmapInfo, 
                  pFrameData->PBitmapBits );
         }
         else 
         {
            /* We will drop as many frames as we have buffers currently chained
             * in the driver. If required, modify the code to save a scaled buffer. */
            if ( pCapture->HBitmap )
               DeleteObject( pCapture->HBitmap );

            /* Create a new capture structure based on the new width and height. */
            if ( !CreateBitmapAndBuffer ( pCapture, gWidth, gHeight, 
                  gBitCount, &ColourMasks[0] ))
            {
               /* Chain the buffer back into the driver list. */
               RGBChainOutputBuffer ( gHRGB, &pCapture->BitmapInfo, 
                     pCapture->PBitmapBits );
            }
         }
      }
   }
}

/******************************************************************************/

RGBMODECHANGEDFN ModeChangedFn;

void RGBCBKAPI ModeChangedFn (
   HWND                 hWnd,
   HRGB                 hRGB,
   PRGBMODECHANGEDINFO  pModeChangedInfo,
   ULONG_PTR            userData )
{
   if ( RGBGetCaptureWidthDefault ( gHRGB, &gWidth) )
      gWidth = 640;
   if ( RGBGetCaptureHeightDefault ( gHRGB, &gHeight ) )
		gHeight = 480;
}

/******************************************************************************/

unsigned long
StartCapture ( 
   unsigned long  input )
{
   unsigned long  error;

   gBChainBuffer = TRUE;

   /* Open RGB input. */
   error = RGBOpenInput ( input, &gHRGB );
   if ( error == 0 )
   {
      /* Maximise the capture rate. */
      error = RGBSetFrameDropping ( gHRGB, 0 );

      if ( error == 0 )
      {
         /* Set the Frame Captured callback function. */
         error = RGBSetFrameCapturedFnEx ( gHRGB, FrameCapturedFnEx, 0 );
         if ( error == 0 )
         {
            /* Set the Mode Changed callback function. */
	         error = RGBSetModeChangedFn ( gHRGB, ModeChangedFn, 0 );
            if ( error == 0 )
            {            
               unsigned long  i;

               /* Get on the wire values for 1:1 width and height. */
               if ( RGBGetCaptureWidthDefault ( gHRGB, &gWidth) || ( gWidth == 0 ))
		            gWidth = 640;
	            if ( RGBGetCaptureHeightDefault ( gHRGB, &gHeight ) || ( gHeight == 0 ) )
		            gHeight = 480;

               for ( i = 0; i < NUM_BUFFERS; i++ )
               {
                  /* Create a capture structure based on the width and height. */
                  error = CreateBitmapAndBuffer ( &Capture[i], gWidth, gHeight, 
                        gBitCount, &ColourMasks[0] );
                  if ( error == 0 )
                  {
                     error = RGBChainOutputBuffer ( gHRGB, 
                           &Capture[i].BitmapInfo, Capture[i].PBitmapBits );
                  }
                  else
                     return error;
               }

               error = RGBUseOutputBuffers ( gHRGB, TRUE );
               if ( error == 0 )
               {
                  error = RGBStartCapture ( gHRGB );
               }
            }
         }
      }
   }

   return error;
}

/******************************************************************************/

unsigned long
StopCapture ( )
{
   if ( RGBUseOutputBuffers( gHRGB, FALSE ) == 0 )
   {
      /* We haven't stopped the capture yet. It's possible we'll be called
       * with another frame of data, using the drivers internal buffers. These
       * should not be chained so we set a flag to indicate it to the callback.
       * Stopping the capture without calling RGBUseOutputBuffers would also 
       * work. */
      gBChainBuffer = FALSE;
   }

   RGBStopCapture ( gHRGB );
   RGBCloseInput ( gHRGB );

   gHRGB = 0;

   return 0;
}

/******************************************************************************/

INT_PTR CALLBACK
DialogProc (
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam )
{
   BOOL           bReturn = FALSE;
   unsigned long  error;

   switch ( message )
   {
      case WM_COMMAND:
      {
         switch ( LOWORD ( wParam ) )
         {
            case IDEXIT:
            {
               if ( gHRGB )
                  StopCapture ( );
               
               EndDialog ( hDlg, FALSE );
               break;
            }

            case IDSTOP:
            {
               StopCapture( );
               EnableWindow ( GetDlgItem ( hDlg, IDSTOP ), FALSE );
               EnableWindow ( GetDlgItem ( hDlg, IDSTART ), TRUE );
               break;
            }

            case IDSTART:
            {
               unsigned long  input = 1;
               
               input = GetDlgItemInt ( hDlg, IDC_INPUT, NULL, FALSE );
               GetDlgItemText ( hDlg, IDC_DIR, gDirectory, 
                  sizeof ( gDirectory ) / sizeof ( gDirectory[0] ) );

               error = StartCapture ( input - 1 );
               if ( error == 0 )
               {
                  EnableWindow ( GetDlgItem ( hDlg, IDSTART ), FALSE );
                  EnableWindow ( GetDlgItem ( hDlg, IDSTOP ), TRUE );
               }
               else
                  // TODO: Message Box.

               break;
            }
         }
         break;
      }
      
      case WM_SYSCOMMAND:
      {
         switch ( wParam )
         {
            case SC_CLOSE:
            {
               if ( gHRGB )
                  StopCapture( );
               
               EndDialog ( hDlg, FALSE );
               break;
            }
         }
         break;
      }
      
      
      case WM_INITDIALOG:
      {
         GetTempPath ( sizeof ( gDirectory ) / sizeof ( gDirectory[0] ),
               gDirectory );
         SetDlgItemText ( hDlg, IDC_DIR, gDirectory );
         SetDlgItemInt ( hDlg, IDC_INPUT, 1, FALSE );
         bReturn = TRUE;
         break;
      }
   }
   return bReturn;
}

/******************************************************************************/

int APIENTRY
_tWinMain (
   HINSTANCE   hInstance,
   HINSTANCE   hPrevInstance,
   LPTSTR      lpCmdLine,
   int         nShowCmd )  
{
   unsigned long  error, i;

   InitCommonControls();
   
   /* Load the RGBEASY API. */
   error = RGBLoad ( &gHRGBDLL );
   if ( error == 0 )
   {         
      DialogBoxParam ( hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL,
         DialogProc, ( LPARAM ) gHRGB );

      RGBFree ( gHRGBDLL );
   }

   for ( i = 0; i < NUM_BUFFERS; i++ )
   {
      if ( Capture[i].HBitmap )
         DeleteObject( Capture[i].HBitmap );
   }

   return error;
}

/******************************************************************************/
