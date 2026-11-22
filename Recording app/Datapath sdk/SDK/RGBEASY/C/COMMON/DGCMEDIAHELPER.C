/*******************************************************************************

Copyright Datapath Ltd. 2014, 2015

File:    dgcmediahelper.c

Purpose: Helper functionality for Audio and Video media types.

History:
         09 JUN 14    JL   Created.

*******************************************************************************/

#if ( (defined WIN32) || (defined _WIN32) )
#include <Windows.h>
#endif

#include "dgctypes.h"
#include "dgcmedia.h"
#include "dgcmediahelper.h"

#if defined(_LOCAL_HEAP_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
/* If we want to find source code point which leak, we can't use functions */
#define DGCAllocFn(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)
#define DGCFreeFn(ptr) free(ptr)
#if defined (__cplusplus)
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__ )
#endif
#else
#define DBG_NEW new
PDGCALLOCFN DGCAllocFn = malloc;
PDGCFREEFN  DGCFreeFn = free;

#ifdef __cplusplus
extern "C" {
#endif
   uint8_t MediaSampleSetAllocFunctionPair( PDGCALLOCFN allocFn,
                                            PDGCFREEFN  freeFn )
   {
      if ( allocFn && freeFn )
      {
         DGCAllocFn = allocFn;
         DGCFreeFn = freeFn;
         return TRUE;
      }
      return FALSE;
   }
#ifdef __cplusplus
}
#endif
#endif

/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/

typedef struct tagSubTypeInfo
{
   DGCMEDIASAMPLESUBTYPE   SubType;
   uint32_t                Bpp;
   uint32_t                NumPlanes;
} SubTypeInfo, *PSubTypeInfo;

SubTypeInfo gSubTypeInfo[] =
{
   { DGCMEDIASAMPLESUBTYPE_UNKNOWN, 0, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_RGB555, 16, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_RGB565, 16, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_RGB888, 32, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_YYYY, 8, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_RGB24, 24, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_YUY2, 16, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_YVYU, 16, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_UYVY, 16, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_NV12, 12, 2 },
   { DGCRAWVIDEOSAMPLESUBTYPE_YV12, 12, 3 },
   { DGCRAWVIDEOSAMPLESUBTYPE_RGB10, 32, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_Y410, 32, 1 },
   { DGCRAWVIDEOSAMPLESUBTYPE_I420, 12, 3 },
   { DGCENCVIDEOSAMPLESUBTYPE_H264, 8, 1 },
};

/******************************************************************************/

uint32_t SubTypeGetIndex( DGCMEDIASAMPLESUBTYPE subType )
{
   uint32_t i;
   for ( i = 0; i < sizeof(gSubTypeInfo) / sizeof(SubTypeInfo); i++ )
   {
      if ( gSubTypeInfo[i].SubType == subType )
      {
         return i;
      }
   }
   return 0; // Unknown
}

/******************************************************************************/

uint32_t SubTypeGetBitDepth( DGCMEDIASAMPLESUBTYPE subType )
{
   return gSubTypeInfo[SubTypeGetIndex( subType )].Bpp;
}

/******************************************************************************/

uint32_t SubTypeGetNumPlanes( DGCMEDIASAMPLESUBTYPE subType )
{
   return gSubTypeInfo[SubTypeGetIndex( subType )].NumPlanes;
}

/******************************************************************************/

uint8_t SubTypeGetPlaneSize( DGCMEDIASAMPLESUBTYPE subType,
                             uint16_t plane,
                             uint32_t width,
                             uint32_t height,
                             uint32_t *pValue )
{
   uint32_t numplanes = SubTypeGetNumPlanes( subType );
   uint32_t bitdepth = SubTypeGetBitDepth( subType );

   if ( (numplanes == 0) || (bitdepth == 0) )
   {
      return FALSE; // unknown
   }

   if ( plane >= numplanes )
   {
      return FALSE; // invalid
   }

   if ( numplanes == 1 )
   {
      *pValue = width * height * bitdepth / 8;
   }
   else
   {
      switch ( subType )
      {
         case DGCRAWVIDEOSAMPLESUBTYPE_NV12:
         {
            if ( plane == 0 )
            {
               *pValue = width * height;
            }
            else
            {
               *pValue = width * height / 2;
            }
            break;
         }

         case DGCRAWVIDEOSAMPLESUBTYPE_YV12:
         case DGCRAWVIDEOSAMPLESUBTYPE_I420:
         {
            if ( plane == 0 )
            {
               *pValue = width * height;
            }
            else
            {
               *pValue = width * height / 4;
            }
            break;
         }

         default:
            return FALSE;
      }
   }

   return TRUE;
}

