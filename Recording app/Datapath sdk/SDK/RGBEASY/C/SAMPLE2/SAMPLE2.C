/*******************************************************************************

Copyright Datapath Ltd. 2008.

File:    SAMPLE2.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to display
         an RGB window and change its capture settings using a dialog.

History:
         16 JAN 08    SB   Created.
         23 NOV 12    RL   Added support for RGBEasy audio using Windows WASAPI.

*******************************************************************************/

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <commctrl.h>

#include <audio.h>
#include <audioAPI.h>
#include <audiocap.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

#include "sample2.h"
#include "resource.h"

/******************************************************************************/

DWORD WINAPI
CreatePropertiesDialog (
   LPVOID   lpParam )
{
   unsigned long     error = 0;
   PWINDOW           pWindow = (PWINDOW)lpParam;
   int               i;
   PROPSHEETPAGE     psp[2];
   PROPSHEETHEADER   psh;

   /* The pages... */   
   i = 0;

   memset ( &psp[i], 0, sizeof ( PROPSHEETPAGE ));
   psp[i].dwSize = sizeof ( PROPSHEETPAGE );
   psp[i].dwFlags = PSP_DEFAULT;
   psp[i].hInstance = pWindow->HInstance;
   psp[i].pszTemplate = (LPTSTR) IDD_DISPLAY;
   psp[i].pszIcon = NULL;
   psp[i].pfnDlgProc = InputSettingsDlgProc;
   psp[i].pszTitle = NULL;
   psp[i].lParam = (LPARAM)pWindow;
   i++;

   memset ( &psp[i], 0, sizeof ( PROPSHEETPAGE ));
   psp[i].dwSize = sizeof ( PROPSHEETPAGE );
   psp[i].dwFlags = PSP_DEFAULT;
   psp[i].hInstance = pWindow->HInstance;
   psp[i].pszTemplate = (LPTSTR) IDD_AUDIO;
   psp[i].pszIcon = NULL;
   psp[i].pfnDlgProc = AudioSettingsDlgProc;
   psp[i].pszTitle = NULL;
   psp[i].lParam = (LPARAM)pWindow;
   i++;

   memset ( &psh, 0, sizeof ( PROPSHEETHEADER ));
   psh.dwSize = sizeof ( PROPSHEETHEADER );
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP;
   psh.hwndParent = pWindow->HWndParent;
   psh.hInstance = pWindow->HInstance;
   psh.pszIcon = NULL;
   psh.pszCaption = TEXT("AV Settings");
   
   psh.nPages = i;
   psh.nStartPage = 0;
   psh.ppsp = psp;
   psh.pszbmWatermark = NULL;
   
   if ( PropertySheet ( &psh ) == -1 )
   {
      error = GetLastError ( );
   }

   return error;
}

/******************************************************************************/

unsigned long
DoProperties (
   PWINDOW pWindow )
{
   unsigned long  error = 0;
   HANDLE         thread;

   pWindow = (PWINDOW)malloc ( sizeof (WINDOW ) );
   if ( pWindow )
   {
      pWindow->BProgram = FALSE;
      pWindow->HCurrentMode = TRUE;

      if ( error == RGBERROR_NO_ERROR )
      {
         thread = CreateThread ( NULL, 0, CreatePropertiesDialog, 
               pWindow, 0, NULL );
         if ( thread )
         {
            CloseHandle ( thread );
            return 0;
         }
      }
      free ( pWindow );
   }

   return GetLastError();;
}

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
            case IDM_INPUTSETTINGS:
            {
               PWINDOW pWindow = (PWINDOW)GetProp ( hWnd, Property );

               if ( pWindow )
               {
                  HANDLE thread = CreateThread ( NULL, 0, 
                        CreatePropertiesDialog, pWindow, 0, NULL );
                  if ( thread )
                     CloseHandle ( thread );
               }
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
         PWINDOW pWindow = (PWINDOW)RemoveProp ( hWnd, Property );

         if ( pWindow )
            RGBCloseInput ( pWindow->HRGB );

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
   WINDOW *pWindow )
{
   unsigned long  error;

   error = RGBOpenInput ( pWindow->Input, &pWindow->HRGB );
   if ( error == 0 )
   {
      /* Create the window. */
      pWindow->HWndParent = CreateWindow ( RGBWindowClass, Caption,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
            (HWND)NULL, NULL, pWindow->HInstance, (LPVOID)pWindow );
      if ( pWindow->HWndParent )
      {
         RGBSetWindow ( pWindow->HRGB,  pWindow->HWndParent );
         ShowWindow ( pWindow->HWndParent, SW_SHOWNORMAL );

         /* Maximise the capture rate. */
         RGBSetFrameDropping ( pWindow->HRGB, 0 );
         return 0;
      }
      else
      {
         error = GetLastError ( );
      }
      RGBCloseInput ( pWindow->HRGB );
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
   signed long    audioSupported;
   WINDOW         window;

   memset ( &window, 0, sizeof(WINDOW) );
   window.HInstance = hInstance;
   window.Input = 1-1;

   CoInitializeEx ( NULL, COINIT_MULTITHREADED );

   /* Register the RGBWindowClass. */
   error = RegisterRGBWindowClass ( window.HInstance );
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

   /* Create the application window on input. */
   error = CreateRGBWindow ( &window );
   if ( error )
   {
      TCHAR buffer[MAX_PATH];

      StringCchPrintf ( buffer, MAX_PATH, TEXT("Error creating the application window: 0x%08x"),
            error );
      MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      return error;
   }

   /* Does the input support audio capture. */
   error = RGBAudioIsAudioSupported ( window.Input, &audioSupported );
   if ( ( error == 0 ) && audioSupported )
   {
      HRESULT hr;

      hr = OpenSharedAudio ( window.Input, &window.HAudioInput );
      if ( FAILED(hr) )
      {
         TCHAR buffer[MAX_PATH];

         StringCchPrintf ( buffer, MAX_PATH, TEXT("Error opening audio input: 0x%08x"), hr );
         MessageBox ( NULL, buffer, Caption, MB_OK | MB_ICONERROR );
      }
   }

   /* Sit in the message loop until we are stopped. */
   while ( GetMessage ( &msg, NULL, 0,0 ))
   {
      TranslateMessage ( &msg );
      DispatchMessage ( &msg );
   }

   if ( window.HAudioInput )
      CloseSharedAudio ( window.HAudioInput );

   if ( hRGBDLL )
   {
      error = RGBFree ( hRGBDLL );
   }

   CoUninitialize ();

   return error;

   UNREFERENCED_PARAMETER ( nShowCmd );
   UNREFERENCED_PARAMETER ( lpCmdLine );
   UNREFERENCED_PARAMETER ( hPrevInstance );
}

/******************************************************************************/
