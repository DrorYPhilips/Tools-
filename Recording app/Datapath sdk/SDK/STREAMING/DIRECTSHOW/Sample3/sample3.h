/*******************************************************************************

Copyright Datapath Ltd. 2008, 2010.

File:    Sample3..h

Purpose: 

History:
         01 DEC 08    RL   Created.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Constants ******************************************************************/

#define APPLICATIONNAME TEXT("DirectShow Vision Sample3\0")
#define CLASSNAME       TEXT("Sample3\0")
#define WM_GRAPHNOTIFY  WM_APP+1
#define UPDATE_STATS    0x100

/* Global Variables ***********************************************************/

typedef struct
{
   HWND                    HWndApp;
   HINSTANCE               HInstance;
  
   IVideoWindow            *PVideoWindow;
   IMediaEventEx           *PMediaEvent;
   IMediaControl           *PMediaControl;

   IGraphBuilder           *PGraphBuilder;
   ICaptureGraphBuilder2   *PCapture;

   DWORD                   DWGraphRegister;

}  GLOBAL;

static GLOBAL Global = {0,};

/******************************************************************************/

typedef struct
{
   HWND                    HWndApp;
   HINSTANCE               HInstance;

   IAMDroppedFrames        *PDroppedFrames;
   IQualProp               *PQualProp;

   float                   FrameRate;

}  STATISTICS, *PSTATISTICS;

/******************************************************************************/