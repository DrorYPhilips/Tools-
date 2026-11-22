/*******************************************************************************

Copyright Datapath Ltd. 2012, 2013.

File:    AUDIOCAP.C

Purpose: WASAPI shared mode audio capture. 
         Minimum supported OS Windows: Vista.

History:
         17 NOV 12    RL   Created.
         20 FEB 13    RL   Use RGBAudioLoadOutputBuffer instead of 
                           RGBAudioChainOutputBuffer

*******************************************************************************/
/*
 * Further details on the Windows Audio Session API (WASAPI) can be found at 
 * the following URL:
 http://msdn.microsoft.com/en-us/library/windows/desktop/dd316756%28v=vs.85%29.aspx
*/

/******************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <strmif.h>
#include <stdio.h>
#include <stddef.h>
#include <tchar.h>
#include <strsafe.h>

#include <Avrt.h>
#include <Audioclient.h>
#include <mmreg.h>
#include <Mmdeviceapi.h>

#include <audio.h>
#include <audioAPI.h>
#include <audiocap.h>

#include <api.h>
#include <rgb.h>
#include <rgbapi.h>
#include <rgberror.h>

/******************************************************************************/

#define EXIT_ON_ERROR(hres)  \
              if (hres) { goto Exit; }
#define BREAK_ON_ERROR(hres)  \
              if (hres) { DebugBreak(); }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->lpVtbl->Release(punk); (punk) = NULL; }

static const CLSID CLSID_MMDeviceEnumerator =  
   { 0xbcde0395, 0xe52f, 0x467c, 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e };

static const IID IID_IMMDeviceEnumerator =  
   { 0xa95664d2, 0x9614, 0x4f35, 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6 };

static const IID IID_IAudioClient =  
   { 0x1cb9ad4c, 0xdbfa, 0x4c32, 0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2 };

static const IID IID_IAudioRenderClient =  
   { 0xf294acfc, 0x3146, 0x4483, 0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2 };

static const GUID __KSDATAFORMAT_SUBTYPE_PCM = 
   { 0x0001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

/******************************************************************************/

/* REFERENCE_TIME time units per millisecond */
#define REFTIMES_PER_MILLISEC  10000
/* Setting AUDIO_WAIT_FOR_DATA to non-zero using the WASAPI interface may cause 
 * audio breaks during prolonged streaming durations. */
#define AUDIO_WAIT_FOR_DATA 0

/* Number of 100ns ticks in one second. */
#define ONE_SECOND   10000000

/******************************************************************************/

typedef struct
{
   /* Thread parms */
   HANDLE               HThread;
   HANDLE               HStartEvent;
   DWORD                DWThreadID;
   HRESULT              Error;
   /* External parms */
   ULONG                Input;
   /* Internal parms */
   BOOL                 BClose;
   HAUDIO               HAudio;
#if AUDIO_WAIT_FOR_DATA
   HANDLE               HBitsInList;
#endif
   ULONG                FormatCount;
   ULONG                FormatIndex;
   PAUDIOCAPS           PFormats;
   WAVEFORMATEXTENSIBLE WaveEx;
   REFERENCE_TIME       BufferDuration;
   ULONG                BufferSize;
   AUDIOCAPTURESTATE    State;
   ULONG                Event;
} AUDIOPARMS, *PAUDIOPARMS;

/******************************************************************************/

ULONG
GetAllocationUnit (
   ULONG avgBytesPerSec, 
   LONGLONG latency ) 
{
   return (ULONG)((avgBytesPerSec * latency) / ONE_SECOND);
}

/******************************************************************************/

LONGLONG GetLatency (ULONGLONG avgBytesPerSec, ULONGLONG allocUnit) 
{
   LONGLONG latency;

   latency = (ONE_SECOND * allocUnit) / avgBytesPerSec;

   return latency ? latency : 1;
}

/******************************************************************************/
#if AUDIO_WAIT_FOR_DATA
void RGBCBKAPI
AudioCapturedFn (
   HAUDIO      hAudio,
   PAUDIODATA  pAudioData,
   ULONG_PTR   pUserData )
{
   /* pAudioData->StartTime and pAudioData->EndTime are not used by WASAPI. */
   PAUDIOPARMS pAudioParms = (PAUDIOPARMS)pUserData;
   SetEvent ( pAudioParms->HBitsInList );
}
#endif

/******************************************************************************/

HRESULT 
StartCapture (
   PAUDIOPARMS pAudioParms )
{
   ULONG error = RGBERROR_ILLEGAL_CALL;

   if ( pAudioParms && pAudioParms->State != ACQUIRE && pAudioParms->HAudio )
   {
#if AUDIO_WAIT_FOR_DATA
      ResetEvent ( pAudioParms->HBitsInList );
#endif
      error = RGBAudioSetCapabilities ( pAudioParms->Input, 
            pAudioParms->FormatIndex );
      if ( error == RGBERROR_NO_ERROR )
      {
         error = RGBAudioSetState ( pAudioParms->HAudio, ACQUIRE );
         if ( error == RGBERROR_NO_ERROR )
         {
            pAudioParms->State = ACQUIRE;
         }
      }
   }
   
   if ( error )
      return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, error);
   else
      return S_OK;
}

