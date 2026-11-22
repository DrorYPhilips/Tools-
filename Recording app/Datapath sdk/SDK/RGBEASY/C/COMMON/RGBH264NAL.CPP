/*******************************************************************************
Copyright Datapath Ltd. 2014.

File:    rgbh264nal.cpp

History: 30 OCT 14    RL   Created.
         12 OCT 15    DC   Added further hard-coded URLs.
                           Included GOP Length as an option in URLs.
         02 MAR 16    DC   Round up frame rate to nearest Hz (100mHz).
                           Makes the exported RTSP URL friendlier to the
                           end user.

*******************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <WinError.h>
#include <stdio.h>
#include <tchar.h>

#if defined(_LOCAL_HEAP_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#if defined(__cplusplus)
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#endif
#else
#define DBG_NEW new
#endif

#include "api.h"

#include "dgctypes.h"
#include "dgcmedia.h"
#include "dgcmediahelper.h"

#include "sqxerror.h"
#include "sqxstreamdefs.h"
#include "sqx.h"

#include "rgb.h"
#include "rgbapi.h"
#include "rgberror.h"
#include "rgbh264nal.h"
#include "rgbeasyh264.h"

/******************************************************************************/

uint32_t _Round(uint32_t value, uint32_t factor)
{
   value += (factor/2);
   value /= factor;
   value *= factor;
   return value;
}

/******************************************************************************/

uint32_t GetInputSignalType (
   uint32_t input,
   uint32_t *pWidth,
   uint32_t *pHeight,
   uint32_t *pFPS )
{
   unsigned long error;
   HRGBDLL  hDLL;

   /* Load the RGBEASY API. */
   error = RGBLoad(&hDLL);
   if (!error)
   {
      SIGNALTYPE signalType;

      error = RGBGetInputSignalType ( input, &signalType,
            (unsigned long*)pWidth, (unsigned long*)pHeight, (unsigned long*)pFPS );
      if (!error)
      {
         if (( signalType == RGB_SIGNALTYPE_NOSIGNAL) || 
            ( signalType == RGB_SIGNALTYPE_OUTOFRANGE ))
         {
            // use defaults
            error = RGBERROR_INVALID_FORMAT;
         }
         else
         {
            /* Round Framerate to nearest Hz (1000mHz) */
            *pFPS = _Round(*pFPS, 1000);
         }
      }
      RGBFree(hDLL);
   }
   return error;
}

/******************************************************************************/

uint32_t GetSupportedH264Inputs (
   uint32_t **ppInputList,
   uint32_t *pInputCount,
   uint32_t *pH264Count )
{
   unsigned long error;
   HRGBDLL  hDLL;

   /* Load the RGBEASY API. */
   error = RGBLoad(&hDLL);
   if (!error)
   {
      long bSupported = FALSE;
      uint32_t numberOfInputs;

      error = RGBGetNumberOfInputs ((unsigned long*)&numberOfInputs );
      if (!error)
      {
         uint32_t x, h264Count;
         uint32_t *pSupported = (uint32_t*)malloc (sizeof(uint32_t)*numberOfInputs);     

         h264Count=0;
         memset ( pSupported, 0, sizeof(uint32_t)*numberOfInputs);
         for ( x=0; x< numberOfInputs; x++)
         {
            error = RGBInputIsMediaSampleTypeSupported( x,
                                 DGCMEDIASAMPLETYPE_ENC_VIDEO,
                                 DGCENCVIDEOSAMPLESUBTYPE_H264,
                                 &bSupported );

            if ( !error && bSupported )
            {
               h264Count++;
               pSupported[x] = 1;
            }
         }
         if ( !h264Count )
            error = RGBERROR_ILLEGAL_CALL;
         else
         {
            *ppInputList = pSupported;
            *pInputCount = numberOfInputs;
            *pH264Count = h264Count;
         }
      }

      RGBFree(hDLL);
   }
   return error;
}

/******************************************************************************/

uint8_t GetNALType(uint8_t *pNalUnit) 
{
	return pNalUnit[0]&0x1F;
}

/******************************************************************************/

BOOL CanH264NALUnitBePurged(uint8_t *pNalUnit) 
{
	uint8_t nal_unit_type = pNalUnit[0]&0x1F;
	uint8_t nal_unit_data = pNalUnit[1];

	if ( nal_unit_type != 1 ) 
   { 
      return FALSE;
	}

   if( nal_unit_data == 0x88 )
   {
      return FALSE;
   }

	return TRUE;
}

/******************************************************************************/

