/*******************************************************************************

Copyright Datapath Ltd. 2008, 2016.

File:    Sample5.cpp

Purpose: 

History:
         01 DEC 08    RL   Created.
         09 OCT 09    RL   No longer uses a smart tee.
                           Use the source Filters 'Preview' pin.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.
         16 NOV 10    RL   Fixed bug exiting the message loop.
         26 APR 12    JL   Added support for User-Mode filters.
         12 SEP 12    JL   Configure WM ASF Writer to record video at the set
                           format. Preview of audio capture.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>
#include <Dshowasf.h>

#include <DGC133ST.h>

#include <VisionGet.h>
#include <VisionPin.h>
#include <VisionUtil.h>
#include <VisionWin.h>

#include "sample5.h"

/* Numer of 100ns ticks in one second. */
#define ONE_SECOND   10000000

// Set to TRUE to allow stream control.
#define USE_STREAM_CONTROL   1// FALSE

/******************************************************************************/

HRESULT
GetGraphAudioFilter(
   IBaseFilter **ppSrcFilter )
{
   HRESULT hr = E_FAIL;
   IBaseFilter *pSrcFilter = NULL;

   if(Global.PGraphBuilder)
   {
      // Query the filter graph for its source.
      hr = Global.PGraphBuilder->FindFilterByName( TEXT("Audio Capture"), 
            &pSrcFilter );
      if(SUCCEEDED(hr))
      {
         *ppSrcFilter = pSrcFilter;
      }
   }
   return hr;
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
   IBaseFilter *pAudioFilter;
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
       // Return the graphs audio filter.
   if (SUCCEEDED(GetGraphAudioFilter(&pAudioFilter)))
   {
      // Cleanup, disconnect the graph downstream of the source filter.
      DisconnectFilter(Global.PGraphBuilder, pAudioFilter);
      SAFE_RELEASE(pAudioFilter); 
   } 

   return S_OK;
}

/******************************************************************************/

HRESULT
SetupAVI()
{
   HRESULT hr;
   IBaseFilter *pMux = NULL;
   IBaseFilter *pSrcFilter = NULL;
   IBaseFilter *pAudioFilter = NULL;

   // Return the graphs source filter.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't retrieve graph's source filter"), hr);
      return hr;
   }

   // Specify the compression and file path.
   hr = Global.PCapture->SetOutputFileName(Global.PFileFormat,
         Global.OutputFileName, &pMux, NULL);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't set output file name."), NULL);
      return hr;
   }
   // Render the capture pin. Specifying PIN_CATEGORY_CAPTURE will include a
   // Smart Tee filter.
   hr = Global.PCapture->RenderStream (&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
         pSrcFilter, NULL, pMux);

   SAFE_RELEASE(pSrcFilter);

   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
          TEXT("Couldn't render the video capture stream.\n")
          TEXT("The source filter may already be in use by another")
          TEXT("application."), hr);
      return hr;
   }
   // Attempt to connect audio filter.
   hr = GetGraphAudioFilter(&pAudioFilter);
   if (SUCCEEDED(hr))
   {
      // Render the capture pin. Specifying PIN_CATEGORY_CAPTURE will include a
      // Smart Tee filter.
      hr = Global.PCapture->RenderStream (&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
            pAudioFilter, NULL, pMux);
      if (SUCCEEDED(hr))
      {
         IConfigAviMux *pConfigMux = NULL;

         hr = pMux->QueryInterface(IID_IConfigAviMux, (void**)&pConfigMux);
         if (SUCCEEDED(hr))
         {
            // When using two devices for audio and video capture, use the
            // audio stream as the master to prevent drift between the two 
            // streams. The AVI Mux filter will adjust playback on the video 
            // capture stream to match the audio stream. 0 video, 1 audio.
            pConfigMux->SetMasterStream(1);
            SAFE_RELEASE(pConfigMux);
         }
      }
      SAFE_RELEASE(pAudioFilter);
   }
   SAFE_RELEASE(pMux);
   return S_OK;
}

/******************************************************************************/