/*******************************************************************************/

HRESULT 
StopCapture (PAUDIOPARMS pAudioParms)
{
   ULONG error = RGBERROR_ILLEGAL_CALL;

   if ( pAudioParms && pAudioParms->State != STOP && pAudioParms->HAudio )
   {
      pAudioParms->BClose = TRUE;
      pAudioParms->State = STOP;

      error = RGBAudioSetState ( pAudioParms->HAudio, STOP );
      if ( error == RGBERROR_NO_ERROR )
      {
         error = RGBAudioReleaseOutputBuffers ( pAudioParms->HAudio );
      }
   }

   if ( error )
      return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, error);
   else
      return S_OK;
}

/*******************************************************************************/

HRESULT
InitializeFormat( PAUDIOPARMS pAudioParms, IAudioClient *pAudioClient )
{
   HRESULT hr = E_FAIL;
   WAVEFORMATEXTENSIBLE  *pActual = NULL;
   WAVEFORMATEXTENSIBLE  test;
   ULONG i;

   for ( i=0; i<pAudioParms->FormatCount; i++ )
   {
      if (pActual)
         CoTaskMemFree(pActual);

      /* Loop the list of formats from RGBEasy to find the best suited to
         the shered memory endpoint format. */
      test.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
      test.Format.nChannels = pAudioParms->PFormats[i].Channels;
      test.Format.nSamplesPerSec = pAudioParms->PFormats[i].SamplesPerSec;
      test.Format.nBlockAlign = pAudioParms->PFormats[i].Channels * 
            (pAudioParms->PFormats[i].SampleDepth / 8);
      test.Format.nAvgBytesPerSec = pAudioParms->PFormats[i].SamplesPerSec * 
            test.Format.nBlockAlign;
      test.Format.wBitsPerSample = pAudioParms->PFormats[i].SampleDepth;
      test.Format.cbSize = 22;
      test.Samples.wValidBitsPerSample =  pAudioParms->PFormats[i].SampleDepth;
      test.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
      test.SubFormat = __KSDATAFORMAT_SUBTYPE_PCM;

      pAudioParms->BufferDuration = GetLatency ( test.Format.nAvgBytesPerSec,  
            pAudioParms->BufferSize );
      /* Does the enpoint support this format. */
      if (pAudioClient->lpVtbl->IsFormatSupported ( pAudioClient,
            AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX*)&test, 
            &(WAVEFORMATEX*)pActual ) != S_OK )
      {
         /* Use the 'Advanced' tab in the speaker properties dialogue to set
          * the prefered sample rate and bit depth. */
         continue;
      }
      /* Initialize the endpoint. */
      hr = pAudioClient->lpVtbl->Initialize ( pAudioClient, AUDCLNT_SHAREMODE_SHARED,
            pAudioParms->Event, pAudioParms->BufferDuration,	0,	(WAVEFORMATEX*)&test, NULL );
      if (FAILED(hr))
		   continue;
      else
         break;
   }

   if (pActual)
       CoTaskMemFree ( pActual );

   if (SUCCEEDED(hr))
   {
      pAudioParms->WaveEx = test;
      pAudioParms->FormatIndex = i;
   }

   return hr;
}

