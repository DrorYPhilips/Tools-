/*******************************************************************************

Copyright Datapath Ltd. 2008, 2014.

File:    SAMPLE3C.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to capture
         RGB data into video device memory and output to a window using DirectX.

History:
         12 APR 11    JL   Created.
         27 APR 11    JL   Asigned a bitmapinfo structure for each of the 
                           bitmapbits.
         04 AUG 11    JL   Added functionality for text and image overlays.
         05 NOV 14    RL   Implemented rate management.
                           Added LiveStream.

*******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <commctrl.h>
#include <Strsafe.h>

#include <d3d9.h>
#include <d3dx9core.h>
#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

#define RGB_UNKNOWN  0
#define RGB_565      1
#define RGB_888      2

#define NUM_BUFFERS  8 /* DirectX allocated buffer requirement. */
#define INPUT        1 /* Which capture card input to use. */
#define NO_BUFFER    0xFF

/* Show the use of DirectX text overlay. */
#define _DISPLAY_TEXT

/* Show the use of DirectX image overlay. */
#define _DISPLAY_IMAGE

/* Static Constants ***********************************************************/

static const TCHAR
   Caption[]   = { TEXT("RGB Sample 3C - DirectX") };

/* Global Structures **********************************************************/

static struct 
{
   COLORREF Mask[3];
}  ColourMasks[] =
{
   { 0x00000000, 0x00000000, 0x00000000, },  /* Unknown */
   { 0x0000f800, 0x000007e0, 0x0000001f, },  /* RGB565 */
   { 0x00ff0000, 0x0000ff00, 0x000000ff, },  /* RGB888 */
};

#ifdef _DEBUG
typedef struct _TRACK
{
/*
   Valid buffer State variables:

   A = Buffer filled and awaiting OpenGL render call in this application
   C = Buffer chained within RGBEasy awiting a DMA
   R = Buffer filled and in OpenGL blocking render call
*/
   TCHAR                State;
} TRACK, *PTRACK;
#endif

typedef struct _MYTHREADINFO {
   HANDLE   hEvent;
   HANDLE   hThread;
   DWORD    dwThreadID;
   HWND     hWnd;
} THREADINFO, *PTHREADINFO;

typedef struct _CAPTUREDBITS {
   struct _CAPTUREDBITS *PNext;
   PVOID                PBits;
} CAPTUREDBITS, *PCAPTUREDBITS;

/* Global Variables ***********************************************************/

static HRGB          gHRGB = 0;

static LPBITMAPINFO  gPBitmapInfo[NUM_BUFFERS]  = { NULL };
static PVOID         gPBitmapBits[NUM_BUFFERS]  = { NULL, };
static unsigned      gBufferIndex               = NO_BUFFER;
static ULONGLONG     gBufferTimeStamp           = 0;
static unsigned      gBuffers                   = 0;
static BOOL          gBChainBuffer              = TRUE;

static PCAPTUREDBITS gpBits;
static THREADINFO    gThreadInfo;

#ifdef _DEBUG
   TRACK             gBufferTrack[NUM_BUFFERS] =  { 0 };
   signed long       gNumDroppedBuffers = 0;
   signed long       gNumCapturedBuffers = 0;
   signed long       gNumRenderedBuffers = 0;
   ULONGLONG         gTickCount64 = 0;
#endif

IDirect3DSurface9    *gpIDirect3DSurface9[NUM_BUFFERS];
IDirect3DDevice9     *gpIDirect3DDevice9;
IDirect3D9           *gpIDirect3D9;
IDirect3DSwapChain9  **gppIDirect3DSwapChain9;

HANDLE               gHMutex;
HANDLE               gHBitsInList   = 0;
BOOL                 gClose         = FALSE;

/******************************************************************************/

#ifdef _DEBUG

void 
PrintTracking ( )
{
   int      i;
   TCHAR    text[256];

   for ( i = 0; i < NUM_BUFFERS; i++ )
   {
      StringCchPrintf(text, 256, TEXT ( "BufID: %d, State: %c\n" ), i,
         gBufferTrack[i].State );
      OutputDebugString ( text );
   }

   StringCchPrintf(text, 256, TEXT ( "Dropped: %d ------\n" ),
      gNumDroppedBuffers );
   OutputDebugString ( text );
}

