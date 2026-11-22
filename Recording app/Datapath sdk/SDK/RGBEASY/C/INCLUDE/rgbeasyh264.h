#pragma once
#include "RGB.H"
#include "rgbh264nal.h"

class CRGBEasyH264
{
public:

   CRGBEasyH264( uint32_t input );
   ~CRGBEasyH264( );

   uint32_t RGBEasyH264Start( uint32_t width,
      uint32_t height,
      uint32_t frameRate,
      DGCENCLEVEL level,
      DGCENCPROFILE profile,
      uint32_t bitrate,
      uint32_t keyframeinterval,
      uint32_t encodeAffinity ,
      uint32_t UdpPort,
      char     *UdpUrl,
      PRGBMEDIASAMPLECAPTUREDFN  CallbackFn,
      PRGBENCRYPTIONKEYSFN pEncryptionKeysFn,
      uintptr_t userdata);

   uint32_t RGBEasyH264GetNAL( uint8_t* pNAL, uint32_t maxSize, uint32_t *pTrunkSize,
                               uint32_t *pCopySize, uint64_t *pTimeStamp, uint64_t *pSeq, uint64_t *pLastNALInSequence );
   uint32_t RGBEasyH264Stop( );
private:

   uint32_t RGBEasyH264Init( uint32_t input );
   uint32_t RGBEasyH264UnInit( );

   PDGCMEDIASAMPLE CreateMediaSample(uint32_t width, uint32_t height, DGCMEDIASAMPLETYPE majorType, DGCMEDIASAMPLESUBTYPE subType);
   void AddNALToList( PH264NAL pH264NAL );
   void CheckListRemoveFirstNALFromList( PH264NAL* ppH264NAL );
   void LockedRemoveNALFromList( PH264NAL pH264NALToRemove, PH264NAL pH264NALPrevToToRemove );

   void RGBEasyH264RealEncoderErrorFn( HRGB hRGB, unsigned long error );
   static void RGBCBKAPI RGBEasyH264EncoderErrorFn( HWND              hWnd,
                                                    HRGB              hRGB,
                                                    unsigned long     error,
                                                    ULONG_PTR         pUserData,
                                                    unsigned long     *pReserved );

   void RGBEasyH264RealEncoderFrameFn( HRGB hRGB, PDGCMEDIASAMPLE pMediaSample );

   void RGBEasyRealEncryptionKeysFn( HRGB hRGB, PSQXENCRYPTIONINFO pEncryptionKeys );
public:
      static void RGBCBKAPI RGBEasyH264EncoderFrameFn( HWND              hWnd,
                                                    HRGB              hRGB,
                                                    PDGCMEDIASAMPLE   pMediaSample,
                                                    ULONG_PTR         pUserData );
      static void RGBCBKAPI RGBEasyEncryptionKeysFn(HWND                 hWnd,
         HRGB                 hRGB,
         PSQXENCRYPTIONINFO   pEncryptionKeys,
         ULONG_PTR            pUserData);

   StreamInfo              m_StreamInfo;

private:
   // If the NAL processor has left any NALs on the list, then
   // we need to be able to remove them in an orderly fashion.
   void DrainNALList( );
#if !defined(USE_OLD_NAL_PROCESSOR)
   unsigned char * GetNextNAL( unsigned char * pBuffer, uint32_t *pBufferLen, unsigned char ** pNAL, unsigned long * NALlength );
   unsigned char * FindNextHeader( unsigned char * pBuffer, int32_t length );
#else
   unsigned char * GetNextNAL( unsigned char * pBuffer, uint32_t *pBufferLen, unsigned char ** pNAL, unsigned long * NALlength );
#endif
   uint32_t                m_error;
   uint32_t                m_input;
   RGBEasyH264_FRAME_STATE m_state;
   HINSTANCE               m_hInstance;
   HRGB                    m_hRGB;

   PDGCMEDIASAMPLE         m_pMediaSample[NUM_SAMPLES];

   PH264NAL                m_pNAL_List;
   HANDLE                  m_hListMutex;
   HANDLE                  m_AdderMutex;
   uint32_t                m_startCodeOffset;
   uint64_t                m_queuedNALs;
   uint64_t                m_unqueuedNALs;
   uint64_t                m_NAL_ListSize;
   uint64_t                m_NAL_seq;
};
