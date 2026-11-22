/*******************************************************************************

Copyright Datapath Ltd. 2009, 2010.

File:    Sample6.h

Purpose: 

History:
         06 OCT 09    RL   Created.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.
         26 APR 12    JL   Added support for User-Mode filters.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Constants ******************************************************************/

#define APPLICATIONNAME TEXT("DirectShow Vision Sample6\0")
#define CLASSNAME       TEXT("Sample6\0")
#define WM_GRAPHNOTIFY  WM_APP+1

/* Global Variables ***********************************************************/

typedef struct
{
   HWND                    HWndApp;
  
   IVideoWindow            *PVideoWindow;
   IMediaEventEx           *PMediaEvent;
   IMediaControl           *PMediaControl;

   IGraphBuilder           *PGraphBuilder;
   ICaptureGraphBuilder2   *PCapture;

   IVision                 *PVision;
   IAMCrossbar             *PCrossbar;

   DWORD                   DWGraphRegister;

}  GLOBAL;

static GLOBAL Global = {0,};

/******************************************************************************/