/******************************************************************************/

HRESULT 
Initialize( 
   PAUDIOPARMS pAudioParms, 
   IAudioClient *pAudioClient )
{
   ULONG error;
#if AUDIO_WAIT_FOR_DATA
   PAUDIOCAPTUREDFN pAudioCaptureFn = AudioCapturedFn;
#else
   PAUDIOCAPTUREDFN pAudioCaptureFn = NULL;
#endif

   if ( !pAudioParms )
      return E_FAIL;

   /* Initialise the RGBEasy capture. */
   error = RGBAudioOpenInput ( pAudioCaptureFn, (ULONG_PTR)pAudioParms, 
         pAudioParms->Input, &pAudioParms->HAudio );
   if ( error == RGBERROR_NO_ERROR )
   {
      error = RGBAudioGetCapabilitiesCount ( pAudioParms->Input, 
            &pAudioParms->FormatCount );
      if ( error == RGBERROR_NO_ERROR )
      {
         pAudioParms->PFormats = (PAUDIOCAPS) malloc ( 
               pAudioParms->FormatCount * sizeof(AUDIOCAPS) );
         if ( pAudioParms->PFormats )
         {
            ULONG i;
            /* Store RGBEasy supported formats. */
            for ( i = 0; i < pAudioParms->FormatCount; i++ )
            {
               pAudioParms->PFormats[i].Size = sizeof(AUDIOCAPS);
               RGBAudioGetCapabilities ( pAudioParms->Input, i, 
                     &pAudioParms->PFormats[i] );
            }
#if AUDIO_WAIT_FOR_DATA
            /* Create event used to signal a frame captured callback. */
            pAudioParms->HBitsInList = CreateEvent ( NULL, FALSE, FALSE, NULL );
#endif
            return InitializeFormat ( pAudioParms, pAudioClient );
         }
      }
   }

   return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, error);
}

/******************************************************************************/

HRESULT
UnInitialize ( 
   PAUDIOPARMS pAudioParms )
{
   /* Stop the RGBEasy capture client. */
   HRESULT hr = StopCapture ( pAudioParms );
   if (SUCCEEDED(hr))
   {
      RGBAudioCloseInput ( pAudioParms->HAudio );
      pAudioParms->HAudio = 0;

      if (pAudioParms->PFormats)
         free(pAudioParms->PFormats);
      pAudioParms->PFormats = 0;

#if AUDIO_WAIT_FOR_DATA
      if (pAudioParms->HBitsInList)
         CloseHandle(pAudioParms->HBitsInList);
      pAudioParms->HBitsInList = 0;
#endif
   }

   return hr;
}

/******************************************************************************/

HRESULT 
LoadData( PAUDIOPARMS pAudioParms, UINT32 size, BYTE *pData )
{
   ULONG error = RGBERROR_ILLEGAL_CALL;

   if ( pAudioParms && pAudioParms->State == ACQUIRE && size && pData )
   {
#if AUDIO_WAIT_FOR_DATA
      /* Pass pData and size to the RGBEasy driver. */
      error = RGBAudioChainOutputBuffer ( pAudioParms->HAudio, size, 0, pData );
      if ( error == RGBERROR_NO_ERROR )
      {
         /* If the output clock for the DAC is faster than the capture clock
            it is possible that RGBEasy will wait for the frame duration
            before releasing the buffer. */
         DWORD dwRet = WaitForSingleObject ( pAudioParms->HBitsInList, INFINITE );
         if ( dwRet == WAIT_OBJECT_0 )
         {
            return S_OK;
         }
         else
         {  
            error = RGBERROR_NOACCESS;
         }
      }
#else
      ULONG written;

      /* The driver will write available data to pData and any remaining pData with
         zero. The driver will not wait for new data to arrive. The WASAPI will then
         determine rate control using any padded zero values. */
      error = RGBAudioLoadOutputBuffer ( pAudioParms->HAudio, size, 0, &written, pData );
      if ( error == RGBERROR_NO_ERROR )
      {
         return S_OK;
      }
#endif
   }
   
   return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, error);
}