BOOL DoesH264NALUnitBeginNewAccessUnit(uint8_t *pNalUnit) 
{
	uint8_t nal_unit_type = pNalUnit[0]&0x1F;

   // Non- VCL
	if ( ( nal_unit_type >= 6 && nal_unit_type <= 9 ) || 
        ( nal_unit_type >= 14 && nal_unit_type <= 18 ))
   {
#if 0
      printf(__FUNCTION__": NEW AU - nal_unit_type(%u) (> 6) && (<= 9)\n", nal_unit_type);
#endif
      return TRUE;
   }

	if ( nal_unit_type <= 5 && nal_unit_type > 0 ) 
   { 
      // We are a VCL NAL. Look at the high bit of the next byte;
      // if it's set, then this NAL starts a new 'access unit':
		if (( pNalUnit[1]&0x80) != 0 ) 
      {
#if 0
         printf( __FUNCTION__": NEW AU - nal_unit_type(%u) (<= 5) && (> 0) - [1](0x%02x)\n",
            nal_unit_type, pNalUnit[1]);
#endif
         return TRUE;
      }
	}

#if 0
   printf( __FUNCTION__": NOT NEW AU - nal_unit_type(%u)\n", nal_unit_type);
#endif
	return FALSE;
}

/******************************************************************************/

CRGBEasyH264::CRGBEasyH264(uint32_t input) :
   m_error(RGBERROR_NO_ERROR),
   m_state(RGBEasyH264_FRAME_STATE_UNINITIALISED),
   m_hInstance(NULL),
   m_startCodeOffset(4),
   m_input(input),
   m_hRGB(NULL),
   m_pNAL_List(NULL),
   m_queuedNALs(0),
   m_unqueuedNALs(0),
   m_NAL_ListSize(0),
   m_NAL_seq(0)
{   
   m_hListMutex = CreateMutex ( NULL, FALSE, NULL );
   m_AdderMutex = CreateMutex ( NULL, FALSE, NULL );
   memset ( &m_StreamInfo, 0, sizeof(m_StreamInfo));
   m_error = RGBEasyH264Init( input );
}

CRGBEasyH264::~CRGBEasyH264()
{
   DrainNALList( );
   RGBEasyH264UnInit();
   CloseHandle(m_hListMutex);
   CloseHandle(m_AdderMutex);
}

void CRGBEasyH264::DrainNALList( )
{
   PH264NAL pNal = m_pNAL_List;

   while ( pNal )
   {
      PH264NAL pNext = pNal->PNext;
      delete[] pNal->PStart;
      delete[] pNal;
      pNal = pNext;
   }
}

/******************************************************************************/

PDGCMEDIASAMPLE CRGBEasyH264::CreateMediaSample(   uint32_t width,
                                                   uint32_t height,
                                                   DGCMEDIASAMPLETYPE majorType,
                                                   DGCMEDIASAMPLESUBTYPE subType)
{   
   PDGCMEDIASAMPLE pMediaSampleIn = MediaSampleAllocate( majorType, subType );
   if ( pMediaSampleIn )
   {
      if (MediaSampleAllocateFormatHeader(pMediaSampleIn))
      {
         PDGCVIDEOHEADER pVideoHeaderOut;

         // Scaled width and height of the capture
         pVideoHeaderOut = (PDGCVIDEOHEADER)pMediaSampleIn->PFormatHeader;
         pVideoHeaderOut->Width = width;
         pVideoHeaderOut->Height = height;

         DGCBUFFERHEADERTYPE   bufferType;
         if (majorType == DGCMEDIASAMPLETYPE_SHARED_RAW_VIDEO || DGCMEDIASAMPLETYPE_SHARED_ENC_VIDEO)
         {
            bufferType = (DGCBUFFERHEADERTYPE)DGCBUFFERHEADERTYPE_SHARED_MEMORY;
            pMediaSampleIn->BufferHeaderType = (DGCBUFFERHEADERTYPE)DGCBUFFERHEADERTYPE_SHARED_MEMORY;
         }
         else
         {
            bufferType = DGCBUFFERHEADERTYPE_MEMORY;
            pMediaSampleIn->BufferHeaderType = DGCBUFFERHEADERTYPE_MEMORY;
         }

         if ( MediaSampleAllocateBufferHeader( pMediaSampleIn, bufferType ) )
         {
            /*    TO DO  */
            /* Ideally the output buffer allocated to recieve encoded data should be at least the size of the raw input buffer.
             * This function should know what is the input buffer format and allocate output buffer based on that.
             * currently it is assumed that input buffer is NV12 format. Hence output buffer allocated is Width * height * 3 / 2
             */

            if ( MediaSampleInitialisePlane(
               pMediaSampleIn, 0, ( width * height * 3 ) / 2 ) )
            {
               return pMediaSampleIn;
            }
         }
      }
      MediaSampleFree( pMediaSampleIn );
   }
   return NULL;
}

/******************************************************************************/

