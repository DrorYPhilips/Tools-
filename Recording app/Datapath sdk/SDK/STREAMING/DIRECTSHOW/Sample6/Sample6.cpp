/*******************************************************************************

Copyright Datapath Ltd. 2009, 2016.

File:    Sample6.cpp

Purpose: 

History:
         06 OCT 09    RL   Created.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.
         16 NOV 10    RL   Fixed bug exiting the message loop.
         26 APR 12    JL   Added support for User-Mode filters.
         01 MAY 12    JL   Uses IBaseFilter::Connect to connect the source
                           filter's output pin to the VMR renderer's input pin.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>

#include <DGC133ST.h>

#include <VisionCmd.h>
#include <VisionGet.h>
#include <VisionFilter.h>
#include <VisionPin.h>
#include <VisionUtil.h>
#include <VisionWin.h>

#include "Sample6.h"

// Set to TRUE to allow cropping.
#define USE_CROPPING    FALSE

#if USE_CROPPING
HRESULT
CroppingSetup(
   IBaseFilter *pSrcFilter );
#endif

/******************************************************************************/

const TCHAR * 
GetPhysicalPinName(
   long lType)
{
   switch (lType) 
   {
      case PhysConn_Video_Tuner:            return L"Video Tuner";
      case PhysConn_Video_Composite:        return L"Video Composite";
      case PhysConn_Video_SVideo:           return L"S-Video";
      case PhysConn_Video_RGB:              return L"Video RGB";
      case PhysConn_Video_YRYBY:            return L"Video YRYBY";
      case PhysConn_Video_SerialDigital:    return L"Video Serial Digital";
      case PhysConn_Video_ParallelDigital:  return L"Video Parallel Digital"; 
      case PhysConn_Video_SCSI:             return L"Video SCSI";
      case PhysConn_Video_AUX:              return L"Video AUX";
      case PhysConn_Video_1394:             return L"Video 1394";
      case PhysConn_Video_USB:              return L"Video USB";
      case PhysConn_Video_VideoDecoder:     return L"Video Decoder";
      case PhysConn_Video_VideoEncoder:     return L"Video Encoder";
      
      case PhysConn_Audio_Tuner:            return L"Audio Tuner";
      case PhysConn_Audio_Line:             return L"Audio Line";
      case PhysConn_Audio_Mic:              return L"Audio Microphone";
      case PhysConn_Audio_AESDigital:       return L"Audio AES/EBU Digital";
      case PhysConn_Audio_SPDIFDigital:     return L"Audio S/PDIF";
      case PhysConn_Audio_SCSI:             return L"Audio SCSI";
      case PhysConn_Audio_AUX:              return L"Audio AUX";
      case PhysConn_Audio_1394:             return L"Audio 1394";
      case PhysConn_Audio_USB:              return L"Audio USB";
      case PhysConn_Audio_AudioDecoder:     return L"Audio Decoder";
      
      default:                              return L"Unknown Type";
   }    
}

/******************************************************************************/