/******************************************************************************/

uint8_t SubTypeGetPlanePitch( DGCMEDIASAMPLESUBTYPE subType,
                              uint16_t plane,
                              uint32_t width,
                              uint32_t *pValue )
{
   uint32_t numplanes = SubTypeGetNumPlanes( subType );
   uint32_t bitdepth = SubTypeGetBitDepth( subType );

   if ( (numplanes == 0) || (bitdepth == 0) )
   {
      return FALSE; // unknown
   }

   if ( plane >= numplanes )
   {
      return FALSE; // invalid
   }

   if ( numplanes == 1 )
   {
      *pValue = width * bitdepth / 8;
   }
   else
   {
      switch ( subType )
      {
         case DGCRAWVIDEOSAMPLESUBTYPE_NV12:
         {
            *pValue = width;
            break;
         }

         case DGCRAWVIDEOSAMPLESUBTYPE_YV12:
         case DGCRAWVIDEOSAMPLESUBTYPE_I420:
         {
            if ( plane == 0 )
            {
               *pValue = width;
            }
            else
            {
               *pValue = width / 2;
            }
            break;
         }

         default:
            return FALSE;
      }
   }

   return TRUE;
}

/******************************************************************************/

uint8_t SubTypeGetPlaneOffsets( DGCMEDIASAMPLESUBTYPE subType,
                                uint16_t plane,
                                uint32_t offsetX,   //pixels in
                                uint32_t offsetY,   //pixels in
                                uint32_t *pValueX,  //bytes out
                                uint32_t *pValueY ) //lines out
{
   uint32_t numplanes = SubTypeGetNumPlanes( subType );
   uint32_t bitdepth = SubTypeGetBitDepth( subType );

   if ( (numplanes == 0) || (bitdepth == 0) )
   {
      return FALSE; // unknown
   }

   if ( plane >= numplanes )
   {
      return FALSE; // invalid
   }

   if ( numplanes == 1 )
   {
      *pValueX = offsetX * bitdepth / 8;
      *pValueY = offsetY;
   }
   else
   {
      switch ( subType )
      {
         case DGCRAWVIDEOSAMPLESUBTYPE_NV12:
         {
            if ( plane == 0 )
            {
               *pValueX = offsetX;
               *pValueY = offsetY;
            }
            else
            {
               *pValueX = offsetX;
               *pValueY = offsetY / 2;
            }
            break;
         }

         case DGCRAWVIDEOSAMPLESUBTYPE_YV12:
         case DGCRAWVIDEOSAMPLESUBTYPE_I420:
         {
            if ( plane == 0 )
            {
               *pValueX = offsetX;
               *pValueY = offsetY;
            }
            else
            {
               *pValueX = offsetX / 2;
               *pValueY = offsetY / 2;
            }
            break;
         }

         default:
            return FALSE;
      }
   }

   return TRUE;
}

/******************************************************************************/

PDGCMEDIASAMPLE MediaSampleAllocateEx( DGCMEDIASAMPLETYPE     type,
                                       DGCMEDIASAMPLESUBTYPE  subType,
                                       DGCBUFFERHEADERTYPE    bufType )
{
   uint32_t mediaSampleSize = sizeof(DGCMEDIASAMPLE);
   PDGCMEDIASAMPLE pMediaSample =
      (PDGCMEDIASAMPLE) DGCAllocFn( mediaSampleSize );
   if ( pMediaSample )
   {
      pMediaSample->Size = mediaSampleSize;
      pMediaSample->MajorType = type;
      pMediaSample->SubType = subType;
      pMediaSample->PFormatHeader = NULL;
      pMediaSample->BufferHeaderType = bufType;
      pMediaSample->PBufferHeader = NULL;
   }

   return pMediaSample;
}


/******************************************************************************/

PDGCMEDIASAMPLE MediaSampleAllocate( DGCMEDIASAMPLETYPE     type,
           	                         DGCMEDIASAMPLESUBTYPE  subType )
{
   return MediaSampleAllocateEx(type, subType, DGCBUFFERHEADERTYPE_UNKNOWN);
}

/******************************************************************************/