void CRGBEasyH264::AddNALToList ( PH264NAL pH264NAL )
{     
   /* Maintain a FIFO of NALU's. */
   if ( pH264NAL )
   {
      PH264NAL pH264NALPrev, pH264NALThis;
      PH264NAL pH264NALPrevToToRemove, pH264NALToRemove;

      /* Aquire Mutex. */
      WaitForSingleObject ( m_hListMutex, INFINITE );

      /* Insert at end of list. */
      pH264NALPrev = NULL;
      pH264NALThis = m_pNAL_List;
      pH264NALToRemove = NULL;
      pH264NALPrevToToRemove = NULL;

      while ( pH264NALThis )
      {
         if(pH264NALToRemove == NULL)
         {
            if(CanH264NALUnitBePurged(pH264NALThis->PStart))
            {
               // NAL could be removed from the list if the list is full
               pH264NALToRemove = pH264NALThis;
               pH264NALPrevToToRemove = pH264NALPrev;
            }
         }
         pH264NALPrev = pH264NALThis;
         pH264NALThis = pH264NALThis->PNext;
      }

      if ( pH264NALPrev )
      {
         pH264NALPrev->PNext = pH264NAL;
      }
      else
      {
         m_pNAL_List = pH264NAL;
      }

      m_queuedNALs++;
      m_NAL_ListSize++;
      pH264NAL->PNext = NULL;

      if( m_NAL_ListSize > 100 )
      {
         if( pH264NALToRemove )
         {
            LockedRemoveNALFromList( pH264NALToRemove, pH264NALPrevToToRemove );
            m_NAL_ListSize--;
         }
      }

      /* Release Mutex. */
      ReleaseMutex ( m_hListMutex );
   }
}

/******************************************************************************/

void CRGBEasyH264::LockedRemoveNALFromList( PH264NAL pH264NALToRemove, PH264NAL pH264NALPrevToToRemove )
{
   PH264NAL pH264NAL = m_pNAL_List;
   while ( pH264NAL )
   {
      if(pH264NAL == pH264NALToRemove)
      {
         if(pH264NALPrevToToRemove)
         {
            // Link previous to next
            pH264NALPrevToToRemove->PNext = pH264NAL->PNext;
         }
         else
         {
            // Assign list head
            m_pNAL_List = pH264NAL->PNext;
         }

         pH264NAL->PNext = NULL; // Stop someone traversing the list through nefarious means.

         // Tidy up
         delete[] pH264NAL->PStart;
         delete[] pH264NAL;

         break;
      }
      pH264NAL = pH264NAL->PNext;
   }
}

/******************************************************************************/

void CRGBEasyH264::CheckListRemoveFirstNALFromList( PH264NAL* ppH264NAL )
{
   PH264NAL pH264NAL = NULL;

   /* Remove the entry from the FIFO list. */
   WaitForSingleObject ( m_hListMutex, INFINITE );

   if ( m_pNAL_List )
   {
      /* Get the first item in the list. */
      pH264NAL = m_pNAL_List;

      /* New pointer to start of list. */
      m_pNAL_List = m_pNAL_List->PNext;
      pH264NAL->PNext = NULL; // Stop someone traversing the list through nefarious means.
      m_unqueuedNALs++;
      m_NAL_ListSize--;
   }

   /* Release Mutex. */
   ReleaseMutex ( m_hListMutex );

   *ppH264NAL = pH264NAL;
}

/******************************************************************************/

void RGBCBKAPI CRGBEasyH264::RGBEasyH264EncoderErrorFn ( HWND           hWnd,
                                                         HRGB           hRGB,
                                                         unsigned long  error,
                                                         ULONG_PTR      pUserData,
                                                         unsigned long  *pReserved)
{
   UNREFERENCED_PARAMETER( hWnd );
   UNREFERENCED_PARAMETER( hRGB );
   UNREFERENCED_PARAMETER( pReserved );
   CRGBEasyH264 *pH264NAL = (CRGBEasyH264*)pUserData;
  /* Handle each error separately based on error type */
   switch (error)
   {
      case RGBERROR_INSUFFICIENT_BUFFERS:
      {
         /* Do Nothing */
         break;
      }
      case RGBERROR_CAPTURE_OUTSTANDING:
      {
         /* Do Nothing */
         break;
      }
      default:
         pH264NAL->RGBEasyH264RealEncoderErrorFn(hRGB, error);
   }
}

void CRGBEasyH264::RGBEasyH264RealEncoderErrorFn( HRGB            hRGB,
                                                  unsigned long   error)
{
   TCHAR buffer[255];
   UNREFERENCED_PARAMETER( hRGB );
   wsprintf (buffer, TEXT( __FUNCTION__ _T(" 0x%x\n")), error );
   _tprintf( buffer ); 
}

/******************************************************************************/

void RGBCBKAPI CRGBEasyH264::RGBEasyH264EncoderFrameFn(  HWND              hWnd,
                                                         HRGB              hRGB,
                                                         PDGCMEDIASAMPLE   pMediaSample,
                                                         ULONG_PTR         pUserData )
{
   CRGBEasyH264 *pH264NAL = (CRGBEasyH264*)pUserData;
   UNREFERENCED_PARAMETER( hWnd );
   if ( pH264NAL )
      pH264NAL->RGBEasyH264RealEncoderFrameFn ( hRGB, pMediaSample );
}

