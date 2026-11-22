/*******************************************************************************

Copyright Datapath Ltd. 2009, 2016.

File:    VisionCmd.cpp

Purpose: 

History:
         31 JAN 09    RL   Created.
         06 OCT 09    RL   Removed the requirement for a 'PinID'.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.
                           Simplified setting of colour formats.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>
#include <stddef.h>

#include <cmdline.h>

#include <DGC133ST.h>

#include <VisionCmd.h>
#include <VisionPin.h>
#include <VisionUtil.h>

/******************************************************************************/

struct {
   PTCHAR  String;
   ULONG   Flag;
   LPCGUID PGuid;
}
ColourFormat[] =
{
   { TEXT("rgb32"),  CMD_FLAGS_SUBTYPE_RGB32,  &MEDIASUBTYPE_RGB32 },
   { TEXT("rgb24"),  CMD_FLAGS_SUBTYPE_RGB24,  &MEDIASUBTYPE_RGB24 },
   { TEXT("rgb565"), CMD_FLAGS_SUBTYPE_RGB565, &MEDIASUBTYPE_RGB565 },
   { TEXT("rgb8"),   CMD_FLAGS_SUBTYPE_RGB8,   &MEDIASUBTYPE_RGB8 },
   { TEXT("yuy2"),   CMD_FLAGS_SUBTYPE_YUY2,   &MEDIASUBTYPE_YUY2 },
   { TEXT("uyvy"),   CMD_FLAGS_SUBTYPE_UYVY,   &MEDIASUBTYPE_UYVY },
   { TEXT("yvyu"),   CMD_FLAGS_SUBTYPE_YVYU,   &MEDIASUBTYPE_YVYU },
   { TEXT("nv12"),   CMD_FLAGS_SUBTYPE_NV12,   &MEDIASUBTYPE_NV12 },
   { TEXT("yv12"),   CMD_FLAGS_SUBTYPE_YV12,   &MEDIASUBTYPE_YV12 },
};

/* Command strings */
static const TCHAR
   FPS[] = TEXT("fps"),
   Buffer[] = TEXT("buffer"),
   Window[] = TEXT("window"),
   Subformat[] = TEXT("subformat"),
   Input[] = TEXT("input"),
   Channel[] = TEXT("channel");

/******************************************************************************/

CMDLNERRFN  CommandLineErrorFn;

unsigned long
CommandLineErrorFn (
   unsigned long  error,
   LPCTSTR        pKeyword,
   int            nKeywordChars,
   LPCTSTR        pValue,
   int            nValueChars,
   int            nParameter,
   PKEYWORDTODATA pKeywordToData,
   PVOID          pVoid )
{
   return 1;
}

/******************************************************************************/

static unsigned long
ExtractSubformat (
   LPCTSTR pValue,
   int     nValueChars,
   PVOID   pData,
   PVOID   pVoid )
{
   PCOMMANDLINE pCommandLine = (PCOMMANDLINE)pData;

   for ( int i = 0; i < _countof(ColourFormat); i++ )
   {
      if ( _tcsicmp ( ColourFormat[i].String, pValue ) == 0 )
      {
         pCommandLine->flags |= ColourFormat[i].Flag;
         memcpy ( &pCommandLine->subformat, ColourFormat[i].PGuid, sizeof(GUID) );
         break;
      }
   }

   return 0;
}

/******************************************************************************/

static unsigned long
ExtractBuffer (
   LPCTSTR pValue,
   int     nChars,
   PVOID   pData,
   PVOID   pVoid )
{
   unsigned long  error;
   PCOMMANDLINE   pCommandLine;
   
   pCommandLine = (PCOMMANDLINE)pData;

   error = ExtractTLWH ( pValue, nChars, (PVOID)2, pVoid );
   if ( error == 0 )
   {
      pCommandLine->flags |= CMD_FLAGS_BUFFER_SIZE;
   }

   return 0;
}

/******************************************************************************/

static unsigned long
ExtractWindow (
   LPCTSTR pValue,
   int     nChars,
   PVOID   pData,
   PVOID   pVoid )
{
   unsigned long  error;
   PCOMMANDLINE   pCommandLine;
   
   pCommandLine = (PCOMMANDLINE)pData;

   error = ExtractTLWH ( pValue, nChars, (PVOID)4, pVoid );
   if ( error == 0 )
   {
      pCommandLine->flags |= CMD_FLAGS_WINDOW_POSITION;
   }

   return 0;
}

/******************************************************************************/

static unsigned long
ExtractInput (
   LPCTSTR pValue,
   int     nChars,
   PVOID   pData,
   PVOID   pVoid )
{
   unsigned long  error;
   PCOMMANDLINE   pCommandLine;
   
   pCommandLine = (PCOMMANDLINE)pData;

   error = ExtractLong ( pValue, nChars, NULL, pVoid );
   if ( error == 0 )
   {
      pCommandLine->flags |= CMD_FLAGS_INPUT_SOURCE;
   }

   return 0;
}

