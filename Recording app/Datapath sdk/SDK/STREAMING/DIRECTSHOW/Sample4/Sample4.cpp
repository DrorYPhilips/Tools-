/*******************************************************************************

Copyright Datapath Ltd. 2008, 2016.

File:    Sample4.cpp

Purpose: 

History:
         28 DEC 08    RL   Created.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.
         16 NOV 10    RL   Fixed bug exiting the message loop.
         26 APR 12    JL   Added support for User-Mode filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>

#include <DGC133ST.h>

#include <VisionCmd.h>
#include <VisionGet.h>
#include <VisionPin.h>
#include <VisionUtil.h>
#include <VisionWin.h>

#include "Sample4.h"

/******************************************************************************/

HRESULT 
OpenInterfaces()
{
   HRESULT hr;

   // Start and stop the capture.
   hr = Global.PGraphBuilder->QueryInterface(IID_IMediaControl,(LPVOID *) 
         &Global.PMediaControl);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Failed to create IMediaControl."), hr);
      return hr;
   }
   // Control the capture within the registered window.
   hr = Global.PGraphBuilder->QueryInterface(IID_IVideoWindow, (LPVOID *) 
         &Global.PVideoWindow);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Failed to create IVideoWindow."), hr);
      return hr;
   }
   // Handle user defined capture events
   hr = Global.PGraphBuilder->QueryInterface(IID_IMediaEvent, (LPVOID *) 
         &Global.PMediaEvent);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Failed to create IMediaEvent."), hr);
      return hr;
   }
   // Set the window handle used to process graph events
   hr = Global.PMediaEvent->SetNotifyWindow((OAHWND)Global.HWndApp, 
         WM_GRAPHNOTIFY, 0);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Failed call to SetNotifyWindow."), hr);
      return hr;
   }
   return hr;
}

/******************************************************************************/

HRESULT
StopCapture()
{
   IBaseFilter *pSrcFilter;
   OAFilterState fs = State_Running;

   if (!Global.PMediaControl || !Global.PMediaEvent || !Global.PVideoWindow)
      return E_POINTER;
   // Get rid of menu garbage.
   InvalidateRect(Global.HWndApp, NULL, TRUE);
   // Stop receiving events.
   Global.PMediaEvent->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);
   // Get state from graph manager.
   Global.PMediaControl->GetState(INFINITE, &fs);
   // Stop the graph if not already stopped.
   if (fs != State_Stopped)
   {
      long evCode;

      Global.PMediaControl->Stop();
      Global.PMediaEvent->WaitForCompletion(INFINITE,&evCode);
   }
   // Finished with PMediaEvent interface.
   SAFE_RELEASE(Global.PMediaEvent);
   // Finished with the PMediaControl interface.
   SAFE_RELEASE(Global.PMediaControl);
   // Relinquish ownership (IMPORTANT!) of the video window.
   // Failing to call put_Owner can lead to assert failures within
   // the video renderer, as it still assumes that it has a valid
   // parent window.
   Global.PVideoWindow->put_Owner(NULL);
   Global.PVideoWindow->put_Visible(OAFALSE);
   // Finished with PVideoWindow interface.
   SAFE_RELEASE(Global.PVideoWindow);
   // Return the graphs source filter.
   if(SUCCEEDED(GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
         &pSrcFilter)))
   {
      // Cleanup, disconnect the graph downstream of the source filter.
      DisconnectFilter(Global.PGraphBuilder, pSrcFilter);
      SAFE_RELEASE(pSrcFilter); 
   } 

   return S_OK;
}

/******************************************************************************/

HRESULT
StartCapture()
{
   HRESULT     hr; 
   IBaseFilter *pSrcFilter;
   IBaseFilter *pVMRFilter = NULL;

   // Return the graphs source filter.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Couldn't retrieve graph's source filter"), hr);
      return hr;
   }
   // Return the graphs render filter.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("VMR"), &pVMRFilter);
   if (FAILED(hr))
      pVMRFilter = NULL;
   // Use this instead of Global.PGraphBuilder->RenderFile.
   hr = Global.PCapture->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, 
         pSrcFilter, NULL, pVMRFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't render the video capture stream.\n")
            TEXT("The source filter may already be in use by another ")
            TEXT("application."), hr);
      SAFE_RELEASE(pSrcFilter);
      SAFE_RELEASE(pVMRFilter);
      return hr;
   }
   // Create capture interfaces.
   hr = OpenInterfaces();
   if(FAILED(hr))
   {
      SAFE_RELEASE(pSrcFilter);
      SAFE_RELEASE(pVMRFilter);
      return hr;
   }
   // Set renderer scaling 1:1
   SetVideoWindow(Global.HWndApp, pSrcFilter, Global.PVideoWindow, NULL, 
         TEXT("Preview"));
   SAFE_RELEASE(pSrcFilter);
   SAFE_RELEASE(pVMRFilter);
   // Start previewing video data.
   hr = Global.PMediaControl->Run();
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Couldn't run the graph."), hr);
      return hr;
   }

   return hr;
}

