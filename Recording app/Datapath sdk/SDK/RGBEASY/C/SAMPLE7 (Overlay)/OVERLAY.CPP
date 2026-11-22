#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define GDIPVER 0x0110
#include <gdiplus.h>
using namespace Gdiplus;

#include <api.h>
#include <cmdline.h>
#include <dgctypes.h>
#include <dgcmedia.h>
#include <rgb.h>
#include <rgbapi.h>

/* Global Variables ***********************************************************/

HRGBDLL hRGBDll = 0;
Bitmap *pBitmap = NULL;
ULONG_PTR gditoken = 0;
BOOL bShutdown = FALSE;

/* Command line arguments *****************************************************/

static const TCHAR _Output[] = { TEXT("Output") };
static const TCHAR _File[] = { TEXT("File") };
static const TCHAR _OffsetX[] = { TEXT("OffsetX") };
static const TCHAR _OffsetY[] = { TEXT("OffsetY") };
static const TCHAR _Timeout[] = { TEXT("Timeout") };
static const TCHAR _Animate[] = { TEXT("Animate") };
static const TCHAR _KeyColour[] = { TEXT("KeyColour") };
static const TCHAR _Input[] = { TEXT("Input") };

#define STARTUP_FLAG_OUTPUT      0x00000001
#define STARTUP_FLAG_FILE        0x00000002
#define STARTUP_FLAG_OFFSETX     0x00000004
#define STARTUP_FLAG_OFFSETY     0x00000008
#define STARTUP_FLAG_TIMEOUT     0x00000010
#define STARTUP_FLAG_ANIMATE     0x00000020
#define STARTUP_FLAG_KEYCOLOUR   0x00000040
#define STARTUP_FLAG_INPUT       0x00000080


#define REQ_STARTUP_FLAGS \
   (STARTUP_FLAG_OUTPUT | STARTUP_FLAG_FILE )

typedef struct
{
   unsigned long  Flags;   /* Flags used to manage command line processing. */
   unsigned long  Output;
   TCHAR File[MAX_PATH];
   unsigned long  OffsetX;
   unsigned long  OffsetY;
   unsigned long  Timeout;
   BOOL  Animate;
   unsigned long KeyColour;
   unsigned long  Input;
}  STARTUP, *PSTARTUP;

STARTUP startup = {
   0,             // Flags
   0,             // Output
   TEXT("\0"),    // File
   0,             // OffsetX
   0,             // OffsetY
   1000,          // Timeout
   FALSE,         // Animate
   0x00FFFFFF,    // KeyColour = White
   0,
};

/******************************************************************************/

unsigned long ExtractOutput(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   unsigned int value;
   int      error;


   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_OUTPUT)
   {
      error = -1;
   }
   else
   {
      error = ExtractUnsignedInt(pValue, nValueChars,
         0, &value);

      if (error == 0)
      {
         if (value > 0)
         {
            pStartUp->Output = value;
            pStartUp->Flags |= STARTUP_FLAG_OUTPUT;
         }
      }
   }

   return error;
}

/******************************************************************************/

unsigned long ExtractFile(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   int      error;

   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_FILE)
   {
      error = -1;
   }
   else
   {
      TCHAR buffer[MAX_PATH];

      error = ExtractString(pValue, nValueChars,
         (PVOID)(sizeof(buffer) / sizeof(TCHAR)), buffer);
      if (error == 0)
      {
         GetFullPathName(buffer,
            sizeof(pStartUp->File) / sizeof(TCHAR),
            pStartUp->File, NULL);

         pStartUp->Flags |= STARTUP_FLAG_FILE;
      }
   }

   return error;
}

/******************************************************************************/

unsigned long ExtractOffsetX(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   unsigned int value;
   int      error;

   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_OFFSETX)
   {
      error = -1;
   }
   else
   {
      error = ExtractUnsignedInt(pValue, nValueChars,
         0, &value);

      if (error == 0)
      {
         pStartUp->OffsetX = value;
         pStartUp->Flags |= STARTUP_FLAG_OFFSETX;
      }
   }

   return error;
}

/******************************************************************************/