uint8_t MediaSampleAllocateFormatHeader( PDGCMEDIASAMPLE pMediaSample )
{
   uint8_t bReturn = FALSE;
   void *pFormatHeader = NULL;

   if ( (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_RAW_VIDEO) ||
        (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_ENC_VIDEO) )
   {
      uint32_t headerSize = sizeof(DGCVIDEOHEADER);
      pFormatHeader = DGCAllocFn( headerSize );
      if ( pFormatHeader )
      {
         PDGCVIDEOHEADER pVideoHeader = (PDGCVIDEOHEADER) pFormatHeader;
         pVideoHeader->Size = headerSize;
         pVideoHeader->Flags = 0;
         pVideoHeader->Width = 0;
         pVideoHeader->Height = 0;
         pVideoHeader->FrameRate = 0;
         bReturn = TRUE;
      }
   }
   if ( (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_SHARED_RAW_VIDEO) ||
        (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_SHARED_ENC_VIDEO) )
   {
      uint32_t headerSize = sizeof(DGCVIDEOHEADER);
      pMediaSample->hFormatHeaderMapFile = (uintptr_t)CreateFileMapping(
         INVALID_HANDLE_VALUE,    // use paging file
         NULL,                    // default security
         PAGE_READWRITE,          // read/write access
         0,                       // maximum object size (high-order DWORD)
         headerSize,              // maximum object size (low-order DWORD)
         NULL);                   // name of mapping object

      if (pMediaSample->hFormatHeaderMapFile == (uintptr_t)NULL)
      {
         return FALSE;
      }

      pFormatHeader = MapViewOfFile((HANDLE)pMediaSample->hFormatHeaderMapFile,   // handle to map object
         FILE_MAP_ALL_ACCESS, // read/write permission
         0,
         0,
         headerSize);
      
      if (pFormatHeader == NULL)
      {
         CloseHandle((HANDLE)pMediaSample->hFormatHeaderMapFile);

         return FALSE;
      }
     
      if ( pFormatHeader )
      {
         PDGCVIDEOHEADER pVideoHeader = (PDGCVIDEOHEADER) pFormatHeader;
         pVideoHeader->Size = headerSize;
         pVideoHeader->Flags = 0;
         pVideoHeader->Width = 0;
         pVideoHeader->Height = 0;
         pVideoHeader->FrameRate = 0;
         bReturn = TRUE;
      }
   }/*
   else if ( (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_RAW_AUDIO) ||
             (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_ENC_AUDIO) )
   {
      uint32_t headerSize = sizeof(DGCAUDIOHEADER);
      pFormatHeader = DGCAllocFn( headerSize );
      if ( pFormatHeader )
      {
         PDGCAUDIOHEADER pVideoHeader = (PDGCAUDIOHEADER) pFormatHeader;
         pVideoHeader->Size = headerSize;
         pVideoHeader->Flags = 0;
         pVideoHeader->SampleDepth = 0;
         pVideoHeader->SampleFrequency = 0;
         bReturn = TRUE;
      }
   }
   */
   pMediaSample->PFormatHeader = pFormatHeader;

   return bReturn;
}

/******************************************************************************/

