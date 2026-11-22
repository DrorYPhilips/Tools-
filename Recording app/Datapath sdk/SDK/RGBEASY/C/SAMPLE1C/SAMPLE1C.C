/*******************************************************************************

Copyright Datapath Ltd. 2008.

File:    SAMPLE1C.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to display
         an RGB window containing owner drawn OSD messages on suitable graphics
         devices. Only graphics devices supporting the accelerated OSD interface
         are supported.

History:
         06 FEB 12     MJE   Created.

*******************************************************************************/

#define STRSAFE_NO_DEPRECATE

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include <api.h>

#include <rgb.h>
#include <rgberror.h>

#include "resource.h"

/* Static Constants ***********************************************************/

static const TCHAR
   RGBWindowClass[]  = { TEXT("RGBSampleWndClass") },
   Property[]        = { TEXT("RGB") },
   Caption[]         = { TEXT("RGB Sample 1C") };

static const TCHAR
   _RGBDllName[] = { TEXT("rgbeasy.dll") };


static const TCHAR   _TimeFormat[] = { TEXT ( "hh:mm:ss:ff" ) };
static const TCHAR   _DrawProp[] = { TEXT ( "_DrawProp_" ) };

static const COLORREF   TextColour = RGB(255, 0, 0);
static const COLORREF   Background = RGB(255, 0, 255);

/******************************************************************************/

typedef struct _DRAWDATA
{
   HRGBOSD  HOSD;
   POINT    Margin;
   SIZE     OSDSize;
}  DRAWDATA, *PDRAWDATA;

/* This is the callback for drawing the OSD. */
RGBOSDDRAWFN DrawOSD;

/* Global Variables ***********************************************************/

static HWND          gHWnd = NULL;
static HINSTANCE     gHInstance = NULL;

/******************************************************************************/

/* Define pointers to functions in the RGB DLL. */
#define API(type,modifier,name,args) \
   type (modifier *name) args = NULL;
#include <rgbapi.h>
#undef API

/******************************************************************************/

LRESULT CALLBACK
RGBWndProc (
   HWND     hWnd,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam )
{
   switch ( message )
   {
      case WM_COMMAND:
      {
         switch ( LOWORD ( wParam ) )
         {
            case IDM_EXIT:
            {
               DestroyWindow ( hWnd );
               break;
            }
         }
         break;
      }

      case WM_TIMER:
      {
         PDRAWDATA   pDrawData;

         pDrawData = ( PDRAWDATA ) GetProp ( hWnd, _DrawProp );

         if ( pDrawData )
         {
            HDC   hDC;

            hDC = GetDC ( hWnd );
            
            if ( hDC )
            {
               /* We just reuse the callback to draw the OSD. */
               DrawOSD ( hWnd, pDrawData->HOSD, hDC, 
                     (ULONG_PTR) pDrawData );
               ReleaseDC ( hWnd, hDC );
            }
         }
         break;
      }

      case WM_CREATE:
      {
         LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT) lParam;

         SetProp ( hWnd, Property, pCreateStruct->lpCreateParams );
         break;
      }

      case WM_CLOSE:
      {
         DestroyWindow ( hWnd );
         break;
      }

      case WM_DESTROY:
      {
         HRGB hRGB = (HRGB)RemoveProp ( hWnd, Property );

         if ( hRGB )
         {
            unsigned long error;

            /* Explicitly clean up the on screen display objects. */
            do
            {
               HRGBOSD hOSD;

               error = RGBGetFirstOSD ( hRGB, &hOSD );
               if ( error == 0 )
               {
                  RGBDetachOSD ( hRGB, hOSD );
                  RGBDeleteOSD ( hOSD );
               }
            }  while ( error == 0 );

            RGBSetWindow ( hRGB, NULL );
            RGBCloseInput ( hRGB );
         }
         PostQuitMessage ( 1 );
         break;
      }

      default:
      {
         return DefWindowProc ( hWnd, message, wParam, lParam );
      }
   }

   return 0;
}

/******************************************************************************/