HRESULT
ConfigureASFWriter(
   IBaseFilter *pASFWriter )
{
   IConfigAsfWriter *pConfig = NULL;
   HRESULT hr;

   if ( pASFWriter == NULL )
      return E_POINTER;

   hr = pASFWriter->QueryInterface(IID_IConfigAsfWriter, (void**)&pConfig);
   if (SUCCEEDED(hr))
   {
      IWMProfile *pProfile = NULL;
      hr = pConfig->GetCurrentProfile( &pProfile );
      if (SUCCEEDED(hr))
      {
         DWORD numStreams = 0;
         IWMStreamConfig *pStream = NULL;
         // numStreams should return as 2; 1 audio and 1 video.
         hr = pProfile->GetStreamCount( &numStreams );
         if (SUCCEEDED(hr))
         {
            for ( DWORD i = 0 ; i < numStreams ; i++ )
            {
               hr = pProfile->GetStream( i, &pStream );
               if (SUCCEEDED(hr))
               {
                  GUID type;
                  hr = pStream->GetStreamType( &type );
                  if (SUCCEEDED(hr))
                  {
                     if ( type == WMMEDIATYPE_Video )
                     {
                        // We found our video stream.
                        break;
                     }
                  }
                  SAFE_RELEASE(pStream);
               }
            }
         }

         if ( pStream )
         {
            IWMMediaProps *pProps = NULL;
            hr = pStream->QueryInterface( IID_IWMMediaProps, (void**)&pProps );
            if (SUCCEEDED(hr))
            {
               IBaseFilter *pSrcFilter = NULL;
               hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"),
                     &pSrcFilter);
               if (SUCCEEDED(hr))
               {
                  AM_MEDIA_TYPE *pmt = NULL;
                  // Get the media type of the source.
                  hr = GetCurrentMediaType( pSrcFilter, &pmt, TEXT("Capture") );
                  if (SUCCEEDED(hr))
                  {
                     DWORD size = 0;
                     // Get the size of the structure.
                     hr = pProps->GetMediaType( NULL, &size );
                     if (SUCCEEDED(hr))
                     {
                        WM_MEDIA_TYPE *pMediaType = (WM_MEDIA_TYPE*)malloc(size);
                        if ( pMediaType )
                        {
                           // Get the stream's media type.
                           hr = pProps->GetMediaType( pMediaType, &size );
                           if (SUCCEEDED(hr))
                           {
                              VIDEOINFOHEADER *pvih = (VIDEOINFOHEADER*)pMediaType->pbFormat;
                              VIDEOINFOHEADER *pvihsrc = (VIDEOINFOHEADER*)pmt->pbFormat;

                              // Set the media type to the size of our source.
                              pvih->rcSource = pvihsrc->rcSource;
                              pvih->rcTarget = pvihsrc->rcTarget;
                              pvih->bmiHeader.biWidth  = pvihsrc->bmiHeader.biWidth;
                              pvih->bmiHeader.biHeight = pvihsrc->bmiHeader.biHeight;

                              hr = pProps->SetMediaType( pMediaType );
                              if (SUCCEEDED(hr))
                              {
                                 // Add the media type changes to the profile.
                                 hr = pProfile->ReconfigStream( pStream );
                                 if (SUCCEEDED(hr))
                                 {
                                    // Set the updated profile.
                                    hr = pConfig->ConfigureFilterUsingProfile( pProfile );
                                 }
                              }
                           }
                           free(pMediaType);
                        }
                     }
                     FreeMediaType(*pmt);
                  }
                  SAFE_RELEASE(pSrcFilter);
               }
               SAFE_RELEASE(pProps);
            }            
            SAFE_RELEASE(pStream);
         }
         else
            hr = E_FAIL;

         SAFE_RELEASE(pProfile);
      }
      SAFE_RELEASE(pConfig);
   }

   return hr;
}

/******************************************************************************/