/******************************************************************************/

HRESULT 
BuildGraph(
   ULONG uIndex)
{
   HRESULT hr;
   IBaseFilter *pSrcFilter = NULL;
   IBaseFilter *pVMR = NULL;

   // Find the requested Vision filter index.
   hr = GetVisionSourceFilter(uIndex, &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Couldn't find source filter."), hr);
      return hr;
   }
   // Add capture filter to our graph.
   hr = Global.PGraphBuilder->AddFilter(pSrcFilter, TEXT("Video Capture"));
   SAFE_RELEASE(pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't add the capture filter to the graph.\n") 
            TEXT("If you have a working video capture device, please make sure\n")
            TEXT("that it is connected and is not being used by another ")
            TEXT("application."), hr);
      return hr;
   }
   // Add VMR7 as the preferred renderer to our graph.
   if (SUCCEEDED(CoCreateInstance(CLSID_VideoMixingRenderer, 0, 
      CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pVMR)))
   {
      Global.PGraphBuilder->AddFilter(pVMR, L"VMR");
      SAFE_RELEASE(pVMR);
   }

   return S_OK;
}

/******************************************************************************/

HRESULT 
UnBuildGraph()
{
   if (Global.PMediaControl)
   {
      OAFilterState fs = State_Running;
      // Get state from graph manager.
      Global.PMediaControl->GetState(INFINITE, &fs);
      // Stop the graph if not already stopped.
      if ( fs != State_Stopped )
         StopCapture();
   }
   // Remove graph filters.
   return RemoveGraphFilters (Global.PGraphBuilder);
}

/******************************************************************************/

HRESULT 
ChangePreviewState(
   int nShow)
{
   HRESULT hr = S_OK;

   if (!Global.PMediaControl)
      return E_POINTER;

   if (nShow)
   {
      OAFilterState fs = State_Running;
      // Get state from graph manager.
      Global.PMediaControl->GetState(1000, &fs);
      if (fs != State_Running)
      {
         // Start previewing video data.
         hr = Global.PMediaControl->Run();
      }
   }
   else
   {
      // Stop previewing video data.
      hr = Global.PMediaControl->StopWhenReady();
   }
   return hr;
}

/******************************************************************************/

HRESULT 
HandleGraphEvent()
{
   long evCode;
   LONG_PTR evParam1, evParam2;
   HRESULT hr = S_OK;

   if (!Global.PMediaEvent)
      return E_POINTER;
   // See evcode.h for full list of system event codes.
   while(SUCCEEDED(Global.PMediaEvent->GetEvent(&evCode, &evParam1, &evParam2, 0)))
   {
      // Free event parameters to prevent memory leaks associated with
      // event parameter data.  While this application is not interested
      // in the received events, applications should always process them.
      hr = Global.PMediaEvent->FreeEventParams(evCode, evParam1, evParam2);
      // Insert event processing code here, if desired
   }
   return hr;
}

/******************************************************************************/

LRESULT CALLBACK 
WndMainProc ( 
   HWND     hWnd, 
   UINT     message, 
   WPARAM   wParam, 
   LPARAM   lParam)
{
   switch (message)
   {
      case WM_GRAPHNOTIFY:
         HandleGraphEvent();
         break;

      case WM_SIZE:
         // Resize the video preview window to match owner window size.
         if (Global.PVideoWindow)
         {
            RECT rc;
            // Make the preview video fill our parent window.
            GetClientRect(Global.HWndApp, &rc);
            // Note that the video renderer will introduce scaling if not 1:1.
            Global.PVideoWindow->SetWindowPosition(rc.left, rc.top, 
                  rc.right,rc.bottom);
         }
         break;

      case WM_WINDOWPOSCHANGED:
         ChangePreviewState(!(IsIconic(hWnd)));
         break;

     case WM_CLOSE:
         // Hide the main window while the graph is destroyed.
         ShowWindow(Global.HWndApp, SW_HIDE);
         break;

      case WM_DESTROY:
         PostQuitMessage(1);
         break;;
   }

   // Pass this message to the video window for notification of system changes
   if (Global.PVideoWindow)
      Global.PVideoWindow->NotifyOwnerMessage((LONG_PTR)hWnd, message, wParam, 
            lParam);

   return DefWindowProc (hWnd, message, wParam, lParam);
}

