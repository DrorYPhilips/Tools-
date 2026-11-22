/*******************************************************************************

Copyright Datapath Ltd. 2008, 2016.

File:    SAMPLE1.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to display
         an RGB window.

History:
         16 JAN 08    SB   Created.
         14 APR 11    RL   Added a call to RGBGetInputInfo.
         22 JUN 16    JL   Demonstrate use of ganging.

*******************************************************************************/

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

#include "resource.h"

/* Static Constants ***********************************************************/

static const TCHAR
   RGBWindowClass[]  = { TEXT("RGBSampleWndClass") },
   Property[]        = { TEXT("RGB") },
   Caption[]         = { TEXT("RGB Sample 1") };

/* Global Variables ***********************************************************/

static HWND          gHWnd = NULL;
static HINSTANCE     gHInstance = NULL;

/******************************************************************************/
#define DO_GANGING

#ifdef DO_GANGING

unsigned long
DoGanging( unsigned long input )
{
   unsigned long error;
   long bSupported;

   error = RGBInputIsGangingSupported( input, &bSupported );

   if ( (error == 0) && bSupported )
   {
      // attempt to support a 4-input card, and a 2-input card
      RGBGANG_TYPE type[] = { RGBGANG_TYPE_2x2, RGBGANG_TYPE_2x1 };
      long i;

      for ( i = 0; i < _countof( type ); i++ )
      {
         error = RGBInputIsGangingTypeSupported( input, type[i], &bSupported );

         if ( (error == 0) && bSupported )
         {
            break;
         }
      }

      if ( error == 0 )
      {
         RGBGANG_TYPE current;

         error = RGBInputGetGangingType( input, &current );

         if ( (error == 0) && (current != type[i]) )
         {
            error = RGBInputSetGangingType( input, type[i] );
         }
      }
   }

   return error;
}

#endif
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
#ifdef DO_GANGING
         DoGanging( input );
#endif
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

int APIENTRY
_tWinMain (
   HINSTANCE   hInstance,
   HINSTANCE   hPrevInstance,
   LPTSTR      lpCmdLine,
   int         nShowCmd )
{
   unsigned long  error;
   MSG            msg;
   HRGBDLL        hRGBDLL = 0;
   unsigned long  uInput = 0;
   RGBINPUTINFO   InputInfo;

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

   error = RGBLoad ( &hRGBDLL );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error returned from RGBLoad: 0x%08x"), error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
   }

   /* Create the application window on input 0. */
   error = CreateRGBWindow ( uInput );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error creating the application window: 0x%08x"),
            error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
   }

   InputInfo.Size = sizeof ( InputInfo );
   error = RGBGetInputInfo ( uInput, &InputInfo );
   if ( error == 0 )
   {
      /* Inspect information data for input. 
   
         Driver
            Major version number.
            Minor version number.
            Micro version number.
            Version revision number.

         Location
            Information about the device's location on the bus; 
            the interpretation of this information is bus-specific.
         
         FirmWare
            Firmware version number.

         VHDL
            Hardware VHDL version number.

         Identifier
            Unique hardware serial number.

         Chassis
            Chassis index and expansion slot number containing the card.
            Index 0 represents the host system, index 1+ for each chassis.
       */
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

   return error;

   UNREFERENCED_PARAMETER ( nShowCmd );
   UNREFERENCED_PARAMETER ( lpCmdLine );
   UNREFERENCED_PARAMETER ( hPrevInstance );
}

/******************************************************************************/
