/*******************************************************************************

Copyright Datapath Ltd. 2017.

File:    overlay.cpp

Purpose: demonstrate overlay functionality

History:
         08 APR 16    JL   Created.
         21 JUL 17    SB   Use IVisionUser2 object to access
                           get_ColourDomainDetected.

*******************************************************************************/

#include <Windows.h>

#define GDIPVER 0x0110
#include <gdiplus.h>

#include <dgc133um.h>

#pragma comment( lib, "GDIPlus" )

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

using namespace Gdiplus;

ULONG_PTR GDIPlusToken = 0;

HRESULT InitializeGDIPlus( ULONG_PTR *pToken )
{
   Status status;
   ULONG_PTR Token;
   GdiplusStartupInput input;

   status = GdiplusStartup( &Token, &input, NULL );
   if ( status == Ok )
   {
      *pToken = Token;
   }

   return (status == Ok) ? S_OK : E_FAIL;
}

/******************************************************************************/

void FinalizeGDIPlus( ULONG_PTR Token )
{
   GdiplusShutdown( Token );
}

/******************************************************************************/

HRESULT GetBitmapBits( WCHAR *pFile,
                       Bitmap **ppBitmap,
                       BitmapData *pBitmapData )
{
   HRESULT hr = E_FAIL;
   Bitmap *pBitmap = new Bitmap( pFile );

   if ( pBitmap )
   {
      PixelFormat pixelFormat = pBitmap->GetPixelFormat();

      if ( (pixelFormat == PixelFormat32bppRGB) ||
           (pixelFormat == PixelFormat32bppARGB) )
      {
         Rect rect;

         rect.X = 0;
         rect.Y = 0;
         rect.Width = pBitmap->GetWidth();
         rect.Height = pBitmap->GetHeight();

         if ( pBitmap->LockBits(
               &rect, ImageLockModeRead, pixelFormat, pBitmapData ) == Ok )
         {
            *ppBitmap = pBitmap;
            hr = S_OK;
         }
      }

      if ( FAILED( hr ) )
      {
         delete pBitmap;
      }
   }

   return hr;
}

/******************************************************************************/

