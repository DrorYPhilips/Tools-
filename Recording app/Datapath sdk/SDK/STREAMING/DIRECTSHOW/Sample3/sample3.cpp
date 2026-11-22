/*******************************************************************************

Copyright Datapath Ltd. 2008, 2016.

File:    Sample3.cpp

Purpose: 

History:
         01 DEC 08    RL   Created.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Removed applications concept of graph state.
         08 SEP 10    RL   Added Stream Information dialogue.
         16 NOV 10    RL   Fixed bug exiting the message loop.
         26 APR 12    JL   Added support for User-Mode filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>

#include <DGC133ST.h>

#include <VisionGet.h>
#include <VisionFilter.h>
#include <VisionPin.h>
#include <VisionUtil.h>
#include <VisionWin.h>

#include "sample3.h"
#include "resource.h"

/* Numer of 100ns ticks in one second. */
#define ONE_SECOND   10000000

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
   IBaseFilter *pSrcFilter, *pVMR;

   // Return the graphs source filter.
   hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), &pSrcFilter);
   if (FAILED(hr))
   {
      ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
            TEXT("Couldn't retrieve graph's source filter"), hr);
      return hr;
   }
   // Return the graphs renderer.
   hr = GetGraphFilter( Global.PGraphBuilder, TEXT( "VMR" ), &pVMR );
   if ( FAILED( hr ) )
      pVMR = NULL;
   // Use this instead of Global.PGraphBuilder->RenderFile.
   hr = Global.PCapture->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, 
         pSrcFilter, NULL, pVMR);
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

   hr = SetLiveStreamPin( pSrcFilter, TEXT("Preview"), LIVESTREAM_CLIENT_1 );

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
   // Try to Add VMR7 renderer to our graph instead.
   hr = CoCreateInstance( CLSID_VideoMixingRenderer, 0,
         CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &pVMR );
   if ( SUCCEEDED( hr ) )
   {
      hr = Global.PGraphBuilder->AddFilter( pVMR, L"VMR" );
      pVMR->Release();
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
ChangeInput(
   UINT uInput)
{
   // Stop capture graph.
   StopCapture();

   // Remove source filter.
   if (SUCCEEDED(UnBuildGraph()))
   {
      // Create new graph.
      if (SUCCEEDED(BuildGraph(uInput)))
      {
         // Start capture.
         if (SUCCEEDED(StartCapture()))
         {
            return TRUE;
         }
      }
   }
   return FALSE;
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

VOID
SetEditFieldData(
   HWND        hDlg,
   PSTATISTICS pStats)
{
    TCHAR buffer[50];
    LONG  lData;
    int   iData;

    if ( !pStats )
       return;

    _sntprintf ( buffer, sizeof ( buffer ) / sizeof ( buffer[0] ), TEXT("%.2f"), 
          pStats->FrameRate );
    SendDlgItemMessage(hDlg, IDC_FRAMERATE, WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);

   if ( pStats->PDroppedFrames )
   {
      pStats->PDroppedFrames->GetNumDropped(&lData);
      wsprintf(buffer,TEXT("%d"),lData);
      SendDlgItemMessage(hDlg, IDC_DROPPEDFRAMES, WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);

      pStats->PDroppedFrames->GetNumNotDropped(&lData);
      wsprintf(buffer,TEXT("%d"),lData);
      SendDlgItemMessage(hDlg, IDC_NOTDROPPEDFRAMES, WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);
   }

   if ( pStats->PQualProp )
   {
      pStats->PQualProp->get_FramesDroppedInRenderer(&iData);
      wsprintf(buffer,TEXT("%d\0"), iData);
      SendDlgItemMessage(hDlg, IDD_QDROPPED, WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);

      pStats->PQualProp->get_FramesDrawn(&iData);
      wsprintf(buffer,TEXT("%d\0"), iData);
      SendDlgItemMessage(hDlg, IDD_QDRAWN, WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);

      pStats->PQualProp->get_AvgFrameRate(&iData);
      wsprintf(buffer,TEXT("%d.%02d\0"), iData/100, iData%100);
      SendDlgItemMessage(hDlg, IDD_QAVGFRM,  WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);

      pStats->PQualProp->get_Jitter(&iData);
      wsprintf(buffer,TEXT("%d\0"), iData);
      SendDlgItemMessage(hDlg, IDD_QJITTER,  WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);

      pStats->PQualProp->get_AvgSyncOffset(&iData);
      wsprintf(buffer,TEXT("%d\0"), iData);
      SendDlgItemMessage(hDlg, IDD_QSYNCAVG, WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);

      pStats->PQualProp->get_DevSyncOffset(&iData);
      wsprintf(buffer,TEXT("%d\0"), iData);
      SendDlgItemMessage(hDlg, IDD_QSYNCDEV, WM_SETTEXT, 0, (DWORD) (LPSTR) buffer);
   }
}

/******************************************************************************/

INT_PTR CALLBACK
DroppedFramesDlgProc(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
{
   switch (message)
   {
      case WM_TIMER:
      {
         if (wParam == UPDATE_STATS)
         {
            SetEditFieldData(hDlg,(PSTATISTICS)GetProp (hDlg, TEXT("Property")));            
         }
         break;
      }

      case WM_CLOSE:
      {
         RemoveProp(hDlg, TEXT("Property"));
         EndDialog(hDlg, 1);
         break;
      }

      case WM_COMMAND:
      {
         switch (LOWORD(wParam))
         {
            case IDOK:
            {
               RemoveProp(hDlg, TEXT("Property"));
               EndDialog(hDlg, 1);
               break;
            }
         }
         break;
      }

      case WM_INITDIALOG:
      {
         PSTATISTICS pStats = (PSTATISTICS)lParam;
         
         SetProp (hDlg, TEXT("Property"), pStats);
         SetTimer (hDlg, UPDATE_STATS , 500, NULL);

         break;
      }

      default:
         break;
   }

   return 0;
}

/******************************************************************************/

void
ProcessCommand ( 
   HWND     hWnd, 
   WORD     uId )
{
   switch (uId)
   {
      case IDM_FILE_EXIT:
         PostMessage(hWnd,WM_CLOSE,0,0);
         break;
      
      case IDM_PROPERTIES_PIN:
      {
         HRESULT hr;
         IBaseFilter *pSrcFilter;

         // We need to tear down the graph downstream of the
         // source capture filter before opening the pin dialog.
         StopCapture();

         hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
               &pSrcFilter);
         if (SUCCEEDED(hr))
         {
            PinPropertiesDlg(Global.HWndApp, pSrcFilter, TEXT("Preview"));
            SAFE_RELEASE(pSrcFilter);
         }
         else
            ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
                  TEXT("Failed to open the pin property page."), hr);

         StartCapture();
         break;
      }
      case IDM_PROPERTIES_FILTER:
      {
         HRESULT hr;
         IBaseFilter *pSrcFilter;

         hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
                  &pSrcFilter);
         if (SUCCEEDED(hr))
         {
            FilterPropertiesDlg(Global.HWndApp, pSrcFilter);
            SAFE_RELEASE(pSrcFilter);
         }
         else
            ErrorMessage(Global.HWndApp, APPLICATIONNAME, 
                  TEXT("Failed to open the filter property page."), hr);

         break;
      }
      case IDM_PROPERTIES_STREAMINFO:
      {
         HRESULT     hr; 
         IBaseFilter *pSrcFilter;
         STATISTICS  stats = {NULL};

         // Return the graphs source filter.
         hr = GetGraphFilter(Global.PGraphBuilder, TEXT("Video Capture"), 
               &pSrcFilter);
         if (SUCCEEDED(hr))
         {
            AM_MEDIA_TYPE *pmt = NULL;
            
            hr = GetCurrentMediaType(pSrcFilter, &pmt, TEXT("Preview"));
            if(SUCCEEDED(hr) && pmt )
            {
               VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pmt->pbFormat;
               stats.FrameRate = (float)ONE_SECOND / (float)pVIH->AvgTimePerFrame;
               DeleteMediaType(pmt);
            }
            else
               break;

            LoadDroppedFramesInterface(pSrcFilter,TEXT("Preview"),&stats.PDroppedFrames);
            LoadRendererQualPropInterface(Global.PCapture,pSrcFilter,&stats.PQualProp);
         
            SAFE_RELEASE(pSrcFilter);
         }

         stats.HWndApp = Global.HWndApp;
         
         DialogBoxParam ( Global.HInstance, MAKEINTRESOURCE ( IDD_STREAMINFO ),
               Global.HWndApp, DroppedFramesDlgProc, (LPARAM)&stats );

         SAFE_RELEASE(stats.PDroppedFrames);
         SAFE_RELEASE(stats.PQualProp);

         break;
      }
      default: 
      {
         // Change input.
         if ((uId >= IDM_INPUTS_START) && (uId < IDM_INPUTS_END))
         {
            ChangeInput(uId-IDM_INPUTS_START);
         }
         break;
      }
   }
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

      case WM_COMMAND:
         ProcessCommand(hWnd,LOWORD(wParam));
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

void 
PopulateDeviceMenu()
{
   MENUITEMINFO   mii;
   HMENU          hDevicesMenu, hMainMenu;

   if ((hMainMenu = GetMenu(Global.HWndApp)) == NULL)
      return;

   if ((hDevicesMenu = CreateMenu()) == NULL)
      return;

   // Create the devices menu.
   mii.cbSize = sizeof(MENUITEMINFO);
   mii.fMask = MIIM_SUBMENU;
   mii.hSubMenu = hDevicesMenu;

   // Add the devices pop up menu.
   if (SetMenuItemInfo(hMainMenu, IDM_DEVICES, FALSE, &mii))
   {
      HRESULT hr;
      TCHAR   buffer[MAX_PATH];
      UINT    iIndex = 0;
      
#pragma warning( disable : 4127 )
      while (1)
#pragma warning( default : 4127 )
      {
         hr = GetVisionFriendlyName(iIndex, buffer, MAX_PATH);
         if( SUCCEEDED(hr) )
         {
            // Add the friendly name
            mii.dwTypeData = buffer;
            mii.fMask = MIIM_TYPE | MIIM_ID;
            mii.fType = MFT_STRING;
            // Set the WM_COMMAND, LOWORD(wParam) parameter.
            mii.wID = IDM_INPUTS_START+iIndex;
   
            // Insert the menu item
            if ( InsertMenuItem (hDevicesMenu, iIndex, TRUE, &mii))
               iIndex++;
            else
               break;
         }
         else
            break;
      }
   }
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
   wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
   wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

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
      Global.HInstance = hInstance;

      // Initialise graph.
      if (SUCCEEDED(Initialise())) 
      {
         BOOL bRet;
         // Add capture devices to menu.
         PopulateDeviceMenu();
         // Build capture graph.
         if (SUCCEEDED(BuildGraph(0)))
         {
            // Start capture graph.
            StartCapture();
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