/******************************************************************************/

HRESULT
Initialise()
{
   HRESULT hr;

   // Create the filter graph.
   hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
         IID_IGraphBuilder, (void **) &Global.PGraphBuilder);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Failed to create PGraphBuilder."), hr);
      return hr;
   }
   // Create the capture graph builder.
   hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
         IID_ICaptureGraphBuilder2, (void **) &Global.PCapture);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Failed to create ICaptureGraphBuilder2."), hr);
      return hr;
   }
   // Attach the filter graph to the capture graph.
   hr = Global.PCapture->SetFiltergraph(Global.PGraphBuilder);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Failed call to set the filter graph."), hr);
      return hr;
   }
   // Add our graph to the running object table, which will allow
   // GraphEdit application to connect to our graph.
   hr = AddGraphToRot(Global.PGraphBuilder, &Global.DWGraphRegister);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Failed to register filter graph with ROT."), hr);
      Global.DWGraphRegister = 0;
   }
   return hr;
}

/******************************************************************************/

void
UnInitialise()
{
   SAFE_RELEASE(Global.PGraphBuilder);
   SAFE_RELEASE(Global.PCapture);

   // Remove the filter graph from the running object table.
   if (Global.DWGraphRegister)
      RemoveGraphFromRot(Global.DWGraphRegister);

   Global.DWGraphRegister = 0;
}

/******************************************************************************/
  
int APIENTRY 
_tWinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPTSTR    pCmdLine,
   int       nCmdShow)
{
   WNDCLASS       wc;
   MSG            msg = {0};
   COMMANDLINE    commandLine;

   // Initialize COM.
   if(FAILED(CoInitialize(NULL)))
   {
     ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
         TEXT("CoInitialize Failed"), NULL);   
     exit(1);
   } 
   // Set defaults.
   commandLine.input = 1;

   // Parse the command line
   ProcessCommandLine(pCmdLine, &commandLine);

   // Register the window class.
   ZeroMemory(&wc, sizeof wc);
   wc.lpfnWndProc   = WndMainProc;
   wc.hInstance     = hInstance;
   wc.lpszClassName = CLASSNAME;
   wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

   if(!RegisterClass(&wc))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("RegisterClass failed."), GetLastError());
      CoUninitialize();
      exit(1);
   }
   // Create the main window, WS_CLIPCHILDREN style is required.
   Global.HWndApp = CreateWindow(CLASSNAME, APPLICATIONNAME,
         WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
         0, 0, hInstance, 0);

   if(Global.HWndApp)
   {
      // Initialise graph.
      if (SUCCEEDED(Initialise())) 
      {
         BOOL bRet;
         // Build capture graph.
         if (SUCCEEDED(BuildGraph(commandLine.input-1)))
         {
            IBaseFilter *pSrcFilter;
            // Set any command line arguments.
            if(SUCCEEDED(GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
                  &pSrcFilter)))
            {
               SetCommandLine(Global.HWndApp, APPLICATIONNAME, &commandLine, 
                     pSrcFilter, TEXT("Preview"));
               // Start capture graph.
               StartCapture();
               // Set any scaling done by the renderer.
               if ( commandLine.flags & CMD_FLAGS_WINDOW_POSITION)
                  SetVideoWindow(Global.HWndApp, pSrcFilter, Global.PVideoWindow, 
                        &commandLine.window, TEXT("Preview"));

               SAFE_RELEASE(pSrcFilter);
            }
         }
         ShowWindow(Global.HWndApp, nCmdShow);
         // Main message loop.
         while((bRet = GetMessage(&msg,Global.HWndApp,0,0)) !=0)
         {
            if (bRet == -1)
               break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
         // Remove source filter.
         UnBuildGraph();
         // Clean up
         UnInitialise();
      }
   }
   // Release COM.
   CoUninitialize();

   return (int) msg.wParam;
}

/******************************************************************************/
