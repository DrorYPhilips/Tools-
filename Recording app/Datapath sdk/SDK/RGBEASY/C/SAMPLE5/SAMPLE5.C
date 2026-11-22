/*******************************************************************************

Copyright Datapath Ltd. 2008, 2014.

File:    SAMPLE5.C

Purpose: VisionRGB-X example program that shows how to display an RGB window
         using AMD DirectGMA or NVIDIA GPUDirect technology.

History:
         04 SEP 12    OM   Created.
         13 AUG 13    OM   Added check for no signal or out of range signal.
         11 OCT 13    OM   Corrected bug, passing wrong parameter for NVIDIA.
         08 APR 14    OM   Added greyscale option.
         10 OCT 14    OM   Fixed NVIDIA bug and tidied up code.
         03 JUN 15    OM   DMA to the GPU should not be done if buffer is returned to capture driver.

*******************************************************************************/

#include <windows.h>
#include <process.h>
#include <tchar.h>
#include <strsafe.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>
#include "resource.h"

#define RGB_UNKNOWN  0
#define RGB_565      1
#define RGB_24       2
#define RGB_888      3

#define FRGB8        8
#define FRGB565      16
#define FRGB24       24
#define FRGB888      32

#define NUM_BUFFERS  8
#define ONE_SECOND   1000 * 1000 * 1000L
#define NO_BUFFER    0xFF

/* Static Constants ***********************************************************/

static const TCHAR
   RGBWindowClass[]  = { TEXT ( "RGBSampleWndClass" ) },
   Caption[]         = { TEXT ( "RGB Sample 5" ) };

