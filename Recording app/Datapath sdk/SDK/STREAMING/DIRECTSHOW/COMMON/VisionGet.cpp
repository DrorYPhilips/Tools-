/*******************************************************************************

Copyright Datapath Ltd. 2008, 2016.

File:    VisionGet.cpp

Purpose: 

History:
         17 JAN 09    RL   Created.
         23 SEP 11    JL   Added the GetDGCInfo function which fills out a 
                           DGCINFO structure containing information about a DGC
                           device associated with a moniker.
         26 APR 12    JL   Added support for DGC User-Mode video filters.
         12 SEP 12    JL   Added support for DGC audio filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>
#include <tchar.h>

#include <VisionGet.h>

typedef struct
{
   TCHAR FriendlyName[255];
   UINT  Input;
} DGCINFO, *PDGCINFO;

/******************************************************************************/

HRESULT
GetDGCInfo (
   IMoniker *pMoniker,
   PDGCINFO pDGCInfo )
{
   // Getting the property page to get the device name.
   IPropertyBag *pPropertyBag;

   HRESULT hr = pMoniker->BindToStorage ( 0, 0, IID_IPropertyBag, 
         (void **)&pPropertyBag );
   if ( SUCCEEDED(hr) )
   {
      VARIANT var;
      // Using type string.
      var.vt = VT_BSTR;

      hr = pPropertyBag->Read ( TEXT("FriendlyName"), &var, NULL );
      if ( hr == S_OK )
      {
         // Set out return value to a failure case; We only want to set
         // it to a success value if we fill out a full DGCINFO structure.
         hr = E_FAIL;

         // Fill out the FriendlyName element.
         if ( _sntprintf ( pDGCInfo->FriendlyName,
               sizeof(pDGCInfo->FriendlyName)/
               sizeof(pDGCInfo->FriendlyName[0]),
               var.bstrVal ) != 0 )
         {
            TCHAR tcInput[3];
            size_t length = _tcslen ( var.bstrVal );

            // get the number from the end of the friendly name
            if ( _sntprintf ( tcInput, sizeof(tcInput)/sizeof(tcInput[0]),
                  &var.bstrVal[length-2] ) != 0 )
            {
               if ( _istdigit(tcInput[0]) && _istdigit(tcInput[1]) )
               {
                  // Fill out the Input element.
                  pDGCInfo->Input = (tcInput[0] - TEXT('0')) *10;
                  pDGCInfo->Input += tcInput[1] - TEXT('0');

                  if ( pDGCInfo->Input > 0 )
                  {
                     // Only if we fill in the entire structure can we set
                     // hr to S_OK.
                     hr = S_OK;
                  }
               }
            }
         }

      }

      SAFE_RELEASE(pPropertyBag);
   }

   return hr;
}

/******************************************************************************/

HRESULT 
GetVisionSourceFilter(
   ULONG       uInput,
   IBaseFilter **ppSrcFilter)
{
   HRESULT        hr;
   IBaseFilter    *pSrc       = NULL;
   IMoniker       *pMoniker   = NULL;
   ICreateDevEnum *pDevEnum   = NULL;
   IEnumMoniker   *pClassEnum = NULL;
   ULONG          uFetched;

   if (!ppSrcFilter)
      return E_POINTER;

   // Enumerate system devices.
   hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
         IID_ICreateDevEnum, (void **) &pDevEnum);
   if (FAILED(hr))
     return hr;

   // Looking for video input devices only.
   hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, 
         &pClassEnum, 0);
   if (FAILED(hr))
   {
      SAFE_RELEASE(pDevEnum);
      return hr;
   }
   // If there are no enumerators for the requested type, then 
   // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
   if (pClassEnum == NULL)
   {
      SAFE_RELEASE(pDevEnum);
      return E_FAIL;
   }
   // Not zero based.
   uInput++;

   // Note that if the Next() call succeeds but there are no monikers,
   // it will return S_FALSE.
   while ( S_OK == ( hr = pClassEnum->Next ( 1, &pMoniker, &uFetched ) ) )
   {
      // Try binding to the users specified input.
      DGCINFO dgcInfo;
      hr = GetDGCInfo ( pMoniker, &dgcInfo );
      if ( hr == S_OK )
      {
         // We have a full DGCINFO structure.
         // Is this the input that the user specified?
         if ( dgcInfo.Input == uInput )
         {
            hr = pMoniker->BindToObject ( 0, 0, IID_IBaseFilter,
                  (void**)&pSrc );
            break;
         }
      }
   }

   if (!pSrc )
   {
      // We haven't bound to the device that the user asked for.
      // Try binding to input 1.
      pClassEnum->Reset();
      while ( S_OK == ( hr = pClassEnum->Next ( 1, &pMoniker, &uFetched ) ) )
      {
         DGCINFO dgcInfo;
         hr = GetDGCInfo ( pMoniker, &dgcInfo );
         if ( hr == S_OK )
         {
            // We have a full DGCINFO structure.
            // Is this input 1?
            if ( dgcInfo.Input == 1 )
            {
               hr = pMoniker->BindToObject ( 0, 0, IID_IBaseFilter,
                     (void**)&pSrc );
               break;
            }
         }
      }
   }

   if (!pSrc )
   {
      // We haven't bound to the device that the user asked for.
      // We haven't bound to input 1.
      // Try binding to the first DGC device we come across.
      pClassEnum->Reset();
      while ( S_OK == ( hr = pClassEnum->Next ( 1, &pMoniker, &uFetched ) ) )
      {
         DGCINFO dgcInfo;
         hr = GetDGCInfo ( pMoniker, &dgcInfo );
         if ( hr == S_OK )
         {
            // We have a full DGCINFO structure.
            // Bind to this moniker, as we know it is a DGC device.
            hr = pMoniker->BindToObject ( 0, 0, IID_IBaseFilter,
                  (void**)&pSrc );
            break;
         }
      }
   }

   if ( pSrc )
      *ppSrcFilter = pSrc;

   // Release resources.
   SAFE_RELEASE(pDevEnum);
   SAFE_RELEASE(pClassEnum);

   if (hr == S_FALSE)
      hr = E_FAIL;

   return hr;   
}