unsigned long
RegisterRGBWindowClass (
   HINSTANCE   hInstance )
{
   WNDCLASS wc;

   wc.style = 0;
   wc.lpfnWndProc = (WNDPROC)RGBWndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = LoadIcon ( hInstance, MAKEINTRESOURCE ( IDI_ICON ));
   wc.hCursor = LoadCursor ( NULL, IDC_ARROW );
   wc.hbrBackground = NULL;
   wc.lpszMenuName =  MAKEINTRESOURCE ( IDR_MENU );
   wc.lpszClassName = RGBWindowClass;

   if ( RegisterClass ( &wc ) == 0 )
   {
      return GetLastError ();
   }
   else
   {
      return 0;
   }
}

/******************************************************************************/

void RGBCBKAPI
DrawOSD (
   HWND           hWnd,
   HRGB           hOSD,
   HDC            hDC,
   ULONG_PTR      userData )
{
   PDRAWDATA   pDrawData = (PDRAWDATA) userData;
   ULONGLONG   dwTickCount;
   TCHAR       timeString[128];
   HDC         hDCCompat;

   dwTickCount = GetTickCount ();

#if (WINVER >= 0x0600)
   /* Format the duration string. */
   GetDurationFormatEx ( LOCALE_NAME_USER_DEFAULT, 0, NULL, 
         dwTickCount * 10000, _TimeFormat, timeString, 
         sizeof ( timeString ) / sizeof ( timeString[0] ) );
#else
   /* GetDurationFormatEx is only available in Vista onwards. Since this
    * is only a sample we'll just print out the tick count when it's not
    * available. */
   StringCchPrintf ( timeString, ( timeString, sizeof ( timeString ) / sizeof ( timeString[0] ),
          TEXT ( "%dms" ), dwTickCount );
#endif

   /* Find out how big the OSD is. */
   hDCCompat = CreateCompatibleDC ( hDC );
   if ( hDCCompat )
   {
      RECT  osdRect = { 0, };
      int   strLength;

      strLength = (int)_tcslen (  timeString );

      if ( DrawText ( hDCCompat, timeString, strLength, &osdRect, 
            DT_LEFT | DT_SINGLELINE | DT_CALCRECT ) )
      {
         if (( osdRect.right != pDrawData->OSDSize.cx ) ||
             ( osdRect.bottom != pDrawData->OSDSize.cy ))
         {
            unsigned long error;

            /* The size of the text string has changed so we update 
             * the OSD location. */
            error = RGBSetOSDArea ( hOSD, pDrawData->Margin.y, 
                  pDrawData->Margin.x, osdRect.right, osdRect.bottom );
            if ( error == 0 )
            {
               pDrawData->OSDSize.cx = osdRect.right;
               pDrawData->OSDSize.cy = osdRect.bottom;
            }
         }

         /* Adjust for the margins. */
         osdRect.top = pDrawData->Margin.y;
         osdRect.left = pDrawData->Margin.x;
         osdRect.bottom += pDrawData->Margin.y;
         osdRect.right += pDrawData->Margin.x;;

         SetTextColor ( hDC, TextColour );
         SetBkColor ( hDC, Background );

         /* Finally draw the OSD. */
         DrawText ( hDC, timeString, strLength, &osdRect, 
               DT_LEFT | DT_SINGLELINE );
      }

      DeleteDC ( hDCCompat );
   }
   return;
}

/******************************************************************************/

unsigned long
SetupOSD ( 
   HWND        hWnd,
   HRGB        hRGB,
   PDRAWDATA   pDrawData )
{
   unsigned long  error;
   BOOL           bAccelerated;
   
   error = RGBIsOSDAccelerated ( &bAccelerated );

   if ( ( error == 0 ) && ( bAccelerated ) )
   {
      HRGBOSD  hOSD;

      error = RGBCreateOSD ( &hOSD );
      if ( error == 0 )
      {
         error = RGBSetOSDOwnerDrawnFn ( hOSD, DrawOSD, 
               (ULONG_PTR) pDrawData );
         if ( error == 0 )
         {
            error = RGBSetOSDType ( hOSD, RGBOSD_TYPE_OWNERDRAWN );

            if ( error == 0 )
            {
               /* We'll always draw the text so the background
                * is transparent. */
               RGBSetKeyColour (hRGB, Background);

               pDrawData->HOSD = hOSD;

               error = RGBAttachOSD ( hRGB, hOSD );
            }
         }
      }
   }

   return error;
}

/******************************************************************************/

unsigned long
CreateRGBWindow (
   unsigned long  input,
   HRGB           *pHRGB )
{
   unsigned long  error;
   HRGB           hRGB;

   error = RGBOpenInput ( input, &hRGB );
   if ( error == 0 )
   {
      /* Create the window. */
      gHWnd = CreateWindow ( RGBWindowClass, Caption,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
            (HWND)NULL, NULL, gHInstance, (LPVOID)hRGB );
      if ( gHWnd )
      {
         RGBSetWindow ( hRGB, gHWnd );
         ShowWindow ( gHWnd, SW_SHOWNORMAL );

         /* Maximise the capture rate. */
         RGBSetFrameDropping ( hRGB, 0 );

         *pHRGB = hRGB;
         return 0;
      }
      else
      {
         error = GetLastError ( );
      }
      RGBCloseInput ( hRGB );
   }

   return error;
}

/******************************************************************************/

unsigned long
LoadRGBDLL (
   HINSTANCE   *pHInstance )
{
   /* List of function names used to load the RGB DLL. */
   static APIFNENTRY fnList[] =
   {
      #define API(type,modifier,name,args) \
         { (FARPROC *)&name, #name },
      #include <rgbapi.h>
      #undef API
      { NULL, NULL }
   };
   HINSTANCE      hInstance;
   unsigned long  error;

   /* Load the interface DLL. */
   error = APILoadLibrary ( _RGBDllName, &hInstance );
   if ( error == 0 )
   {
      /* Find each of the functions exported by the interface. */
      if ( APILoadFunctions ( hInstance, fnList, NULL ))
      {
         *pHInstance = hInstance;
      }
      else
      {
         APIFreeLibrary ( hInstance );
         error = API_ERROR_INCOMPATIBLE_API;
      }
   }

   return error;
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
   MSG            msg;
   HINSTANCE      hDLLInstance;
   DRAWDATA       drawData;
   HRGBDLL        hRGBDLL = 0;
   HRGB           hRGB;

   gHInstance = hInstance;

   /* Register the RGBWindowClass. */
   error = RegisterRGBWindowClass ( gHInstance );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error Registering the window class: 0x%08x"), error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
   }

   error = LoadRGBDLL( &hDLLInstance );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Unable to load %s."), _RGBDllName );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
   }
   
   error = RGBLoad ( &hRGBDLL );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error returned from RGBLoad: 0x%08x"), error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
   }

   /* Create the application window on input 0. */
   error = CreateRGBWindow ( 0, &hRGB );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error creating the application window: 0x%08x"),
            error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
   }


   drawData.Margin.x = 
         drawData.Margin.y = 10;

   drawData.OSDSize.cx = 
         drawData.OSDSize.cy = 0;         /* This is calculated in the callback. */

   error = SetupOSD ( gHWnd, hRGB, &drawData );

   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error creating the OSD: 0x%08x"),
            error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
   }
   else
   {
      /* We save the draw data away as a property so the timer
       * can access it. */
      SetProp (gHWnd, _DrawProp, ( HANDLE ) &drawData );

      /* Now create a timer to update the OSD on a 100ms interval. */
      SetTimer ( gHWnd, 0, 100, NULL );
   }

   /* Sit in the message loop until we are stopped. */
   while ( GetMessage ( &msg, NULL, 0,0 ))
   {
      TranslateMessage ( &msg );
      DispatchMessage ( &msg );
   }

   if ( hRGBDLL )
   {
      error = RGBFree ( hRGBDLL );
   }

   if ( hDLLInstance )
   {
      error = APIFreeLibrary ( hDLLInstance );
   }
   

   return error;

   UNREFERENCED_PARAMETER ( nShowCmd );
   UNREFERENCED_PARAMETER ( lpCmdLine );
   UNREFERENCED_PARAMETER ( hPrevInstance );
}

/******************************************************************************/