/******************************************************************************/

DWORD WINAPI
AudioThread (
   LPVOID   lpThreadParameter )
{
   HRESULT hr;
   PAUDIOPARMS pAudioParms = (PAUDIOPARMS)lpThreadParameter;
   IMMDeviceEnumerator *pEnumerator = NULL;
   IMMDevice *pDevice = NULL;
   IAudioClient *pAudioClient = NULL;
   IAudioRenderClient *pRenderClient = NULL;
   UINT32 bufferFrameCount;
   BYTE *pData;
   DWORD taskIndex = 0;
   DWORD flags = 0;
   HANDLE hEvent = NULL;
   HANDLE hTask = NULL;
   UINT32 numFramesPadding;
   UINT32 numFramesAvailable;

   /* Initialise WASAPI interfaces and find audio endpoint. */
   hr = CoCreateInstance ( &CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, 
         &IID_IMMDeviceEnumerator, (void**)&pEnumerator);
   EXIT_ON_ERROR(hr)

   hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator, eRender, 
         eConsole, &pDevice);
   EXIT_ON_ERROR(hr)

   hr = pDevice->lpVtbl->Activate(pDevice, &IID_IAudioClient, CLSCTX_ALL,
         NULL, (void**)&pAudioClient);
   EXIT_ON_ERROR(hr)

   /* Agree on a shared endpoint format. */
   hr = Initialize( pAudioParms, pAudioClient );
   EXIT_ON_ERROR(hr)

   if ( pAudioParms->Event )
   {
      /* Create an event handle and register it for buffer-event notifications. */
	   hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	   if (hEvent == NULL)
	   {
		   hr = E_FAIL;
		   EXIT_ON_ERROR(hr)
	   }
	   hr = pAudioClient->lpVtbl->SetEventHandle(pAudioClient,hEvent);
      EXIT_ON_ERROR(hr)
   }

   /* Get the actual size of the two allocated buffers. */
   hr = pAudioClient->lpVtbl->GetBufferSize(pAudioClient,&bufferFrameCount);
   EXIT_ON_ERROR(hr)

   /* Is what we asked for different.. */
   if (pAudioParms->BufferSize / pAudioParms->WaveEx.Format.nBlockAlign != 
         bufferFrameCount)
   {
      /* Update our internal structure to match. */
      pAudioParms->BufferSize = bufferFrameCount * 
            pAudioParms->WaveEx.Format.nBlockAlign;
      pAudioParms->BufferDuration = GetLatency ( 
            pAudioParms->WaveEx.Format.nAvgBytesPerSec, pAudioParms->BufferSize );
   }

   hr = pAudioClient->lpVtbl->GetService(pAudioClient, &IID_IAudioRenderClient,
         (void**)&pRenderClient);
   EXIT_ON_ERROR(hr)

   /* Ask MMCSS to temporarily boost the thread priority to reduce glitches 
      while the low-latency stream plays. */
   hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
   if (hTask == NULL)
   {
       hr = E_FAIL;
       EXIT_ON_ERROR(hr)
   }

   /* Start RGBEasy audio capture. */
   hr = StartCapture (pAudioParms);
   EXIT_ON_ERROR(hr)

   /* Get the client buffer from the audio endpoint. */
   hr = pRenderClient->lpVtbl->GetBuffer(pRenderClient, bufferFrameCount, 
         &pData);
   EXIT_ON_ERROR(hr)

   /* Fill the entire buffer, this represents the stream latency. */
   hr = LoadData(pAudioParms, ( bufferFrameCount * 
         pAudioParms->WaveEx.Format.nBlockAlign), pData);
   EXIT_ON_ERROR(hr)

   hr = pRenderClient->lpVtbl->ReleaseBuffer(pRenderClient, bufferFrameCount, 
         flags);
   EXIT_ON_ERROR(hr)

   /* Start playing */
   hr = pAudioClient->lpVtbl->Start(pAudioClient);
   EXIT_ON_ERROR(hr)
   
   /* Allow OpenSharedAudio thread to continue. */
   SetEvent ( pAudioParms->HStartEvent );

   while (!pAudioParms->BClose)
   {
      if ( pAudioParms->Event )
      {        
         /* Wait for next buffer event to be signaled. */
         DWORD retval = WaitForSingleObject(hEvent, 5000);
         if (retval != WAIT_OBJECT_0)
         {
            pAudioClient->lpVtbl->Stop(pAudioClient);
            hr = ERROR_TIMEOUT;
            EXIT_ON_ERROR(hr)
         } 
      }
      else
      {
         /* Sleep for half the buffer duration. */
         Sleep((DWORD)pAudioParms->BufferDuration/REFTIMES_PER_MILLISEC/2);
      }

      /* Now see how much buffer space is available. */
      hr = pAudioClient->lpVtbl->GetCurrentPadding(pAudioClient, 
               &numFramesPadding);
      EXIT_ON_ERROR(hr)

      numFramesAvailable = bufferFrameCount - numFramesPadding;
      if ( numFramesAvailable )
      {
         /* Grab the next empty buffer from the audio endpoint. */
         hr = pRenderClient->lpVtbl->GetBuffer(pRenderClient, 
               numFramesAvailable, &pData);
         EXIT_ON_ERROR(hr)

         /* Load the buffer with data from the audio source. */
         hr = LoadData(pAudioParms, numFramesAvailable * 
            pAudioParms->WaveEx.Format.nBlockAlign, pData);
         EXIT_ON_ERROR(hr)

         hr = pRenderClient->lpVtbl->ReleaseBuffer(pRenderClient, 
            numFramesAvailable, flags);
         EXIT_ON_ERROR(hr)
      }
   }
   /* Stop endpoint playing. */
   hr = pAudioClient->lpVtbl->Stop(pAudioClient);
   EXIT_ON_ERROR(hr)

   Exit:
   
   UnInitialize ( pAudioParms );

   if ( pAudioParms->Event )
      if (hEvent != NULL)
         CloseHandle(hEvent);

   if (hTask != NULL)
      AvRevertMmThreadCharacteristics(hTask);

   SAFE_RELEASE(pEnumerator)
   SAFE_RELEASE(pDevice)
   SAFE_RELEASE(pAudioClient)
   SAFE_RELEASE(pRenderClient)

   pAudioParms->Error = hr;

   /* Allow OpenSharedAudio thread to continue. */
   SetEvent ( pAudioParms->HStartEvent );
   return 0;
}