/******************************************************************************/

void RGBCBKAPI CRGBEasyH264::RGBEasyEncryptionKeysFn(HWND                 hWnd,
   HRGB                 hRGB,
   PSQXENCRYPTIONINFO   pEncryptionKeys,
   ULONG_PTR            pUserData)
{
   UNREFERENCED_PARAMETER(hWnd);
   CRGBEasyH264 *pH264NAL = (CRGBEasyH264*)pUserData;
   /* Handle each error separately based on error type */
   if (pH264NAL)
   {
      pH264NAL->RGBEasyRealEncryptionKeysFn(hRGB, pEncryptionKeys);
   }
}

void CRGBEasyH264::RGBEasyRealEncryptionKeysFn(HRGB                 hRGB,
   PSQXENCRYPTIONINFO   pEncryptionKeys)
{

}

#if !defined(USE_OLD_NAL_PROCESSOR)

// A blisteringly fast search through memory.
// To satisfy licensing: http://www.sanmayce.com/Railgun/index.html
// It says code is free, but mention that homepage (but not *where* to mention that homepage).
// This does not work for cbPattern == 1.  Speed is derived from comparing as little as possible
// and skipping as much as possible...
unsigned char * SpeedyFind( char * pbTarget, char * pbPattern, uint32_t cbTarget, uint32_t cbPattern )
{
   char * pbTargetMax = pbTarget + cbTarget;
   register uint32_t ulHashPattern;
   uint32_t count, countSTATIC;

   if ( cbPattern > cbTarget ) return( NULL );

   countSTATIC = cbPattern - 2;

   pbTarget = pbTarget + cbPattern;
   ulHashPattern = ( *( uint16_t * ) ( pbPattern ) );

   for ( ;; )
   {
      if ( ulHashPattern == ( *( uint16_t * ) ( pbTarget - cbPattern ) ) )
      {
         count = countSTATIC;
         while ( count && *( char * ) ( pbPattern + 2 + ( countSTATIC - count ) ) == *( char * ) ( pbTarget - cbPattern + 2 + ( countSTATIC - count ) ) )
         {
            count--;
         }
         if ( count == 0 ) return( (unsigned char *) ( pbTarget - cbPattern ) );
      }
      pbTarget++;
      if ( pbTarget > pbTargetMax ) return( NULL );
   }
}

unsigned char * CRGBEasyH264::FindNextHeader( unsigned char * pBuffer, int32_t length )
{
   DWORD dwNALHeader = 0x01000000;
   return SpeedyFind( reinterpret_cast<char *>(pBuffer), reinterpret_cast< char * >( &dwNALHeader ), length, 4 );
}

// It is incumbent upon the caller to call this next time with pBuffer set to 
// pBufferOld + NALlength, to move past the NAL this function returns this time.
unsigned char * CRGBEasyH264::GetNextNAL( unsigned char * pBuffer, uint32_t *pBufferLen, unsigned char ** pNAL, unsigned long * NALlength )
{
//   char DbgBuffer[1024] = { 0 };
   uint32_t blen = *pBufferLen;
//   sprintf( DbgBuffer, "Buffer Start: %p Buffer Len: %d - Buffer end: %p\n", pBuffer, *pBufferLen, pBuffer + ( *pBufferLen ) );
//   OutputDebugStringA( DbgBuffer );
   unsigned char *pThis = NULL;
   unsigned char *pNext = NULL;
   
   pThis = FindNextHeader( pBuffer, *pBufferLen ); // Should be immediate.

   if ( pThis == NULL )
   {
      // eob - so fake a pNext.
      pNext = NULL;
      return reinterpret_cast<unsigned char *>( pNext );
   }

   pThis += 4;
   
//   sprintf( DbgBuffer, "Got NAL at address %p - NAL Header at: %p\n", pThis, pThis - 4 );
//   OutputDebugStringA( DbgBuffer );
#if 0
         uint8_t prevNALType = GetNALType(pThis);
         if(prevNALType == 1)
         {
            printf("********** FOUND non-IDR Coded Frame **********\n");
         }
         else if(prevNALType == 5)
         {
            printf("********** FOUND I-Frame **********\n");
         }
         else if(prevNALType == 6)
         {
            printf("========== FOUND P-Frame ==========\n");
         }
         else if(prevNALType == 7)
         {
            printf("########## FOUND SPS ##########\n");
         }
         else if(prevNALType == 8)
         {
            printf("$$$$$$$$$$ FOUND PPS $$$$$$$$$$\n");
         }
         else if(prevNALType != 0)
         {
            printf(">>>>>>>>>> FOUND %u <<<<<<<<<<\n", prevNALType);
         }
#endif
   
   pNext = FindNextHeader( pThis, ( blen - (uint32_t)(pThis - pBuffer) ) ); // scan to start of next nal/eob
//   sprintf( DbgBuffer, "Got next header at address %p\n", pNext );
//   OutputDebugStringA( DbgBuffer );
   if ( pNext == NULL )
   {
      // eob - so fake a pNext.
//      sprintf( DbgBuffer, "NAL is last in buffer.  Buffer end pointer now %p\n", pBuffer + ( *pBufferLen ) );
//      OutputDebugStringA( DbgBuffer );
      pNext = pBuffer + *pBufferLen;
   }
   unsigned long nallen = (uint32_t)(pNext - pThis);
//   sprintf( DbgBuffer, "NAL length is %d\n", nallen );
//   OutputDebugStringA( DbgBuffer );
   *pNAL = pThis;
   *NALlength = nallen;
   *pBufferLen -= ( nallen + 4 );
//   sprintf( DbgBuffer, "*pBufferLen is now: %d\n", *pBufferLen );
//   OutputDebugStringA( DbgBuffer );
//   sprintf( DbgBuffer, "Returning %p as new start of buffer\n", pNext );
//   OutputDebugStringA( DbgBuffer );
   return reinterpret_cast<unsigned char *>(pNext);
}

