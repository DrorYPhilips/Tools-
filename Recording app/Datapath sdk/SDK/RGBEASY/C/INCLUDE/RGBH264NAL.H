/*******************************************************************************
Copyright Datapath Ltd. 2014.

File:    rgbh264nal.h

History: 30 OCT 14    RL   Created.
         12 OCT 15    DC   Added further hard-coded URLs.
                           Included GOP Length as an option in URLs.
         
*******************************************************************************/
#pragma once

// Internal buffer count containing multiple NALU's
#define NUM_SAMPLES  8

typedef enum _RGBEasyH264_FRAME_STATE
{
   RGBEasyH264_FRAME_STATE_UNINITIALISED,
   RGBEasyH264_FRAME_STATE_INITIALISED,
   RGBEasyH264_FRAME_STATE_STARTED,
   RGBEasyH264_FRAME_STATE_STOPING,
   RGBEasyH264_FRAME_STATE_STOPPED,
}  RGBEasyH264_FRAME_STATE, *PRGBEasyH264_FRAME_STATE;

typedef struct _StreamInfo {
   uint32_t Width;
   uint32_t Height;
   uint32_t FPS;
   uint32_t EncodeAffinity;
} StreamInfo, *PStreamInfo;

typedef struct _H264NAL
{
   struct _H264NAL   *PNext;
   uint8_t           *PStart;
   uint32_t          Size;
   uint64_t          StartTime;
   uint64_t          Seq;
   uint64_t          bLastNALInSeq;
} H264NAL, *PH264NAL;

uint32_t GetInputSignalType (
   uint32_t input,
   uint32_t *pWidth,
   uint32_t *pHeight,
   uint32_t *pFPS );

uint32_t GetSupportedH264Inputs (
   uint32_t **ppInputList,
   uint32_t *plength,
   uint32_t *pH264Count);

BOOL DoesH264NALUnitBeginNewAccessUnit(
   uint8_t *pNalUnit);