HRESULT
SetupWMF()
{
   HRESULT hr;
   IBaseFilter *pASFWriter = NULL;
   IBaseFilter *pSrcFilter = NULL;
   IBaseFilter *pAudioFilter = NULL;

   // Return the graphs source filter.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't retrieve graph's source filter"), hr);
      return hr;
   }

   // Specify the compression and file path.
   hr = Global.PCapture->SetOutputFileName(Global.PFileFormat,
         Global.OutputFileName, &pASFWriter, NULL);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't set output file name."), NULL);
      return hr;
   }

   hr = ConfigureASFWriter( pASFWriter );

   // Render the capture pin. Specifying PIN_CATEGORY_CAPTURE will include a
   // Smart Tee filter.
   hr = Global.PCapture->RenderStream (&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
         pSrcFilter, NULL, pASFWriter);

   SAFE_RELEASE(pSrcFilter);

   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't render the video capture stream.\n")
            TEXT("The source filter may already be in use by another ")
            TEXT("application."), hr);
      return hr;
   }
   // Attempt to connect audio filter.
   hr = GetGraphAudioFilter(&pAudioFilter);
   if (SUCCEEDED(hr))
   {
      // Render the capture pin. Specifying PIN_CATEGORY_CAPTURE will include a
      // Smart Tee filter.
      hr = Global.PCapture->RenderStream (&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
            pAudioFilter, NULL, pASFWriter);

      SAFE_RELEASE(pAudioFilter);
   }
   SAFE_RELEASE(pASFWriter);
   // If we don't connect the audio input, the ASF writer will not run.
   return hr;
}

/******************************************************************************/

HRESULT
StartCapture()
{
   HRESULT     hr;
   IBaseFilter *pSrcFilter = NULL;
   IBaseFilter *pSrcAudioFilter = NULL;

   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't retrieve graph's source filter"), hr);
      return hr;
   }

   // Determine the file encoder.
   if ( memcmp(Global.PFileFormat, &MEDIASUBTYPE_Avi, sizeof(GUID)) == 0)
   {
      hr = SetupAVI();
      if(FAILED(hr))
         return hr;
   }
   else if ( memcmp(Global.PFileFormat, &MEDIASUBTYPE_Asf, sizeof(GUID)) == 0)
   {   
      hr = SetupWMF();
      if(FAILED(hr))
         return hr;
   }
   else
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't determine encoder."), NULL);
      return E_FAIL;
   }

   hr = Global.PCapture->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, 
         pSrcFilter, NULL, NULL);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
         TEXT("Failed to render the preview pin."), hr);
   }
   // Play audio preview.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Audio Capture"), &pSrcAudioFilter);
   if (SUCCEEDED(hr))
      hr = Global.PCapture->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Audio,
            pSrcAudioFilter, NULL, NULL);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Failed to render graph's audio source filter preview pin"), hr);
   }
   // Create capture interfaces.
   hr = OpenInterfaces();
   if(FAILED(hr))
      return hr;
   // Set renderer scaling 1:1
   SetVideoWindow(Global.HWndApp, pSrcFilter, Global.PVideoWindow, NULL, 
         TEXT("Capture"));
   SAFE_RELEASE(pSrcFilter);
   SAFE_RELEASE(pSrcAudioFilter);
   // Start capturing video data.
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
   IBaseFilter *pAudioFilter = NULL;

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
   // Find an audio filter.
   hr = GetAudioSourceFilter(uIndex, &pAudioFilter);
   if (SUCCEEDED(hr))
   {
      // Add audio filter to our graph.
      hr = Global.PGraphBuilder->AddFilter(pAudioFilter, TEXT("Audio Capture"));
      SAFE_RELEASE(pAudioFilter);
      if (FAILED(hr))
      {
         ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
               TEXT("Couldn't add the audio filter to the graph.\n") 
               TEXT("If you have a working audio capture device, please make sure\n")
               TEXT("that it is connected and is not being used by another ")
               TEXT("application."), hr);
      }
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