#else
unsigned char * CRGBEasyH264::GetNextNAL(unsigned char * pBuffer, uint32_t *pBufferLen, unsigned char ** pNAL, unsigned long * NALlength)
{
   uint32_t nallen = *pBufferLen;
   bool bSeekingEnd = false;
   bool bSingleNAL = false;
   bool bIDRFrame = false;
   bool bSPS = false;
   bool bPPS = false;
   unsigned char * pData = pBuffer, *pNALStart;
   uint8_t NALType = 0;
   uint8_t prevNALType = 0;
   uint32_t BufLen = *pBufferLen;
   //printf("CRGBEasyH264::GetNextNAL: parse pBuffer(0x%p) *pBufferLen(%u) pData(0x%p) BufLen(%u)\n",
   //   pBuffer, *pBufferLen, pData, BufLen);
   while (BufLen != 0)
   {
      unsigned long *pHeader;
         
      pHeader = reinterpret_cast<unsigned long *>(pData);
      if (*pHeader == 0x01000000)
      {
         prevNALType = NALType;
         NALType = GetNALType(pData + 4);
#if 0
         if(prevNALType == 1)
         {
            printf("********** FOUND non-IDR Coded Frame **********\n");
         }
         else if(prevNALType == 5)
         {
            printf("********** FOUND I-Frame **********\n");
         }
         else if(prevNALType == 6)
         {
            printf("========== FOUND P-Frame ==========\n");
         }
         else if(prevNALType == 7)
         {
            printf("########## FOUND SPS ##########\n");
         }
         else if(prevNALType == 8)
         {
            printf("$$$$$$$$$$ FOUND PPS $$$$$$$$$$\n");
         }
         else if(prevNALType != 0)
         {
            printf(">>>>>>>>>> FOUND %u <<<<<<<<<<\n", prevNALType);
         }
#endif
         if (bSeekingEnd)
         {
            // NAL Ends.
            *NALlength = pData - pNALStart;
            *pNAL = pNALStart;
            *pBufferLen = BufLen;
            //printf("CRGBEasyH264::GetNextNAL: 1) bSeekingEnd return pData(0x%p) pNALStart(0x%p) diff(%u) *NALLength(%u) pNAL(0x%p) BufLen(%u)\n",
            //   pData, pNALStart, ((uint32_t)pData - (uint32_t)pNALStart), *NALlength, *pNAL, BufLen);
            return pData;
         }
         // NAL begins
         pNALStart = pData+4; // Skip header.
         bSeekingEnd = true;
         pData += 3; // We know we can optimise these bytes out of the search.
         BufLen -= 3; // And commensurately shorten the buffer.
         //printf("CRGBEasyH264::GetNextNAL: !bSeekingEnd pData(0x%p) pNALStart(0x%p) BufLen(%u)\n",
         //   pData, pNALStart, BufLen);
      }

      {
         pData++;
         BufLen -= 1;
      }
   }

#if 0
   if(NALType == 1)
   {
      printf("********** FOUND non-IDR Coded Frame **********\n");
   }
   else if(NALType == 5)
   {
      printf("********** FOUND I-Frame **********\n");
   }
   else if(NALType == 6)
   {
      printf("========== FOUND P-Frame ==========\n");
   }
   else if(NALType == 7)
   {
      printf("########## FOUND SPS ##########\n");
   }
   else if(NALType == 8)
   {
      printf("$$$$$$$$$$ FOUND PPS $$$$$$$$$$\n");
   }
   else if(NALType != 0)
   {
      printf(">>>>>>>>>> FOUND %u <<<<<<<<<<\n", prevNALType);
   }
#endif

   if (bSeekingEnd)
   {
      *NALlength = pData - pNALStart;
      *pNAL = pNALStart;
      *pBufferLen = 0;
      //printf("CRGBEasyH264::GetNextNAL: 2) bSeekingEnd return pData(0x%p) pNALStart(0x%p) diff(%u) *NALLength(%u) pNAL(0x%p) BufLen(%u)\n",
      //   pData, pNALStart, ((uint32_t)pData - (uint32_t)pNALStart), *NALlength, *pNAL, BufLen);
      return pData;
   }
   // Non NAL data in the buffer...
   printf( __FUNCTION__": Non NAL Data found in buffer\n");
   return NULL;
}
#endif
void CRGBEasyH264::RGBEasyH264RealEncoderFrameFn(  HRGB              hRGB,
                                                   PDGCMEDIASAMPLE   pMediaSample )
{
   PDGCMEMORYBUFFERHEADER pHeader = (PDGCMEMORYBUFFERHEADER) pMediaSample->PBufferHeader;
   uint32_t size = pHeader->Planes[0].ActualLength;
   unsigned char *pStart = (unsigned char *)pHeader->Planes[0].PBuffer;
   uint32_t NALs = 0;
   // We need to protect against any other buffer dissector from starting to add its buffer's NALs
   // to the (singleton) list of NALs until we've completely finished adding our NALs from this buffer.
   // With any luck, this thread will run before any other later thread starts to, although timestamps
   // *should* sort any interleaving out.  Even so, mutexing against other threads running whilst we're
   // processing this buffer seems like a good idea...

   WaitForSingleObject(m_AdderMutex, INFINITE);

   /* An RGBEasy buffer containing elementary H264 data can contain multiple NAL units.
   * However, a single buffer will always contain a single VCR NAL and possible multiple
   * non-VCR NAL units. For this reason we split the buffer into NAL units in this
   * function and assign the same time stamp to all NAL units. */
   uint64_t TimeStamp;
   BOOL bTSSource;
   switch (pMediaSample->SubType)
   {
      case DGCENCVIDEOSAMPLESUBTYPE_H264:
      {
         unsigned char *pBuffer = pStart, *pNALData;
         unsigned long length;

         // GetNextNAL side-effects the size; it is adjusted down by the size of the data consumed by the
         // NAL which is found.
         // pBuffer is unmodified, but the position in the buffer after the NAL found is returned, and it's
         // beholden on the caller to pass that pointer in to the next call to continue the search.
         // size will be 0 when the search is over, and NULL will be returned.
         // by defintion, a NAL ends at the end of a buffer, there can be no partial NALs.
#if DEBUG_NAL_
         uint64_t* pData = (uint64_t*)pStart;
         uint64_t* pDataEnd = (uint64_t*)(pStart + (size-8));
         printf( __FUNCTION__": size(%d) - data(0x%llx)(0x%llx) - TimeStamp(%lu)\n",
          size, *pData, *pDataEnd, TimeStamp);
#endif
         if ( pHeader->StartTime == 0 )
         {
            TimeStamp = pMediaSample->BufferTimeStamp;
            bTSSource = FALSE;
         } else
         {
            TimeStamp = pHeader->StartTime;
            bTSSource = TRUE;
         }
#if 0
         printf( "Actual Timestamp being used in "__FUNCTION__" Source: %s : %llu\n", bTSSource ? "pHeader" : "pMediaSample", TimeStamp );
#endif
         PH264NAL pNAL = NULL;
         while (size != 0 && ((pBuffer = GetNextNAL(pBuffer, &size, &pNALData, &length)) != NULL))
         {
            // Found a NAL.  Add it to the list.
            pNAL = DBG_NEW H264NAL;
            pNAL->StartTime = TimeStamp;
            pNAL->Size = length;
            pNAL->PStart = DBG_NEW uint8_t[length];
            pNAL->Seq = m_NAL_seq;
            memcpy(pNAL->PStart, pNALData, length);
            pNAL->PNext = NULL;
            pNAL->bLastNALInSeq = false;
            if ( size != 0 )
            {
               AddNALToList( pNAL );
            }
            NALs++;
         }
         if (pNAL)
         {
            pNAL->bLastNALInSeq = true;
            AddNALToList(pNAL);
            m_NAL_seq++;
         }
#if 0
         printf( __FUNCTION__": found %u NALs in Frame\n", NALs);
#endif
         // Kick off the next data Acquisition from the encoder (if we're still in a capturing state).
         if (m_state == RGBEasyH264_FRAME_STATE_STARTED)
         {
            m_error = RGBChainMediaSample(hRGB, pMediaSample);
         }
         break;
      }

      case DGCRAWVIDEOSAMPLESUBTYPE_NV12:
      {
         // Application can use the intermediate NV12 buffer for eg. for Thumbnail display .
         break;
      }

   }

   // And release the buffer NAL content processing mutex.
   ReleaseMutex(m_AdderMutex);

}