/******************************************************************************/

static unsigned long
ExtractChannel (
   LPCTSTR pValue,
   int     nChars,
   PVOID   pData,
   PVOID   pVoid )
{
   unsigned long  error;
   PCOMMANDLINE   pCommandLine;
   
   pCommandLine = (PCOMMANDLINE)pData;
   
   error = ExtractLong ( pValue, nChars, NULL, pVoid );
   if ( error == 0 )
   {
      pCommandLine->flags |= CMD_FLAGS_CHANNEL_SOURCE;
   }
   
   return 0;
}

/******************************************************************************/

static unsigned long
ExtractFPS (
   LPCTSTR pValue,
   int     nChars,
   PVOID   pData,
   PVOID   pVoid )
{
   unsigned long  error;
   PCOMMANDLINE   pCommandLine;
   
   pCommandLine = (PCOMMANDLINE)pData;

   error = ExtractLong ( pValue, nChars, NULL, pVoid );
   if ( error == 0 )
   {
      pCommandLine->flags |= CMD_FLAGS_CAPTURE_RATE;
   }

   return 0;
}

/******************************************************************************/

BOOL
ProcessCommandLine(
   LPCTSTR        lpStrCommandLine,
   PCOMMANDLINE   pCommandLine)
{
   KEYWORDTODATA  keywordToData[] = 
   {
      { Subformat, ExtractSubformat, offsetof ( COMMANDLINE, subformat ), pCommandLine },
      { Buffer, ExtractBuffer, offsetof ( COMMANDLINE, buffer ), pCommandLine },
      { Window, ExtractWindow, offsetof ( COMMANDLINE, window ), pCommandLine },
      { Input, ExtractInput, offsetof ( COMMANDLINE, input ), pCommandLine },
      { Channel, ExtractChannel, offsetof ( COMMANDLINE, channel ), pCommandLine },
      { FPS, ExtractFPS, offsetof ( COMMANDLINE, fps ), pCommandLine },

      { NULL, NULL, 0, NULL },
   };

   pCommandLine->flags =  0;

   ParseCommandLine ( (LPTSTR)lpStrCommandLine, keywordToData, pCommandLine,
         CommandLineErrorFn );

   return TRUE;
}

/******************************************************************************/

BOOL
SetCommandLine(
   HWND           hWnd,
   LPCTSTR        pTitle,
   PCOMMANDLINE   pCommandLine,
   IBaseFilter    *pSrcFilter,
   LPCTSTR        pPinName)
{
   HRESULT hr;
   AM_MEDIA_TYPE *pmt;

   if (!pCommandLine->flags)
      return FALSE;

   if(!pSrcFilter)
   {
      ErrorMessage(hWnd, pTitle, TEXT("Invalid source filter."), E_POINTER);
      return FALSE;
   }
   // Get on the wire media type.
   if ( GetCurrentMediaType(pSrcFilter, &pmt, pPinName) != S_OK )
      return FALSE;

   for ( int i = 0; i < _countof(ColourFormat); i++ )
   {
      if (pCommandLine->flags & ColourFormat[i].Flag)
      {
         hr = SetColourSpace(pSrcFilter, pPinName, ColourFormat[i].PGuid, &pmt);
         if (FAILED(hr))
            ErrorMessage(hWnd, pTitle, TEXT("Failed to set colour space."), hr);
         break;
      }
   }
   // Set the capture buffer size.
   if (pCommandLine->flags & CMD_FLAGS_BUFFER_SIZE)
   {
      hr = SetBufferSize(pCommandLine->buffer.x, pCommandLine->buffer.y, pmt);
      if (FAILED(hr))
         ErrorMessage(hWnd, pTitle, TEXT("Failed to set buffer size."), hr);
   }
   // Set the capture rate in frames per second.
   if (pCommandLine->flags & CMD_FLAGS_CAPTURE_RATE)
   {
      hr = SetFrameRate(pCommandLine->fps, pmt);
      if (FAILED(hr))
         ErrorMessage(hWnd, pTitle, TEXT("Failed to set capture rate."), hr);
   }
   // Set the requested Media Type.
   hr = SetPinMediaType(pSrcFilter, pmt, pPinName);
   // It is our responsibility to free the AM_MEDIA_TYPE memory.
   DeleteMediaType(pmt);
   if (FAILED(hr))
   {
      ErrorMessage(hWnd, pTitle, TEXT("Failed to set requested Media Type."), hr);
      return FALSE;
   }
   else
      return TRUE;
}

/******************************************************************************/