/*******************************************************************************

Copyright Datapath Ltd. 2008, 2010.

File:    Sample5.h

Purpose: 

History:
         01 DEC 08    RL   Created.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Constants ******************************************************************/

#define APPLICATIONNAME TEXT("DirectShow Vision Sample5\0")
#define CLASSNAME       TEXT("Sample5\0")
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

   TCHAR                   OutputFileName[MAX_PATH];
   GUID                    *PFileFormat;

}  GLOBAL;

static GLOBAL Global = {0,};

/******************************************************************************/