uint8_t MediaSampleAllocateBufferHeader( PDGCMEDIASAMPLE       pMediaSample,
                                         DGCBUFFERHEADERTYPE   bufferType )
{
   uint8_t  bReturn = FALSE;
   uint32_t numPlanes = SubTypeGetNumPlanes( pMediaSample->SubType );
   void *pBufferHeader = NULL;
      
   switch ( bufferType )
   {
      case DGCBUFFERHEADERTYPE_MEMORY:
      {
         uint32_t bufferSize = sizeof(DGCMEMORYBUFFERHEADER);
         pBufferHeader = DGCAllocFn(bufferSize);
         if (pBufferHeader)
         {
            uint32_t i;
            PDGCMEMORYBUFFERHEADER pMemoryBufferHeader =
               (PDGCMEMORYBUFFERHEADER)pBufferHeader;

            pMemoryBufferHeader->Size = sizeof(DGCMEMORYBUFFERHEADER);
            pMemoryBufferHeader->Flags = 0;
            pMemoryBufferHeader->StartTime = 0;
            pMemoryBufferHeader->EndTime = 0;
            pMemoryBufferHeader->NumberOfPlanes = numPlanes;

            for (i = 0; i < numPlanes; i++)
            {
               PDGCMEMORYBUFFER pDesc = &pMemoryBufferHeader->Planes[i];
               if (pDesc)
               {
                  pDesc->Size = sizeof(DGCMEMORYBUFFER);
                  pDesc->Flags = 0;
                  pDesc->PBuffer = NULL;
                  pDesc->Length = 0;
                  pDesc->Pitch = 0;
                  pDesc->OffsetX = 0;
                  pDesc->OffsetY = 0;
                  pDesc->ActualLength = 0;
               }
            }

            bReturn = TRUE;
         }
         break;
      }
      case DGCBUFFERHEADERTYPE_SHARED_MEMORY:
      {
         
         uint32_t bufferSize = sizeof(DGCMEMORYBUFFERHEADER);       

         pMediaSample->hBuffHeaderMapFile = (uintptr_t)CreateFileMapping(
         INVALID_HANDLE_VALUE,    // use paging file
         NULL,                    // default security
         PAGE_READWRITE,          // read/write access
         0,                       // maximum object size (high-order DWORD)
         bufferSize,              // maximum object size (low-order DWORD)
         0);                      // name of mapping object

         if (pMediaSample->hBuffHeaderMapFile == (uintptr_t)NULL)
         {
            return FALSE;
         }

         pBufferHeader = MapViewOfFile((HANDLE)pMediaSample->hBuffHeaderMapFile,   // handle to map object
            FILE_MAP_ALL_ACCESS, // read/write permission
            0,
            0,
            bufferSize);


         if (pBufferHeader == NULL)
         {
            CloseHandle((HANDLE)pMediaSample->hBuffHeaderMapFile);

            return FALSE;
         }

         if ( pBufferHeader )
         {
            uint32_t i;
            PDGCMEMORYBUFFERHEADER pMemoryBufferHeader =
               (PDGCMEMORYBUFFERHEADER)pBufferHeader;
            
            pMemoryBufferHeader->Size = sizeof(DGCMEMORYBUFFERHEADER);
            pMemoryBufferHeader->Flags = 0;
            pMemoryBufferHeader->StartTime = 0;
            pMemoryBufferHeader->EndTime = 0;
            pMemoryBufferHeader->NumberOfPlanes = numPlanes;
            pMemoryBufferHeader->AllocationStrategy = DGCBUFFERTYPE_INDIVIDUAL;
            pMemoryBufferHeader->SlabAllocationSize = 0;

            for ( i = 0; i < numPlanes; i++ )
            {
               PDGCMEMORYBUFFER pDesc = &pMemoryBufferHeader->Planes[i];
               if ( pDesc )
               {
                  pDesc->Size = sizeof(DGCMEMORYBUFFER);
                  pDesc->Flags = 0;
                  pDesc->PBuffer = NULL;
                  pDesc->Length = 0;
                  pDesc->Pitch = 0;
                  pDesc->OffsetX = 0;
                  pDesc->OffsetY = 0;
                  pDesc->ActualLength = 0;
               }
            }

            bReturn = TRUE;
         }
         break;
      }
      case DGCMEMORYBUFFERHEADER_SGT:
      {
         uint32_t bufferSize = sizeof(DGCMEMORYBUFFERHEADERSGT);
         pBufferHeader = DGCAllocFn(bufferSize);
         if (pBufferHeader)
         {
            uint32_t i;
            PDGCMEMORYBUFFERHEADERSGT pMemoryBufferHeader =
               (PDGCMEMORYBUFFERHEADERSGT)pBufferHeader;

            pMemoryBufferHeader->Size = sizeof(DGCMEMORYBUFFERHEADERSGT);
            pMemoryBufferHeader->Flags = 0;
            pMemoryBufferHeader->StartTime = 0;
            pMemoryBufferHeader->EndTime = 0;
            pMemoryBufferHeader->NumberOfPlanes = numPlanes;

            for (i = 0; i < numPlanes; i++)
            {
               PDGCMEMORYBUFFER pDesc = &pMemoryBufferHeader->Planes[i];
               if (pDesc)
               {
                  pDesc->Size = sizeof(DGCMEMORYBUFFERHEADERSGT);
                  pDesc->Flags = 0;
                  pDesc->PBuffer = NULL;
                  pDesc->Length = 0;
                  pDesc->Pitch = 0;
                  pDesc->OffsetX = 0;
                  pDesc->OffsetY = 0;
                  pDesc->ActualLength = 0;
               }
               pMemoryBufferHeader->pMDL[i] = 0;
               pMemoryBufferHeader->pWinSGL[i] = 0;
               pMemoryBufferHeader->pPhyBuffer[i] = 0;
            }
            bReturn = TRUE;
         }
         break;
      }
   }

   pMediaSample->BufferHeaderType = bufferType;
   pMediaSample->PBufferHeader = pBufferHeader;

   return bReturn;
}

