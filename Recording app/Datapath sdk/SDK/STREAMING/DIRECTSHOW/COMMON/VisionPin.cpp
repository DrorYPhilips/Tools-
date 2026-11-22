/*******************************************************************************

Copyright Datapath Ltd. 2009, 2016.

File:    VisionPin.cpp

Purpose: 

History:
         31 JAN 09    RL   Created.
         06 OCT 09    RL   Removed the requirement for a 'PinID'.
         06 OCT 09    RL   Use 'DGC133ST.h' instead of 
                           '#import <DGC133ST.tlb> no_namespace' for 
                           streaming plugin interface declarations.
         06 OCT 09    RL   Accomodated functions for VIDEOINFOHEADER2.
         06 OCT 09    RL   Added GetDetectMediaType and SetVideo2MediaType.
         26 APR 12    JL   Added support for User-Mode filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>
#include <dvdmedia.h>

#include <DGC133ST.h>

#include <VisionFilter.h>
#include <VisionPin.h>

/* Numer of 100ns ticks in one second. */
#define ONE_SECOND   10000000

/******************************************************************************/

HRESULT
GetSourcePinInterface(
   IBaseFilter    *pSrcFilter,
   IPin           **ppPin,
   LPCTSTR        pPinName)
{
   IPin *pPin;
   IEnumPins *pEnum;
   HRESULT hr;

   if(!pSrcFilter)
      return E_POINTER;

   hr = pSrcFilter->EnumPins(&pEnum);
   if (FAILED(hr))
      return hr;

   hr = E_FAIL;
   pEnum->Reset(); 
   while (pEnum->Next(1, &pPin, NULL) == S_OK)
   {
      PIN_INFO Info;
      // Capture or preview pin.
      hr = pPin->QueryPinInfo(&Info);
      if (SUCCEEDED(hr))
      {
         Info.pFilter->Release();
         hr = E_FAIL;
         if (_tcsicmp (Info.achName, pPinName) == 0)
         {
            *ppPin = pPin;
            hr = S_OK;
            break;
         }
      }
      SAFE_RELEASE(pPin);
   }
   SAFE_RELEASE(pEnum);
   return hr;
}

/******************************************************************************/

HRESULT
GetCurrentMediaType(
   IBaseFilter    *pSrcFilter,
   AM_MEDIA_TYPE  **ppmt,
   LPCTSTR        pPinName)
{
   HRESULT hr;
   IPin *pPin;
   IAMStreamConfig *pIAMS = NULL;

   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;

   // Return the pins streaming interface.
   hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pIAMS);
   if (SUCCEEDED(hr))
   {
      AM_MEDIA_TYPE *pmt = NULL;
      // Get the current format.
      hr = pIAMS->GetFormat( &pmt );
      if(SUCCEEDED(hr))
      {
         *ppmt = pmt;
      }
      SAFE_RELEASE(pIAMS);
   }
   SAFE_RELEASE(pPin);
   return hr;
}

/******************************************************************************/

HRESULT
SetPinMediaType(
   IBaseFilter    *pSrcFilter,
   AM_MEDIA_TYPE  *pmt,
   LPCTSTR        pPinName)
{
   HRESULT hr;
   IPin *pPin;
   IAMStreamConfig *pIAMS = NULL;
   
   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;
   
   // Return the pins streaming interface.
   hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pIAMS);
   if (SUCCEEDED(hr))
   {
      // Set the current format.
      hr = pIAMS->SetFormat(pmt);
      SAFE_RELEASE(pIAMS);
   }
   SAFE_RELEASE(pPin);
   return hr;
}

/******************************************************************************/

HRESULT
SetBufferSize( 
   LONG          width,
   LONG          height,
   AM_MEDIA_TYPE  *pAMT)
{
   if (pAMT->formattype == FORMAT_VideoInfo)
   {
      VIDEOINFOHEADER *pVIH;
      // Set the data format.
      pVIH = (VIDEOINFOHEADER*)pAMT->pbFormat;
      pVIH->bmiHeader.biWidth = width;
      pVIH->bmiHeader.biHeight = height;
      pVIH->bmiHeader.biSizeImage = 
            width * abs(height) * (pVIH->bmiHeader.biBitCount/8);
      pAMT->lSampleSize = pVIH->bmiHeader.biSizeImage;
   }
   else if (pAMT->formattype == FORMAT_VideoInfo2)
   {
      VIDEOINFOHEADER2 *pVIH2;
      // Set the data format.
      pVIH2 = (VIDEOINFOHEADER2*)pAMT->pbFormat;
      pVIH2->bmiHeader.biWidth = width;
      pVIH2->bmiHeader.biHeight = height;
      pVIH2->dwPictAspectRatioX = width;
      pVIH2->dwPictAspectRatioY = height;
      pVIH2->bmiHeader.biSizeImage = 
            width * abs(height) * (pVIH2->bmiHeader.biBitCount/8);
      pAMT->lSampleSize = pVIH2->bmiHeader.biSizeImage;
   }
   else
   {
      return E_UNEXPECTED;
   }

   return S_OK;
}

