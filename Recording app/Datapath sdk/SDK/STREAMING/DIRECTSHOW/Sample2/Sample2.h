/*******************************************************************************

Copyright Datapath Ltd. 2008, 2017.

File:    Sample2.h

Purpose: 

History:
         28 NOV 08    RL   Created.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.
         10 AUG 10    RL   Use IVisionEvent2.
         26 APR 12    JL   Added support for User-Mode filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.
         21 JUL 17    SB   Added IVisionUser2.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Constants ******************************************************************/

#define APPLICATIONNAME TEXT("DirectShow Vision Sample2\0")
#define CLASSNAME       TEXT("Sample2\0")
#define WM_GRAPHNOTIFY    WM_APP+1
#define WM_SIGNAL_CHANGED WM_APP+2

/* Global Variables ***********************************************************/

typedef struct
{
   HWND                    HWndApp;
  
   IVideoWindow            *PVideoWindow;
   IMediaEventEx           *PMediaEvent;
   IMediaControl           *PMediaControl;

   IGraphBuilder           *PGraphBuilder;
   ICaptureGraphBuilder2   *PCapture;
#ifdef DO_GANGING
   IVisionUserGanging      *PVisionGanging;
#endif
#ifdef DO_OVERLAY
   IVisionUserOverlay      *PVisionOverlay;
   IVisionUser2            *PVisionUser2;
#endif
   IVision                 *PVision;
   IVisionEvent2           *PVisionEvent2;

   CVisionCallBack         *PModeChangeCallBack;

   ULONG_PTR               PDeviceEventHandle;

   DWORD                   DWGraphRegister;

   ULONG                   InputWidth;
   ULONG                   InputHeight;

}  GLOBAL;

static GLOBAL Global = {0,};

/******************************************************************************/
#ifdef DO_OVERLAY

const WPARAM        OVERLAY_TIMER   = 0xDADA;
const unsigned long OVERLAY_TIMEOUT = 500; // ms

HRESULT OverlayBegin( IVisionUser2 *pVision,
                      IVisionUserOverlay *pOverlay,
                      unsigned long width,
                      unsigned long height,
                      unsigned long timeout );

void    OverlayEnd( IVisionUserOverlay *pOverlay );

HRESULT WriteOverlay( IVisionUser2 *pVision,
                      IVisionUserOverlay *pOverlay,
                      unsigned long width,
                      unsigned long height );

HRESULT OverlayDisable( IVisionUserOverlay *pOverlay );

HRESULT OverlayKeepAlive( IVisionUserOverlay *pOverlay );

#endif
/******************************************************************************/