#endif

/******************************************************************************/

RGBFRAMECAPTUREDFNEX  FrameCapturedFn;

void RGBCBKAPI FrameCapturedFnEx (
   HWND                 hWnd,
   HRGB                 hRGB,
   PRGBFRAMEDATA        pFrameData,
   ULONG_PTR            userData )
{
   unsigned i;

   for ( i = 0; i < gBuffers; i++ )
   {  
      LPBITMAPINFOHEADER   pBitmapInfo = pFrameData->PBitmapInfo;
      void                 *pBitmapBits = pFrameData->PBitmapBits;
      ULONGLONG            timeStamp = pFrameData->TimeStamp;

      if ( (pBitmapBits == gPBitmapBits[i]) && pBitmapInfo && gBChainBuffer )
      {
#ifdef _DEBUG
         if (gTickCount64 == 0)
            gTickCount64 = GetTickCount64();

         gNumCapturedBuffers++;
#endif
         /* Wait for access to the protected buffer index variable. */
         WaitForSingleObject ( gHMutex, INFINITE );

#ifdef _DEBUG
         /* Store the state variable. */
         gBufferTrack[i].State = TEXT ( 'A' );
#endif
         /* A valid buffer doesn't exist. It is stored so the buffer can be 
            rendered later. */
         
         if (timeStamp < gBufferTimeStamp)
         {
            RGBChainOutputBuffer ( hRGB,
               gPBitmapInfo[i], 
               (void*) gPBitmapBits[i] );

#ifdef _DEBUG
            /* Increase counter of dropped buffers. */
            gNumDroppedBuffers++;
#endif
         }
         else
         {
            if (gBufferIndex != NO_BUFFER)
            {
               /* A valid buffer is waiting to be rendered.
                  Therefore, the capturing rate is faster than the rendering rate.
                  To keep minimum latency this buffer is dropped and returned to
                  RGBEasy. */
               RGBChainOutputBuffer(hRGB,
                  gPBitmapInfo[gBufferIndex],
                  (void*)gPBitmapBits[gBufferIndex]);

#ifdef _DEBUG
               /* Increase counter of dropped buffers. */
               gNumDroppedBuffers++;
#endif
            }

            /* Store the index of the latest captured buffer. */
            gBufferIndex = i;
            gBufferTimeStamp = timeStamp;
            SetEvent(gHBitsInList);
         }

         /* Release Mutex. */
         ReleaseMutex ( gHMutex );
         break;
      }
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
      signed long    liveStream;
      LIVESTREAM     val = LIVESTREAM_1;

      /* Check for LiveStream support. */
      error = RGBInputIsLiveStreamSupported( input, &liveStream );

      if (error == 0)
      {
         if ( liveStream )
         {
            /* Enable LiveStream. */
            RGBSetLiveStream( gHRGB, val );
         }
      }

      /* Maximise the capture rate. */
      error = RGBSetFrameDropping ( gHRGB, 0 );

      if ( error == 0 )
      {
         /* Set the Frame Captured callback function. */
         error = RGBSetFrameCapturedFnEx ( gHRGB, FrameCapturedFnEx, 0 );
         if ( error == 0 )
         {
            /* If we haven't a bitmap we still continue, the 
             * driver will allocate it's own buffers which 
             * we can use. */
            if ( gBuffers )
            {
               unsigned i;

               for ( i = 0; i < gBuffers; i++ )
               {
                  error = RGBChainOutputBuffer ( gHRGB, gPBitmapInfo[i], 
                     gPBitmapBits[i] );
               }
            }

            error = RGBUseOutputBuffers ( gHRGB, TRUE );
            if ( error == 0 )
            {
               gBChainBuffer = TRUE;
            }
            
            error = RGBStartCapture ( gHRGB );
         }
      }
   }

   return error;
}

/******************************************************************************/

