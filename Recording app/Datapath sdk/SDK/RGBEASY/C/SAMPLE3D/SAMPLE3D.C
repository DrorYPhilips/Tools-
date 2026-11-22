/*******************************************************************************

Copyright Datapath Ltd. 2014.

File:    SAMPLE3D.C

Purpose: Example program that shows how to capture video media samples into user
         defined buffers.

History:
         07 JUL 14    SB   Created.
         12 DEC 14    JL   Demonstrate capture of encoded video to disk.

*******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <commctrl.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>
#include <dgcmediahelper.h>

#include "resource.h"

#define NUM_BUFFERS     3
#define WM_RESETCOUNT   WM_APP+1
#define WM_NEWFRAME     WM_APP+2

/* Global Variables ***********************************************************/

static HINSTANCE        gHInstance = NULL;
static HRGBDLL          gHRGBDLL = 0;
static HRGB             gHRGB = 0;
static TCHAR            gDirectory[MAX_PATH - 32] = { 0, };
static unsigned long    gForceClose = FALSE;
static BOOL             gBChainBuffer = TRUE;
static PDGCMEDIASAMPLE  gPMediaSample[NUM_BUFFERS];

HANDLE ghStreamFile = INVALID_HANDLE_VALUE; // File for concatenated data

int gNumFrames = 0;

/******************************************************************************/

#if 0
   // Intel Media SDK requirements for encoding of NV12 planar data.
   #define Align16(u) (((u + 15) >> 4) << 4)
   #define Align32(u) (((uint32_t)((u)+31)) & (~(uint32_t)31))
#else
   #define Align16(u) (u)
   #define Align32(u) (u)
#endif

/******************************************************************************/

PDGCMEDIASAMPLE
CreateMediaSample(
   DGCMEDIASAMPLETYPE      majorType,
   DGCMEDIASAMPLESUBTYPE   subType,
   uint32_t                width,
   uint32_t                height,
   uint8_t                 interlaced)
{
   PDGCMEDIASAMPLE pMediaSample;

   pMediaSample = MediaSampleAllocate( majorType, subType );
   if (pMediaSample)
   {
      if (MediaSampleAllocateFormatHeader(pMediaSample))
      {
         PDGCVIDEOHEADER pVideoHeaderOut;

         // Scaled width and height of the capture
         pVideoHeaderOut = (PDGCVIDEOHEADER)pMediaSample->PFormatHeader;
         pVideoHeaderOut->Width = width;
         pVideoHeaderOut->Height = height;

         if (MediaSampleAllocateBufferHeader(pMediaSample, DGCBUFFERHEADERTYPE_MEMORY))
         {
            BOOL     bSuccess = TRUE;
            uint32_t plane;

            for (plane = 0; (plane < SubTypeGetNumPlanes(subType)) && bSuccess; plane++)
            {
               PDGCMEMORYBUFFERHEADER pHeader;
               uint32_t  length, frameWidth, frameHeight;

               pHeader = (PDGCMEMORYBUFFERHEADER)pMediaSample->PBufferHeader;

               frameWidth = Align16( width );

               if (subType == DGCRAWVIDEOSAMPLESUBTYPE_NV12)
               {
                  // Buffer Pitch - same for both planes
                  pHeader->Planes[plane].Pitch = frameWidth;

                  if (plane == 0)
                  {
                     // Y Plane
                     frameHeight = height;
                  }
                  else
                  {
                     // UV Frame
                     frameHeight = height / 2;
                  }
                  if (interlaced)
                  {
                     frameHeight = Align16(frameHeight);
                  }
                  else
                  {
                     frameHeight = Align32(frameHeight);
                  }
                  length = frameWidth * frameHeight;
               }
               else
               {
                  pHeader->Planes[plane].Pitch = frameWidth * SubTypeGetBitDepth( subType ) / 8;

                  if (interlaced)
                  {
                     frameHeight = Align16( height );
                  }
                  else
                  {
                     frameHeight = Align32( height );
                  }
                  length = frameWidth * frameHeight * SubTypeGetBitDepth(subType) / 8;
               }

               // Allocate the buffer.
               bSuccess = MediaSampleInitialisePlane(pMediaSample, plane, length);
            }

            if (bSuccess)
            {
               return pMediaSample;
            }
         }
      }
      MediaSampleFree(pMediaSample);
   }
   return NULL;
}

/******************************************************************************/

