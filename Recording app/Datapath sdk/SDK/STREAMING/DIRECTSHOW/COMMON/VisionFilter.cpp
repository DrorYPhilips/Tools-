/*******************************************************************************

Copyright Datapath Ltd. 2009, 2017.

File:    VisionFilter.cpp

Purpose: 

History:
         31 JAN 09    RL   Created.
         06 OCT 09    RL   Use 'DGC133ST.h' instead of 
                           '#import <DGC133ST.tlb> no_namespace' for 
                           streaming plugin interface declarations.
         23 DEC 09    RL   Added LoadVisionEventInterface.
         26 APR 12    JL   Added support for User-Mode filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.
         21 JUL 17    SB   Added IVisionUser2.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>

#include <DGC133ST.h>

#include <VisionFilter.h>

/******************************************************************************/

HRESULT
LoadVisionInterface(
   IBaseFilter*   pSrcFilter,
   IVisionUser**  ppVisionUser)
{
   HRESULT        hr;
   IVisionUser*   pIVisionUser = NULL;

   hr = pSrcFilter->QueryInterface(__uuidof(IVisionUser),
          reinterpret_cast<void**>(&pIVisionUser) );
   if (SUCCEEDED(hr))
   {
      *ppVisionUser = pIVisionUser;
   }

   return hr;
}

/******************************************************************************/

HRESULT
LoadVisionInterface(
   IBaseFilter*   pSrcFilter,
   IVisionUser2**  ppVisionUser)
{
   HRESULT        hr;
   IVisionUser2*   pIVisionUser = NULL;

   hr = pSrcFilter->QueryInterface(__uuidof(IVisionUser2),
      reinterpret_cast<void**>(&pIVisionUser));
   if (SUCCEEDED(hr))
   {
      *ppVisionUser = pIVisionUser;
   }

   return hr;
}

/******************************************************************************/

HRESULT
LoadVisionGangingInterface(
   IBaseFilter*   pSrcFilter,
   IVisionUserGanging** ppVisionGanging )
{
   HRESULT        hr;
   IVisionUserGanging*   pIVisionGanging = NULL;

   hr = pSrcFilter->QueryInterface(__uuidof(IVisionUserGanging),
          reinterpret_cast<void**>(&pIVisionGanging) );
   if (SUCCEEDED(hr))
   {
      *ppVisionGanging = pIVisionGanging;
   }

   return hr;
}

/******************************************************************************/

HRESULT
LoadVisionOverlayInterface(
   IBaseFilter*   pSrcFilter,
   IVisionUserOverlay** ppVisionOverlay )
{
   HRESULT        hr;
   IVisionUserOverlay*   pIVisionOverlay = NULL;

   hr = pSrcFilter->QueryInterface(__uuidof(IVisionUserOverlay),
          reinterpret_cast<void**>(&pIVisionOverlay) );
   if (SUCCEEDED(hr))
   {
      *ppVisionOverlay = pIVisionOverlay;
   }

   return hr;
}

/******************************************************************************/

HRESULT
LoadControlConfigInterface(
   void*   pSrcFilter,
   void**  ppControlConfig)
{
   UNREFERENCED_PARAMETER(pSrcFilter);
   UNREFERENCED_PARAMETER(ppControlConfig);
   return E_NOTIMPL;
}

/******************************************************************************/

HRESULT
LoadCrossbarInterface(
   void*  pCapGraph2,
   void*  pSrcFilter,
   void** ppCrossbar)
{
   UNREFERENCED_PARAMETER(pCapGraph2);
   UNREFERENCED_PARAMETER(pSrcFilter);
   UNREFERENCED_PARAMETER(ppCrossbar);
   return E_NOTIMPL;
}

/******************************************************************************/

HRESULT
LoadVisionEventInterface(
    IBaseFilter*       pSrcFilter,
    IVisionUserEvent** ppVisionUserEvent)
{
   HRESULT            hr;
   IVisionUserEvent*  pVisionUserEvent = NULL;

   hr = pSrcFilter->QueryInterface(__uuidof(IVisionUserEvent),
      reinterpret_cast<void**>(&pVisionUserEvent) );
   if (SUCCEEDED(hr))
   {
      *ppVisionUserEvent = pVisionUserEvent;
   }

   return hr;
}

/******************************************************************************/