void
StopCapture ( )
{
   if ( RGBUseOutputBuffers( gHRGB, FALSE ) == 0 )
   {
      /* We haven't stopped the capture yet. It's possible we'll be called
       * with another frame of data. These buffers
       * should not be chained so we set a flag to indicate it to the callback.
       * Stopping the capture without calling RGBUseOutputBuffers would also 
       * work. */
      gBChainBuffer = FALSE;
   }

   RGBStopCapture ( gHRGB );
   RGBCloseInput ( gHRGB );

   gHRGB = 0;
}

/******************************************************************************/

void
ReleaseVideoDevice ( )
{
   if ( gpIDirect3D9 )
      gpIDirect3D9->lpVtbl->Release ( gpIDirect3D9 );

   if ( gpIDirect3DDevice9 )
      gpIDirect3DDevice9->lpVtbl->Release ( gpIDirect3DDevice9 );
}

/******************************************************************************/

void
ReleaseVideoDeviceBuffers ( )
{
   unsigned nBuffer;

   for ( nBuffer = 0; nBuffer < gBuffers; nBuffer++ )
   {
      if ( gpIDirect3DSurface9[nBuffer] )
         gpIDirect3DSurface9[nBuffer]->lpVtbl->UnlockRect ( 
               gpIDirect3DSurface9[nBuffer] );

      if ( gpIDirect3DSurface9[nBuffer] )
         gpIDirect3DSurface9[nBuffer]->lpVtbl->Release ( 
               gpIDirect3DSurface9[nBuffer] );

      if ( gppIDirect3DSwapChain9[nBuffer] )
         gppIDirect3DSwapChain9[nBuffer]->lpVtbl->Release (
               gppIDirect3DSwapChain9[nBuffer] );
   }

   free ( gppIDirect3DSwapChain9 );
}

/******************************************************************************/

HRESULT
CreateVideoDevice ( 
   HWND                    hWnd,
   int                     width,
   int                     height,
   D3DPRESENT_PARAMETERS   *pParms,
   D3DDISPLAYMODE          *pd3DDisplayMode )
{
   HRESULT  hr = E_FAIL;

   gpIDirect3D9 = Direct3DCreate9 ( D3D_SDK_VERSION );
   if ( gpIDirect3D9 )
   {
      /* Retrieves the current display mode of the adapter. */
      hr = gpIDirect3D9->lpVtbl->GetAdapterDisplayMode ( gpIDirect3D9, 
            D3DADAPTER_DEFAULT, pd3DDisplayMode );

      if ( SUCCEEDED(hr) )
      {
         pParms->Windowed             = TRUE;
         pParms->BackBufferCount      = 1;
         pParms->BackBufferWidth      = width;
         pParms->BackBufferHeight     = height;
         pParms->BackBufferFormat     = pd3DDisplayMode->Format;
         pParms->SwapEffect           = D3DSWAPEFFECT_COPY; 
         pParms->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
         pParms->Flags                = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

         /* Creates a device to represent the display adapter. */
         hr = gpIDirect3D9->lpVtbl->CreateDevice ( gpIDirect3D9, 
               D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, 
               D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, 
               pParms, &gpIDirect3DDevice9 );
      }
   }

   return hr;
}

/******************************************************************************/

HRESULT
CreateVideoDeviceBuffer (
   D3DPRESENT_PARAMETERS   *pParms,
   D3DLOCKED_RECT          *plockedRect,
   unsigned                nBuffer )
{
   HRESULT  hr;

   hr = gpIDirect3DDevice9->lpVtbl->CreateAdditionalSwapChain (
         gpIDirect3DDevice9, pParms, &gppIDirect3DSwapChain9[nBuffer] );

   if ( SUCCEEDED(hr) )
   {
      hr = gppIDirect3DSwapChain9[nBuffer]->lpVtbl->GetBackBuffer (
            gppIDirect3DSwapChain9[nBuffer], 0, D3DBACKBUFFER_TYPE_MONO,
            &gpIDirect3DSurface9[nBuffer] );

      if ( SUCCEEDED(hr) )
      {
         hr = gpIDirect3DSurface9[nBuffer]->lpVtbl->LockRect ( 
               gpIDirect3DSurface9[nBuffer], plockedRect, NULL, 
               D3DLOCK_NOSYSLOCK );
      }
   }

   return hr;
}

/******************************************************************************/