unsigned long
WriteToFile(
   HANDLE   hFile,
   LPCVOID  lpBuffer,
   DWORD    dwWrite)
{
   BOOL  bSuccess;
   DWORD dwWritten;

   bSuccess = WriteFile(hFile, lpBuffer, dwWrite, &dwWritten, NULL);
   if (!bSuccess)
   {
      return GetLastError();
   }
   else if (dwWrite != dwWritten)
   {
      return ERROR_WRITE_FAULT;
   }
   return 0;
}

/******************************************************************************/

RGBMEDIASAMPLECAPTUREDFN MediaSampleCapturedFn;

void RGBCBKAPI
MediaSampleCapturedFn(
   HWND              hWnd,
   HRGB              hRGB,
   PDGCMEDIASAMPLE   pMediaSample,
   ULONG_PTR         userData)
{
   HWND hDlg = (HWND) userData;

   if (pMediaSample)
   {
      TCHAR buffer[MAX_PATH];
      HANDLE hFile;

      PDGCMEMORYBUFFERHEADER pMemoryBufferHeader;

      pMemoryBufferHeader = (PDGCMEMORYBUFFERHEADER)pMediaSample->PBufferHeader;

      if ( ghStreamFile != INVALID_HANDLE_VALUE )
      {
         hFile = ghStreamFile;
      }
      else
      {
         StringCchPrintf(buffer, MAX_PATH, TEXT("%s\\RGB_%I64u.capture"),
            gDirectory, pMemoryBufferHeader->StartTime);

         hFile = CreateFile(buffer, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);
      }

      if (hFile != INVALID_HANDLE_VALUE)
      {
         uint32_t i;
         for (i = 0; i < SubTypeGetNumPlanes(pMediaSample->SubType); i++)
         {
            PDGCMEMORYBUFFER pPlane = &pMemoryBufferHeader->Planes[i];
            WriteToFile(hFile, pPlane->PBuffer, pPlane->ActualLength);
         }

         if ( hFile != ghStreamFile )
            CloseHandle(hFile);
      }

      PostMessage( hDlg, WM_NEWFRAME, 0, 0 );
   }

   if (gBChainBuffer)
   {
      /* We own the buffer until it is passed back into the driver. Once we
         * chain it back in it will be reused in another capture. */
      RGBChainMediaSample(gHRGB, pMediaSample);
   }
}

/******************************************************************************/

unsigned long
StartCapture(
   unsigned long  input,
   HWND           hDlg )
{
   unsigned long  error;

   gBChainBuffer = TRUE;

   /* Open RGB input. */
   error = RGBOpenInput(input, &gHRGB);
   if (error == 0)
   {
      /* Maximise the capture rate. */
      error = RGBSetFrameDropping(gHRGB, 0);

      if (error == 0)
      {
         /* Set the Frame Captured callback function. */
         error = RGBSetMediaSampleCapturedFn(gHRGB,
                                             MediaSampleCapturedFn,
                                             (ULONG_PTR) hDlg);
         if (error == 0)
         {
            RGBMODEINFO modeInfo;
            unsigned long  i;
            uint32_t width = 640;
            uint32_t height = 480;
            uint8_t interlaced = 0;

            DGCMEDIASAMPLETYPE majorType;
            DGCMEDIASAMPLESUBTYPE subType;
            uint32_t bSupported = FALSE;
            uint8_t bStreamData = FALSE;

            modeInfo.Size = sizeof(modeInfo);
            RGBGetModeInfo(gHRGB, &modeInfo);

            if (modeInfo.State == RGB_STATE_CAPTURING)
            {
               RGBGetCaptureWidthDefault(gHRGB, &width);
               RGBGetCaptureHeightDefault(gHRGB, &height);
               interlaced = (uint8_t)modeInfo.BInterlaced;
            }

            RGBInputIsMediaSampleTypeSupported( input,
                                                DGCMEDIASAMPLETYPE_ENC_VIDEO,
                                                DGCENCVIDEOSAMPLESUBTYPE_H264,
                                                &bSupported );
            if ( bSupported )
            {
               majorType = DGCMEDIASAMPLETYPE_ENC_VIDEO;
               subType = DGCENCVIDEOSAMPLESUBTYPE_H264;
               bStreamData = TRUE;
            }
            else
            {
               majorType = DGCMEDIASAMPLETYPE_RAW_VIDEO;
               subType = DGCRAWVIDEOSAMPLESUBTYPE_NV12;
            }

            if ( bStreamData )
            {
               TCHAR buffer[MAX_PATH];
               StringCchPrintf( buffer, MAX_PATH, TEXT( "%s\\RGB_ENC.capture" ),
                                gDirectory );
               ghStreamFile = CreateFile( buffer, GENERIC_WRITE, 0,
                                          NULL, CREATE_ALWAYS,
                                          FILE_ATTRIBUTE_NORMAL, NULL );
            }

            for (i = 0; i < NUM_BUFFERS; i++)
            {
               gPMediaSample[i] = CreateMediaSample(majorType, subType,
                                                    width, height, interlaced);
               if (gPMediaSample[i])
               {
                  error = RGBChainMediaSample(gHRGB, gPMediaSample[i]);
               }
               else
               {
                  return ERROR_OUTOFMEMORY;
               }
            }

            error = RGBUseOutputBuffers(gHRGB, TRUE);
            if (error == 0)
            {
               PostMessage( hDlg, WM_RESETCOUNT, 0, 0 );

               error = RGBStartCapture(gHRGB);
            }
         }
      }
   }

   return error;
}