BOOL
GetFilePath (
   HWND     hWnd,
   LPTSTR   pFilePath )
{
	OPENFILENAME   ofn;
   LPCTSTR        pFilters  = TEXT("Windows Media Audio Video (*.wmv)\0*.wmv\0")
                        TEXT("Audio Video Interleave (*.avi)\0*.avi\0");
  
   ofn.lStructSize = sizeof ( OPENFILENAME );
   ofn.hwndOwner = hWnd;
   ofn.hInstance = NULL;
   ofn.lpstrCustomFilter = NULL;
   ofn.nMaxCustFilter = 0;
   ofn.nFilterIndex = 1;
   ofn.lpstrFile = pFilePath;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = MAX_PATH;
   ofn.lpstrInitialDir = NULL;
   ofn.lpstrTitle = NULL;
   ofn.lpstrDefExt = TEXT("wmv");
   ofn.lpstrFilter = pFilters;
   ofn.lCustData = 0;
   ofn.lpTemplateName = NULL;
   ofn.Flags = OFN_LONGNAMES | OFN_NOREADONLYRETURN |
         OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
         OFN_EXPLORER;

   if ( GetSaveFileName (&ofn))
   {
      if ( _tcscmp ( pFilePath+(_tcslen(pFilePath)-3), TEXT("avi")) == 0)
         Global.PFileFormat = (GUID*)&MEDIASUBTYPE_Avi;
      else if ( _tcscmp ( pFilePath+(_tcslen(pFilePath)-3), TEXT("wmv")) == 0)
         Global.PFileFormat = (GUID*)&MEDIASUBTYPE_Asf;
      else
         return FALSE;

      return TRUE;
   }
   return FALSE;
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

#if USE_STREAM_CONTROL
HRESULT
StreamControlSetup (
   IBaseFilter *pSrcFilter )
{
   HRESULT hr;
   IPin *pPin;

   // Only valid for the Capture pin.
   hr = GetSourcePinInterface( pSrcFilter, &pPin, TEXT("Capture") );
   if ( SUCCEEDED(hr) )
   {
      IAMStreamControl *pIAMS = NULL;
      // Return the pin's control interface.
      hr = pPin->QueryInterface( IID_IAMStreamControl, (void **)&pIAMS );
      if ( SUCCEEDED(hr) )
      {
         REFERENCE_TIME rtStart = 0;
         REFERENCE_TIME rtStop  = 10 * ONE_SECOND; // 10 seconds in nano-seconds.
         BOOL bSendExtraFrame = FALSE;
         DWORD dwStartCookie  = 0;
         DWORD dwStopCookie   = 0;

         // See the MSDN documentation for the IAMStreamControl interface.
         pIAMS->StartAt( &rtStart, dwStartCookie );
         pIAMS->StopAt( &rtStop, bSendExtraFrame, dwStopCookie );

         SAFE_RELEASE(pIAMS);
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
   // Initialize COM.
   if(FAILED(CoInitialize(NULL)))
   {
     ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
         TEXT("CoInitialize Failed"), NULL);   
     exit(1);
   } 
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
         WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN | WS_VISIBLE,
         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
         0, 0, hInstance, 0);

   if(Global.HWndApp)
   {
      // Initialise graph.
      if (SUCCEEDED(Initialise())) 
      {
         // Build capture graph.
         if (SUCCEEDED(BuildGraph(0)))
         {
            if ( GetFilePath(Global.HWndApp, Global.OutputFileName))
            {
               IBaseFilter *pSrcFilter;
               BOOL        bRet;

               if(SUCCEEDED(GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
                     &pSrcFilter)))
               {
                  // Configure the capture format.
                  PinPropertiesDlg(Global.HWndApp, pSrcFilter, TEXT("Capture"));

#if USE_STREAM_CONTROL
                  // Demonstrate the use of IAMStreamControl.
                  StreamControlSetup(pSrcFilter);
#endif
                  SAFE_RELEASE(pSrcFilter);
               }

               // TODO: Configure your prefered audio format.
               // Start capture graph.
               StartCapture();  

               // Main message loop.
               while((bRet = GetMessage(&msg,Global.HWndApp,0,0)) !=0)
               {
                  if (bRet == -1)
                     break;

                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }
            }
            // Remove source filter.
            UnBuildGraph();
         }
         // Clean up
         UnInitialise();
      }
   }
   // Release COM.
   CoUninitialize();

   return (int) msg.wParam;
}

/******************************************************************************/