static struct 
{
   COLORREF Mask[3];
}  ColourMasks[] =
{
   { 0x00000000, 0x00000000, 0x00000000,},  /* Unknown */
   { 0x0000f800, 0x000007e0, 0x0000001f,},  /* RGB565 */
   { 0x00ff0000, 0x0000ff00, 0x000000ff,},  /* RGB24 */
   { 0x00ff0000, 0x0000ff00, 0x000000ff,},  /* RGB888 */
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

/* Global Variables ***********************************************************/
typedef struct
{
   HWND                 HWnd;
   HINSTANCE            HInstance;
   HRGBDLL              HRGBDLL;
   HDC                  HDC;
   HGLRC                HGLRC;
   HANDLE               HOGLSetupEvent;
   HANDLE               HRenderThread;
   HANDLE               HMutex;
   int                  XPos;
   int                  YPos;
   unsigned long        Width;
   unsigned long        Height;
   unsigned long        RefreshRate;
   SIGNALTYPE           SignalType;
   unsigned long        Input;
   unsigned int         CaptureFormat; 
   unsigned int         ColourFormat;
   unsigned int         ByteFormat;
   unsigned int         RgbFormat;
   unsigned int         FormatSize;
   unsigned int         BufferIndex;
   unsigned int         *PDataBuffer;
   GLuint               OGLBuffer[NUM_BUFFERS];
   GLuint               OGLTexture[NUM_BUFFERS];
   int                  Viewport[4];
   LPBITMAPINFO         PBitmapInfo[NUM_BUFFERS];
   HRGB                 HRGB;
   BOOL                 BChainBuffer;
   BOOL                 BThreadRunning;
   GRAPHICSHARDWARE     Hardware;
   HANDLE               HCapturedBuffer;
   unsigned long        Error;
#ifdef _DEBUG
   TRACK                BufferTrack[NUM_BUFFERS];
   signed long          NumDroppedBuffers;
   signed long          NumRepeatedBuffers;
#endif
} GLOBAL;

GLOBAL Global =
{
   NULL,                      /* HWnd */
   NULL,                      /* HInstance */
   0,                         /* HRGBDLL */
   NULL,                      /* HDC */
   NULL,                      /* HGLRC */
   NULL,                      /* HOGLSetupEvent */
   NULL,                      /* HRenderThread */
   NULL,                      /* HMutex */
   CW_USEDEFAULT,             /* XPos */
   CW_USEDEFAULT,             /* YPos */
   800,                       /* Width */
   600,                       /* Height */
   0,                         /* RefreshRate */
   RGB_SIGNALTYPE_NOSIGNAL,   /* SignalType */
   0,                         /* Input */
   FRGB24,                    /* CaptureFormat possible values: FRGB8, FRGB565, FRGB24, FRGB888 */
   0,                         /* ColourFormat */
   0,                         /* ByteFormat */
   0,                         /* RgbFormat */
   0,                         /* FormatSize */
   NO_BUFFER,                 /* BufferIndex */
   NULL,                      /* PDataBuffer */
   { 0 },                     /* OGLBuffer */
   { 0 },                     /* OGLTexture */
   { 0 },                     /* Viewport */
   { NULL },                  /* PBitmapInfo */
   0,                         /* HRGB */
   TRUE,                      /* BChainBuffer */
   TRUE,                      /* BThreadRunning */
   GPU_AMD,                   /* Hardware */
   NULL,                      /* HCapturedBuffer */
   0,                         /* Error */
#ifdef _DEBUG
   { 0 },                     /* BufferTrack */
   0,                         /* NumDroppedBuffers */
   0,                         /* NumRepeatedBuffers */
#endif
};

#ifdef _DEBUG
/******************************************************************************/

void 
PrintTracking ( )
{
   int      i;
   TCHAR    text[MAX_PATH];

   for ( i = 0; i < NUM_BUFFERS; i++ )
   {
      StringCchPrintf ( text, MAX_PATH, TEXT ( "BufID: %d, State: %c\n" ), i,
         Global.BufferTrack[i].State );
      OutputDebugString ( text );
   }

   StringCchPrintf ( text, MAX_PATH, TEXT ( "Dropped: %d Repeated: %d ------\n" ), 
      Global.NumDroppedBuffers, Global.NumRepeatedBuffers );
   OutputDebugString ( text );
}

#endif

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
   pBitmapInfo->bmiHeader.biSizeImage      = 0;
   pBitmapInfo->bmiHeader.biXPelsPerMeter  = 3000;
   pBitmapInfo->bmiHeader.biYPelsPerMeter  = 3000;
   pBitmapInfo->bmiHeader.biClrUsed        = 0;
   pBitmapInfo->bmiHeader.biClrImportant   = 0;
   pBitmapInfo->bmiHeader.biSizeImage      = width * height * bitCount / 8 ;

   switch ( bitCount )
   {
      case 8:
      {
         int i;
         for( i = 0; i < 256; i++ )
         {
            pBitmapInfo->bmiColors[i].rgbRed = i;
            pBitmapInfo->bmiColors[i].rgbGreen = i;
            pBitmapInfo->bmiColors[i].rgbBlue = i;
            pBitmapInfo->bmiColors[i].rgbReserved = 0;
         }

         Global.FormatSize = 1;
         Global.ColourFormat = GL_LUMINANCE;
         Global.ByteFormat = GL_UNSIGNED_BYTE;
         Global.RgbFormat = RGB_PIXELFORMAT_GREY;

         pBitmapInfo->bmiHeader.biCompression = BI_RGB;
         break;
      }

      case 16:
      {
         memcpy ( &pBitmapInfo->bmiColors, &ColourMasks[RGB_565], 
            sizeof(ColourMasks[RGB_565]) );
         Global.FormatSize = 2;
         Global.ColourFormat = GL_RGB;
         Global.ByteFormat = GL_UNSIGNED_SHORT_5_6_5;
         Global.RgbFormat = RGB_PIXELFORMAT_565;

         pBitmapInfo->bmiHeader.biCompression = BI_BITFIELDS;
         break;
      }

      case 24:
      {
         memcpy ( &pBitmapInfo->bmiColors, &ColourMasks[RGB_24], 
            sizeof(ColourMasks[RGB_24]) );
         Global.FormatSize = 3;
         Global.ColourFormat = GL_BGR_EXT;
         Global.ByteFormat = GL_UNSIGNED_BYTE;
         Global.RgbFormat = RGB_PIXELFORMAT_RGB24;

         pBitmapInfo->bmiHeader.biCompression = BI_RGB;
         break;
      }

      case 32:
      {
         memcpy ( &pBitmapInfo->bmiColors, &ColourMasks[RGB_888], 
            sizeof(ColourMasks[RGB_888]) );
         Global.FormatSize = 4;
         Global.ColourFormat = GL_BGRA_EXT;
         Global.ByteFormat = GL_UNSIGNED_BYTE;
         Global.RgbFormat = RGB_PIXELFORMAT_888;

         pBitmapInfo->bmiHeader.biCompression = BI_BITFIELDS;
         break;
      }
   }
}

/******************************************************************************/

void
FreeBitmapInformation ( )
{
   int j;

   /* Clear the BitmapInfo. */
   for ( j = 0; j < NUM_BUFFERS; j++ )
   {
      if ( Global.PBitmapInfo[j] != NULL )
      {
         free ( Global.PBitmapInfo[j] );
      }
   }
}

/******************************************************************************/
/* Rendering function. */

