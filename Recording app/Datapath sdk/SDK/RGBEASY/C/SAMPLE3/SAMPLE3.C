/*******************************************************************************

Copyright Datapath Ltd. 2008.

File:    SAMPLE3.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to capture
         RGB data without a window.

History:
         16 JAN 08    SB   Created.
         09 DEC 09   MJE   Set to always DMA through system memory. Save an
                           RGB888 bitmap. No longer call RGBCloseCapture within
                           a callback. 

*******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>


/* Static Constants ***********************************************************/

static const TCHAR
   Caption[]   = { TEXT("RGB Sample 3") };

/* Global Variables ***********************************************************/

static HINSTANCE  gHInstance = NULL;
static HANDLE     gHEvent = NULL;
static HRGBDLL    gHRGBDLL = 0;
static HRGB       gHRGB = 0;

static unsigned long gFrameCount = 0;
static unsigned long gForceClose = FALSE;
static TCHAR         gDirectory[MAX_PATH-32] = { 0, };

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

   _tprintf ( TEXT(".") );

   if (( gFrameCount >= userData ) || gForceClose )
   {
      _tprintf ( TEXT("\n") );
      SetEvent ( gHEvent );
   }
}

/******************************************************************************/

BOOL WINAPI
HandlerRoutine(
  DWORD dwCtrlType )
{
   gForceClose = TRUE;
   return TRUE;
}

/******************************************************************************/

int
_tmain (
   int  argc,  
   TCHAR *  argv[],  
   TCHAR *  envp[] )   
{
   unsigned long  error, input, framesToCapture;

   if ( argc != 4 )
   {
      _tprintf (
            TEXT("Usage: SAMPLE3.EXE <input> <number of frames to capture> <capture directory>\n"));
      return -1;
   }

   input = _ttol(argv[1]);
   framesToCapture = _ttol(argv[2]);
   StringCchCopy ( gDirectory, sizeof(gDirectory)/sizeof(gDirectory[0]), argv[3] );

   _tprintf ( TEXT("SAMPLE3.EXE\n") );
   _tprintf ( TEXT("   Input %d\n"), input );
   _tprintf ( TEXT("   Frames to capture %d\n"), framesToCapture );


   /* Add a CTRL-C handler routine. */
   SetConsoleCtrlHandler ( HandlerRoutine, TRUE );

   gHEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
   if ( gHEvent )
   {
      /* Load the RGBEASY API. */
      error = RGBLoad ( &gHRGBDLL );
      if ( error == 0 )
      {
         /* Open RGB input 1. */
         error = RGBOpenInput ( input, &gHRGB );
         if ( error == 0 )
         {
            /* Maximise the capture rate. */
            error = RGBSetFrameDropping ( gHRGB, 0 );

            /* We want to save the data so it must be send to system memory
             * rather than directly to the graphics card. */
            error = RGBSetDMADirect ( gHRGB, FALSE );

            /* Capture RGB888. */
            error = RGBSetPixelFormat ( gHRGB, RGB_PIXELFORMAT_888 );

            if ( error == 0 )
            {
               /* Set the Frame Captured callback function. */
               error = RGBSetFrameCapturedFn ( gHRGB, FrameCapturedFn,
                     framesToCapture );
               if ( error == 0 )
               {
                  error = RGBStartCapture ( gHRGB );
                  if ( error == 0 )
                  {
                     WaitForSingleObject ( gHEvent, INFINITE );
                     RGBStopCapture ( gHRGB );
                  }

                  RGBSetFrameCapturedFn ( gHRGB, NULL, 0 );
               }
            }
            
            RGBCloseInput ( gHRGB );
         }

         RGBFree ( gHRGBDLL );
      }

      CloseHandle ( gHEvent );
   }
   else
   {
      error = GetLastError ();
   }

   if ( error )
   {
      _tprintf ( TEXT("   ERROR 0x%08x\n"), error );
   }

   return error;
}

/******************************************************************************/
