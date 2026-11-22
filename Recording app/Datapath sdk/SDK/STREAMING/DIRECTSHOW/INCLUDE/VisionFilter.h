/*******************************************************************************

Copyright Datapath Ltd. 2009, 2017.

File:    VisionFilter.h

Purpose: 

History:
         31 JAN 09    RL   Created.
         26 APR 12    JL   Added support for User-Mode filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.
         21 JUL 17    SB   Added IVisionUser2.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Declarations ***************************************************************/

HRESULT
LoadVisionInterface(
   IBaseFilter    *pSrcFilter,
   IVisionUser    **ppVisionUser);

HRESULT
LoadVisionInterface(
   IBaseFilter    *pSrcFilter,
   IVisionUser2    **ppVisionUser2);

HRESULT
LoadVisionGangingInterface(
   IBaseFilter    *pSrcFilter,
   IVisionUserGanging   **ppVisionGanging );

HRESULT
LoadVisionOverlayInterface(
   IBaseFilter    *pSrcFilter,
   IVisionUserOverlay   **ppVisionOverlay );

// Unsupported, do not use. Returns E_NOTIMPL.
HRESULT
LoadControlConfigInterface (
   void  *pSrcFilter,
   void  **ppControlConfig);

// Unsupported, do not use. Returns E_NOTIMPL.
HRESULT
LoadCrossbarInterface(
   void*  pCapGraph2,
   void*  pSrcFilter,
   void** ppCrossbar);

HRESULT
LoadVisionEventInterface(
   IBaseFilter*       pSrcFilter,
   IVisionUserEvent** ppVisionUserEvent);

HRESULT
LoadVisionInterface(
   IBaseFilter    *pSrcFilter,
   IVision        **ppVision);

HRESULT
LoadControlConfigInterface(
   IBaseFilter           *pSrcFilter,
   IDGC103ControlConfig  **ppControlConfig);

HRESULT
LoadCrossbarInterface(
   ICaptureGraphBuilder2*  pCapGraph2,
   IBaseFilter*            pSrcFilter,
   IAMCrossbar**           ppCrossbar);

HRESULT
LoadVisionEventInterface(
   IBaseFilter*   pSrcFilter,
   IVisionEvent** ppVisionEvent);

HRESULT
LoadVisionEventInterface(
   IBaseFilter*    pSrcFilter,
   IVisionEvent2** ppVisionEvent);

#define LoadVisionEventInterface2 LoadVisionEventInterface

HRESULT
FilterPropertiesDlg(
   HWND        hWnd,
   IBaseFilter *pSrcFilter);

/******************************************************************************/