/******************************************************************************/

HRESULT
SetFrameRate(
   ULONG          frameRate,
   AM_MEDIA_TYPE  *pAMT)
{
   if (pAMT->formattype == FORMAT_VideoInfo)
   {
      VIDEOINFOHEADER *pVIH;
      // Set the capture interval.
      pVIH = (VIDEOINFOHEADER*)pAMT->pbFormat;
      pVIH->AvgTimePerFrame = (int)(ONE_SECOND / (frameRate+(float)0.0005));
   }
   else if (pAMT->formattype == FORMAT_VideoInfo2)
   {
      VIDEOINFOHEADER2 *pVIH2;
      // Set the capture interval.
      pVIH2 = (VIDEOINFOHEADER2*)pAMT->pbFormat;
      pVIH2->AvgTimePerFrame = (int)(ONE_SECOND / (frameRate+(float)0.0005));
   }
   else
   {
      return E_UNEXPECTED;
   }

   return S_OK;
}

/******************************************************************************/

HRESULT
SetColourSpace(
   IBaseFilter    *pSrcFilter,
   LPCTSTR        pPinName,
   const GUID     *pColourSpace,
   AM_MEDIA_TYPE  **ppAMT)
{
   HRESULT hr = S_OK;
   IPin *pPin;
   IAMStreamConfig *pIAMS = NULL;

   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;

   // Return the pins streaming interface.
   hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pIAMS);
   if (SUCCEEDED(hr))
   {
      AM_MEDIA_TYPE  *pmt;
      VIDEO_STREAM_CONFIG_CAPS scc;
      int size = sizeof(VIDEO_STREAM_CONFIG_CAPS);
      int count;

      pIAMS->GetNumberOfCapabilities ( &count, &size );
      while ( ( hr = pIAMS->GetStreamCaps( count-1, &pmt, (BYTE*)&scc ) ) == S_OK )
      {
         // Check for the selected colourspace and video format.
         if ( 
            (memcmp(&pmt->subtype, pColourSpace, sizeof(GUID)) == 0) &&
            (memcmp(&pmt->formattype, &(*ppAMT)->formattype, sizeof(GUID)) == 0)
            )
         {
            *ppAMT = pmt;
            break;
         }
         DeleteMediaType(pmt);
         count--;
      }

      if (hr == S_FALSE)
         hr = E_FAIL;

      SAFE_RELEASE(pIAMS);
   }

   SAFE_RELEASE(pPin);

   return hr;
}

/******************************************************************************/

HRESULT
SetVideo2MediaType(
   IBaseFilter    *pSrcFilter,
   unsigned long  width,
   unsigned long  height,
   unsigned long  fps,
   GUID           *pColourSpace,
   LPCTSTR        pPinName)
{
   HRESULT hr;
   IPin *pPin;
   IAMStreamConfig *pIAMS = NULL;
   
   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;

   // Return the pins streaming interface.
   hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pIAMS);
   if (SUCCEEDED(hr))
   {
      AM_MEDIA_TYPE  *pmt;
      VIDEO_STREAM_CONFIG_CAPS scc;
      int size = sizeof(VIDEO_STREAM_CONFIG_CAPS);
      int count;
      
      pIAMS->GetNumberOfCapabilities ( &count, &size );
      while ( ( hr = pIAMS->GetStreamCaps( count-1, &pmt, (BYTE*)&scc ) ) == S_OK )
      {
         if ( memcmp(&pmt->subtype, pColourSpace, sizeof(GUID)) == 0)
         {
            if (pmt->formattype == FORMAT_VideoInfo2)
            {
               if (fps)
                  SetFrameRate(fps,pmt);
               if (width && height)
                  SetBufferSize(width,height,pmt);
               hr = pIAMS->SetFormat(pmt);
               break;
            }
         }
         DeleteMediaType(pmt);
         count--;
      }

      if (hr == S_FALSE)
         hr = E_FAIL;

      SAFE_RELEASE(pIAMS);
   }
   SAFE_RELEASE(pPin);
   
   return hr;
}

