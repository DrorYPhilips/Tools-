/*******************************************************************************

Copyright Datapath Ltd. 2008.

File:    SAMPLE3.C

Purpose: VisionRGB-PRO and VisionRGB-X example program that shows how to display
         captured RGB data within a control on a dialog.

History:
         16 JAN 08    SB   Created.

*******************************************************************************/

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>

#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

#include "resource.h"

/* Static Constants ***********************************************************/

static const TCHAR
   Caption[]   = { TEXT("RGB Sample 4") };

/* Global Variables ***********************************************************/

static HINSTANCE  gHInstance = NULL;
static HRGB       hRGB[2] = { 0, 0 };

/******************************************************************************/

INT_PTR CALLBACK
DialogProc (
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam )
{
   BOOL        bReturn = FALSE;

   switch ( message )
   {
      case WM_COMMAND:
      {
         switch ( LOWORD ( wParam ) )
         {
            case IDCLOSE:
            {
               if ( IsWindowEnabled ( GetDlgItem ( hDlg, IDC_CAPTURE1 )))
               {
                  RGBSetWindow ( hRGB[0], NULL );
               }
               if ( hRGB[1] &&
                    IsWindowEnabled ( GetDlgItem ( hDlg, IDC_CAPTURE2 )))
               {
                  RGBSetWindow ( hRGB[1], NULL );
               }
               EndDialog ( hDlg, LOWORD ( wParam ));
               break;
            }
         }
         break;
      }

      case WM_SYSCOMMAND:
      {
         switch ( wParam )
         {
            case SC_CLOSE:
            {
               if ( IsWindowEnabled ( GetDlgItem ( hDlg, IDC_CAPTURE1 )))
               {
                  RGBSetWindow ( hRGB[0], NULL );
               }
               if ( hRGB[1] &&
                    IsWindowEnabled ( GetDlgItem ( hDlg, IDC_CAPTURE2 )))
               {
                  RGBSetWindow ( hRGB[1], NULL );
               }
               EndDialog ( hDlg, FALSE );
               break;
            }
            break;
         }
         break;
      }


      case WM_INITDIALOG:
      {
         if ( RGBSetWindow ( hRGB[0], GetDlgItem ( hDlg, IDC_CAPTURE1 )))
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_CAPTURE1 ), FALSE );
         }
         if (( hRGB[1] == 0 ) || 
             ( RGBSetWindow ( hRGB[1], GetDlgItem ( hDlg, IDC_CAPTURE2 ))))
         {
            EnableWindow ( GetDlgItem ( hDlg, IDC_CAPTURE2 ), FALSE );
         }
         bReturn = TRUE;
         break;
      }
   }
   return bReturn;
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
   HRGBDLL        hRGBDLL = 0;

   /* Load the RGBEASY API. */
   error = RGBLoad ( &hRGBDLL );
   if ( error == 0 )
   {
      /* Open the first input. */
      error = RGBOpenInput ( 0, &hRGB[0] );
      if ( error == 0 )
      {
         RGBOpenInput ( 1, &hRGB[1] );
         DialogBoxParam ( hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL,
               DialogProc, ( LPARAM )hRGB );
         if ( hRGB[1] )
         {
            RGBCloseInput ( hRGB[1] );
         }
         RGBCloseInput ( hRGB[0] );
      }
      RGBFree ( hRGBDLL );
   }

   return error;

   UNREFERENCED_PARAMETER ( nShowCmd );
   UNREFERENCED_PARAMETER ( lpCmdLine );
   UNREFERENCED_PARAMETER ( hPrevInstance );
}

/******************************************************************************/