/******************************************************************************/

uint8_t MediaSampleInitialisePlane( PDGCMEDIASAMPLE       pMediaSample,
                                    uint16_t              plane,
                                    uint32_t              bufferSize )
{
   uint8_t  bReturn = FALSE;

   switch ( pMediaSample->BufferHeaderType )
   {
      case DGCBUFFERHEADERTYPE_MEMORY:
      {
         PDGCMEMORYBUFFERHEADER pMemoryBufferHeader =
            (PDGCMEMORYBUFFERHEADER)pMediaSample->PBufferHeader;
         if (pMemoryBufferHeader)
         {
            pMemoryBufferHeader->AllocationStrategy = DGCBUFFERTYPE_INDIVIDUAL;

            if (pMemoryBufferHeader->NumberOfPlanes > plane)
            {
               PDGCMEMORYBUFFER pDesc =
                  &pMemoryBufferHeader->Planes[plane];

               pDesc->PBuffer = DGCAllocFn(bufferSize);
               if (pDesc->PBuffer)
               {
                  pDesc->Length = bufferSize;
                  bReturn = TRUE;
               }
            }
         }

         break;
      }
      case DGCBUFFERHEADERTYPE_SHARED_MEMORY:
      {
         PDGCMEMORYBUFFERHEADER pMemoryBufferHeader =
            (PDGCMEMORYBUFFERHEADER) pMediaSample->PBufferHeader;
         if ( pMemoryBufferHeader )
         {
            pMemoryBufferHeader->AllocationStrategy = DGCBUFFERTYPE_INDIVIDUAL;

            if ( pMemoryBufferHeader->NumberOfPlanes > plane )
            {
               PDGCMEMORYBUFFER pDesc =
                  &pMemoryBufferHeader->Planes[plane];
                  pMemoryBufferHeader->hPlaneMapFile = (uintptr_t)CreateFileMapping(
                  INVALID_HANDLE_VALUE,    // use paging file
                  NULL,                    // default security
                  PAGE_READWRITE,          // read/write access
                  0,                       // maximum object size (high-order DWORD)
                  bufferSize,              // maximum object size (low-order DWORD)
                  0);                      // name of mapping object

               if (pMemoryBufferHeader->hPlaneMapFile == (uintptr_t)NULL)
               {
                  return FALSE;
               }

               pDesc->PBuffer = MapViewOfFile((HANDLE)pMemoryBufferHeader->hPlaneMapFile,   // handle to map object
                  FILE_MAP_ALL_ACCESS, // read/write permission
                  0,
                  0,
                  bufferSize);

               if ( pDesc->PBuffer )
               {
                  pDesc->Length = bufferSize;
                  bReturn = TRUE;
               }
            }
         }

         break;
      }
   }

   return bReturn;
}

/******************************************************************************/

uint8_t MediaSampleInitialisePlaneWH( PDGCMEDIASAMPLE       pMediaSample,
                                      uint16_t              plane,
                                      uint32_t              width,
                                      uint32_t              height )
{
   uint8_t bReturn = FALSE;

   if ( pMediaSample->MajorType == DGCMEDIASAMPLETYPE_RAW_VIDEO ||
        pMediaSample->MajorType == DGCMEDIASAMPLETYPE_SHARED_RAW_VIDEO)
   {
      uint32_t bufferSize;

      bReturn = SubTypeGetPlaneSize( pMediaSample->SubType, plane,
                                     width, height, &bufferSize );
      if ( bReturn )
      {
         uint32_t pitch;         

         bReturn = SubTypeGetPlanePitch( pMediaSample->SubType, plane,
                                         width, &pitch );
         if ( bReturn )
         {
            PDGCMEMORYBUFFERHEADER pHeader;

            pHeader = (PDGCMEMORYBUFFERHEADER)pMediaSample->PBufferHeader;

            pHeader->Planes[plane].Pitch = pitch;            
      
            bReturn = MediaSampleInitialisePlane( pMediaSample,
                                                  plane,
                                                  bufferSize );
         }
      }
   }

   return bReturn;
}