/******************************************************************************/

HRESULT
PinPropertiesDlg(
   HWND        hWnd,
   IBaseFilter *pSrcFilter,
   LPCTSTR     pPinName)
{
   HRESULT hr;
   CAUUID cauuid;
   IAMStreamConfig *pIAMS = NULL;
   ISpecifyPropertyPages *pPropertPage;
   IPin *pPin;

   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;

   // Return the pins streaming interface.
   hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pIAMS);
   if(FAILED(hr))
      return hr;

   // Find an interface to query.
   hr = pIAMS->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pPropertPage);
   if(FAILED(hr))
      return hr;

   // Return its registered property page(s).
   hr = pPropertPage->GetPages(&cauuid);
   if (FAILED(hr))
      return hr;

   // Launch the property page.
   hr = OleCreatePropertyFrame(hWnd, 30, 30, NULL, 1,
         (IUnknown **)&pIAMS, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);
   if (FAILED(hr))
      return hr;

   // Free property page resources.
   CoTaskMemFree(cauuid.pElems);
   SAFE_RELEASE(pPin);
   SAFE_RELEASE(pIAMS);
   SAFE_RELEASE(pPropertPage);

   return hr;
}

/******************************************************************************/

HRESULT
LoadDroppedFramesInterface(
   IBaseFilter          *pSrcFilter,
   LPCTSTR              pPinName,
   IAMDroppedFrames     **ppIAMDF)
{
   HRESULT hr;
   IPin *pPin;
   IAMDroppedFrames *pIAMDF = NULL;

   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;

   // Return the pins dropped frames interface.
   hr = pPin->QueryInterface(IID_IAMDroppedFrames, (void **)&pIAMDF);
   if (SUCCEEDED(hr))
   {
      *ppIAMDF = pIAMDF;
   }
   SAFE_RELEASE(pPin);
   return hr;
}

/******************************************************************************/

HRESULT
GetLiveStreamPin(
   IBaseFilter       *pSrcFilter,
   LPCTSTR           pPinName,
   LIVESTREAM_CLIENT *pLiveStreamClient)
{
   HRESULT     hr;
   IPin        *pPin = NULL;
   IVisionPin  *pIVisionPin = NULL;

   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;

   hr = pPin->QueryInterface(__uuidof(IVisionPin),
                             reinterpret_cast<void**>(&pIVisionPin));
   if (FAILED(hr))
   {
      // IVisionUserPin inherits from IVisionPin
      hr = pPin->QueryInterface(__uuidof(IVisionUserPin),
                                reinterpret_cast<void**>(&pIVisionPin));
   }
   if (SUCCEEDED(hr)) 
   {
      hr = pIVisionPin->get_LiveStream(pLiveStreamClient);
      SAFE_RELEASE(pIVisionPin);
   }
   
   SAFE_RELEASE(pPin);
   return hr;
}

/******************************************************************************/

HRESULT
SetLiveStreamPin (
   IBaseFilter       *pSrcFilter,
   LPCTSTR           pPinName,
   LIVESTREAM_CLIENT LiveStreamClient)
{
   HRESULT     hr;
   IPin        *pPin = NULL;
   IVisionPin  *pIVisionPin = NULL;

   hr = GetSourcePinInterface(pSrcFilter, &pPin, pPinName);
   if (FAILED(hr))
      return hr;

   hr = pPin->QueryInterface(__uuidof(IVisionPin),
                             reinterpret_cast<void**>(&pIVisionPin));
   if (FAILED(hr))
   {
      // IVisionUserPin inherits from IVisionPin
      hr = pPin->QueryInterface(__uuidof(IVisionUserPin),
                                reinterpret_cast<void**>(&pIVisionPin));
   }
   if (SUCCEEDED(hr)) 
   {
      hr = pIVisionPin->put_LiveStream(LiveStreamClient);
      SAFE_RELEASE(pIVisionPin);
   }
   
   SAFE_RELEASE(pPin);
   return hr;
}

/******************************************************************************/