void 
DisplayCrossbarInfo(
   IAMCrossbar *pXBar)
{
   HRESULT hr;
   TCHAR buffer[255];
   long cOutput = -1, cInput = -1;
   hr = pXBar->get_PinCounts(&cOutput, &cInput);
   long i;
   
   for (i = 0; i < cOutput; i++)
   {
      long lRelated = -1, lType = -1, lRouted = -1;
      
      hr = pXBar->get_CrossbarPinInfo(FALSE, i, &lRelated, &lType);
      hr = pXBar->get_IsRoutedTo(i, &lRouted);
      
      wsprintf(buffer, L"Output pin %d: %s\n", i, GetPhysicalPinName(lType));
      OutputDebugString(buffer);
      wsprintf(buffer, L"\tRelated out: %d, Routed in: %d\n", lRelated, lRouted);
      OutputDebugString(buffer);
      wsprintf(buffer, L"\tSwitching Matrix: ");
      OutputDebugString(buffer);
      
      for (long j = 0; j < cInput; j++)
      {
         hr = pXBar->CanRoute(i, j);
         wsprintf(buffer, L"%d-%s", j, (S_OK == hr ? L"Yes" : L"No"));
         OutputDebugString(buffer);
      }
      wsprintf(buffer, L"\n\n");
      OutputDebugString(buffer);
   }
   
   for (i = 0; i < cInput; i++)
   {
      long lRelated = -1, lType = -1;
      
      hr = pXBar->get_CrossbarPinInfo(TRUE, i, &lRelated, &lType);
      
      wsprintf(buffer, L"Input pin %d - %s\n", i, GetPhysicalPinName(lType));
      OutputDebugString(buffer);
      wsprintf(buffer, L"\tRelated in: %d\n", lRelated);
      OutputDebugString(buffer);
   }
}

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
   if (SUCCEEDED(GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
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
   HRESULT hr;
   IBaseFilter *pSrcFilter, *pVMRFilter;
   
   // Return the graphs source filter.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Couldn't retrieve graph's source filter"), hr);
      return hr;
   }
   // Return the graphs source filter.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("VMR"), &pVMRFilter);
   if (FAILED(hr))
      pVMRFilter = NULL;

   if ( SUCCEEDED(hr) )
   {
      IEnumPins *pEnumPins;
      hr = pVMRFilter->EnumPins( &pEnumPins );
      if ( SUCCEEDED(hr) )
      {
         IPin *pVMRPin;
         // Find the input pin on the VMR renderer.
         hr = pEnumPins->Next( 1, &pVMRPin, NULL );
         if ( SUCCEEDED(hr) )
         {
            IPin *pPin;
            hr = GetSourcePinInterface( pSrcFilter, &pPin, TEXT("Preview") );
            if ( SUCCEEDED(hr) )
            {
               AM_MEDIA_TYPE  *pmt;
               hr = GetCurrentMediaType( pSrcFilter, &pmt, TEXT("Preview") );
               if ( SUCCEEDED(hr) )
               {
                  // Connect the source filter's output pin directly to the
                  // VMR renderer's input pin.
                  hr = pPin->Connect( pVMRPin, pmt );
                  DeleteMediaType(pmt);
               }
               SAFE_RELEASE(pPin);
            }
            SAFE_RELEASE(pVMRPin);
         }
         SAFE_RELEASE(pEnumPins);
      }
   }
   // If the VMR filter couldn't be loaded or connected to, it may be that an
   // additional transform filter is required. RenderStream can add additional
   // transform filters to the graph.
   if (FAILED(hr))
   {
      // Use this instead of Global.PGraphBuilder->RenderFile.
      hr = Global.PCapture->RenderStream (&PIN_CATEGORY_PREVIEW,
            &MEDIATYPE_Video, pSrcFilter, NULL, pVMRFilter);
   }
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't render the video capture stream.\n")
            TEXT("The source filter may already be in use by another ")
            TEXT("application."), hr);
      return hr;
   }
   // Create capture interfaces.
   hr = OpenInterfaces();
   if(FAILED(hr))
      return hr;
   // Set renderer scaling 1:1
   SetVideoWindow(Global.HWndApp, pSrcFilter, Global.PVideoWindow, NULL, 
         TEXT("Preview"));
#if USE_CROPPING
   // Demonstrate the use of IVisionPin for cropping.
   CroppingSetup( pSrcFilter );