void EndBitmapBits( Bitmap *pBitmap,
                    BitmapData *pBitmapData )
{
   pBitmap->UnlockBits( pBitmapData );
   delete pBitmap;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

const TCHAR Folder[] = TEXT( "C:\\Images" );

HRESULT GetFilePath( unsigned long width,
                     unsigned long height,
                     TCHAR *pFile )
{
   HRESULT hr = S_OK;

   wsprintf( pFile, TEXT( "%s\\%dx%d_overlay.bmp" ), Folder, width, height );
   if ( GetFileAttributes( pFile ) == INVALID_FILE_ATTRIBUTES )
   {
      wsprintf( pFile, TEXT( "%s\\overlay.bmp" ), Folder );
      if ( GetFileAttributes( pFile ) == INVALID_FILE_ATTRIBUTES )
      {
         hr = E_FAIL;
      }
   }

   return hr;
}

/******************************************************************************/

void AdjustAlphaChannel( unsigned long width, unsigned long height,
                         unsigned long stride, void *pBuffer, unsigned long keycolour )
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
BOOL bClear = TRUE;

HRESULT WriteOverlay( IVisionUser2 *pVision,
                      IVisionUserOverlay *pOverlay,
                      unsigned long width,
                      unsigned long height )
{
   HRESULT hr;
   signed long bSupported;
   signed long bEnabled = FALSE;

   hr = pOverlay->IsOverlaySupported( &bSupported );
   if ( SUCCEEDED( hr ) && (bSupported == FALSE) )
   {
      hr = E_NOINTERFACE;
   }

   if ( SUCCEEDED( hr ) )
   {
      // Is overlay already enabled for this input?
      // If it's enabled by another application, we may want to leave it alone.
      hr = pOverlay->IsOverlayEnabled( &bEnabled );
   }

   if ( SUCCEEDED( hr ) && bClear )
   {
      byte *pClear = new byte[width * height * 4];
      if (  pClear )
      {
         ZeroMemory( pClear, width * height * 4 );
         pOverlay->SetOverlayImage( width, height, width * 4,
                                    CLRDOMAINDETECT_AUTO, pClear, 0, 0 );
         delete[] pClear;
      }
   }

   // assuming that our image is full range RGB
   CLRDOMAIN colourdomain = CLRDOMAINDETECT_RGB;
   Bitmap *pBitmap = NULL;
   BitmapData bitmapData;

   if ( SUCCEEDED( hr ) )
   {
      TCHAR file[MAX_PATH];

      hr = GetFilePath( width, height, file );
      if ( SUCCEEDED( hr ) )
      {
         hr = GetBitmapBits( file, &pBitmap, &bitmapData );
         if ( SUCCEEDED( hr ) )
         {
            AdjustAlphaChannel( bitmapData.Width, bitmapData.Height, bitmapData.Stride,
                                bitmapData.Scan0, 0x00FFFFFF);
         }
      }
   }

   if ( SUCCEEDED( hr ) )
   {
      CLRDOMAIN colourdomainDetected;

      hr = pVision->get_ColourDomainDetected( &colourdomainDetected );

      // The Vision source filter will automatically colour-convert the
      // supplied overlay image into the colour space of the input/output
      // video signal.
      // We could colour-convert the image here to ensure that the
      // image we supply will not be manipulated at all.

      //ColourConvert( colourdomain,         // From (our bitmap format)
      //               colourdomainDetected, // To
      //               bitmapData.Scan0 );   // Image
      //colourdomain = colourdomainDetected;
   }

   if ( SUCCEEDED( hr ) )
   {
      unsigned long offsetX = 0, offsetY = 0;

      hr = pOverlay->SetOverlayImage( bitmapData.Width,
                                      bitmapData.Height,
                                      bitmapData.Stride,
                                      colourdomain,
                                      bitmapData.Scan0,
                                      offsetX,
                                      offsetY );

      if ( SUCCEEDED( hr ) && (bEnabled == FALSE) )
      {
         hr = pOverlay->OverlayEnable();
      }
   }

   if ( pBitmap )
   {
      EndBitmapBits( pBitmap, &bitmapData );
   }

   return hr;
}

/******************************************************************************/

HRESULT OverlayBegin( IVisionUser2 *pVision,
                      IVisionUserOverlay *pOverlay,
                      unsigned long width,
                      unsigned long height,
                      unsigned long timeout )
{
   HRESULT hr;
   signed long bSupported;

   hr = pOverlay->IsOverlaySupported( &bSupported );

   if ( SUCCEEDED( hr ) && (bSupported == FALSE) )
   {
      hr = E_NOINTERFACE;
   }

   if ( SUCCEEDED( hr ) )
   {
      hr = InitializeGDIPlus( &GDIPlusToken );
   }

   unsigned long min, max, def, current;

   if ( SUCCEEDED( hr ) )
   {
      hr = pOverlay->get_OverlayTimeoutMinimum( &min );
   }

   if ( SUCCEEDED( hr ) )
   {
      hr = pOverlay->get_OverlayTimeoutMaximum( &max );
   }

   if ( SUCCEEDED( hr ) )
   {
      hr = pOverlay->get_OverlayTimeoutDefault( &def );
   }

   if ( SUCCEEDED( hr ) )
   {
      hr = pOverlay->get_OverlayTimeout( &current );
   }

   if ( SUCCEEDED( hr ) )
   {
      hr = pOverlay->put_OverlayTimeout( timeout );
   }

   if ( SUCCEEDED( hr ) && width && height ) // valid signal
   {
      hr = WriteOverlay(pVision, pOverlay, width, height );
   }

   if ( FAILED( hr ) )
   {
      if ( GDIPlusToken )
      {
         FinalizeGDIPlus( GDIPlusToken );
         GDIPlusToken = 0;
      }
   }

   return hr;
}

/******************************************************************************/

void OverlayEnd( IVisionUserOverlay *pOverlay )
{
   pOverlay->OverlayDisable();

   if ( GDIPlusToken )
   {
      FinalizeGDIPlus( GDIPlusToken );
      GDIPlusToken = 0;
   }
}

/******************************************************************************/

HRESULT OverlayDisable( IVisionUserOverlay *pOverlay )
{
   return pOverlay->OverlayDisable();
}

/******************************************************************************/

HRESULT OverlayKeepAlive( IVisionUserOverlay *pOverlay )
{
   return pOverlay->OverlayKeepAlive();
}

/******************************************************************************/
