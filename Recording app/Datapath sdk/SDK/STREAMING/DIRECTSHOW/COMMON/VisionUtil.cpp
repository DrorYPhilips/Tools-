/*******************************************************************************

Copyright Datapath Ltd. 2009, 2010.

File:    VisionUtil.cpp

Purpose: 

History:
         16 FEB 09    RL   Created.
         06 OCT 09    RL   Added DisconnectAndRemoveFilter.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Added GetGraphFilter.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>

#include <VisionUtil.h>

/******************************************************************************/

void 
ErrorMessage(
   HWND     hWnd,
   LPCTSTR  pTitle,
   TCHAR    *pFormat,
   HRESULT  hr)
{
   int      iChars = 0;
   TCHAR    buffer[1024];
   PTCHAR   pBuffer, pResult = NULL;

   const size_t NUMCHARS = sizeof(buffer) / sizeof(buffer[0]);

   pBuffer = buffer;

   if (hr)
   {
      iChars = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&pResult, 0, NULL);
      if (iChars)
      {
         // Use a bounded buffer size to prevent buffer overruns.  Limit count to
         // character size minus one to allow for a NULL terminating character.
         iChars = wsprintf(buffer, TEXT("%s \r\n\r\n Error 0x%x. %s"), 
            pFormat, hr, pResult);   
      }
      else
      {
         iChars = wsprintf(buffer, TEXT("%s \r\n\r\n Error 0x%x."), pFormat, hr);
      }
      // Ensure that the formatted string is NULL-terminated.
      buffer[NUMCHARS-1] = TEXT('\0');
   }
   if ( !iChars )
      pBuffer=pFormat;

   // Display the error.
   MessageBox(hWnd, pBuffer, pTitle, MB_OK | MB_ICONERROR);

   if (pResult)
      LocalFree(pResult);
}

/******************************************************************************/

HRESULT 
AddGraphToRot(
   IUnknown *pUnkGraph, 
   DWORD    *pdwRegister) 
{
   HRESULT hr;
   IMoniker *pMoniker;
   IRunningObjectTable *pROT;
   TCHAR buffer[128];

   if (!pUnkGraph || !pdwRegister)
      return E_POINTER;

   if (FAILED(GetRunningObjectTable(0, &pROT)))
      return E_FAIL;

   wsprintf(buffer, TEXT("FilterGraph %08x pid %08x\0"), (DWORD_PTR)pUnkGraph, 
           GetCurrentProcessId());

   hr = CreateItemMoniker(TEXT("!"), buffer, &pMoniker);
   if (SUCCEEDED(hr)) 
   {
      // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
      // to the object.  Using this flag will cause the object to remain
      // registered until it is explicitly revoked with the Revoke() method.
      //
      // Not using this flag means that if GraphEdit remotely connects
      // to this graph and then GraphEdit exits, this object registration 
      // will be deleted, causing future attempts by GraphEdit to fail until
      // this application is restarted or until the graph is registered again.
      hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
            pMoniker, pdwRegister);
      SAFE_RELEASE(pMoniker);
   }

   SAFE_RELEASE(pROT);
   return hr;
}

/******************************************************************************/

void 
RemoveGraphFromRot(
   DWORD pdwRegister)
{
   IRunningObjectTable *pROT;
   // Removes a filter graph from the Running Object Table.
   if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
   {
      pROT->Revoke(pdwRegister);
      SAFE_RELEASE(pROT);
   }
}

/******************************************************************************/

void 
DisconnectFilter(
   IGraphBuilder  *pGraphBuilder,
   IBaseFilter    *pSrcFilter)
{
   HRESULT hr;

   if(pGraphBuilder)
   {
      IPin *pP = 0, *pTo = 0;
      ULONG u;
      IEnumPins *pins = NULL;
      PIN_INFO pininfo;

      // Enumerate the source filter pins.
      hr = pSrcFilter->EnumPins(&pins);
      pins->Reset();
      while(hr == NOERROR)
      {
         hr = pins->Next(1, &pP, &u);
         if(hr == S_OK && pP)
         {
            pP->ConnectedTo(&pTo);
            if(pTo)
            {
                hr = pTo->QueryPinInfo(&pininfo);
                if(hr == NOERROR)
                {
                    if(pininfo.dir == PINDIR_INPUT)
                    {
                        // For all output pins connected to this input pin.
                        DisconnectFilter(pGraphBuilder, pininfo.pFilter);
                        hr = pGraphBuilder->Disconnect(pTo);
                        hr = pGraphBuilder->Disconnect(pP);
                    }
                    pininfo.pFilter->Release();
                }
                pTo->Release();
            }
            pP->Release();
         }
      }
      if(pins)
        pins->Release();
   }
}

/******************************************************************************/