uint8_t MediaSampleInitialisePlanes( PDGCMEDIASAMPLE pMediaSample,
                                     uint32_t width,
                                     uint32_t height,
                                     DGCALLOCATION Allocation,
                                     DWORD dwPlaneAlignment)
{
   uint32_t i = 0;
   PDGCMEMORYBUFFERHEADER pMemoryBufferHeader = NULL;
   uint32_t bufsize = 0, dwTruePlaneSize = 0;
   PDGCMEMORYBUFFER pDesc = NULL;
   uint8_t *pBuffer = NULL, *pLastBuffer = NULL;
   DWORD dwPlaneSize = 0;
   DWORD dwAlignMask = 0;
   if ( dwPlaneAlignment == 0 )
   {
      dwPlaneAlignment = 16;
   }
   switch ( dwPlaneAlignment )
   {
   case 1:
      break; // At the moment, an alignment of anything other than 1 will break the kernel interface.
             // as there's no way for the kernel to know where the dead space in the slab of allocated
             // memory is between planes.  So we only allow 1 at the moment.
   case 2:
   case 4:
   case 8:
   case 16:
   case 32:
   case 64:
   case 128:
   case 256:
   default:
      return 0;
   }
   dwAlignMask = dwPlaneAlignment - 1;
   if ( !(pMediaSample->MajorType == DGCMEDIASAMPLETYPE_RAW_VIDEO ||
        pMediaSample->MajorType == DGCMEDIASAMPLETYPE_SHARED_RAW_VIDEO))
   {
      return 0;
   }
   if ( ( pMediaSample == NULL ) || ( pMediaSample->PBufferHeader == NULL ) )
   {
      return 0;
   }
   pMemoryBufferHeader = ( PDGCMEMORYBUFFERHEADER ) pMediaSample->PBufferHeader;
   if ( !(pMediaSample->BufferHeaderType == DGCBUFFERHEADERTYPE_MEMORY ||
        pMediaSample->BufferHeaderType == DGCBUFFERHEADERTYPE_SHARED_MEMORY))
   {
      return 0;
   }
   pMemoryBufferHeader->AllocationStrategy = Allocation;
   switch ( Allocation )
   {
   case DGCBUFFERTYPE_ONE_SLAB:
      // All planes allocated in one memory slab, then individual plane
      // pointers point within this slab. Plane[0] "owns" the entire slab.
      for ( i = 0; i < pMemoryBufferHeader->NumberOfPlanes; i++ )
      {         
         SubTypeGetPlaneSize( pMediaSample->SubType, i,
                              width, height, &dwPlaneSize );
         // Make all planes a multiple of at least 16 bytes for alignment purposes.
         if ( dwPlaneSize & dwAlignMask ) {
            dwPlaneSize += ( dwPlaneAlignment - ( dwPlaneSize & dwAlignMask ) );
         }
         bufsize += dwPlaneSize;
      }
      // Allocate all the memory, have it "owned" by plane 0
      pBuffer = DGCAllocFn( bufsize );
      if ( pBuffer == NULL )
      {
         return 1;
      }
      pDesc = &pMemoryBufferHeader->Planes[0];
      pDesc->PBuffer = pBuffer;
      pDesc->Size    = sizeof( DGCMEMORYBUFFER );
      SubTypeGetPlaneSize( pMediaSample->SubType, 0, width, height, &dwTruePlaneSize );
      pDesc->Length = dwTruePlaneSize;
      pDesc->Pitch  = width;
      pDesc->OffsetX = pDesc->OffsetY = 0;
      pDesc->ActualLength = dwTruePlaneSize;
      pLastBuffer = pBuffer;
      // Then "allocate" the remaining planes by filling in their pointers
      // based on the length and start of the previous plane's buffer.
      for ( i = 1; i < pMemoryBufferHeader->NumberOfPlanes; i++ )
      {
         DWORD dwTruePlaneSize;
         SubTypeGetPlaneSize( pMediaSample->SubType, i-1,
                              width, height, &dwTruePlaneSize );
         dwPlaneSize = dwTruePlaneSize;
         if ( dwTruePlaneSize & dwAlignMask ) {
            dwPlaneSize += ( dwPlaneAlignment - ( dwTruePlaneSize & dwAlignMask ) );
         }
         pDesc = &pMemoryBufferHeader->Planes[i];
         pDesc->PBuffer = pLastBuffer + dwPlaneSize;
         // Can't lie to the plane consumers about the size.
         pDesc->Length = dwTruePlaneSize;
         pDesc->Pitch  = width;
         pDesc->OffsetX = pDesc->OffsetY = 0;
         pDesc->ActualLength = dwTruePlaneSize;
         pLastBuffer = pDesc->PBuffer;
      }
      pMemoryBufferHeader->SlabAllocationSize = bufsize;
      break;
   case DGCBUFFERTYPE_INDIVIDUAL:
      // All planes allocated individually.
      for ( i = 0; i < pMemoryBufferHeader->NumberOfPlanes; i++ )
      {
         MediaSampleInitialisePlaneWH( pMediaSample, i, width, height );
      }
      break;
   default:
      return 0;
   }
   return 1;
}