unsigned long ExtractOffsetY(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   unsigned int value;
   int      error;

   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_OFFSETY)
   {
      error = -1;
   }
   else
   {
      error = ExtractUnsignedInt(pValue, nValueChars,
         0, &value);

      if (error == 0)
      {
         pStartUp->OffsetY = value;
         pStartUp->Flags |= STARTUP_FLAG_OFFSETY;
      }
   }

   return error;
}

/******************************************************************************/

unsigned long ExtractTimeout(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   unsigned int value;
   int      error;

   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_TIMEOUT)
   {
      error = -1;
   }
   else
   {
      error = ExtractUnsignedInt(pValue, nValueChars,
         0, &value);

      if (error == 0)
      {
         pStartUp->Timeout = value;
         pStartUp->Flags |= STARTUP_FLAG_TIMEOUT;
      }
   }

   return error;
}

/******************************************************************************/

unsigned long ExtractAnimate(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   int      error = 0;

   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_ANIMATE)
   {
      error = -1;
   }
   else
   {
      pStartUp->Flags |= STARTUP_FLAG_ANIMATE;
      pStartUp->Animate = TRUE;
   }

   return error;
}

/******************************************************************************/

unsigned long ExtractKeyColour(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   int      error;
   UINT     data[4] = {-1, -1, -1, -1};

   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_KEYCOLOUR)
   {
      error = -1;
   }
   else
   {
      error = ExtractTLWH(pValue, nValueChars, (PVOID)4, data);
      if (error == 0)
      {
         if ((data[0] > 255) || (data[1] > 255) || (data[2] > 255) || (data[3] > 255))
         {
            error = -1;
         }
         else
         {
            pStartUp->Flags |= STARTUP_FLAG_KEYCOLOUR;
            pStartUp->KeyColour = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
         }
      }
   }

   return error;
}

/******************************************************************************/

unsigned long ExtractInput(LPCTSTR pValue, int nValueChars, PVOID pData, PVOID pVoid)
{
   PSTARTUP pStartUp;
   unsigned int value;
   int      error;

   pStartUp = (PSTARTUP)pVoid;

   if (pStartUp->Flags & STARTUP_FLAG_INPUT)
   {
      error = -1;
   }
   else
   {
      error = ExtractUnsignedInt(pValue, nValueChars,
         0, &value);

      if (error == 0)
      {
         if (value > 0)
         {
            pStartUp->Input = value;
            pStartUp->Flags |= STARTUP_FLAG_INPUT;
         }
      }
   }

   return error;
}

/******************************************************************************/

unsigned long CommandLineErrorFn(unsigned long error, LPCTSTR pKeyword,
   int nKeywordChars, LPCTSTR pValue, int nValueChars, int nParameter,
   PKEYWORDTODATA pKeywordToData, PVOID pVoid)
{
   return 1;
}

/******************************************************************************/

void StartUpValuesFromCommandLine(int argc, TCHAR *argv[], PSTARTUP pStartUp)
{
   int i;

   /* Definition of the command line interface. */
   KEYWORDTODATA  KeywordToData[] =
   {
      { _Output, ExtractOutput, 0, pStartUp },
      { _File, ExtractFile, 0, pStartUp },
      { _OffsetX, ExtractOffsetX, 0, pStartUp },
      { _OffsetY, ExtractOffsetY, 0, pStartUp },
      { _Timeout, ExtractTimeout, 0, pStartUp },
      { _Animate, ExtractAnimate, 0, pStartUp },
      { _KeyColour, ExtractKeyColour, 0, pStartUp},
      { _Input, ExtractInput, 0, pStartUp},
      { NULL, NULL, 0, NULL },
   };

   for (i = 1; i < argc; i++)
   {
      ParseCommandLine(argv[i], KeywordToData, pStartUp, CommandLineErrorFn);
   }
}

/******************************************************************************/