HRESULT
LoadVisionInterface(
   IBaseFilter*   pSrcFilter,
   IVision**      ppVision)
{
   HRESULT        hr;
   IVision*       pIVision = NULL;

   hr = pSrcFilter->QueryInterface(__uuidof(IVision),
          reinterpret_cast<void**>(&pIVision) );
   if (SUCCEEDED(hr))
   {
      *ppVision = pIVision;
   }
   else
   {
      // IVisionUser inherits from IVision
      hr = LoadVisionInterface(pSrcFilter,
          reinterpret_cast<IVisionUser**>(ppVision));
   }

   return hr;
}

/******************************************************************************/

/*
  The IDGC103ControlConfig interface is provided for backward compatability 
  with DGC103 devices. For new applications please use the 'IVision' interface.
*/

HRESULT
LoadControlConfigInterface(
   IBaseFilter*            pSrcFilter,
   IDGC103ControlConfig**  ppControlConfig)
{
   HRESULT                 hr;
   IDGC103ControlConfig*   pControlConfig = NULL;
   
   hr = pSrcFilter->QueryInterface(__uuidof(IDGC103ControlConfig),
         reinterpret_cast<void**>(&pControlConfig) );
   if (SUCCEEDED(hr))
   {
      *ppControlConfig = pControlConfig;
   }
   
   return hr;
}

/******************************************************************************/

HRESULT
LoadCrossbarInterface(
   ICaptureGraphBuilder2*  pCapGraph2,
   IBaseFilter*            pSrcFilter,
   IAMCrossbar**           ppCrossbar)
{
   HRESULT        hr;
   IAMCrossbar*   pCrossbar = NULL;
   
   hr = pCapGraph2->FindInterface(&LOOK_UPSTREAM_ONLY, NULL,
         pSrcFilter, IID_IAMCrossbar, reinterpret_cast<void**>(&pCrossbar) );
   if (SUCCEEDED(hr))
   {
      *ppCrossbar = pCrossbar;
   }
   
   return hr;
}

/******************************************************************************/

HRESULT
LoadVisionEventInterface(
    IBaseFilter*   pSrcFilter,
    IVisionEvent** ppVisionEvent)
{
   HRESULT        hr;
   IVisionEvent*  pVisionEvent = NULL;
   
   hr = pSrcFilter->QueryInterface(__uuidof(IVisionEvent),
      reinterpret_cast<void**>(&pVisionEvent) );
   if (SUCCEEDED(hr))
   {
      *ppVisionEvent = pVisionEvent;
   }
   
   return hr;
}

/******************************************************************************/

HRESULT
LoadVisionEventInterface(
    IBaseFilter*    pSrcFilter,
    IVisionEvent2** ppVisionEvent)
{
   HRESULT        hr;
   IVisionEvent2* pVisionEvent = NULL;
   
   hr = pSrcFilter->QueryInterface(__uuidof(IVisionEvent2),
      reinterpret_cast<void**>(&pVisionEvent) );
   if (SUCCEEDED(hr))
   {
      *ppVisionEvent = pVisionEvent;
   }
   else
   {
      // IVisionUserEvent inherits from IVisionEvent2
      hr = LoadVisionEventInterface(pSrcFilter,
         reinterpret_cast<IVisionUserEvent**>(ppVisionEvent));
   }

   return hr;
}

/******************************************************************************/

HRESULT
FilterPropertiesDlg(
   HWND        hWnd,
   IBaseFilter *pSrcFilter)
{
   HRESULT hr;
   CAUUID cauuid;
   ISpecifyPropertyPages *pPropertyPage;

   if(!pSrcFilter)
      return E_POINTER;

   // Query the source filter directly for property pages.
   hr = pSrcFilter->QueryInterface(IID_ISpecifyPropertyPages,
      (void **)&pPropertyPage);
   if(hr == S_OK)
   {
      // Return its registered property page(s).
      hr = pPropertyPage->GetPages(&cauuid);
      if(hr == S_OK)
      {
         // Launch the property page.
         hr = OleCreatePropertyFrame(hWnd, 30, 30, NULL, 1,
               (IUnknown **)&pSrcFilter, cauuid.cElems,
               (GUID *)cauuid.pElems, 0, 0, NULL);

         CoTaskMemFree(cauuid.pElems);
      }
      SAFE_RELEASE(pPropertyPage);
   }
   return hr;
}

/******************************************************************************/