void
CreateBitmapInformation (
   BITMAPINFO  *pBitmapInfo,
   int         width,
   int         height,
   int         bitCount )
{
   pBitmapInfo->bmiHeader.biWidth          = width;
   pBitmapInfo->bmiHeader.biHeight         = -height;
   pBitmapInfo->bmiHeader.biBitCount       = bitCount;
   pBitmapInfo->bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
   pBitmapInfo->bmiHeader.biPlanes         = 1;
   pBitmapInfo->bmiHeader.biCompression    = BI_BITFIELDS;
   pBitmapInfo->bmiHeader.biSizeImage      = 0;
   pBitmapInfo->bmiHeader.biXPelsPerMeter  = 3000;
   pBitmapInfo->bmiHeader.biYPelsPerMeter  = 3000;
   pBitmapInfo->bmiHeader.biClrUsed        = 0;
   pBitmapInfo->bmiHeader.biClrImportant   = 0;
   pBitmapInfo->bmiHeader.biSizeImage      = width * height * bitCount / 8 ;

   switch ( bitCount )
   {
      case 16:
      {
         memcpy ( &pBitmapInfo->bmiColors, &ColourMasks[RGB_565], 
               sizeof(ColourMasks[RGB_565]) );
         break;
      }
      case 32:
      {
         memcpy ( &pBitmapInfo->bmiColors, &ColourMasks[RGB_888], 
               sizeof(ColourMasks[RGB_888]) );
         break;
      }
      default:
      {
         memcpy ( &pBitmapInfo->bmiColors, &ColourMasks[RGB_UNKNOWN], 
               sizeof(ColourMasks[RGB_UNKNOWN]) );
      }
   }
}

/******************************************************************************/

int
GetDesktopPixelDepth (
   unsigned pixelDepth )
{
   /* 32bit colour format. */
   if ( pixelDepth == D3DFMT_X8R8G8B8 )
   {
      return 32;
   }
   /* 16bit colour format. */
   else if ( pixelDepth == D3DFMT_R5G6B5  )
   {
      return 16;
   }
   /* Unknown colour format. */
   else
   {
      return 0;
   }
}

/******************************************************************************/
#ifdef _DISPLAY_TEXT
int
DisplayText (
   LONG        offset,
   ID3DXFont   *pID3DXFont )
{
   HRESULT hr = S_OK;

   /* The text to display */
   TCHAR tcText[256] = TEXT("SAMPLE 3C");

   /* The text colour. */
   D3DCOLOR fontColor = D3DCOLOR_RGBA(0xff, 0x00, 0x00, 0xff);

   /* Destination rectangle for the text. */
   RECT rct = { offset + 10, 10, 500, 300 };

#ifdef _DEBUG
   if (gTickCount64)
   {
      /* Capture and Render rates*/
      double captureRate = 1000.0 * (double)gNumCapturedBuffers / (double)(GetTickCount64() - gTickCount64);
      double renderRate  = 1000.0 * (double)gNumRenderedBuffers / (double)(GetTickCount64() - gTickCount64);

      StringCchPrintf(tcText, 256, TEXT("Capture frames = %d rate = %.2fHz\nRender  frames = %d rate = %.2fHz\nDropped frames = %d"),
         gNumCapturedBuffers, captureRate, gNumRenderedBuffers, renderRate, gNumDroppedBuffers);
   }
#endif

   if (pID3DXFont)
   {
      /* Adjust the rect. */
      pID3DXFont->lpVtbl->DrawText(pID3DXFont, NULL, tcText, -1, &rct,
         DT_CALCRECT, fontColor);

      /* Draw the text. */
      hr = pID3DXFont->lpVtbl->DrawText(pID3DXFont, NULL, tcText, -1, &rct,
         0, fontColor);
   }

   return hr;
}
#endif
/******************************************************************************/