/******************************************************************************/

void MediaSampleFree( PDGCMEDIASAMPLE pMediaSample )
{
   PDGCMEMORYBUFFER pDesc = NULL;
   if ( pMediaSample )
   {
      if ( pMediaSample->PFormatHeader )
      {
         if ((pMediaSample->MajorType == DGCMEDIASAMPLETYPE_RAW_VIDEO) ||
            (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_ENC_VIDEO))
         {
            DGCFreeFn( pMediaSample->PFormatHeader );
         }
        
         if ((pMediaSample->MajorType == DGCMEDIASAMPLETYPE_SHARED_RAW_VIDEO) ||
            (pMediaSample->MajorType == DGCMEDIASAMPLETYPE_SHARED_ENC_VIDEO))
         {
            UnmapViewOfFile(pMediaSample->PFormatHeader);
            CloseHandle((HANDLE)pMediaSample->hFormatHeaderMapFile);
         }
      }

      if ( pMediaSample->PBufferHeader )
      {
         switch ( pMediaSample->BufferHeaderType )
         {
            case DGCBUFFERHEADERTYPE_MEMORY:
            {
               uint16_t i;
               PDGCMEMORYBUFFERHEADER pMemoryBufferHeader =
                  (PDGCMEMORYBUFFERHEADER) pMediaSample->PBufferHeader;
               switch ( pMemoryBufferHeader->AllocationStrategy )
               {
               case DGCBUFFERTYPE_ONE_SLAB:
                  pDesc = &pMemoryBufferHeader->Planes[0];
                  DGCFreeFn( pDesc->PBuffer );
                  break;
               case DGCBUFFERTYPE_INDIVIDUAL:
                  for ( i = 0; i < pMemoryBufferHeader->NumberOfPlanes; i++ )
                  {
                     if ( pMemoryBufferHeader->Planes[i].PBuffer )
                     {
                        DGCFreeFn( pMemoryBufferHeader->Planes[i].PBuffer );
                     }
                  }
                  break;
               }
               DGCFreeFn( pMediaSample->PBufferHeader );
            }
            break;

            case DGCBUFFERHEADERTYPE_SHARED_MEMORY:
            {
               uint16_t i;
               PDGCMEMORYBUFFERHEADER pMemoryBufferHeader =
                  (PDGCMEMORYBUFFERHEADER)pMediaSample->PBufferHeader;
               switch (pMemoryBufferHeader->AllocationStrategy)
               {
               case DGCBUFFERTYPE_ONE_SLAB:
                  pDesc = &pMemoryBufferHeader->Planes[0];
                  DGCFreeFn(pDesc->PBuffer);
                  break;
               case DGCBUFFERTYPE_INDIVIDUAL:
                  for (i = 0; i < pMemoryBufferHeader->NumberOfPlanes; i++)
                  {
                     if (pMemoryBufferHeader->Planes[i].PBuffer)
                     {
                        UnmapViewOfFile(pMemoryBufferHeader->Planes[i].PBuffer);
                        if (pMemoryBufferHeader->hPlaneMapFile)
                           CloseHandle((HANDLE)pMemoryBufferHeader->hPlaneMapFile);
                     }
                  }
                  break;
               }
            }
            UnmapViewOfFile(pMediaSample->PBufferHeader);
            CloseHandle((HANDLE)pMediaSample->hBuffHeaderMapFile);
            break;
         }
      }
      DGCFreeFn( pMediaSample );
   }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
