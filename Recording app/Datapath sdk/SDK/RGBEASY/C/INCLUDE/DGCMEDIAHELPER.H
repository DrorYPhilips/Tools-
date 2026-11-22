/*******************************************************************************

Copyright Datapath Ltd. 2014, 2015

File:    dgcmediahelper.h

Purpose: Helper functionality for Audio and Video media types.

History:
         09 JUN 14    JL   Created.

*******************************************************************************/
#if defined (_LOCAL_HEAP_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

typedef void* (*PDGCALLOCFN) (uintptr_t size);
typedef void  (*PDGCFREEFN ) (void *pData);
typedef void* (*PDGCALLOCFNALIGN) (uintptr_t size, size_t bytealign);
typedef void  (*PDGCFREEFNALIGN) (void *pData);

uint32_t SubTypeGetBitDepth( DGCMEDIASAMPLESUBTYPE subType );
uint32_t SubTypeGetNumPlanes( DGCMEDIASAMPLESUBTYPE subType );

// For use with DGCRAWVIDEOSAMPLESUBTYPE_* values
uint8_t SubTypeGetPlaneSize( DGCMEDIASAMPLESUBTYPE subType,
                             uint16_t plane,
                             uint32_t width,
                             uint32_t height,
                             uint32_t *pValue );

uint8_t SubTypeGetPlanePitch( DGCMEDIASAMPLESUBTYPE subType,
                              uint16_t plane,
                              uint32_t width,
                              uint32_t *pValue );

uint8_t SubTypeGetPlaneOffsets( DGCMEDIASAMPLESUBTYPE subType,
                                uint16_t plane,
                                uint32_t offsetX,   //pixels
                                uint32_t offsetY,   //pixels
                                uint32_t *pValueX,  //bytes
                                uint32_t *pValueY );//bytes

#if defined (_LOCAL_HEAP_DEBUG)
#define MediaSampleSetAllocFunctionPair( allocFn, freeFn)
#else
uint8_t MediaSampleSetAllocFunctionPair( PDGCALLOCFN allocFn,
                                         PDGCFREEFN  freeFn );
#endif

PDGCMEDIASAMPLE MediaSampleAllocate( DGCMEDIASAMPLETYPE     type,
                                     DGCMEDIASAMPLESUBTYPE  subType );

PDGCMEDIASAMPLE MediaSampleAllocateEx( DGCMEDIASAMPLETYPE     type,
                                       DGCMEDIASAMPLESUBTYPE  subType,
                                       DGCBUFFERHEADERTYPE    bufType );

uint8_t MediaSampleAllocateFormatHeader( PDGCMEDIASAMPLE pMediaSample );

uint8_t MediaSampleAllocateBufferHeader( PDGCMEDIASAMPLE       pMediaSample,
                                         DGCBUFFERHEADERTYPE   bufferType );

uint8_t MediaSampleInitialisePlane( PDGCMEDIASAMPLE       pMediaSample,
                                    uint16_t              plane,
                                    uint32_t              bufferSize );

uint8_t MediaSampleInitialisePlaneWH( PDGCMEDIASAMPLE       pMediaSample,
                                      uint16_t              plane,
                                      uint32_t              width,
                                      uint32_t              height );

uint8_t MediaSampleInitialisePlanes( PDGCMEDIASAMPLE pMediaSample,
                                     uint32_t width,
                                     uint32_t height,
                                     DGCALLOCATION Allocation,
                                     DWORD dwPlaneAlignment );
   
void MediaSampleFree( PDGCMEDIASAMPLE pMediaSample );

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