DWORD WINAPI
DirectXThread (
   THREADINFO  *pgThreadInfo )
{
   D3DPRESENT_PARAMETERS   parms = {0};
   D3DDISPLAYMODE          d3DDisplayMode;
   D3DLOCKED_RECT          lockedRect[NUM_BUFFERS];
   HRESULT                 hr;
   RECT                    rect;

#ifdef _DISPLAY_TEXT
   ID3DXFont               *pID3DXFont = NULL;
#endif
#ifdef _DISPLAY_IMAGE
   IDirect3DSurface9       *pTextureSurface = NULL;
   IDirect3DTexture9       *pTexture        = NULL;
#endif

   /* Ensure the default window does not scale in DirectX. */
   GetClientRect ( pgThreadInfo->hWnd, &rect ); 

   /* Initialise our DirectX device. */
   hr = CreateVideoDevice ( pgThreadInfo->hWnd, rect.right, rect.bottom, 
      &parms, &d3DDisplayMode );

   if ( SUCCEEDED(hr) )
   {
      unsigned bitCount, nBuffer;

      bitCount = GetDesktopPixelDepth ( d3DDisplayMode.Format );

      /* Allocate memory for our list of backbuffers.*/
      gppIDirect3DSwapChain9 = malloc ( NUM_BUFFERS * 
            sizeof(IDirect3DSwapChain9*) );

      if ( gppIDirect3DSwapChain9 )
      {
         /* Now create the video memory we are going to capture into. */
         for ( nBuffer = 0, gBuffers = 0; nBuffer < NUM_BUFFERS; nBuffer++ )
         {
            gPBitmapInfo[gBuffers] = (LPBITMAPINFO) malloc ( 
                  sizeof(BITMAPINFOHEADER) + ( 3 * sizeof(DWORD) ) );

            if ( gPBitmapInfo[gBuffers] )
            {
               CreateBitmapInformation ( gPBitmapInfo[gBuffers], rect.right, 
                     rect.bottom, bitCount );

               hr = CreateVideoDeviceBuffer ( &parms, &lockedRect[gBuffers], 
                     gBuffers );

               if ( SUCCEEDED(hr) )
               {
                  gPBitmapBits[gBuffers] = lockedRect[gBuffers].pBits;

                  if ( gPBitmapBits[gBuffers] )
                  {
                     /* Ammend the buffer size with the specified pitch. */
                     gPBitmapInfo[gBuffers]->bmiHeader.biSizeImage = 
                        lockedRect[gBuffers].Pitch *
                        abs(gPBitmapInfo[gBuffers]->bmiHeader.biHeight);

                     gBuffers++;
                  }
                  else
                  {
                     /* TODO: Error. */
                  }
               }
               else
               {
                  /* TODO: Error. */
               }
            }
         }
#ifdef _DISPLAY_TEXT
         /* Create the font that will be used to display the text. */
         D3DXCreateFont ( gpIDirect3DDevice9, 0, 0, FW_BOLD, 1, FALSE,
               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
               DEFAULT_PITCH | FF_DONTCARE, TEXT("Courier New"), &pID3DXFont );
#endif
#ifdef _DISPLAY_IMAGE
         {
            /* Load a texture from a file. */
            HRESULT hr = D3DXCreateTextureFromFileEx ( gpIDirect3DDevice9,
                  TEXT("Overlay.png"), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                  D3DUSAGE_RENDERTARGET, d3DDisplayMode.Format, D3DPOOL_DEFAULT,
                  D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture );

            if ( SUCCEEDED(hr) )
            {
               /* Obtain the surface the the texture. */
               pTexture->lpVtbl->GetSurfaceLevel ( pTexture, 0,
                     &pTextureSurface );
            }
         }
#endif
         /* Create event used to signal a completed operation. */
         gHBitsInList = CreateEvent ( NULL, FALSE, FALSE, NULL );

         gHMutex = CreateMutex ( NULL, FALSE, NULL );

          /* Allow WinMain to continue. */
         SetEvent ( pgThreadInfo->hEvent );

         while ( !gClose )
         {
            unsigned index;

            /* Wait until there is a frame in the buffer. */
            if ( WaitForSingleObject ( gHBitsInList, INFINITE ) 
               == WAIT_OBJECT_0 )
            {
               if ( !gClose )
               {
                  HRESULT  hr = E_FAIL;
            
                  /* Wait for access to the protected buffer index variable. */
                  WaitForSingleObject ( gHMutex, INFINITE );
                  /* Take a local copy of gBufferIndex so buffer mutex can be 
                     released. */
                  index = gBufferIndex;
                  /* Set the index as no buffer ready for any new index. */
                  gBufferIndex = NO_BUFFER;
                  /* Release mutex to the protected buffer index variable. */
                  ReleaseMutex ( gHMutex );

                  if ( index != NO_BUFFER )
                  {
#ifdef _DEBUG
                     gNumRenderedBuffers++;

                     /* Buffer is about to be rendered. */
                     gBufferTrack[index].State = TEXT ( 'R' );
#endif
                     hr = gpIDirect3DSurface9[index]->lpVtbl->UnlockRect ( 
                           gpIDirect3DSurface9[index] );
                     if ( SUCCEEDED(hr) )
                     {
                        /* Begin DirectX drawing. All DirectX drawing must take
                           place between BeginScene and EndScene. */
                        gpIDirect3DDevice9->lpVtbl->BeginScene ( 
                              gpIDirect3DDevice9 );
   #ifdef _DISPLAY_TEXT
                        /* Specify the surface to draw to. */
                        gpIDirect3DDevice9->lpVtbl->SetRenderTarget (
                              gpIDirect3DDevice9, 0, gpIDirect3DSurface9[index] );
                        /* Write text to current surface. */
                        DisplayText ( pTextureSurface? 200 : 0, pID3DXFont );
   #endif
   #ifdef _DISPLAY_IMAGE
                        {
                           RECT rct = { 0, 0, 200, 100 };

                           if ( pTextureSurface )
                           {
                               /* Stretch texture onto current buffer. */
                              gpIDirect3DDevice9->lpVtbl->StretchRect ( 
                                    gpIDirect3DDevice9, pTextureSurface, NULL,
                                    gpIDirect3DSurface9[index], &rct,
                                    D3DTEXF_LINEAR );
                           }
                        }
   #endif
                        /* End DirectX drawing. */
                        gpIDirect3DDevice9->lpVtbl->EndScene (
                              gpIDirect3DDevice9 );

                        /* Presents the contents of the next buffer in the 
                           sequence of back buffers owned by the swap chain. */
                        gppIDirect3DSwapChain9[index]->lpVtbl->Present (
                              gppIDirect3DSwapChain9[index], NULL, NULL, 
                              pgThreadInfo->hWnd, NULL, 0 );  

                        /* Lock the buffer before passing it back to the 
                           swapchain. */
                        hr = gpIDirect3DSurface9[index]->lpVtbl->LockRect ( 
                              gpIDirect3DSurface9[index],  
                              &lockedRect[index], NULL, D3DLOCK_NOSYSLOCK );
                        if (SUCCEEDED(hr))
                        {
                           gPBitmapBits[index] = lockedRect[index].pBits;

                           if (gPBitmapBits[index])
                           {
                              /* Ammend the buffer size with the specified pitch. */
                              gPBitmapInfo[index]->bmiHeader.biSizeImage =
                                 lockedRect[index].Pitch *
                                 abs(gPBitmapInfo[index]->bmiHeader.biHeight);
                           }
                        }
                     }

                     if ( SUCCEEDED(hr) && gBChainBuffer )
                     {
                        /* We own the buffer until it is passed back into the  
                         * driver. Once we chain it back in, it will be reused in  
                         * another capture. */
                        RGBChainOutputBuffer ( gHRGB, 
                           gPBitmapInfo[index], 
                           (void*) gPBitmapBits[index] );
                     }
#ifdef _DEBUG
                     /* Buffer is in RGBEasy. */
                     gBufferTrack[index].State = TEXT ( 'C' );
#endif   
                  }
               }
            }
         }

         /* Clear the BitmapInfo. */
         for ( nBuffer = 0; nBuffer < gBuffers; nBuffer++ )
         {
            free ( gPBitmapInfo[nBuffer] );
            gPBitmapInfo[nBuffer] = NULL;
         }

         /* Release the DirectX buffers and devices. */
         ReleaseVideoDeviceBuffers ( );
         ReleaseVideoDevice ( );

#ifdef _DISPLAY_TEXT
         if ( pID3DXFont )
         {
            /* Release the font. */
            pID3DXFont->lpVtbl->Release ( pID3DXFont );
         }
#endif
#ifdef _DISPLAY_IMAGE
         if ( pTextureSurface )
         {
            /* Release the surface. */
            pTextureSurface->lpVtbl->Release ( pTextureSurface );
         }

         if ( pTexture )
         {
            /* Release the texture. */
            pTexture->lpVtbl->Release ( pTexture );
         }
#endif
         CloseHandle ( gHMutex );
      }
   }

   /* Allow WinMain thread to continue. */
   SetEvent ( pgThreadInfo->hEvent );
   CloseHandle ( pgThreadInfo->hThread );
   return 0;
}