/******************************************************************************/

unsigned long
StopCapture()
{
   unsigned long i;

   if (RGBUseOutputBuffers(gHRGB, FALSE) == 0)
   {
      /* We haven't stopped the capture yet. It's possible we'll be called
       * with another frame of data, using the drivers internal buffers. These
       * should not be chained so we set a flag to indicate it to the callback.
       * Stopping the capture without calling RGBUseOutputBuffers would also
       * work. */
      gBChainBuffer = FALSE;
   }

   RGBStopCapture(gHRGB);

   if ( ghStreamFile != INVALID_HANDLE_VALUE )
   {
      CloseHandle( ghStreamFile );
      ghStreamFile = INVALID_HANDLE_VALUE;
   }

   RGBCloseInput(gHRGB);

   gHRGB = 0;

   for ( i = 0; i < NUM_BUFFERS; i++ )
   {
      if ( gPMediaSample[i] )
         MediaSampleFree( gPMediaSample[i] );
   }

   return 0;
}

/******************************************************************************/

INT_PTR CALLBACK
DialogProc(
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam)
{
   BOOL           bReturn = FALSE;
   unsigned long  error;

   switch (message)
   {
      case WM_COMMAND:
      {
         switch (LOWORD(wParam))
         {
            case IDEXIT:
            {
               if (gHRGB)
                  StopCapture();

               EndDialog(hDlg, FALSE);
               break;
            }

            case IDSTOP:
            {
               StopCapture();
               EnableWindow(GetDlgItem(hDlg, IDSTOP), FALSE);
               EnableWindow(GetDlgItem(hDlg, IDSTART), TRUE);
               break;
            }

            case IDSTART:
            {
               unsigned long  input = 1;

               input = GetDlgItemInt(hDlg, IDC_INPUT, NULL, FALSE);
               GetDlgItemText(hDlg, IDC_DIR, gDirectory,
                  sizeof(gDirectory) / sizeof(gDirectory[0]));

               error = StartCapture(input - 1, hDlg);
               if (error == 0)
               {
                  EnableWindow(GetDlgItem(hDlg, IDSTART), FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDSTOP), TRUE);
               }
               else
               {
                  // TODO: Message Box.
                  break;
               }
            }
         }
         break;
      }

      case WM_NEWFRAME:
      {
         gNumFrames++;
         SetDlgItemInt( hDlg, IDC_FRAMES, gNumFrames, FALSE );
         break;
      }

      case WM_RESETCOUNT:
      {
         gNumFrames = 0;
         SetDlgItemInt( hDlg, IDC_FRAMES, gNumFrames, FALSE );
         break;
      }

      case WM_SYSCOMMAND:
      {
         switch (wParam)
         {
            case SC_CLOSE:
            {
               if (gHRGB)
                  StopCapture();

               EndDialog(hDlg, FALSE);
               break;
            }
         }
         break;
      }

      case WM_INITDIALOG:
      {
         GetTempPath(sizeof(gDirectory) / sizeof(gDirectory[0]),
            gDirectory);
         SetDlgItemText(hDlg, IDC_DIR, gDirectory);
         SetDlgItemInt(hDlg, IDC_INPUT, 1, FALSE);
         bReturn = TRUE;
         break;
      }
   }
   return bReturn;
}

/******************************************************************************/

int APIENTRY
_tWinMain(
   HINSTANCE   hInstance,
   HINSTANCE   hPrevInstance,
   LPTSTR      lpCmdLine,
   int         nShowCmd)
{
   unsigned long  error;

   InitCommonControls();

   /* Load the RGBEASY API. */
   error = RGBLoad(&gHRGBDLL);
   if (error == 0)
   {
      DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL,
         DialogProc, (LPARAM)gHRGB);

      RGBFree(gHRGBDLL);
   }

   return error;
}

/******************************************************************************/