/******************************************************************************/

uint32_t CRGBEasyH264::RGBEasyH264Start( uint32_t width,
                                         uint32_t height,
                                         uint32_t frameRate,
                                         DGCENCLEVEL level,
                                         DGCENCPROFILE profile,
                                         uint32_t bitrate,
                                         uint32_t keyframeinterval,
                                         uint32_t encodeAffinity,
                                         uint32_t UdpPort,
                                         char     *UdpUrl,
                                         PRGBMEDIASAMPLECAPTUREDFN  CallbackFn,
                                         PRGBENCRYPTIONKEYSFN pEncryptionKeysFn,
                                         uintptr_t userdata)
{
   if ( m_error )
   {
      printf( __FUNCTION__": oops m_error(0x%x)\n", m_error);
      return m_error;
   }
   if ( m_state != RGBEasyH264_FRAME_STATE_INITIALISED )
   {
      printf( __FUNCTION__": (m_state != RGBEasyH264_FRAME_STATE_INITIALISED)\n");
      return RGBERROR_INVALID_STATE;
   }

   m_StreamInfo.Width = width;
   m_StreamInfo.Height = height;
   m_StreamInfo.FPS = frameRate;
   m_StreamInfo.EncodeAffinity = encodeAffinity;

   if ( !m_StreamInfo.Width || !m_StreamInfo.Height || !m_StreamInfo.FPS )
   {
      printf( __FUNCTION__": ( !m_StreamInfo.Width || !&m_StreamInfo.Height || !m_StreamInfo.FPS )\n");
      m_error = RGBERROR_INVALID_FORMAT;
      return m_error;
   }

   m_error = RGBOpenInput( m_input, &m_hRGB );
   if ( !m_error )
   {
      uint32_t i;

      if(m_StreamInfo.EncodeAffinity != SQXDEVICETYPE_UNSPECIFIED)
      {
         m_error = RGBSetEncodeAffinity( m_hRGB, m_StreamInfo.EncodeAffinity );
         if (m_error)
         {
            printf( __FUNCTION__": RGBSetHardwareAffinity\n");
           return m_error;
         }
      }

      //set Udp multicast group and port number
      //no need to set multicast if rtsp!
      if (UdpPort != 0)
      {
         m_error = RGBSetEncodeUdpMulticast(m_hRGB, UdpPort, UdpUrl);
         if (m_error)
         {
            printf(__FUNCTION__": RGBSetEncodeUdpMulticast\n");
            return m_error;
         }
      }

      m_error = RGBSetCaptureRate( m_hRGB, m_StreamInfo.FPS );
      if ( m_error )
      {
         printf( __FUNCTION__": RGBSetCaptureRate\n");
         return m_error;
      }

      m_error = RGBSetEncodeLevel( m_hRGB, level );
      if ( m_error )
      {
         printf( __FUNCTION__": RGBSetEncodeLevel\n");
         return m_error;
      }     

      m_error = RGBSetEncodeProfile( m_hRGB, profile );
      if ( m_error )
      {
         printf( __FUNCTION__": RGBSetEncodeProfile\n");
         return m_error;
      }

      m_error = RGBSetEncodeBitRate( m_hRGB, bitrate );
      if ( m_error )
      {
         printf( __FUNCTION__": RGBSetEncodeBitRate\n");
         return m_error;
      }
      
      // set every 90th frame to be a key frame
      m_error = RGBSetEncodeKeyFrameInterval( m_hRGB, keyframeinterval );
      if ( m_error )
      {
         printf( __FUNCTION__": RGBSetEncodeKeyFrameInterval\n");
         return m_error;
      }      

      m_error = RGBSetErrorFn( m_hRGB, RGBEasyH264EncoderErrorFn, (ULONG_PTR)this );
      m_error = RGBSetEncryptionKeysFn(m_hRGB, pEncryptionKeysFn, (ULONG_PTR)this);

      if ( !m_error )
      {
         m_error = RGBSetMediaSampleCapturedFn( m_hRGB, CallbackFn, (ULONG_PTR)this );
         if ( !m_error )
         { 
            for ( i = 0; i < NUM_SAMPLES; i++ )
            {
               m_pMediaSample[i] = CreateMediaSample(m_StreamInfo.Width, m_StreamInfo.Height,
                  (DGCMEDIASAMPLETYPE)DGCMEDIASAMPLETYPE_SHARED_ENC_VIDEO, DGCENCVIDEOSAMPLESUBTYPE_H264);
#if 0
               {
                  PDGCMEMORYBUFFERHEADER pHeader = (PDGCMEMORYBUFFERHEADER) m_pMediaSample[i]->PBufferHeader;
               }
#endif
               m_error = RGBChainMediaSample( m_hRGB, m_pMediaSample[i] );
            }            
            if ( !m_error )
            {
               m_error = RGBUseOutputBuffers(m_hRGB, TRUE);
               if (!m_error)
               {
                  m_error = RGBStartCapture(m_hRGB);
                  if (!m_error)
                     m_state = RGBEasyH264_FRAME_STATE_STARTED;
                  else
                  {
                     printf( __FUNCTION__": RGBStartCapture m_error(0x%x)\n", m_error);
                  }
                  return m_error;
               }
               RGBUseOutputBuffers(m_hRGB, FALSE);
            }
            
            for ( i = 0; i < NUM_SAMPLES; i++ )
            {
               MediaSampleFree( m_pMediaSample[i] );
            }
         }
      }
      RGBCloseInput(m_hRGB);
   }
   printf(" __FUNCTION__: Couldn't start capture - error (0x%x)\n", m_error);
   return m_error;
}