/******************************************************************************/

INT_PTR CALLBACK
WndMainProc (
   HWND     hWnd,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam )
{

#ifdef _DEBUG
   switch ( message )
   { 
      case WM_TIMER:
      {
         if ( wParam == 666 )
         {
            PrintTracking ( );
         }
         break;
      }
   }
#endif

   return DefWindowProc ( hWnd, message, wParam, lParam );
}

/******************************************************************************/

int APIENTRY
_tWinMain (
   HINSTANCE   hInstance,
   HINSTANCE   hPrevInstance,
   LPTSTR      lpCmdLine,
   int         nShowCmd )
{
   HRGBDLL  hRGBDLL;
   unsigned error;
   
   InitCommonControls ( );

   /* Load the RGBEASY API. */
   error = RGBLoad ( &hRGBDLL );
   if ( error == 0 )
   {    
      WNDCLASS wc;

      ZeroMemory ( &wc, sizeof(wc) );
      wc.lpfnWndProc    = WndMainProc;
      wc.hInstance      = hInstance;
      wc.lpszClassName  = Caption;
      wc.hbrBackground  = (HBRUSH)GetStockObject (BLACK_BRUSH);
      wc.hCursor        = LoadCursor ( NULL, IDC_ARROW );

      if ( RegisterClass ( &wc ) )
      {
         /* We need to create the window early, as the DirectX function,
            CreateDevice requires the hWnd. */
         gThreadInfo.hWnd = CreateWindow ( Caption, Caption,
               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
               CW_USEDEFAULT, CW_USEDEFAULT, 
               0, 0, hInstance, 0 );

         if ( gThreadInfo.hWnd )
         {
            /* Create event used to signal a completed Direct3D operation. */
            gThreadInfo.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
            if ( gThreadInfo.hEvent )
            {
               /* Create a DirectX thread to perform all Direct3D 
                  operations. */
               gThreadInfo.hThread = ( HANDLE ) CreateThread ( NULL, 0, 
                  &DirectXThread, &gThreadInfo, 0, &gThreadInfo.dwThreadID );
               if ( gThreadInfo.hThread )
               {
                  MSG   msg;
                  BOOL  bRet;

                  /* Boost thread priority. */
                  SetThreadPriority ( gThreadInfo.hThread, 
                     THREAD_PRIORITY_HIGHEST );

#ifdef _DEBUG
                  SetTimer( gThreadInfo.hWnd, 666, 333, NULL );
#endif

                  /* Wait for the DirectX initialisation to finish. */
                  if ( WaitForSingleObject ( gThreadInfo.hEvent, INFINITE ) 
                        == WAIT_OBJECT_0 )
                  {
                     if ( StartCapture ( INPUT - 1 ) == 0 )
                     {
                        ShowWindow ( gThreadInfo.hWnd, SW_SHOWNORMAL );

                        while ( ( bRet = GetMessage ( &msg, gThreadInfo.hWnd,
                              0, 0 ) ) != 0 )
                        {
                           if ( bRet == -1 )
                              break;

                           TranslateMessage ( &msg );
                           DispatchMessage ( &msg );
                        }
                        gClose = TRUE;
                     }

                     /* Signal DirectXThread to stop waiting for new 
                        frames. */
                     SetEvent ( gHBitsInList );

                     StopCapture ( );
#ifdef _DEBUG
                     KillTimer ( gThreadInfo.hWnd, 666 );
#endif

                     /* Wait for DirectXThread to finish. */
                     WaitForSingleObject ( gThreadInfo.hThread, INFINITE );
                  }
               }
            }
         }
      }
      RGBFree ( hRGBDLL );
   }

   return 0;
}

/******************************************************************************/
