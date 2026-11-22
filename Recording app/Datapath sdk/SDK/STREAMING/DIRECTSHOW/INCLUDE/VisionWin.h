/*******************************************************************************

Copyright Datapath Ltd. 2009.

File:    VisionWin.h

Purpose: 

History:
         13 Feb 09    RL   Created.

*******************************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Declarations ***************************************************************/

HRESULT 
SetVideoWindow(
   HWND           hWnd,
   IBaseFilter    *pSrcFilter,
   IVideoWindow   *pVideoWindow,
   RECT           *pRect,
   LPCTSTR        pPinName);

HRESULT
SetWindowStyle(
   IVideoWindow   *IVideoWindow,
   BOOL           bTitleMenuBorders);

/******************************************************************************/