void 
DisconnectAndRemoveFilter(
  IGraphBuilder  *pGraphBuilder,
  IBaseFilter    *pSrcFilter)
{
   HRESULT hr;
   
   if(pGraphBuilder)
   {
      IPin *pP = 0, *pTo = 0;
      ULONG u;
      IEnumPins *pins = NULL;
      PIN_INFO pininfo;
      
      // Enumerate the source filter pins.
      hr = pSrcFilter->EnumPins(&pins);
      pins->Reset();
      while(hr == NOERROR)
      {
         hr = pins->Next(1, &pP, &u);
         if(hr == S_OK && pP)
         {
            pP->ConnectedTo(&pTo);
            if(pTo)
            {
               hr = pTo->QueryPinInfo(&pininfo);
               if(hr == NOERROR)
               {
                  if(pininfo.dir == PINDIR_INPUT)
                  {
                     // For all output pins connected to this input pin.
                     DisconnectFilter(pGraphBuilder, pininfo.pFilter);
                     hr = pGraphBuilder->Disconnect(pTo);
                     hr = pGraphBuilder->Disconnect(pP);
                     hr = pGraphBuilder->RemoveFilter(pininfo.pFilter);
                  }
                  pininfo.pFilter->Release();
               }
               pTo->Release();
            }
            pP->Release();
         }
      }
      if(pins)
         pins->Release();
   }
}

/******************************************************************************/

HRESULT
RemoveGraphFilters (
   IFilterGraph *pGraph )
{
   IEnumFilters *pEnum = NULL;
   IBaseFilter *pFilter;
   ULONG cFetched;
   HRESULT hr;

   if (!pGraph)
      return E_POINTER;
   
   hr = pGraph->EnumFilters(&pEnum);
   if (FAILED(hr)) 
      return hr;

   pEnum->Reset();
   while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
   {
      // Remove the filter.
      hr = pGraph->RemoveFilter(pFilter);
      // Reset the enumerator.
      pEnum->Reset();
      pFilter->Release();
   }

   pEnum->Release();
   return hr;
}


/******************************************************************************/

HRESULT
RemoveGraphFiltersIfNotConnected (
   IFilterGraph *pGraph )
{
   IEnumFilters *pEnumFilters = NULL;
   IBaseFilter *pFilter;
   ULONG cFetched;
   HRESULT hr;

   if (!pGraph)
      return E_POINTER;
   
   hr = pGraph->EnumFilters(&pEnumFilters);
   if (FAILED(hr)) 
      return hr;

   pEnumFilters->Reset();
   while(pEnumFilters->Next(1, &pFilter, &cFetched) == S_OK)
   {
      IPin *pPin, *pPinConnected;
      IEnumPins *pEnumPins;
      BOOL bFilterConnected = FALSE;

      pFilter->EnumPins(&pEnumPins);
      hr = E_FAIL;
      pEnumPins->Reset(); 
      while (pEnumPins->Next(1, &pPin, NULL) == S_OK)
      {
         // Is the pin connected..
         if ((pPin->ConnectedTo(&pPinConnected) == S_OK ) && pPinConnected)
         {
            bFilterConnected = TRUE;
            SAFE_RELEASE(pPin);
            break;
         }
         SAFE_RELEASE(pPin);
      }
      pEnumPins->Release(); 

      if  (!bFilterConnected)
      {
         // Remove the filter.
         pGraph->RemoveFilter(pFilter);
         // Reset the enumerator.
         pEnumFilters->Reset();
      }
      pFilter->Release();
   }

   pEnumFilters->Release();
   return S_OK;
}

/******************************************************************************/

HRESULT 
GetGraphFilter(
   IFilterGraph *pGraph,
   LPCTSTR      pFilterName,
   IBaseFilter  **ppSrcFilter)
{
   HRESULT hr;
   IBaseFilter *pSrcFilter = NULL;

   if (!pGraph)
      return E_POINTER;
   
   // Query the filter graph for its source.
   hr = pGraph->FindFilterByName( pFilterName, &pSrcFilter );
   if(SUCCEEDED(hr))
   {
      *ppSrcFilter = pSrcFilter;
   }

   return hr;
}

/******************************************************************************/

HRESULT
LoadRendererQualPropInterface(
   ICaptureGraphBuilder2   *pBuilder,
   IBaseFilter             *pSrcFilter,
   IQualProp               **ppIQP)
{
   HRESULT hr;
   IQualProp *pIQP = NULL;

   hr = pBuilder->FindInterface(&LOOK_DOWNSTREAM_ONLY,NULL,pSrcFilter,
         IID_IQualProp,(void **)&pIQP);
   if(SUCCEEDED(hr))
   {
      *ppIQP = pIQP;
   }
   
   return hr;
}

/******************************************************************************/