/******************************************************************************/

uint32_t CRGBEasyH264::RGBEasyH264GetNAL ( 
   uint8_t  *pNAL, 
   uint32_t maxSize, 
   uint32_t *pTrunkSize,
   uint32_t *pCopySize,
   uint64_t *pTimeStamp,
   uint64_t *pSeqNum,
   uint64_t *pLastNALInSequence)
{
   if ( m_error )
      return m_error;
   if ( m_state != RGBEasyH264_FRAME_STATE_STARTED )
      return RGBERROR_INVALID_STATE;
 
   PH264NAL pH264NAL = NULL;

   CheckListRemoveFirstNALFromList( &pH264NAL );
   if ( pH264NAL )
   {
      if ( pH264NAL->Size > maxSize )
      {
         *pTrunkSize = pH264NAL->Size - maxSize;
			*pCopySize = maxSize;
      }
      else
      {
         *pTrunkSize = 0;
			*pCopySize = pH264NAL->Size;
      }
      *pTimeStamp = pH264NAL->StartTime;
      *pSeqNum = pH264NAL->Seq;
      *pLastNALInSequence = pH264NAL->bLastNALInSeq;
      memcpy ( pNAL, pH264NAL->PStart, *pCopySize );
      delete[] pH264NAL->PStart;
      delete[] pH264NAL;
   }
   else
   {
      return RGBERROR_BUFFER_NOT_VALID;
   }
   return m_error;
}