/******************************************************************************/

HRESULT 
GetVisionFriendlyName(
   ULONG uInput,
   TCHAR *pBuffer,
   ULONG uBufferLength )
{
   HRESULT hr;
   IMoniker *pMoniker = NULL;
   ICreateDevEnum *pDevEnum = NULL;
   IEnumMoniker *pClassEnum = NULL;
   ULONG uFetched;

   // Enumerate system devices.
   hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
         IID_ICreateDevEnum, (void **) &pDevEnum);
   if (FAILED(hr))
     return hr;

   // Looking for video input devices only.
   hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, 
         &pClassEnum, 0);
   if (FAILED(hr))
     return hr;

   // If there are no enumerators for the requested type, then 
   // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
   if (pClassEnum == NULL)
      return E_FAIL;

   // Not zero based.
   uInput++;

   // Note that if the Next() call succeeds but there are no monikers,
   // it will return S_FALSE.
   while ( S_OK == ( hr = pClassEnum->Next ( 1, &pMoniker, &uFetched ) ) )
   {
      DGCINFO dgcInfo;
      hr = GetDGCInfo ( pMoniker, &dgcInfo );
      if ( hr == S_OK )
      {
         // We have a full DGCINFO structure.
         // Is this the input that the user specified?
         if ( dgcInfo.Input == uInput )
         {
            _sntprintf ( pBuffer, uBufferLength, TEXT("%s"),
                  dgcInfo.FriendlyName );
            SAFE_RELEASE(pMoniker);
            break;
         }
      }
      SAFE_RELEASE(pMoniker);
   }

   // Release resources.
   SAFE_RELEASE(pDevEnum);
   SAFE_RELEASE(pClassEnum);

   if (hr == S_FALSE)
      hr = E_FAIL;

   return hr;   
}

/******************************************************************************/

HRESULT 
GetAudioSourceFilter(
   ULONG       uInput,
   IBaseFilter **ppSrcFilter)
{
   HRESULT hr;
   IBaseFilter *pSrc = NULL;
   IMoniker *pMoniker = NULL;
   ICreateDevEnum *pDevEnum = NULL;
   IEnumMoniker *pClassEnum = NULL;
   ULONG uFetched;

   if (!ppSrcFilter)
      return E_POINTER;

   // Enumerate system devices.
   hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
         IID_ICreateDevEnum, (void **) &pDevEnum);
   if (FAILED(hr))
     return hr;

   // Looking for video input devices only.
   hr = pDevEnum->CreateClassEnumerator (CLSID_AudioInputDeviceCategory, 
         &pClassEnum, 0);
   if (FAILED(hr))
     return hr;

   // If there are no enumerators for the requested type, then 
   // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
   if (pClassEnum == NULL)
      return E_FAIL;

   // Not zero based.
   uInput++;

   // Note that if the Next() call succeeds but there are no monikers,
   // it will return S_FALSE.
   while(S_OK == (hr = pClassEnum->Next( 1, &pMoniker, &uFetched ) ))
   {
      // Try binding to the users specified input.
      DGCINFO dgcInfo;
      hr = GetDGCInfo ( pMoniker, &dgcInfo );
      if ( hr == S_OK )
      {
         // We have a full DGCINFO structure.
         // Is this the input that the user specified?
         if ( dgcInfo.Input == uInput )
         {
            hr = pMoniker->BindToObject ( 0, 0, IID_IBaseFilter,
                  (void**)&pSrc );
            SAFE_RELEASE(pMoniker);
            break;
         }
      }
      SAFE_RELEASE(pMoniker);
   }

#if DGC_ONLY
   if ( !pSrc )
   {
      // If we specified DGC_ONLY but couldn't find an audio source, try to
      // load a non DGC audio source filter.
      pClassEnum->Reset();
      while(S_OK == (hr = pClassEnum->Next( 1, &pMoniker, &uFetched ) ))
      {
         // Getting the property page to get the device name.
         IPropertyBag *pPropertyBag;
         hr = pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, 
               (void **)&pPropertyBag );
         if( SUCCEEDED(hr) )
         {
            hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
            if( SUCCEEDED(hr) )
            {
               // Copy the found filter pointer to the output parameter.
               // Do NOT Release() the reference, since it will still be used
               // by the calling function.
               SAFE_RELEASE(pPropertyBag);
               SAFE_RELEASE(pMoniker);
               break;
            }
            SAFE_RELEASE(pPropertyBag);
         }
         SAFE_RELEASE(pMoniker);
      }
   }
#endif // if DGC_ONLY

   if ( pSrc )
      *ppSrcFilter = pSrc;

   // Release resources.
   SAFE_RELEASE(pDevEnum);
   SAFE_RELEASE(pClassEnum);

   if (hr == S_FALSE)
      hr = E_FAIL;

   return hr;   
}

/******************************************************************************/