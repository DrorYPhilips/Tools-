/*******************************************************************************

Copyright Datapath Ltd. 2012.

File:    SampleAV.h

Purpose: 

History:
         23 NOV 12    RL   Created.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Constants ******************************************************************/

#define APPLICATIONNAME TEXT("DirectShow Vision SampleAV\0")
#define CLASSNAME       TEXT("SampleAV\0")
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

   DWORD                   DWGraphRegister;

}  GLOBAL;

static GLOBAL Global = {0,};

/******************************************************************************/