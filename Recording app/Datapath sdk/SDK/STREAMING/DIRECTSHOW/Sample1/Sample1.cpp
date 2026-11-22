/*******************************************************************************

Copyright Datapath Ltd. 2008.

File:    Sample1.cpp

Purpose: 

History:
         01 JAN 08    RL   Created.
         26 APR 12    JL   Added support for User-Mode filters.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>

#include <VisionGet.h>

#include "Sample1.h"

/******************************************************************************/

int APIENTRY 
_tWinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPTSTR    pCmdLine,
   int       nCmdShow)
{
   IBaseFilter *pSrcFilter = NULL;
   // Initialize COM.
   CoInitialize(NULL);
   // Find the requested Vision filter index.
   if (SUCCEEDED(GetVisionSourceFilter(0, &pSrcFilter)))
   {
      IGraphBuilder *pGraphBuilder;
      IMediaControl *pMediaControl;
      ICaptureGraphBuilder2 *pCaptureGraphBuilder2;
      // Create FilterGraph
      CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, 
            (LPVOID *)&pGraphBuilder);
      // Get MediaControl Interface
      pGraphBuilder->QueryInterface(IID_IMediaControl, (LPVOID *)&pMediaControl);
      // Prepare media file using source filter
      pGraphBuilder->AddFilter(pSrcFilter, TEXT("Video Capture"));
      // Create GraphBuilder, CaptureGraphBuilder2
      CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
      IID_ICaptureGraphBuilder2, (LPVOID *)&pCaptureGraphBuilder2);
      // Set FilterGraph for CaptureGraphBuilder2
      pCaptureGraphBuilder2->SetFiltergraph(pGraphBuilder);
      // Build Graph
      pCaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, 
            &MEDIATYPE_Video, pSrcFilter, NULL, NULL);
      // Start playing
      pMediaControl->Run();
      // This is to block while playing
      MessageBox(NULL,	TEXT("Stop Capture"),APPLICATIONNAME,	MB_OK);
      // Release resources.
      SAFE_RELEASE(pMediaControl);
      SAFE_RELEASE(pCaptureGraphBuilder2);
      SAFE_RELEASE(pGraphBuilder);
      SAFE_RELEASE(pSrcFilter);
   }

   // Finalize COM
   CoUninitialize();

   return 0;
}

/******************************************************************************/