int
DrawGLScene (
   unsigned int index )
{
   GLsync        fence;

   if ( Global.Hardware == GPU_NVIDIA )
   {
      RGBDirectGPUNVIDIAOp ( Global.HRGB, index, NVIDIA_GPU_WAIT );
   }

   /* Clear screen and depth buffer. */
   glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   /* Reset the current matrix. */
   glLoadIdentity ( );

   glBindTexture ( GL_TEXTURE_2D, Global.OGLTexture[index] );

   if ( Global.Hardware == GPU_AMD )
   {
      glBindBuffer ( GL_PIXEL_UNPACK_BUFFER, Global.OGLBuffer[index] );
      glTexSubImage2D ( GL_TEXTURE_2D, 0, 0, 0, Global.Width, Global.Height, Global.ColourFormat,
         Global.ByteFormat, NULL );
   }

   /* Insert Sync object to check for completion. */
   fence = glFenceSync ( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );

   /* Draw a square. */
   glBegin ( GL_QUADS );
   /* Front Face. */
   /* Bottom left of the texture and quad. */
   glTexCoord2f ( 0.0f, 1.0f );
   glVertex3f ( -1.0f, -1.0f,  0.0f );
   /* Bottom right of the texture and quad. */
   glTexCoord2f ( 1.0f, 1.0f ); 
   glVertex3f ( 1.0f, -1.0f, 0.0f );
   /* Top right of the texture and quad. */
   glTexCoord2f (1.0f, 0.0f );
   glVertex3f ( 1.0f,  1.0f,  0.0f );
   /* Top left of the texture and quad. */
   glTexCoord2f (0.0f, 0.0f );
   glVertex3f (-1.0f,  1.0f,  0.0f );
   glEnd ( );

   glBindTexture ( GL_TEXTURE_2D, 0 );

   if ( glIsSync ( fence ) )
   {
      glClientWaitSync ( fence, GL_SYNC_FLUSH_COMMANDS_BIT, ONE_SECOND );
      glDeleteSync ( fence );
   }

   if ( Global.Hardware == GPU_NVIDIA )
   {
      RGBDirectGPUNVIDIAOp ( Global.HRGB, index, NVIDIA_GPU_END );
   }

   return TRUE;
}

/******************************************************************************/
/* Stops capture. */

void 
StopCapture ( )
{
   if ( Global.HRGBDLL )
   {
      RGBStopCapture ( Global.HRGB );
      RGBSetFrameCapturedFn ( Global.HRGB, NULL, 0 );
      RGBDirectGPUClose ( Global.HRGB );
   }
}

/******************************************************************************/
/* Close RGBEasy. */

void
CloseRGBEasy ( )
{
   if ( Global.HRGBDLL )
   {
      RGBCloseInput ( Global.HRGB );
      RGBFree ( Global.HRGBDLL );
   }
}

/******************************************************************************/
/* Window Proc. */

LRESULT CALLBACK
RGBWndProc (
   HWND     hWnd,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam )
{
   switch ( message )
   {
      case WM_CHAR:
      {
         switch ( wParam )
         {
            case VK_ESCAPE:
            {
               DestroyWindow ( hWnd );
               break;
            }
         }
         return 0;
      }

   #ifdef _DEBUG
      case WM_TIMER:
      {
         if ( wParam == 666 )
         {
            PrintTracking ( );
         }
         break;
      }
   #endif

      case WM_DESTROY:
      {
         PostQuitMessage ( 0 );
         return 0;
      }
   }

   return DefWindowProc ( hWnd, message, wParam, lParam );
}

/******************************************************************************/

unsigned long
RegisterRGBWindowClass (
   HINSTANCE   hInstance )
{
   WNDCLASS wc;

   wc.style = 0;
   wc.lpfnWndProc = (WNDPROC) RGBWndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = LoadIcon ( hInstance, MAKEINTRESOURCE ( IDI_ICON ) );
   wc.hCursor = LoadCursor ( NULL, IDC_ARROW );
   wc.hbrBackground = NULL;
   wc.lpszMenuName =  NULL;
   wc.lpszClassName = RGBWindowClass;

   if ( RegisterClass ( &wc ) == 0 )
   {
      return GetLastError ( );
   }
   else
   {
      return 0;
   }
}

/******************************************************************************/
/* The RGBEasy frame capture callback function. */

