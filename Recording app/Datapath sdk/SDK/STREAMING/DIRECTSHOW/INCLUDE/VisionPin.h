/*******************************************************************************

Copyright Datapath Ltd. 2009.

File:    VisionPin.h

Purpose: 

History:
         31 JAN 09    RL   Created.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Declarations ***************************************************************/

HRESULT
GetCurrentMediaType(
   IBaseFilter    *pSrcFilter,
   AM_MEDIA_TYPE  **ppmt,
   LPCTSTR        pPinName);
   
HRESULT
SetPinMediaType(
   IBaseFilter    *pSrcFilter,
   AM_MEDIA_TYPE  *pmt,
   LPCTSTR        pPinName);

HRESULT
SetBufferSize( 
   LONG          width,
   LONG          height,
   AM_MEDIA_TYPE  *pAMT);

HRESULT
SetFrameRate(
   ULONG          frameRate,
   AM_MEDIA_TYPE  *pAMT);

HRESULT
SetColourSpace(
   IBaseFilter    *pSrcFilter,
   LPCTSTR        pPinName,
   const GUID     *pColourSpace,
   AM_MEDIA_TYPE  **ppAMT);

HRESULT
PinPropertiesDlg(
   HWND        hWnd,
   IBaseFilter *pSrcFilter,
   LPCTSTR     pPinName);

HRESULT
SetVideo2MediaType(
   IBaseFilter    *pSrcFilter,
   unsigned long  width,
   unsigned long  height,
   unsigned long  fps,
   GUID           *pColourSpace,
   LPCTSTR        pPinName);

HRESULT
GetSourcePinInterface(
   IBaseFilter    *pSrcFilter,
   IPin           **ppPin,
   LPCTSTR        pPinName);

HRESULT
LoadDroppedFramesInterface(
   IBaseFilter          *pSrcFilter,
   LPCTSTR              pPinName,
   IAMDroppedFrames     **ppIAMDF);

HRESULT
SetLiveStreamPin(
   IBaseFilter          *pSrcFilter,
   LPCTSTR              pPinName,
   LIVESTREAM_CLIENT    LiveStreamClient);

HRESULT
GetLiveStreamPin(
   IBaseFilter          *pSrcFilter,
   LPCTSTR              pPinName,
   LIVESTREAM_CLIENT    *pLiveStreamClient);

/******************************************************************************/