#define ERROR_NO_ERROR                 0
#define STARTUP_ERROR                  1
#define MEMORY_ERROR                   2
#define GDIPLUSSTARTUP_ERROR           3
#define FILE_ERROR                     4
#define GETRAWFORMAT_ERROR             5
#define GETPIXELFORMAT_ERROR           6
#define LOCKBITS_ERROR                 7
#define RGBLOAD_ERROR                  100
#define RGBGETINPUTSIGNALTYPE_ERROR    101
#define NOSIGNAL_OUTOFRANGE_ERROR      102
#define RGBOPENINPUT_ERROR             103
#define RGBSETCAPTUREWIDTH_ERROR       104
#define RGBSETCAPTUREHEIGHT_ERROR      105
#define RGBGETMODEINFO_ERROR           106
#define RGBOVERLAYSETIMAGE_ERROR       107
#define RGBOVERLAYFLIPBUFFER_ERROR     108
#define RGBOVERLAYSETTIMEOUT_ERROR     109
#define RGBOVERLAYENABLE_ERROR         110
#define RGBOVERLAYKEEPALIVE_ERROR      111
#define RGBOVERLAYMAPPING_ERROR        112

int ErrorReturn(int code, int error)
{
   TCHAR buffer[256];

   switch (code)
   {
   case ERROR_NO_ERROR:
      _stprintf_s(buffer, 256, TEXT("Goodbye...."));
      break;

   case STARTUP_ERROR:
      _stprintf_s(buffer, 256, TEXT("Usage: overlay133.exe -output=<n> -file=<filename> [ input=<n> -offsetX=<x> -offsetY=<y> -Timeout=<t> ]"));
      break;
   case MEMORY_ERROR:
      _stprintf_s(buffer, 256, TEXT("Memory allocation error."));
      break;
   case GDIPLUSSTARTUP_ERROR:
      _stprintf_s(buffer, 256, TEXT("GdiplusStartup: error = 0x%08x."), error);
      break;
   case FILE_ERROR:
      _stprintf_s(buffer, 256, TEXT("File Error: error = 0x%08x."), error);
      break;
   case GETRAWFORMAT_ERROR:
      _stprintf_s(buffer, 256, TEXT("GetRawFormat: error = 0x%08x"), error);
      break;
   case GETPIXELFORMAT_ERROR:
      _stprintf_s(buffer, 256, TEXT("GetPixelFormat: error = 0x%08x"), error);
      break;
   case LOCKBITS_ERROR:
      _stprintf_s(buffer, 256, TEXT("LockBits: error = 0x%08x"), error);
      break;
   case RGBLOAD_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBLoad: error = 0x%08x"), error);
      break;
   case RGBGETINPUTSIGNALTYPE_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBGetInputSignalType: error = 0x%08x"), error);
      break;
   case NOSIGNAL_OUTOFRANGE_ERROR:
      _stprintf_s(buffer, 256, TEXT("NoSignal or InvalidSignal on Input %d"), startup.Output);
      break;
   case RGBOPENINPUT_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBOpenInput: error = 0x%08x"), error);
      break;
   case RGBSETCAPTUREWIDTH_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBSetCaptureWidth: error = 0x%08x"), error);
      break;
   case RGBSETCAPTUREHEIGHT_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBSetCaptureHeight: error = 0x%08x"), error);
      break;
   case RGBGETMODEINFO_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBGetModeInfo: error = 0x%08x"), error);
      break;
   case RGBOVERLAYSETIMAGE_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBOutputSetOverlayImage: error = 0x%08x"), error);
      break;
   case RGBOVERLAYFLIPBUFFER_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBOutputFlipOverlayBuffer: error = 0x%08x"), error);
      break;
   case RGBOVERLAYSETTIMEOUT_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBOverlaySetTimeout: error = 0x%08x"), error);
      break;
   case RGBOVERLAYENABLE_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBOutputEnableOverlay: error = 0x%08x"), error);
      break;
   case RGBOVERLAYKEEPALIVE_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBOutputKeepOverlayAlive: error = 0x%08x"), error);
      break;
   case RGBOVERLAYMAPPING_ERROR:
      _stprintf_s(buffer, 256, TEXT("RGBOutputGetInputNumber/RGBOutputSetInputNumber: error = 0x%08x"), error);
      break;
   default:
      _stprintf_s(buffer, 256, TEXT("Unknown Error code: %d, error = 0x%08x"), code, error);
      break;
   }

   _tprintf(buffer);

   if (pBitmap)
   {
      delete pBitmap;
      pBitmap = NULL;
   }

   if (gditoken)
   {
      GdiplusShutdown(gditoken);
      gditoken = 0;
   }

   if (hRGBDll)
   {
      RGBFree(hRGBDll);
      hRGBDll = 0;
   }

   exit(code);

   //
   // Keep the compiler happy
   //
   return code;
}