/******************************************************************************/

uint32_t CRGBEasyH264::RGBEasyH264Stop()
{
   if ( m_error )
      return m_error;
   if ( m_state != RGBEasyH264_FRAME_STATE_STARTED )
      return RGBERROR_INVALID_STATE;
   
   m_state = RGBEasyH264_FRAME_STATE_STOPING;
   m_error = RGBUseOutputBuffers(m_hRGB, FALSE);
   if ( !m_error )
   {
      m_error = RGBStopCapture(m_hRGB);  
      if ( !m_error )
      {
         m_error = RGBCloseInput(m_hRGB);
         for ( int i = 0; i < NUM_SAMPLES; i++ )
         {
            MediaSampleFree( m_pMediaSample[i] );
         }

         m_state = RGBEasyH264_FRAME_STATE_STOPPED;
      }
      else
      {
         printf("Couldn't stop capture, RGBInput remains open.\n");
      }
   }
   return m_error;
}

/******************************************************************************/

uint32_t CRGBEasyH264::RGBEasyH264Init( uint32_t input )
{
   HRGBDLL  hDLL;

   uint32_t bSupported = FALSE;

   if ( m_error )
      return m_error;
   if ( m_state != RGBEasyH264_FRAME_STATE_UNINITIALISED )
      return RGBERROR_INVALID_STATE;

   /* Load the RGBEASY API. */
   m_error = RGBLoad(&hDLL);
   if (!m_error)
   {
      m_error = RGBInputIsMediaSampleTypeSupported( input,
                              DGCMEDIASAMPLETYPE_ENC_VIDEO,
                              DGCENCVIDEOSAMPLESUBTYPE_H264,
                              (long*)&bSupported );

      if ( !m_error && bSupported )
      {
         m_state = RGBEasyH264_FRAME_STATE_INITIALISED;
         return m_error;
      }
      else
         m_error = RGBERROR_INVALID_INPUT;

      RGBFree(hDLL);
   }

   return m_error;
}

/******************************************************************************/

uint32_t CRGBEasyH264::RGBEasyH264UnInit( )
{
   if ( m_error )
      return m_error;
   if ( m_state != RGBEasyH264_FRAME_STATE_STOPPED )
      return RGBERROR_INVALID_STATE;

   m_state = RGBEasyH264_FRAME_STATE_UNINITIALISED;
   return 0;
}

/******************************************************************************/