/******************************************************************************/

HRESULT
GetAudioFormatIndex (
   HAUDIOINPUT handle,
   ULONG       *pIndex )
{
   PAUDIOPARMS pAudioParms = (PAUDIOPARMS)handle;

   if ( !pAudioParms )
      return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, RGBERROR_INVALID_HANDLE);

   if ( pAudioParms->Error )
         return pAudioParms->Error;

   *pIndex = pAudioParms->FormatIndex;

   return S_OK;
}

/******************************************************************************/

HRESULT
OpenSharedAudio (
   ULONG        input,
   PHAUDIOINPUT pHandle )
{  
   PAUDIOPARMS pAudioParms = malloc( sizeof(AUDIOPARMS) );
   HRESULT hr = S_FALSE;
   
   if ( !pAudioParms )
      return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 
            RGBERROR_INSUFFICIENT_MEMORY);

   memset ( pAudioParms, 0, sizeof(AUDIOPARMS) );

   /* Internal parameters. */
   pAudioParms->Input = input;
   pAudioParms->State = STOP;
   pAudioParms->Event = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
   /* This value represents the latency. */
   pAudioParms->BufferSize = 0x4000;
   /* Create event used to signal thread startup. */
   pAudioParms->HStartEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
   if (pAudioParms->HStartEvent)
   {
      /* Create an audio thread. */
      pAudioParms->HThread = ( HANDLE ) CreateThread (NULL, 0, 
            &AudioThread, pAudioParms, 0, &pAudioParms->DWThreadID );
      if (pAudioParms->HThread)
      {
         /* Wait for thread initialisation. */
         if ( WaitForSingleObject ( pAudioParms->HStartEvent, INFINITE ) 
               == WAIT_OBJECT_0 )
         {
            if ( pAudioParms->Error )
            {
               /* Audio startup error. */
               hr = pAudioParms->Error;
               CloseHandle (pAudioParms->HStartEvent);
               free (pAudioParms);
               return hr;
            }
            else
            {
               /* No error. */
               *pHandle = (HAUDIOINPUT)pAudioParms;
               return S_OK;
            }
         }
         else
         {
            /* Thread time out error. */
            CloseHandle (pAudioParms->HThread);
            CloseHandle (pAudioParms->HStartEvent);
            free (pAudioParms);
            return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 
                     RGBERROR_THREAD_FAILURE);
         }
      }
   }
   return hr;
}