/******************************************************************************/

BOOL WINAPI
CTRLC_HandlerRoutine(
   DWORD dwCtrlType)
{
   bShutdown = TRUE;
   return TRUE;
}

/******************************************************************************/

void AdjustAlphaChannel(unsigned long width, unsigned long height,
                        unsigned long stride, void *pBuffer, unsigned long keycolour)
{
#define COLOUR_MASK  0x00ffffff
#define FULL_ALPHA   0xff000000

   for (unsigned long i = 0; i < height; i++)
   {
      for (unsigned long j = 0; j < width; j++)
      {
         unsigned long pixel = (i * stride / 4) + j;
         if ((((unsigned long*)pBuffer)[pixel] & COLOUR_MASK) == (keycolour & COLOUR_MASK))
         {
            ((unsigned long*)pBuffer)[pixel] = keycolour;
         }
         else
         {
            ((unsigned long*)pBuffer)[pixel] |= FULL_ALPHA;
         }
      }
   }
}

/******************************************************************************/

int _tmain(int argc, TCHAR* argv[])
{
   SIGNALTYPE signalType;
   RGBMODEINFO modeInfo = { sizeof(RGBMODEINFO), };
   unsigned long captureWidth, captureHeight, refreshRate, error;
   bool bStereo = false;
   GUID guid, *pDimensionIDs;
   GdiplusStartupInput gdiplusInput;
   Status status;
   PixelFormat pixelFormat;
   UINT count, frameCount, totalBuffer, iCurrentFrame, xdir, ydir;
   PropertyItem *pItem = NULL;
   Rect rect;

   // Pick up the command line options
   error = 0;
   StartUpValuesFromCommandLine(argc, argv, &startup);
   if ((startup.Flags & REQ_STARTUP_FLAGS) != REQ_STARTUP_FLAGS)
      return ErrorReturn(STARTUP_ERROR, error);

   // Adjust output number from User range (1 to N) to SDK range (0 to N-1)
   //
   startup.Output -= 1;

   //
   // Load the SDK
   //
   error = RGBLoad(&hRGBDll);
   if (error)
      return ErrorReturn(RGBLOAD_ERROR, error);

   //
   // Check if an output to input mapping has been specified
   //
   if (startup.Flags & STARTUP_FLAG_INPUT)
   {
      //
      // Adjust input number from User range (1 to N) to SDK range (0 to N-1)
      //
      startup.Input -= 1;

      error = RGBOutputSetInputNumber(startup.Output, startup.Input);
      if (error)
         return ErrorReturn(RGBOVERLAYMAPPING_ERROR, error);
   }
   else
   {
      //
      // Find out the input output mapping
      //
      error = RGBOutputGetInputNumber(startup.Output, &startup.Input);
      if (error)
         return ErrorReturn(RGBOVERLAYMAPPING_ERROR, error);
   }

   //
   // Check if there is a valid signal on the mapped input
   //
   error = RGBGetInputSignalType(startup.Input, &signalType, &captureWidth,
      &captureHeight, &refreshRate);
   if (error)
      return ErrorReturn(RGBGETINPUTSIGNALTYPE_ERROR, error);
   if ((signalType == RGB_SIGNALTYPE_NOSIGNAL) || (signalType == RGB_SIGNALTYPE_OUTOFRANGE))
      return ErrorReturn(NOSIGNAL_OUTOFRANGE_ERROR, NOSIGNAL_OUTOFRANGE_ERROR);

   //
   // Determine if this is a dual-stream or stereoscopic mode
   //
   error = RGBInputGetModeInfo(startup.Input, &modeInfo);
   if (error)
      return ErrorReturn(RGBGETMODEINFO_ERROR, error);
   bStereo = modeInfo.BDVI && ((modeInfo.DigitalType == RGB_TYPE_SDI) &&
      ((modeInfo.SDIFlags == SDI_TYPE_3GB_DS) || (modeInfo.SDIFlags == SDI_TYPE_3GB_STEREO)));

   //
   // Clear the overlay buffers
   //
   bool clear = true;
   if (clear)
   {
      unsigned long i, width, stride;
      PBYTE p = NULL;

      //
      // For dual-stream or stereoscopic we have to clear left/right halves separately
      //
      if (bStereo)
         width = captureWidth / 2;
      else
         width = captureWidth;

      stride = width * 4;

      p = new BYTE[stride * captureHeight];
      if (p == NULL)
         return ErrorReturn(MEMORY_ERROR, error);
      ZeroMemory(p, stride * captureHeight);

      //
      // The overlay is double buffered and we have to clear each buffer in turn.
      // For dual stream / stereoscopic modes we have to clear each stream independently.
      //
      for (i = 0; i < 2; i++)
      {
         if (bStereo)
         {
            //
            // Left eye / Stream 1
            // 
            error = RGBOutputSetOverlayImage(startup.Output, width, captureHeight, stride,
               RGB_COLOURDOMAINDETECT_RGB, p, 0, 0);
            if (error)
               return ErrorReturn(RGBOVERLAYSETIMAGE_ERROR, error);
            //
            // Right eye / Stream 2
            //
            error = RGBOutputSetOverlayImage(startup.Output, width, captureHeight, stride,
               RGB_COLOURDOMAINDETECT_RGB, p, width, 0);
            if (error)
               return ErrorReturn(RGBOVERLAYSETIMAGE_ERROR, error);
         }
         else
         {
            error = RGBOutputSetOverlayImage(startup.Output, captureWidth, captureHeight, stride,
               RGB_COLOURDOMAINDETECT_RGB, p, 0, 0);
            if (error)
               return ErrorReturn(RGBOVERLAYSETIMAGE_ERROR, error);
         }
         //
         // Flip overlay buffers
         //
         error = RGBOutputFlipOverlayBuffer(startup.Output);
         if (error)
            return ErrorReturn(RGBOVERLAYFLIPBUFFER_ERROR, error);
      }
      delete p;
   }


   //
   // Start up Gdiplus
   //
   status = GdiplusStartup(&gditoken, &gdiplusInput, NULL);
   if (status != Ok)
      return ErrorReturn(GDIPLUSSTARTUP_ERROR, status);

   //
   // Open the bitmap file
   //
   pBitmap = new Bitmap(startup.File);
   status = pBitmap->GetRawFormat(&guid);
   if (status != Ok)
      return ErrorReturn(GETRAWFORMAT_ERROR, status);

   //
   // We only support PixelFormat32bppRGB or PixelFormat32bppARGB
   //
   pixelFormat = pBitmap->GetPixelFormat();
   if (!((pixelFormat == PixelFormat32bppRGB) || (pixelFormat == PixelFormat32bppARGB)))
      return ErrorReturn(GETPIXELFORMAT_ERROR, status);

   //
   // First of all we should get the number of frame dimensions
   // Images considered by GDI+ as: frames[animation_frame_index][how_many_animation];
   //
   count = pBitmap->GetFrameDimensionsCount();
   pDimensionIDs = new GUID[count];
   if (pDimensionIDs == NULL)
      return ErrorReturn(MEMORY_ERROR, status);

   //
   // Get the list of frame dimensions from the Image object.
   //
   pBitmap->GetFrameDimensionsList(pDimensionIDs, count);

   //
   // Get the number of frames in the first dimension.
   //
   frameCount = pBitmap->GetFrameCount(&pDimensionIDs[0]);
   delete pDimensionIDs;

   if (frameCount > 1)
   {
   //
   // PropertyTagFrameDelay is a pre-defined identifier to present frame-delays by GDI+
   //
      totalBuffer = pBitmap->GetPropertyItemSize(PropertyTagFrameDelay);
      pItem = (PropertyItem*)malloc(totalBuffer);
      if (pItem == NULL)
         return ErrorReturn(MEMORY_ERROR, status);
      pBitmap->GetPropertyItem(PropertyTagFrameDelay, totalBuffer, pItem);
   }

   rect.X = rect.Y = 0;
   rect.Width = pBitmap->GetWidth();
   rect.Height = pBitmap->GetHeight();

   //
   // Set the CTRL-C handler
   //
   SetConsoleCtrlHandler(CTRLC_HandlerRoutine, TRUE);

   //
   // Set the overlay time out
   //
   error = RGBOverlaySetTimeout(startup.Timeout);
   if (error)
      return ErrorReturn(RGBOVERLAYSETTIMEOUT_ERROR, error);

   //
   // Enable the overlay
   //
   error = RGBOutputEnableOverlay(startup.Output);
   if (error)
      return ErrorReturn(RGBOVERLAYENABLE_ERROR, error);


   _tprintf(TEXT("Press CTRL-C do exit the program\n"));

   //Set Current Frame at #0
   iCurrentFrame = 0;
   xdir = ydir = 2;
   do
   {
      BitmapData bitmapData;

      //
      // Send a overlay keep alive
      //
      error = RGBOutputKeepOverlayAlive(startup.Output);
      if (error)
         return ErrorReturn(RGBOVERLAYKEEPALIVE_ERROR, error);

      //
      // Select the overlay bitmap
      //
      guid = FrameDimensionTime;
      pBitmap->SelectActiveFrame(&guid, iCurrentFrame);

      //
      // Lock the overlay bitmap into memory
      //
      status = pBitmap->LockBits(&rect, ImageLockModeRead, pixelFormat, &bitmapData);
      if (status != Ok)
         return ErrorReturn(LOCKBITS_ERROR, status);

      //
      // The overlay bitmap may or may not contain a valid alpha channel.
      // Unfortunatly GDI+ strips it out due to the lack of proper support for alpha in BMP files.
      //
      if (pixelFormat == PixelFormat32bppRGB)
      {
         AdjustAlphaChannel(bitmapData.Width, bitmapData.Height, bitmapData.Stride, bitmapData.Scan0,
                            startup.KeyColour);
      }

      //
      // Write the overlay bitmap
      //
      error = RGBOutputSetOverlayImage(startup.Output, bitmapData.Width, bitmapData.Height, bitmapData.Stride,
         RGB_COLOURDOMAINDETECT_RGB709_FULL, bitmapData.Scan0, startup.OffsetX, startup.OffsetY);
      if (error)
         return ErrorReturn(RGBOVERLAYSETIMAGE_ERROR, error);

      //
      // Flip the overlay buffer
      //
      error = RGBOutputFlipOverlayBuffer(startup.Output);
      if (error)
         return ErrorReturn(RGBOVERLAYFLIPBUFFER_ERROR, error);

      //
      // Unlock the overlay bitmap from memory
      //
      pBitmap->UnlockBits(&bitmapData);

      if (startup.Animate)
      {
         //
         // Move the overlay bitmap across the overlay
         //
         unsigned long X1, X2;
         if (bStereo)
         {
            if (startup.OffsetX < captureWidth / 2)
            {
               //
               // Left eye / Stream 1
               //
               X1 = captureWidth / 2;
               X2 = captureWidth;
            }
            else
            {
               //
               // Right eye / Stream 2
               //
               X1 = 0;
               X2 = captureWidth / 2;

            }
         }
         else
         {
            X1 = 0;
            X2 = captureWidth;
         }

         if ((startup.OffsetX + bitmapData.Width + xdir) >= X2)
            xdir = -2;
         else if (startup.OffsetX == X1)
            xdir = +2;
         startup.OffsetX += xdir;

         if ((startup.OffsetY + bitmapData.Height + ydir) >= captureHeight)
            ydir = -2;
         else if (startup.OffsetY == 0)
            ydir = +2;
         startup.OffsetY += ydir;
      }

      if (frameCount > 1)
      {
         //
         // Try to match the playback speed
         //
         Sleep(((UINT*)pItem[0].value)[iCurrentFrame] * 10);

         //
         // Select the next frame
         //
         iCurrentFrame = (++iCurrentFrame) % frameCount;
      }

   } while (bShutdown == FALSE);

   return ErrorReturn(0, 0);
}