#endif
   SAFE_RELEASE(pSrcFilter);
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
   ULONG input,
   ULONG channel)
{
   HRESULT hr;
   IBaseFilter *pSrcFilter = NULL;
   IBaseFilter *pVMR = NULL;

   // Find the requested Vision filter index.
   hr = GetVisionSourceFilter(input, &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't find source filter."), hr);
      return hr;
   }
   // Add capture filter to our graph.
   hr = Global.PGraphBuilder->AddFilter(pSrcFilter, TEXT("Video Capture"));
   
   if(SUCCEEDED(LoadCrossbarInterface(Global.PCapture, pSrcFilter, 
      &Global.PCrossbar)))
   {
      // Use Global.PCrossbar
      DisplayCrossbarInfo (Global.PCrossbar);
      Global.PCrossbar->Route(0,channel);
      SAFE_RELEASE(Global.PCrossbar);
   }

   SAFE_RELEASE(pSrcFilter);
   
   // Add VMR9 renderer to our graph.
   hr = CoCreateInstance(CLSID_VideoMixingRenderer9, 0, 
      CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pVMR);
   if (!SUCCEEDED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Failed to load preferred VMR9."), hr);

      // Try to Add VMR7 renderer to our graph instead.
      hr = CoCreateInstance(CLSID_VideoMixingRenderer, 0, 
         CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pVMR);
      if (!SUCCEEDED(hr))
      {
         ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Failed to load preferred VMR7."), hr);
      }
   }
   if (SUCCEEDED(hr))
      hr = Global.PGraphBuilder->AddFilter(pVMR, L"VMR");

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

#if USE_CROPPING
HRESULT
CroppingSetup (
   IBaseFilter *pSrcFilter )
{
   HRESULT hr;
   IPin *pPin;

   // Only valid for the Capture pin.
   hr = GetSourcePinInterface( pSrcFilter, &pPin, TEXT("Preview") );
   if ( SUCCEEDED(hr) )
   {
      IVisionPin *pIVUP = NULL;
      // Return the pin interface.
      hr = pPin->QueryInterface( __uuidof(IVisionPin), (void **)&pIVUP );
      if ( FAILED( hr ) )
      {
         // IVisionUserPin inherits from IVisionPin
         hr = pPin->QueryInterface( __uuidof(IVisionUserPin), (void **)&pIVUP );
      }
      if ( SUCCEEDED(hr) )
      {
         CROP cropDef = {0};
         hr = pIVUP->get_CroppingDefault( &cropDef );
         if ( SUCCEEDED(hr) )
         {
            CROP cropSet = {0};
            // Show only the central 'quarter' of the capture.
            cropSet.Top  = cropDef.Height / 4;
            cropSet.Left = cropDef.Width  / 4;
            cropSet.Height = cropDef.Height / 2;
            cropSet.Width  = cropDef.Width  / 2;

            hr = pIVUP->put_Cropping( &cropSet );
            if ( SUCCEEDED(hr) )
            {
               hr = pIVUP->put_CroppingEnabled( TRUE );
            }
         }

         SAFE_RELEASE(pIVUP);
      }
      SAFE_RELEASE(pPin);
   }
   return hr;
}
#endif

/******************************************************************************/

int APIENTRY 
_tWinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPTSTR    pCmdLine,
   int       nCmdShow)
{
   WNDCLASS  wc;
   MSG msg = {0};
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
   commandLine.channel = 1;
   commandLine.buffer.x = 0;
   commandLine.buffer.y = 0;
   commandLine.fps = 0;
   memcpy (&commandLine.subformat, &MEDIASUBTYPE_YUY2, sizeof(GUID));
   
   // Parse the command line
   ProcessCommandLine( pCmdLine, &commandLine);

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
         if (SUCCEEDED(BuildGraph(commandLine.input-1,commandLine.channel-1)))
         {
            IBaseFilter *pSrcFilter;
            // Set any command line arguments.
            if(SUCCEEDED(GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
                  &pSrcFilter)))
            {
               HRESULT hr;

               hr = SetVideo2MediaType(pSrcFilter,commandLine.buffer.x,  
                     commandLine.buffer.y, commandLine.fps, 
                     &commandLine.subformat, TEXT("Preview"));
               if (FAILED(hr))
               {
                  ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
                     TEXT("Failed SetVideo2MediaType."), hr);
                  UnBuildGraph();
                  UnInitialise();
                  return hr;
               }
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