void 
RGBCBKAPI FrameCapturedFnEx (
   HWND                 hWnd,
   HRGB                 hRGB,
   PRGBFRAMEDATA        pFrameData,
   ULONG_PTR            pUserData  )
{
   unsigned int i;

   for ( i = 0; i < NUM_BUFFERS; i++ )
   {
      if ( ( pFrameData->PBitmapBits == (void *) Global.PDataBuffer[i] ) &&
         pFrameData->PBitmapInfo && Global.BChainBuffer )
      {
         /* Wait for access to the protected buffer index variable. */
         WaitForSingleObject ( Global.HMutex, INFINITE );

#ifdef _DEBUG
         /* Store the state variable. */
         Global.BufferTrack[i].State = TEXT ( 'A' );
#endif

         /* A valid buffer doesn't exist. It is stored so the buffer can be 
            rendered later. */
         if ( Global.BufferIndex == NO_BUFFER )
         {
             if ( Global.Hardware == GPU_NVIDIA )
            {
               /* Buffer is copied to the graphics card by DMA operation. */
               RGBDirectGPUNVIDIAOp ( hRGB, i, NVIDIA_GPU_COPY );
            }

            Global.BufferIndex = i;
         }
         else
         {
            /* A valid buffer is waiting to be rendered.
               Therefore, the capturing rate is faster than the rendering rate.
               To keep minimum latency this buffer is dropped and returned to
               RGBEasy. */
            RGBChainOutputBufferEx ( Global.HRGB, 
               Global.PBitmapInfo[Global.BufferIndex], 
               (void*) Global.PDataBuffer[Global.BufferIndex], 
               RGB_BUFFERTYPE_DIRECTGMA );

            /* Store the index of the latest captured buffer. */
            Global.BufferIndex = i;

#ifdef _DEBUG
            /* Increase counter of dropped buffers. */
            Global.NumDroppedBuffers++;
#endif
         }

         /* Release Mutex. */
         ReleaseMutex ( Global.HMutex );

         /* A valid buffer is stored, tell the render thread to process it. */
         SetEvent ( Global.HCapturedBuffer );
         break;
      }
   }
}

/******************************************************************************/
/* Setup for the OpenGL textures. */

void 
SetupOpenGLTextures ( ) 
{
   unsigned int i;

   /* Generate texture name for Global.OGLTexture. */
   glGenTextures ( NUM_BUFFERS, Global.OGLTexture );

   for ( i = 0; i < NUM_BUFFERS; i++ )
   {
      /* Bind Global.OGLTexture to the texture container. */
      glBindTexture ( GL_TEXTURE_2D, Global.OGLTexture[i] );

      glTexImage2D ( GL_TEXTURE_2D, 0, 3, Global.Width, Global.Height, 0, Global.ColourFormat,
         Global.ByteFormat, NULL );

      /* Linear filtering. */
      glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
      glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

      /* Unbind Global.OGLTexture from OpenGL texture container. */
      glBindTexture ( GL_TEXTURE_2D, 0 );
   }
}

/******************************************************************************/
/* OpenGL Initialization. */

