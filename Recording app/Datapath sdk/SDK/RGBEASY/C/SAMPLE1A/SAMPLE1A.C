/*******************************************************************************

Copyright Datapath Ltd. 2008.

File:    SAMPLE1A.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to display
         an RGB window.

History:
         16 JAN 08    SB   Created.

*******************************************************************************/

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
   Caption[]         = { TEXT("RGB Sample 1A") };

static const TCHAR
   _RGBDllName[] = { TEXT("rgbeasy.dll") };

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

unsigned long
CreateRGBWindow (
   unsigned long  input )
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

   HRGBDLL        hRGBDLL = 0;

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

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Unable to load %s. 0x%08x"), _RGBDllName, error );
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
   error = CreateRGBWindow ( 0 );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error creating the application window: 0x%08x"),
            error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
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
