/*******************************************************************************

Copyright Datapath Ltd. 2008, 2010.

File:    SAMPLE3A.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to capture
         RGB data into a user defined buffer.

History:
         16 JAN 08    SB   Created.
         09 DEC 09   MJE   Set to always DMA through system memory. Save an
                           RGB888 bitmap. No longer call RGBCloseCapture within
                           a callback. 
         01 APR 09   MJE   Use RGBUseOutputBuffers and RGBChainOutputBuffer to 
                           capture into bitmaps we allocate.                           .
         11 MAY 10    TM   Fixed stop start bug by re-initializing variable
                           gBChainBuffer when starting the capture.

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
   Caption[]   = { TEXT("RGB Sample 3A") };

/* Global Variables ***********************************************************/

static HINSTANCE  gHInstance = NULL;
static HRGBDLL    gHRGBDLL = 0;
static HRGB       gHRGB = 0;

static unsigned long gFrameCount = 0;
static unsigned long gForceClose = FALSE;
static TCHAR         gDirectory[MAX_PATH-32] = { 0, };

static LPBITMAPINFO  gPBitmapInfo = NULL;
static PVOID         gPBitmapBits[NUM_BUFFERS] = { NULL, };
static HBITMAP       gHBitmaps[NUM_BUFFERS] = { NULL, };
static unsigned long gBuffers = 0;
static BOOL          gBChainBuffer = TRUE;

static struct
{
   COLORREF Mask[3];
}  ColourMasks[] =
{
   { 0x00ff0000, 0x0000ff00, 0x000000ff, },  /* RGB888 */
};

/******************************************************************************/

RGBFRAMECAPTUREDFN  FrameCapturedFn;

void RGBCBKAPI FrameCapturedFn (
   HWND                 hWnd,
   HRGB                 hRGB,
   LPBITMAPINFOHEADER   pBitmapInfo,
   void                 *pBitmapBits,
   ULONG_PTR            userData )
{
   TCHAR buffer[MAX_PATH];
   

   StringCchPrintf ( buffer, MAX_PATH, TEXT("%s\\RGBCAPT%d.BMP"), gDirectory, gFrameCount++ );
   if ( ( pBitmapInfo ) && ( pBitmapBits ) )
      RGBSaveBitmap ( hRGB, pBitmapInfo, pBitmapBits, buffer );

   if ( gBChainBuffer )
   {
      /* We own the buffer until it is passed back into the driver. Once we
       * chain it back in it will be reused in another capture. */
      RGBChainOutputBuffer ( gHRGB, gPBitmapInfo, pBitmapBits );
   }
}

/******************************************************************************/

unsigned long
StartCapture ( 
   unsigned long  input )
{
   unsigned long  error;

   /* Open RGB input. */
   error = RGBOpenInput ( input, &gHRGB );
   if ( error == 0 )
   {
      /* Maximise the capture rate. */
      error = RGBSetFrameDropping ( gHRGB, 0 );

      if ( error == 0 )
      {
         /* Set the Frame Captured callback function. */
         error = RGBSetFrameCapturedFn ( gHRGB, FrameCapturedFn, 0 );
         if ( error == 0 )
         {
            /* If we haven't a bitmap we still continue, the 
             * driver will allocate it's own buffers which 
             * we can use. */
            if ( gBuffers )
            {
               unsigned long i;
            
               for ( i = 0; i < gBuffers; i++ )
               {
                  error = RGBChainOutputBuffer ( gHRGB, gPBitmapInfo, 
                     gPBitmapBits[i] );
               }

               error = RGBUseOutputBuffers ( gHRGB, TRUE );
               if ( error == 0 )
               {
                  gBChainBuffer = TRUE;
               }
            }            
            error = RGBStartCapture ( gHRGB );
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
   unsigned long  error;

   InitCommonControls();

   gPBitmapInfo = ( LPBITMAPINFO ) malloc ( sizeof ( BITMAPINFOHEADER ) + 
         ( 3 * sizeof ( DWORD ) ) );

   /* Setup the bitmap header. */
   if ( gPBitmapInfo )
   {
      BITMAPINFOHEADER  *pBitmapInfoHeader;
      HDC               hDC;

      pBitmapInfoHeader = &gPBitmapInfo->bmiHeader;

      pBitmapInfoHeader->biSize = sizeof ( BITMAPINFOHEADER );
      pBitmapInfoHeader->biPlanes = 1;
      pBitmapInfoHeader->biCompression = BI_BITFIELDS;
      pBitmapInfoHeader->biSizeImage = 0;
      pBitmapInfoHeader->biXPelsPerMeter = 3000;
      pBitmapInfoHeader->biYPelsPerMeter = 3000;
      pBitmapInfoHeader->biClrUsed = 0;
      pBitmapInfoHeader->biClrImportant = 0;

      pBitmapInfoHeader->biWidth = 640;
      pBitmapInfoHeader->biHeight = -480;

      /* The bitmap format must match the pixel format
       * requested in RGBSetPixelFormat */
      pBitmapInfoHeader->biBitCount = 32;

      pBitmapInfoHeader->biSizeImage = pBitmapInfoHeader->biWidth *
         abs(pBitmapInfoHeader->biHeight) *
         ( pBitmapInfoHeader->biBitCount / 8 );

      memcpy ( &gPBitmapInfo->bmiColors, &ColourMasks[0], 
            sizeof ColourMasks[0] );

      hDC = GetDC ( NULL );

      if ( hDC )
      {
         unsigned long  i;

         /* Now create the DIBs we are going to capture into. */
         for ( i = 0, gBuffers = 0;
               i < sizeof ( gHBitmaps ) / sizeof ( gHBitmaps[0] );
               i++ )
         {
            
            gHBitmaps[gBuffers] = CreateDIBSection ( hDC, gPBitmapInfo, 
               DIB_RGB_COLORS, &gPBitmapBits[gBuffers], NULL, 0 );

            if ( gHBitmaps[gBuffers] )
            {
               gBuffers++;
            }
         }
      }
      ReleaseDC ( NULL, hDC );
   }

   /* Load the RGBEASY API. */
   error = RGBLoad ( &gHRGBDLL );
   if ( error == 0 )
   {         
      DialogBoxParam ( hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL,
         DialogProc, ( LPARAM ) gHRGB );

      RGBFree ( gHRGBDLL );
   }

   if ( gBuffers )
   {
      unsigned long  i;
      for ( i = 0; i < gBuffers; i++ )
      {
         DeleteObject( gHBitmaps[i] );
      }
   }

   if ( gPBitmapInfo ) 
      free ( gPBitmapInfo );

   return error;
}

/******************************************************************************/