/******************************************************************************/

HRESULT
CloseSharedAudio (
   HAUDIOINPUT handle )
{
   PAUDIOPARMS pAudioParms = (PAUDIOPARMS)handle;

   if ( !pAudioParms )
      return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, RGBERROR_INVALID_HANDLE);

   if ( pAudioParms->Error )
         return pAudioParms->Error;

   pAudioParms->BClose = TRUE;
   /* Wait for the audio thread to finish. */
   WaitForSingleObject ( pAudioParms->HThread, INFINITE );
   CloseHandle ( pAudioParms->HThread );

   if (pAudioParms->HStartEvent)
      CloseHandle (pAudioParms->HStartEvent);

   free (pAudioParms);

   return S_OK;
}

 /******************************************************************************/

VOID 
LaunchAdvancedDefaultSpeakerPropertiesDlg ( )
{
   IMMDeviceEnumerator *pEnumerator = NULL;
   IMMDevice *pEndpoint = NULL;
   IPropertyStore *pProps = NULL;
   HRESULT hr;

   /* Initialise WASAPI interfaces and find audio endpoint. */
   hr = CoCreateInstance ( &CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, 
         &IID_IMMDeviceEnumerator, (void**)&pEnumerator);
   EXIT_ON_ERROR(hr)

   hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator, eRender, 
         eConsole, &pEndpoint);
   EXIT_ON_ERROR(hr)

   if ( pEndpoint )
   {
      LPWSTR pwszID = NULL;
      TCHAR cmd[255];

      /* Get the defaults endpoint ID string. */
      hr = pEndpoint->lpVtbl->GetId(pEndpoint,&pwszID);
      EXIT_ON_ERROR(hr)

      StringCchPrintf ( cmd, MAX_PATH, 
            TEXT("Shell32.dll,Control_RunDLL mmsys.cpl,,%s,advanced"), pwszID );
      ShellExecute(NULL, TEXT("open"), TEXT("rundll32.exe"), cmd, NULL, 
            SW_SHOWNORMAL );
      CoTaskMemFree(pwszID);
   }
   Exit:
   SAFE_RELEASE(pProps)
   SAFE_RELEASE(pEndpoint)
   SAFE_RELEASE(pEnumerator)
}

/******************************************************************************/