/*******************************************************************************

Copyright Datapath Ltd. 2009, 2016.

File:    VisionWin.cpp

Purpose: 

History:
         13 Feb 09    RL   Created.
         06 OCT 09    RL   Removed the requirement for a 'PinID'.
         06 OCT 09    RL   Accomodated functions for VIDEOINFOHEADER2.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>
#include <dvdmedia.h>

#include <DGC133ST.h>

#include <VisionPin.h>

/******************************************************************************/

HRESULT 
SetupVideoWindow(
   IVideoWindow   *pVideoWindow,
   RECT           *pPosition,
   HWND           hWnd)
{
   HRESULT hr;
   RECT    clRect;

   // Note that the video renderer will introduce scaling if not 1:1.
   if (!pVideoWindow)
      return E_POINTER;
   // Set the active video window to be a child of the Global.HWndApp.
   hr = pVideoWindow->put_Owner((OAHWND)hWnd);
   if (FAILED(hr))
      return hr;
   // Set Global.HWndApp 1:1 with consideration for borders.
   SetWindowPos(hWnd, NULL, pPosition->left, pPosition->top, 
         pPosition->right, pPosition->bottom,  SWP_NOZORDER);
   // Set video window inside Global.HWndApp.
   hr = pVideoWindow->put_WindowStyle(WS_CHILD);
   if (FAILED(hr))
      return hr;
   GetClientRect(hWnd, &clRect);
   // Set video window coordinates relative to Global.HWndApp.
   hr = pVideoWindow->SetWindowPosition(clRect.left, clRect.top,
            clRect.right, clRect.bottom);
   if (FAILED(hr))
      return hr;
   // Make the video window visible, now that it is properly positioned.
   hr = pVideoWindow->put_Visible(OATRUE);
   if (FAILED(hr))
      return hr;

   return hr;
}

/******************************************************************************/

HRESULT 
SetVideoWindow(
   HWND           hWnd,
   IBaseFilter    *pSrcFilter,
   IVideoWindow   *pVideoWindow,
   RECT           *pRect,
   LPCTSTR        pPinName)
{
   HRESULT  hr;
   RECT     position;

   // Set up defaults.
   position.top = position.left = 0;
   position.right = 320;
   position.bottom = 240;

   if (pRect)
   {
      memcpy(&position, pRect, sizeof(RECT));
   }
   else
   {     
      if (pSrcFilter)
      {   
         // Set the client area 1:1, no scaling with the capture buffer.
         AM_MEDIA_TYPE *pmt = NULL;
         // Ensure RenderStream or SetFormat is called before GetPinMediaType.
         hr = GetCurrentMediaType(pSrcFilter, &pmt, pPinName);
         if(SUCCEEDED(hr) && pmt )
         {
            POINT   borders;
            RECT    clRect;
            // Get the current Global.HWndApp coordinates.
            GetClientRect(hWnd, &clRect);
            GetWindowRect(hWnd, &position);
            // Calculate Global.HWndApp borders.
            borders.x = (position.right - position.left) - clRect.right;
            borders.y = (position.bottom - position.top) - clRect.bottom;

            if (pmt->formattype == FORMAT_VideoInfo)
            {
               VIDEOINFOHEADER *pVIH;
               pVIH = (VIDEOINFOHEADER*)pmt->pbFormat;

               // Add the buffer width and height for 1:1 default.
               position.right = pVIH->bmiHeader.biWidth + borders.x;
               position.bottom = abs(pVIH->bmiHeader.biHeight) + borders.y;

            }
            else if (pmt->formattype == FORMAT_VideoInfo2)
            {
               VIDEOINFOHEADER2 *pVIH2;
               pVIH2 = (VIDEOINFOHEADER2*)pmt->pbFormat;
               
               // Add the buffer width and height for 1:1 default.
               position.right = pVIH2->bmiHeader.biWidth + borders.x;
               position.bottom = abs(pVIH2->bmiHeader.biHeight) + borders.y;
            }
            // It is our responsibility to free the AM_MEDIA_TYPE memory.
            DeleteMediaType(pmt);
         }
      }
   }

   return SetupVideoWindow(pVideoWindow, &position,hWnd);
}

/******************************************************************************/

HRESULT
SetWindowStyle(
   IVideoWindow   *IVideoWindow,
   BOOL           bTitleMenuBorders)
{
   long style;

   IVideoWindow->get_WindowStyleEx(&style);
   if ( bTitleMenuBorders )
   {
      style &= ~( WS_BORDER | WS_CAPTION | WS_THICKFRAME );
   }
   else
   {
      style |= WS_CAPTION | WS_BORDER | WS_THICKFRAME;
   }
   
   return IVideoWindow->put_WindowStyleEx(style);
}

/******************************************************************************/
