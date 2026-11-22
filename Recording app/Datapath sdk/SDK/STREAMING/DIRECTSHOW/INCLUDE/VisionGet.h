
/*******************************************************************************

Copyright Datapath Ltd. 2009.

File:    VisionGet.h

Purpose: 

History:
         17 JAN 09    RL   Created.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Declarations ***************************************************************/

HRESULT 
GetVisionSourceFilter(
   ULONG       uInput,
   IBaseFilter **ppSrcFilter);

HRESULT 
GetVisionFriendlyName(
   ULONG uInput,
   TCHAR *pBuffer,
   ULONG uBufferLength);

HRESULT 
GetAudioSourceFilter(
   ULONG       uInput,
   IBaseFilter **ppSrcFilter);

/******************************************************************************/