void 
InitializeOpenGL ( ) 
{
   /* Black background. */
   glClearColor ( 0.0f, 0.0f, 1.0f, 0.5f );

   /* Enables depth testing. */
   glEnable ( GL_DEPTH_TEST );

   /* Enable texturing. */
   glEnable ( GL_TEXTURE_2D );

   /* Enable smooth shading. */
   glShadeModel ( GL_SMOOTH );

   /* Set rasterization mode. */
   glPolygonMode ( GL_FRONT, GL_FILL );

   /* Set type of perspective calculations. */
   glHint ( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

   /* Activate Sync to VBlank to avoid tearing. */
   wglSwapIntervalEXT ( 1 );
}

/******************************************************************************/
/* Free OpenGL window setup. */

void
FreeOpenGLSetup ( )
{
   /* Delete named textures. */
   glDeleteTextures ( NUM_BUFFERS, Global.OGLTexture );

   if ( Global.Hardware == GPU_AMD )
   {
      /* Free buffers only needed for AMD hardware. */
      glDeleteBuffers ( NUM_BUFFERS, &Global.OGLBuffer[0] );
   }

   if (Global.HGLRC){
      wglMakeCurrent ( NULL, NULL );
      wglDeleteContext(Global.HGLRC);
   }

   if (Global.HDC){
      ReleaseDC(Global.HWnd,Global.HDC);
   }
}

/******************************************************************************/
/* OpenGL window setup. */

BOOL
SetupOpenGLWindow ( )
{
   /* OpenGL pixel format. */
   GLuint   pixelFormat;

   /* OpenGL pixel format descriptor. */
   PIXELFORMATDESCRIPTOR pfd=
   {
      sizeof(PIXELFORMATDESCRIPTOR),   /* pfd size */
      1,                               /* Version number */
      PFD_DRAW_TO_WINDOW |             /* Format must support window */
      PFD_SUPPORT_OPENGL |             /* Format must support OpenGL */
      PFD_DOUBLEBUFFER,                /* Must support double buffering */
      PFD_TYPE_RGBA,                   /* Request an RGBA format */
      Global.CaptureFormat,            /* Select color depth */
      0, 0, 0, 0, 0, 0,                /* Color bits ignored */
      0,                               /* No alpha buffer */
      0,                               /* Shift bit ignored */
      0,                               /* No accumulation buffer */
      0, 0, 0, 0,                      /* Accumulation bits ignored */
      16,                              /* 16Bit z-buffer (depth buffer) */
      0,                               /* No stencil buffer */
      0,                               /* No auxiliary buffer */
      PFD_MAIN_PLANE,                  /* Main drawing layer */
      0,                               /* Reserved */
      0, 0, 0                          /* Layer masks ignored */
   };

   /* OpenGL get device context. */
   if  ( !( Global.HDC = GetDC ( Global.HWnd ) ) )
   {
      MessageBox ( NULL, TEXT ( "Can't create OpenGL device context." ),
         TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
      return FALSE;
   }

   /* OpenGL find pixel format. */
   if ( !( pixelFormat = ChoosePixelFormat ( Global.HDC, &pfd ) ) )
   {
      MessageBox ( NULL, TEXT ( "Can't find a suitable pixel format." ),
         TEXT ( "ERROR" ) , MB_OK | MB_ICONEXCLAMATION );
      return FALSE;
   }

   /* OpenGL set pixel format. */
   if ( !SetPixelFormat ( Global.HDC, pixelFormat, &pfd ) )
   {
      MessageBox ( NULL, TEXT ( "Can't set the pixel format." ) , TEXT ( "ERROR" ),
         MB_OK | MB_ICONEXCLAMATION);
      return FALSE;
   }

   /* OpenGL create rendering context. */
   if ( !( Global.HGLRC = wglCreateContext ( Global.HDC ) ) )
   {
      MessageBox ( NULL, TEXT ( "Can't create OpenGL rendering context." ),
         TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
      return FALSE;
   }

   /* OpenGL activate rendering context. */
   if ( !wglMakeCurrent( Global.HDC, Global.HGLRC ) )
   {
      MessageBox ( NULL, TEXT ( "Can't activate OpenGL rendering context." ),
         TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
      return FALSE;
   }

   /* Load OpenGL Glew library. */
   if ( glewInit ( ) != GLEW_OK )
   {
      MessageBox ( NULL, TEXT ( "Can't load OpenGL Glew." ),
         TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
      return FALSE;
   }

   return TRUE;
}

/******************************************************************************/
/* Setup for the comunication with graphics card. */

unsigned long
SetupDirectGPU ( )
{
   unsigned int          j;
   GPUTRANSFERDESCRIPTOR gpuTransfer;
   unsigned long         error = 0;

   /* Fill in structure to define parameters for the
      AMD DirectGMA functionality. */
   gpuTransfer.Size = sizeof(GPUTRANSFERDESCRIPTOR);
   gpuTransfer.Buffer = &Global.PDataBuffer;
   gpuTransfer.Width = Global.Width;
   gpuTransfer.Height = Global.Height;
   gpuTransfer.OglByteFormat = Global.ByteFormat;
   gpuTransfer.OglColourFormat = Global.ColourFormat;
   gpuTransfer.FormatSize = Global.FormatSize;
   gpuTransfer.NumBuffers = NUM_BUFFERS;

   /* Try to initialize RGBEasy for AMD GPU (DirectGMA) communication. */
   Global.Hardware = GPU_AMD;
   gpuTransfer.GpuBrand = Global.Hardware;

   /* Only needed for AMD hardware. */
   glGenBuffers ( NUM_BUFFERS, &Global.OGLBuffer[0] );
   /* Note: for AMD the buffer object names are passed. */
   gpuTransfer.OglObject = &Global.OGLBuffer[0];

   error = RGBDirectGPUInit ( Global.HRGB, &gpuTransfer );
   if ( error )
   {
      /* Free buffers only needed for AMD hardware. */
      glDeleteBuffers ( NUM_BUFFERS, &Global.OGLBuffer[0] );

      /* Try to initialize RGBEasy to NVIDIA GPU (GPUDirect) communication. */
      Global.Hardware = GPU_NVIDIA;
      gpuTransfer.GpuBrand = Global.Hardware;

      /* Note: for NVIDIA the texture names are passed. */
      gpuTransfer.OglObject = &Global.OGLTexture[0];

      error = RGBDirectGPUInit ( Global.HRGB, &gpuTransfer );
   }

   if (error == 0)
   {
      /* The size of the buffer allocated is stored in
         the field bufferSize. The size of the buffer is re-assigned to the
         bitmap header for any changes in pitch. */
      for ( j = 0; j < NUM_BUFFERS; j++ )
      {
         Global.PBitmapInfo[j]->bmiHeader.biSizeImage = gpuTransfer.BufferSize;
      }
   }

   return error;
}

/******************************************************************************/
/* Thread in charge of OpenGL operations. */

DWORD WINAPI 
OpenGLThread (
    void *pData )
{
   int            j;
   unsigned int   index;

   if ( SetupOpenGLWindow ( ) == TRUE )
   {
      /* Create Bitmap Information for capture. */
      for ( j = 0; j < NUM_BUFFERS; j++ )
      {
         if ( Global.CaptureFormat != 8 )
         {
            Global.PBitmapInfo[j] = (LPBITMAPINFO) malloc ( 
               sizeof(BITMAPINFOHEADER) + ( 3 * sizeof(DWORD) ) );
         }
         else
         {
            Global.PBitmapInfo[j] = (LPBITMAPINFO) malloc ( 
               sizeof(BITMAPINFOHEADER) + ( 256 * sizeof(DWORD) ) );
         }

         CreateBitmapInformation ( Global.PBitmapInfo[j], Global.Width,
            Global.Height, Global.CaptureFormat );
      }

      SetupOpenGLTextures ( );

      InitializeOpenGL ( );

      Global.Error = SetupDirectGPU ( );

      /* Allow WinMain to continue. */
      SetEvent ( Global.HOGLSetupEvent );

      if ( Global.Error == 0 )
      {
         while ( Global.BThreadRunning )
         {
            /* Wait for access to the protected buffer index variable. */
            WaitForSingleObject ( Global.HMutex, INFINITE );
            /* Take a local copy of Global.BufferIndex so buffer mutex can be 
               released. */
            index = Global.BufferIndex;
            /* Set the index as no buffer ready for any new index. */
            Global.BufferIndex = NO_BUFFER;
            /* Release mutex to the protected buffer index variable. */
            ReleaseMutex ( Global.HMutex );
            /* If a valid buffer index exists, render it. If it does not, the 
               previous capture buffer will be rastered again from graphics card 
               memory. This may happen when the render rate is faster
               than the capture rate. */
            if ( index != NO_BUFFER )
            {
#ifdef _DEBUG
               /* Buffer is about to be rendered. */
               Global.BufferTrack[index].State = TEXT ( 'R' );
#endif
               /* Render buffer. */
               DrawGLScene ( index );
               SwapBuffers ( Global.HDC );

               /* Now that the buffer has been rendered give it back to RGBEasy. */
               Global.Error = RGBChainOutputBufferEx ( Global.HRGB, 
                  Global.PBitmapInfo[index], (void*) Global.PDataBuffer[index],
                  RGB_BUFFERTYPE_DIRECTGMA );
#ifdef _DEBUG
               /* Buffer is in RGBEasy. */
               Global.BufferTrack[index].State = TEXT ( 'C' );
#endif
               /* Reset the event as rendering has finished. */
               ResetEvent ( Global.HCapturedBuffer );
            }
#ifdef _DEBUG
            else
            {
               Global.NumRepeatedBuffers++;
            }
#endif

            WaitForSingleObject ( Global.HCapturedBuffer, INFINITE );
         }
      }

      FreeBitmapInformation();

      FreeOpenGLSetup ( );
   }

   return 0;
}

/******************************************************************************/
/* Setup capture. */

unsigned long
StartCapture ( )
{
   unsigned long  error = 0;
   unsigned int   j;
   signed long    liveStream;
   LIVESTREAM     val = LIVESTREAM_1;

   /* Check for LiveStream support. */
   error = RGBInputIsLiveStreamSupported(Global.Input, &liveStream );

   if (error == 0)
   {
      if ( liveStream )
      {
         /* Enable LiveStream. */
         RGBSetLiveStream(Global.HRGB, val );
      }
   }

   /* Maximise the capture rate. */
   error = RGBSetFrameDropping ( Global.HRGB, 0 );
   if ( error == 0 )
   {
      /* Set Capture format. */
      error = RGBSetPixelFormat ( Global.HRGB, Global.RgbFormat );
      if ( error == 0 )
      {
         /* Set the Frame Captured callback function. */
         error = RGBSetFrameCapturedFnEx ( Global.HRGB, FrameCapturedFnEx, 0 );
         if ( error == 0 )
         {
            error = RGBSetOutputSize ( Global.HRGB, Global.Width, Global.Height );
            if ( error != 0 )
            {
               MessageBox ( NULL, TEXT ( "Couldn't set the output size." ),
                  TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
            }
            else
            {
               /* Pass buffers to RGBEasy.*/
               for ( j =0; j < NUM_BUFFERS; j++ )
               {
                  error = RGBChainOutputBufferEx ( Global.HRGB, 
                     Global.PBitmapInfo[j], (void*) Global.PDataBuffer[j], 
                     RGB_BUFFERTYPE_DIRECTGMA);

                  if ( error != 0 )
                  {
                     MessageBox ( NULL, TEXT ( "Couldn't set output buffer." ),
                        TEXT ( "ERROR" ) , MB_OK | MB_ICONEXCLAMATION );
                  }
#ifdef _DEBUG
                  /* Buffer is in RGBEasy. */
                  Global.BufferTrack[j].State = TEXT ( 'C' );
#endif
               }

               error = RGBUseOutputBuffers ( Global.HRGB, TRUE );
               if ( error == 0 )
               {
                  Global.BChainBuffer = TRUE;
               }

               error = RGBStartCapture ( Global.HRGB );
               if ( error != 0 )
               {
                  MessageBox ( NULL, TEXT ( "Couldn't start capture." ),
                     TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
               }
            }
         }
      }
   }

   return error;
}

/******************************************************************************/
/* Create window. */

unsigned long
CreateRGBWindow (
   int x,
   int y )
{
   unsigned long  error = 0;

   Global.HWnd = CreateWindowEx ( WS_EX_APPWINDOW, RGBWindowClass, Caption, 
      WS_POPUP, x, y, Global.Width, Global.Height, (HWND) NULL, NULL, 
      Global.HInstance, (LPVOID) Global.HRGB );

   if ( Global.HWnd != NULL )
   {
      /* Display window. */
      ShowWindow ( Global.HWnd, SW_SHOWNORMAL );
   }
   else
   {
      error = GetLastError ( );
   }

   return error;
}

/******************************************************************************/
/* Parse command line arguments. */
/* Use -input=input_number to specify input (0,1,2,3, etc.). Defaults to 0.
   Use -format=format_number to specify format (0 = 8 bit grayscale, 
        1 = 24 bit colour, 2 = 32 bit colour). Defaults to 1.
   Use -xpos=value to specifiy window x position. Defaults to 0.
   Use -ypos=value to specifiy window y position. Defaults to 0. */

void
ParseCommandLine(
   LPTSTR      lpCmdLine )
{
   TCHAR          cmd[MAX_PATH];
   TCHAR          *pStr;
   unsigned int   cmdLen;
   LPTSTR         nextToken,token;
   unsigned long  tmpValue;

   cmdLen = (unsigned int) _tcslen ( lpCmdLine );
   if ( cmdLen > 0 )
   {
      wcscpy_s ( cmd, sizeof(cmd) / sizeof(cmd[0]), lpCmdLine );

      /* Get input number, 0-based. */
      pStr = _tcsstr ( cmd, TEXT ( "-input=" ) );
      
      if ( pStr != NULL )
      {
         token = _tcstok_s ( pStr, TEXT ( "= \n" ), &nextToken );
         Global.Input = _ttoi ( nextToken );

         /* Restore string. */
         wcscpy_s ( cmd, sizeof(cmd) / sizeof(cmd[0]), lpCmdLine );
      }
      else
      {
         Global.Input = 0;
      }

      /* Get capture format, 0-based. */
      pStr = _tcsstr ( cmd, TEXT ( "-format=" ) );
      if (pStr != NULL )
      {
         token = _tcstok_s ( pStr, TEXT ( "= \n" ), &nextToken );
         tmpValue = _ttoi ( nextToken );

         /* Restore string. */
         wcscpy_s ( cmd, sizeof(cmd) / sizeof(cmd[0]), lpCmdLine );

         switch ( tmpValue )
         {
         case 0:
            /* Capture 8 bit greyscale. */
            Global.CaptureFormat = FRGB8;
            break;
         case 1:
            /* Capture 16 bit colour RGB. */
            Global.CaptureFormat = FRGB565;
            break;
         case 2:
            /* Capture 24 bit colour RGB. */
            Global.CaptureFormat = FRGB24;
            break;
         case 3:
            /* Capture 32 bit colour RGB. */
            Global.CaptureFormat = FRGB888;            
            break;
         default:
            /* Capture 24 bit colour RGB. */
            Global.CaptureFormat = FRGB24;
            break;   
         }
      }
      else
      {
         /* Capture 24 bit colour RGB. */
         Global.CaptureFormat = FRGB24;
      }

      /* Get window x position. */
      pStr = _tcsstr ( cmd, TEXT ( "-xpos=" ) );
      if ( pStr != NULL )
      {
         token = _tcstok_s ( pStr, TEXT ( "= \n" ), &nextToken );
         Global.XPos = _ttoi ( nextToken );

         /* Restore string. */
         wcscpy_s ( cmd, sizeof(cmd) / sizeof(cmd[0]), lpCmdLine );
      }
      else
      {
         Global.XPos = 0;
      }

      /* Get window y position. */
      pStr = _tcsstr ( cmd, TEXT ( "-ypos=" ) );
      if ( pStr != NULL )
      {
         token = _tcstok_s ( pStr, TEXT ( "= \n" ), &nextToken );
         Global.YPos = _ttoi ( nextToken );

         /* Restore string. */
         wcscpy_s ( cmd, sizeof(cmd) / sizeof(cmd[0]), lpCmdLine );
      }
      else
      {
         Global.YPos = 0;
      }
   }
}

/******************************************************************************/
/* Main function. */

int APIENTRY
_tWinMain (
   HINSTANCE   hInstance,
   HINSTANCE   hPrevInstance,
   LPTSTR      lpCmdLine,
   int         showCmd )
{
   int            myFlag = 0;
   MSG            msg;

   ParseCommandLine ( lpCmdLine );

   Global.HInstance = hInstance;

   /* Register the RGBWindowClass. */
   Global.Error = RegisterRGBWindowClass ( Global.HInstance );
   if ( Global.Error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH,
         TEXT ( "Error registering the window class: 0x%08x" ), Global.Error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return Global.Error;
   }

   Global.Error = RGBLoad ( &Global.HRGBDLL );
   if ( Global.Error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH,
         TEXT ( "Error returned from RGBLoad: 0x%08x" ), Global.Error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return Global.Error;
   }

   /* Open RGB input. */
   Global.Error = RGBOpenInput ( Global.Input, &Global.HRGB );

   if ( Global.Error == 0 )
   {
      Global.Error = RGBGetInputSignalType ( Global.Input, &Global.SignalType,
         &Global.Width, &Global.Height, &Global.RefreshRate );

      if ( ( Global.SignalType == RGB_SIGNALTYPE_NOSIGNAL ) || 
         ( Global.SignalType == RGB_SIGNALTYPE_OUTOFRANGE ) )
      {
         Global.Width = 800;
         Global.Height = 600;
      }

      if ( Global.Error != 0 )
      {
         MessageBox ( NULL, TEXT ( "Couln't recognize input signal type." ),
            TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
         CloseRGBEasy ( );
      }
      else
      {

         /* Create the application window. */
         Global.Error = CreateRGBWindow ( Global.XPos, Global.YPos );

         if ( Global.Error )
         {
            TCHAR buffer[MAX_PATH];

            StringCchPrintf ( buffer, MAX_PATH,
               TEXT ( "Error creating the application window: 0x%08x" ), 
               Global.Error );
            MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
            return Global.Error;
         }

         /* Create event used to signal a completed operation. */
         Global.HCapturedBuffer = CreateEvent ( NULL, FALSE, FALSE, NULL );

         /* Create mutex for shared data/ */
         Global.HMutex = CreateMutex ( NULL, FALSE, NULL );

         /* Create event used to signal a OpenGL setup and initialization completed. */
         Global.HOGLSetupEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

         /* Create a OpenGL thread to perform all OpenGL operations. */
         Global.HRenderThread = (HANDLE) CreateThread ( NULL, 0, &OpenGLThread,
            NULL, 0, NULL );

#ifdef _DEBUG
         SetTimer(Global.HWnd, 666, 333, NULL);
#endif

         /* Wait for the OpenGL initialisation to finish. */
         if ( WaitForSingleObject ( Global.HOGLSetupEvent, INFINITE ) 
            == WAIT_OBJECT_0 )
         {
            if ( Global.Error == 0 )
            {
               /* Setup and start capture. */
               StartCapture ( );

               /* Message loop. */
               while ( GetMessage ( &msg, NULL, 0, 0 ) )
               {
                  TranslateMessage ( &msg );
                  DispatchMessage ( &msg );
               }

               Global.BThreadRunning = FALSE;
               SetEvent ( Global.HCapturedBuffer );

               /* Wait for the rendering thread to end. */
               WaitForSingleObject (  Global.HRenderThread, INFINITE );
            }
            else
            {
               if ( Global.Error != 0 )
               {
                  MessageBox ( NULL,TEXT ( "Couldn't start GPU communication module." ), 
                     TEXT ( "ERROR" ), MB_OK | MB_ICONEXCLAMATION );
                  DestroyWindow ( Global.HWnd );
               }
            }
         }

         StopCapture();
         CloseRGBEasy();

#ifdef _DEBUG
         KillTimer ( Global.HWnd, 666 );
#endif

         CloseHandle ( Global.HRenderThread );
         CloseHandle ( Global.HOGLSetupEvent );
         CloseHandle ( Global.HMutex );
         CloseHandle ( Global.HCapturedBuffer );
      }
   }
   else
   {
      RGBFree ( Global.HRGBDLL );
   }

   UnregisterClass ( RGBWindowClass,Global.HInstance );

   UNREFERENCED_PARAMETER ( showCmd );
   UNREFERENCED_PARAMETER ( lpCmdLine );
   UNREFERENCED_PARAMETER ( hPrevInstance );

   return Global.Error;
}

/******************************************************************************/