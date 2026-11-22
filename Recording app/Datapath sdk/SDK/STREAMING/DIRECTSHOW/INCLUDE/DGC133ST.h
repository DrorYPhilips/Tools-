

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Feb 16 12:37:47 2022
 */
/* Compiler settings for DGC133ST.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __DGC133ST_h__
#define __DGC133ST_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IVision_FWD_DEFINED__
#define __IVision_FWD_DEFINED__
typedef interface IVision IVision;
#endif 	/* __IVision_FWD_DEFINED__ */


#ifndef __IVisionUser_FWD_DEFINED__
#define __IVisionUser_FWD_DEFINED__
typedef interface IVisionUser IVisionUser;
#endif 	/* __IVisionUser_FWD_DEFINED__ */


#ifndef __IDGC103ControlConfig_FWD_DEFINED__
#define __IDGC103ControlConfig_FWD_DEFINED__
typedef interface IDGC103ControlConfig IDGC103ControlConfig;
#endif 	/* __IDGC103ControlConfig_FWD_DEFINED__ */


#ifndef __IVisionUser2_FWD_DEFINED__
#define __IVisionUser2_FWD_DEFINED__
typedef interface IVisionUser2 IVisionUser2;
#endif 	/* __IVisionUser2_FWD_DEFINED__ */


#ifndef __IVisionUserGanging_FWD_DEFINED__
#define __IVisionUserGanging_FWD_DEFINED__
typedef interface IVisionUserGanging IVisionUserGanging;
#endif 	/* __IVisionUserGanging_FWD_DEFINED__ */


#ifndef __IVisionUserOverlay_FWD_DEFINED__
#define __IVisionUserOverlay_FWD_DEFINED__
typedef interface IVisionUserOverlay IVisionUserOverlay;
#endif 	/* __IVisionUserOverlay_FWD_DEFINED__ */


#ifndef __IVisionCallBack_FWD_DEFINED__
#define __IVisionCallBack_FWD_DEFINED__
typedef interface IVisionCallBack IVisionCallBack;
#endif 	/* __IVisionCallBack_FWD_DEFINED__ */


#ifndef __IVisionEvent_FWD_DEFINED__
#define __IVisionEvent_FWD_DEFINED__
typedef interface IVisionEvent IVisionEvent;
#endif 	/* __IVisionEvent_FWD_DEFINED__ */


#ifndef __IVisionEvent2_FWD_DEFINED__
#define __IVisionEvent2_FWD_DEFINED__
typedef interface IVisionEvent2 IVisionEvent2;
#endif 	/* __IVisionEvent2_FWD_DEFINED__ */


#ifndef __IVisionUserEvent_FWD_DEFINED__
#define __IVisionUserEvent_FWD_DEFINED__
typedef interface IVisionUserEvent IVisionUserEvent;
#endif 	/* __IVisionUserEvent_FWD_DEFINED__ */


#ifndef __IVisionPin_FWD_DEFINED__
#define __IVisionPin_FWD_DEFINED__
typedef interface IVisionPin IVisionPin;
#endif 	/* __IVisionPin_FWD_DEFINED__ */


#ifndef __IVisionUserPin_FWD_DEFINED__
#define __IVisionUserPin_FWD_DEFINED__
typedef interface IVisionUserPin IVisionUserPin;
#endif 	/* __IVisionUserPin_FWD_DEFINED__ */


#ifndef __IVisionUserPin2_FWD_DEFINED__
#define __IVisionUserPin2_FWD_DEFINED__
typedef interface IVisionUserPin2 IVisionUserPin2;
#endif 	/* __IVisionUserPin2_FWD_DEFINED__ */


#ifndef __IVisionUserPinGamut_FWD_DEFINED__
#define __IVisionUserPinGamut_FWD_DEFINED__
typedef interface IVisionUserPinGamut IVisionUserPinGamut;
#endif 	/* __IVisionUserPinGamut_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __VisionStreaming_LIBRARY_DEFINED__
#define __VisionStreaming_LIBRARY_DEFINED__

/* library VisionStreaming */
/* [control][helpstring][version][uuid] */ 


enum __MIDL___MIDL_itf_DGC133ST_0000_0000_0001
    {	VISION_NOSIGNAL	= 0,
	VISION_ANALOG	= ( VISION_NOSIGNAL + 1 ) ,
	VISION_DVI	= ( VISION_ANALOG + 1 ) ,
	VISION_VIDEO	= ( VISION_DVI + 1 ) ,
	VISION_SDI	= ( VISION_VIDEO + 1 ) ,
	VISION_INVALID	= ( VISION_SDI + 1 ) ,
	VISION_DISPLAYPORT	= ( VISION_INVALID + 1 ) 
    } ;
typedef /* [public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0000_0000_0002
    {	BLACK	= 0,
	WHITE	= ( BLACK + 1 ) ,
	YELLOW	= ( WHITE + 1 ) ,
	CYAN	= ( YELLOW + 1 ) ,
	GREEN	= ( CYAN + 1 ) ,
	MAGENTA	= ( GREEN + 1 ) ,
	RED	= ( MAGENTA + 1 ) ,
	BLUE	= ( RED + 1 ) ,
	GREY	= ( BLUE + 1 ) 
    } 	COLOUR;

#define	MAX_COLOUR	( 9 )

struct CLRBALANCE
    {
    long BrightnessRed;
    long BrightnessGreen;
    long BrightnessBlue;
    long ContrastRed;
    long ContrastGreen;
    long ContrastBlue;
    } ;
typedef /* [public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0000_0000_0003
    {	CLRDOMAINDETECT_RGB709_FULL	= 0,
	CLRDOMAINDETECT_YUV709_FULL	= 1,
	CLRDOMAINDETECT_YUV601_FULL	= 2,
	CLRDOMAINDETECT_YUV709_STUDIO	= 3,
	CLRDOMAINDETECT_YUV601_STUDIO	= 4,
	CLRDOMAINDETECT_AUTO	= 5,
	CLRDOMAINDETECT_RGB709_STUDIO	= 6,
	CLRDOMAINDETECT_YUV2020_FULL	= 7,
	CLRDOMAINDETECT_YUV2020_STUDIO	= 8,
	CLRDOMAINDETECT_RGB601_FULL	= 9,
	CLRDOMAINDETECT_RGB601_STUDIO	= 10,
	CLRDOMAINDETECT_RGB2020_FULL	= 11,
	CLRDOMAINDETECT_RGB2020_STUDIO	= 12,
	CLRDOMAINDETECT_LAST	= 13,
	CLRDOMAINDETECT_RGB	= CLRDOMAINDETECT_RGB709_FULL,
	CLRDOMAINDETECT_HDYPrPb	= CLRDOMAINDETECT_YUV709_FULL,
	CLRDOMAINDETECT_YPrPb	= CLRDOMAINDETECT_YUV601_FULL,
	CLRDOMAINDETECT_HDYCrCb	= CLRDOMAINDETECT_YUV709_STUDIO,
	CLRDOMAINDETECT_YCrCb	= CLRDOMAINDETECT_YUV601_STUDIO
    } 	CLRDOMAINDETECT;

typedef /* [public][public] */ enum __MIDL___MIDL_itf_DGC133ST_0000_0000_0003 CLRDOMAIN;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0000_0000_0004
    {	PIXEL_GAMUT_709	= 0,
	PIXEL_GAMUT_601	= 1,
	PIXEL_GAMUT_2020	= 2,
	PIXEL_GAMUT_COUNT	= 3
    } 	PIXEL_GAMUT;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0000_0000_0005
    {	PIXEL_RANGE_FULL	= 0,
	PIXEL_RANGE_LIMITED	= 1,
	PIXEL_RANGE_COUNT	= 2
    } 	PIXEL_RANGE;

typedef /* [public][public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0001_0149_0001
    {	GANG_TYPE_DISABLED	= 0,
	GANG_TYPE_2x1	= 4,
	GANG_TYPE_1x2	= 5,
	GANG_TYPE_3x1	= 6,
	GANG_TYPE_1x3	= 7,
	GANG_TYPE_4x1	= 2,
	GANG_TYPE_1x4	= 3,
	GANG_TYPE_2x2	= 1
    } 	GANG_TYPE;


enum __MIDL___MIDL_itf_DGC133ST_0001_0152_0001
    {	VISION_EVENT_SIGNAL_CHANGE	= 0x1,
	VISION_EVENT_CHANNEL_CHANGE	= 0x2,
	VISION_EVENT_PARAMETERS_CHANGE	= 0x4,
	VISION_EVENT_ANY_CHANGE	= 0x7
    } ;
struct CROP
    {
    long Top;
    long Left;
    unsigned long Width;
    unsigned long Height;
    } ;
typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0001_0155_0001
    {	DEINTERLACE_WEAVE	= 0,
	DEINTERLACE_BOB	= 1,
	DEINTERLACE_FIELD_0	= 2,
	DEINTERLACE_FIELD_1	= 3
    } 	DEINTERLACE_METHOD;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0001_0155_0002
    {	LIVESTREAM_CLIENT_OFF	= 0,
	LIVESTREAM_CLIENT_1	= 1
    } 	LIVESTREAM_CLIENT;


enum __MIDL___MIDL_itf_DGC133ST_0001_0156_0001
    {	LIVESTREAM_UNSUPPORTED	= 0,
	LIVESTREAM_SELECTABLE	= 1,
	LIVESTREAM_ALWAYS_ON	= 2
    } ;
typedef /* [public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0001_0157_0001
    {	ENC_PROFILE_UNKNOWN	= 0,
	ENC_H264_PROFILE_UNKNOWN	= 0,
	ENC_H264_PROFILE_BASELINE	= ( ENC_H264_PROFILE_UNKNOWN + 1 ) ,
	ENC_H264_PROFILE_MAIN	= ( ENC_H264_PROFILE_BASELINE + 1 ) ,
	ENC_H264_PROFILE_EXTENDED	= ( ENC_H264_PROFILE_MAIN + 1 ) ,
	ENC_H264_PROFILE_HIGH	= ( ENC_H264_PROFILE_EXTENDED + 1 ) 
    } 	ENCPROFILE;

typedef /* [public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_DGC133ST_0001_0157_0002
    {	ENC_LEVEL_UNKNOWN	= 0,
	ENC_H264_LEVEL_UNKNOWN	= 0,
	ENC_H264_LEVEL_1	= ( ENC_H264_LEVEL_UNKNOWN + 1 ) ,
	ENC_H264_LEVEL_1B	= ( ENC_H264_LEVEL_1 + 1 ) ,
	ENC_H264_LEVEL_1_1	= ( ENC_H264_LEVEL_1B + 1 ) ,
	ENC_H264_LEVEL_1_2	= ( ENC_H264_LEVEL_1_1 + 1 ) ,
	ENC_H264_LEVEL_1_3	= ( ENC_H264_LEVEL_1_2 + 1 ) ,
	ENC_H264_LEVEL_2	= ( ENC_H264_LEVEL_1_3 + 1 ) ,
	ENC_H264_LEVEL_2_1	= ( ENC_H264_LEVEL_2 + 1 ) ,
	ENC_H264_LEVEL_2_2	= ( ENC_H264_LEVEL_2_1 + 1 ) ,
	ENC_H264_LEVEL_3	= ( ENC_H264_LEVEL_2_2 + 1 ) ,
	ENC_H264_LEVEL_3_1	= ( ENC_H264_LEVEL_3 + 1 ) ,
	ENC_H264_LEVEL_3_2	= ( ENC_H264_LEVEL_3_1 + 1 ) ,
	ENC_H264_LEVEL_4	= ( ENC_H264_LEVEL_3_2 + 1 ) ,
	ENC_H264_LEVEL_4_1	= ( ENC_H264_LEVEL_4 + 1 ) ,
	ENC_H264_LEVEL_4_2	= ( ENC_H264_LEVEL_4_1 + 1 ) ,
	ENC_H264_LEVEL_5	= ( ENC_H264_LEVEL_4_2 + 1 ) ,
	ENC_H264_LEVEL_5_1	= ( ENC_H264_LEVEL_5 + 1 ) 
    } 	ENCLEVEL;


EXTERN_C const IID LIBID_VisionStreaming;

#ifndef __IVision_INTERFACE_DEFINED__
#define __IVision_INTERFACE_DEFINED__

/* interface IVision */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVision;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7e9a3511-8465-4776-bfb8-070ee732b4c9")
    IVision : public IUnknown
    {
    public:
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HorPosition( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorPosition( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorPositionMinimum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorPositionMaximum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorPositionDefault( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VerPosition( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VerPosition( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VerPositionMinimum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VerPositionMaximum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VerPositionDefault( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HorScale( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorScale( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorScaleMinimum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorScaleMaximum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorScaleDefault( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CaptureWidth( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureWidth( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureWidthMinimum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureWidthMaximum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureWidthDefault( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CaptureHeight( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureHeight( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureHeightMinimum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureHeightMaximum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureHeightDefault( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Brightness( 
            /* [in] */ long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Brightness( 
            /* [retval][out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BrightnessMinimum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BrightnessMaximum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BrightnessDefault( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Contrast( 
            /* [in] */ long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Contrast( 
            /* [retval][out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ContrastMinimum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ContrastMaximum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ContrastDefault( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlackLevel( 
            /* [in] */ long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlackLevel( 
            /* [retval][out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlackLevelMinimum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlackLevelMaximum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlackLevelDefault( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Phase( 
            /* [in] */ long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Phase( 
            /* [retval][out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhaseMinimum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhaseMaximum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PhaseDefault( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SignalType( 
            /* [out] */ unsigned long *pSignalType) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Saturation( 
            /* [in] */ long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Saturation( 
            /* [retval][out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SaturationMinimum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SaturationMaximum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SaturationDefault( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Hue( 
            /* [in] */ long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Hue( 
            /* [retval][out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HueMinimum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HueMaximum( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HueDefault( 
            /* [out] */ long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VideoStandard( 
            /* [retval][out] */ USHORT *pValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCurrentVDIFAsUserDefined( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteCurrentVDIFAsUserDefined( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetVDIFDescriptionStr( 
            /* [in] */ USHORT length,
            /* [out] */ WCHAR *pBuffer) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDeviceDriver( 
            /* [out] */ DWORD *pMmajor,
            /* [out] */ DWORD *pMinor,
            /* [out] */ DWORD *pMicro,
            /* [out] */ DWORD *pRevision) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDeviceFirmware( 
            /* [out] */ DWORD *pFirmware) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDeviceVHDL( 
            /* [out] */ DWORD *pVHDL) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDeviceIdentifier( 
            /* [out] */ DWORD *pLow,
            /* [out] */ DWORD *pHigh) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDeviceLocation( 
            /* [out] */ DWORD *pBus,
            /* [out] */ DWORD *pDevice,
            /* [out] */ DWORD *pFunction) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VFlipCapture( 
            /* [retval][out] */ DWORD *pVFlipCapture) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VFlipCapture( 
            /* [in] */ DWORD vFlipCapture) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NoSignalFrame( 
            /* [retval][out] */ DWORD *pNoSignalFrame) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NoSignalFrame( 
            /* [in] */ DWORD noSignalFrame) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NoSignalSingleFrame( 
            /* [retval][out] */ DWORD *pNoSignalSingleFrame) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NoSignalSingleFrame( 
            /* [in] */ DWORD noSignalSingleFrame) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NoSignalCounter( 
            /* [retval][out] */ DWORD *pNoSignalCounter) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NoSignalCounter( 
            /* [in] */ DWORD noSignalCounter) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetNoSignalText( 
            /* [in] */ USHORT length,
            /* [out] */ CHAR *pNoSignalText) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetNoSignalText( 
            /* [in] */ CHAR *pNoSignalText) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVision * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVision * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVision * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorPosition )( 
            IVision * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPosition )( 
            IVision * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionMinimum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionMaximum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionDefault )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VerPosition )( 
            IVision * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPosition )( 
            IVision * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionMinimum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionMaximum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionDefault )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorScale )( 
            IVision * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScale )( 
            IVision * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleMinimum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleMaximum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleDefault )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CaptureWidth )( 
            IVision * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidth )( 
            IVision * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthMinimum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthMaximum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthDefault )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CaptureHeight )( 
            IVision * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeight )( 
            IVision * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightMinimum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightMaximum )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightDefault )( 
            IVision * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Brightness )( 
            IVision * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Brightness )( 
            IVision * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessMinimum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessMaximum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessDefault )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Contrast )( 
            IVision * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Contrast )( 
            IVision * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastMinimum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastMaximum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastDefault )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlackLevel )( 
            IVision * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevel )( 
            IVision * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelMinimum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelMaximum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelDefault )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Phase )( 
            IVision * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Phase )( 
            IVision * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseMinimum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseMaximum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseDefault )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SignalType )( 
            IVision * This,
            /* [out] */ unsigned long *pSignalType);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Saturation )( 
            IVision * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Saturation )( 
            IVision * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationMinimum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationMaximum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationDefault )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hue )( 
            IVision * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hue )( 
            IVision * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueMinimum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueMaximum )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueDefault )( 
            IVision * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VideoStandard )( 
            IVision * This,
            /* [retval][out] */ USHORT *pValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCurrentVDIFAsUserDefined )( 
            IVision * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DeleteCurrentVDIFAsUserDefined )( 
            IVision * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetVDIFDescriptionStr )( 
            IVision * This,
            /* [in] */ USHORT length,
            /* [out] */ WCHAR *pBuffer);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceDriver )( 
            IVision * This,
            /* [out] */ DWORD *pMmajor,
            /* [out] */ DWORD *pMinor,
            /* [out] */ DWORD *pMicro,
            /* [out] */ DWORD *pRevision);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceFirmware )( 
            IVision * This,
            /* [out] */ DWORD *pFirmware);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceVHDL )( 
            IVision * This,
            /* [out] */ DWORD *pVHDL);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceIdentifier )( 
            IVision * This,
            /* [out] */ DWORD *pLow,
            /* [out] */ DWORD *pHigh);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceLocation )( 
            IVision * This,
            /* [out] */ DWORD *pBus,
            /* [out] */ DWORD *pDevice,
            /* [out] */ DWORD *pFunction);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VFlipCapture )( 
            IVision * This,
            /* [retval][out] */ DWORD *pVFlipCapture);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VFlipCapture )( 
            IVision * This,
            /* [in] */ DWORD vFlipCapture);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalFrame )( 
            IVision * This,
            /* [retval][out] */ DWORD *pNoSignalFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalFrame )( 
            IVision * This,
            /* [in] */ DWORD noSignalFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalSingleFrame )( 
            IVision * This,
            /* [retval][out] */ DWORD *pNoSignalSingleFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalSingleFrame )( 
            IVision * This,
            /* [in] */ DWORD noSignalSingleFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalCounter )( 
            IVision * This,
            /* [retval][out] */ DWORD *pNoSignalCounter);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalCounter )( 
            IVision * This,
            /* [in] */ DWORD noSignalCounter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetNoSignalText )( 
            IVision * This,
            /* [in] */ USHORT length,
            /* [out] */ CHAR *pNoSignalText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetNoSignalText )( 
            IVision * This,
            /* [in] */ CHAR *pNoSignalText);
        
        END_INTERFACE
    } IVisionVtbl;

    interface IVision
    {
        CONST_VTBL struct IVisionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVision_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVision_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVision_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVision_put_HorPosition(This,value)	\
    ( (This)->lpVtbl -> put_HorPosition(This,value) ) 

#define IVision_get_HorPosition(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPosition(This,pValue) ) 

#define IVision_get_HorPositionMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionMinimum(This,pValue) ) 

#define IVision_get_HorPositionMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionMaximum(This,pValue) ) 

#define IVision_get_HorPositionDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionDefault(This,pValue) ) 

#define IVision_put_VerPosition(This,value)	\
    ( (This)->lpVtbl -> put_VerPosition(This,value) ) 

#define IVision_get_VerPosition(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPosition(This,pValue) ) 

#define IVision_get_VerPositionMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionMinimum(This,pValue) ) 

#define IVision_get_VerPositionMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionMaximum(This,pValue) ) 

#define IVision_get_VerPositionDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionDefault(This,pValue) ) 

#define IVision_put_HorScale(This,value)	\
    ( (This)->lpVtbl -> put_HorScale(This,value) ) 

#define IVision_get_HorScale(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScale(This,pValue) ) 

#define IVision_get_HorScaleMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleMinimum(This,pValue) ) 

#define IVision_get_HorScaleMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleMaximum(This,pValue) ) 

#define IVision_get_HorScaleDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleDefault(This,pValue) ) 

#define IVision_put_CaptureWidth(This,value)	\
    ( (This)->lpVtbl -> put_CaptureWidth(This,value) ) 

#define IVision_get_CaptureWidth(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidth(This,pValue) ) 

#define IVision_get_CaptureWidthMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthMinimum(This,pValue) ) 

#define IVision_get_CaptureWidthMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthMaximum(This,pValue) ) 

#define IVision_get_CaptureWidthDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthDefault(This,pValue) ) 

#define IVision_put_CaptureHeight(This,value)	\
    ( (This)->lpVtbl -> put_CaptureHeight(This,value) ) 

#define IVision_get_CaptureHeight(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeight(This,pValue) ) 

#define IVision_get_CaptureHeightMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightMinimum(This,pValue) ) 

#define IVision_get_CaptureHeightMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightMaximum(This,pValue) ) 

#define IVision_get_CaptureHeightDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightDefault(This,pValue) ) 

#define IVision_put_Brightness(This,value)	\
    ( (This)->lpVtbl -> put_Brightness(This,value) ) 

#define IVision_get_Brightness(This,pValue)	\
    ( (This)->lpVtbl -> get_Brightness(This,pValue) ) 

#define IVision_get_BrightnessMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessMinimum(This,pValue) ) 

#define IVision_get_BrightnessMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessMaximum(This,pValue) ) 

#define IVision_get_BrightnessDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessDefault(This,pValue) ) 

#define IVision_put_Contrast(This,value)	\
    ( (This)->lpVtbl -> put_Contrast(This,value) ) 

#define IVision_get_Contrast(This,pValue)	\
    ( (This)->lpVtbl -> get_Contrast(This,pValue) ) 

#define IVision_get_ContrastMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastMinimum(This,pValue) ) 

#define IVision_get_ContrastMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastMaximum(This,pValue) ) 

#define IVision_get_ContrastDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastDefault(This,pValue) ) 

#define IVision_put_BlackLevel(This,value)	\
    ( (This)->lpVtbl -> put_BlackLevel(This,value) ) 

#define IVision_get_BlackLevel(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevel(This,pValue) ) 

#define IVision_get_BlackLevelMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelMinimum(This,pValue) ) 

#define IVision_get_BlackLevelMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelMaximum(This,pValue) ) 

#define IVision_get_BlackLevelDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelDefault(This,pValue) ) 

#define IVision_put_Phase(This,value)	\
    ( (This)->lpVtbl -> put_Phase(This,value) ) 

#define IVision_get_Phase(This,pValue)	\
    ( (This)->lpVtbl -> get_Phase(This,pValue) ) 

#define IVision_get_PhaseMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseMinimum(This,pValue) ) 

#define IVision_get_PhaseMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseMaximum(This,pValue) ) 

#define IVision_get_PhaseDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseDefault(This,pValue) ) 

#define IVision_get_SignalType(This,pSignalType)	\
    ( (This)->lpVtbl -> get_SignalType(This,pSignalType) ) 

#define IVision_put_Saturation(This,value)	\
    ( (This)->lpVtbl -> put_Saturation(This,value) ) 

#define IVision_get_Saturation(This,pValue)	\
    ( (This)->lpVtbl -> get_Saturation(This,pValue) ) 

#define IVision_get_SaturationMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationMinimum(This,pValue) ) 

#define IVision_get_SaturationMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationMaximum(This,pValue) ) 

#define IVision_get_SaturationDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationDefault(This,pValue) ) 

#define IVision_put_Hue(This,value)	\
    ( (This)->lpVtbl -> put_Hue(This,value) ) 

#define IVision_get_Hue(This,pValue)	\
    ( (This)->lpVtbl -> get_Hue(This,pValue) ) 

#define IVision_get_HueMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HueMinimum(This,pValue) ) 

#define IVision_get_HueMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HueMaximum(This,pValue) ) 

#define IVision_get_HueDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HueDefault(This,pValue) ) 

#define IVision_get_VideoStandard(This,pValue)	\
    ( (This)->lpVtbl -> get_VideoStandard(This,pValue) ) 

#define IVision_SetCurrentVDIFAsUserDefined(This)	\
    ( (This)->lpVtbl -> SetCurrentVDIFAsUserDefined(This) ) 

#define IVision_DeleteCurrentVDIFAsUserDefined(This)	\
    ( (This)->lpVtbl -> DeleteCurrentVDIFAsUserDefined(This) ) 

#define IVision_GetVDIFDescriptionStr(This,length,pBuffer)	\
    ( (This)->lpVtbl -> GetVDIFDescriptionStr(This,length,pBuffer) ) 

#define IVision_GetDeviceDriver(This,pMmajor,pMinor,pMicro,pRevision)	\
    ( (This)->lpVtbl -> GetDeviceDriver(This,pMmajor,pMinor,pMicro,pRevision) ) 

#define IVision_GetDeviceFirmware(This,pFirmware)	\
    ( (This)->lpVtbl -> GetDeviceFirmware(This,pFirmware) ) 

#define IVision_GetDeviceVHDL(This,pVHDL)	\
    ( (This)->lpVtbl -> GetDeviceVHDL(This,pVHDL) ) 

#define IVision_GetDeviceIdentifier(This,pLow,pHigh)	\
    ( (This)->lpVtbl -> GetDeviceIdentifier(This,pLow,pHigh) ) 

#define IVision_GetDeviceLocation(This,pBus,pDevice,pFunction)	\
    ( (This)->lpVtbl -> GetDeviceLocation(This,pBus,pDevice,pFunction) ) 

#define IVision_get_VFlipCapture(This,pVFlipCapture)	\
    ( (This)->lpVtbl -> get_VFlipCapture(This,pVFlipCapture) ) 

#define IVision_put_VFlipCapture(This,vFlipCapture)	\
    ( (This)->lpVtbl -> put_VFlipCapture(This,vFlipCapture) ) 

#define IVision_get_NoSignalFrame(This,pNoSignalFrame)	\
    ( (This)->lpVtbl -> get_NoSignalFrame(This,pNoSignalFrame) ) 

#define IVision_put_NoSignalFrame(This,noSignalFrame)	\
    ( (This)->lpVtbl -> put_NoSignalFrame(This,noSignalFrame) ) 

#define IVision_get_NoSignalSingleFrame(This,pNoSignalSingleFrame)	\
    ( (This)->lpVtbl -> get_NoSignalSingleFrame(This,pNoSignalSingleFrame) ) 

#define IVision_put_NoSignalSingleFrame(This,noSignalSingleFrame)	\
    ( (This)->lpVtbl -> put_NoSignalSingleFrame(This,noSignalSingleFrame) ) 

#define IVision_get_NoSignalCounter(This,pNoSignalCounter)	\
    ( (This)->lpVtbl -> get_NoSignalCounter(This,pNoSignalCounter) ) 

#define IVision_put_NoSignalCounter(This,noSignalCounter)	\
    ( (This)->lpVtbl -> put_NoSignalCounter(This,noSignalCounter) ) 

#define IVision_GetNoSignalText(This,length,pNoSignalText)	\
    ( (This)->lpVtbl -> GetNoSignalText(This,length,pNoSignalText) ) 

#define IVision_SetNoSignalText(This,pNoSignalText)	\
    ( (This)->lpVtbl -> SetNoSignalText(This,pNoSignalText) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVision_INTERFACE_DEFINED__ */


#ifndef __IVisionUser_INTERFACE_DEFINED__
#define __IVisionUser_INTERFACE_DEFINED__

/* interface IVisionUser */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18AE65DB-E8B0-4e4f-A72A-108A151EB507")
    IVisionUser : public IVision
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NoSignalTextColour( 
            /* [retval][out] */ COLOUR *pColour) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NoSignalTextColour( 
            /* [in] */ COLOUR colour) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NoSignalBackgroundColour( 
            /* [retval][out] */ COLOUR *pColour) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NoSignalBackgroundColour( 
            /* [in] */ COLOUR colour) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InvalidSignalFrame( 
            /* [retval][out] */ DWORD *pInvalidSignalFrame) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_InvalidSignalFrame( 
            /* [in] */ DWORD invalidSignalFrame) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InvalidSignalSingleFrame( 
            /* [retval][out] */ DWORD *pInvalidSignalSingleFrame) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_InvalidSignalSingleFrame( 
            /* [in] */ DWORD invalidSignalSingleFrame) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InvalidSignalCounter( 
            /* [retval][out] */ DWORD *pInvalidSignalCounter) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_InvalidSignalCounter( 
            /* [in] */ DWORD invalidSignalCounter) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InvalidSignalTimings( 
            /* [retval][out] */ DWORD *pInvalidSignalTimings) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_InvalidSignalTimings( 
            /* [in] */ DWORD invalidSignalTimings) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInvalidSignalText( 
            /* [in] */ USHORT length,
            /* [out] */ CHAR *pInvalidSignalText) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetInvalidSignalText( 
            /* [in] */ CHAR *pInvalidSignalText) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InvalidSignalTextColour( 
            /* [retval][out] */ COLOUR *pColour) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_InvalidSignalTextColour( 
            /* [in] */ COLOUR colour) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InvalidSignalBackgroundColour( 
            /* [retval][out] */ COLOUR *pColour) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_InvalidSignalBackgroundColour( 
            /* [in] */ COLOUR colour) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInputFrameRate( 
            /* [in] */ unsigned long *pFrameRate) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ColourBalance( 
            /* [in] */ struct CLRBALANCE *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourBalance( 
            /* [retval][out] */ struct CLRBALANCE *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourBalanceMinimum( 
            /* [out] */ struct CLRBALANCE *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourBalanceMaximum( 
            /* [out] */ struct CLRBALANCE *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourBalanceDefault( 
            /* [out] */ struct CLRBALANCE *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ColourDomain( 
            /* [in] */ CLRDOMAINDETECT value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourDomain( 
            /* [retval][out] */ CLRDOMAINDETECT *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourDomainDefault( 
            /* [retval][out] */ CLRDOMAINDETECT *pValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionUserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUser * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUser * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorPosition )( 
            IVisionUser * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPosition )( 
            IVisionUser * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionMinimum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionMaximum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionDefault )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VerPosition )( 
            IVisionUser * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPosition )( 
            IVisionUser * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionMinimum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionMaximum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionDefault )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorScale )( 
            IVisionUser * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScale )( 
            IVisionUser * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleMinimum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleMaximum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleDefault )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CaptureWidth )( 
            IVisionUser * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidth )( 
            IVisionUser * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthMinimum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthMaximum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthDefault )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CaptureHeight )( 
            IVisionUser * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeight )( 
            IVisionUser * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightMinimum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightMaximum )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightDefault )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Brightness )( 
            IVisionUser * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Brightness )( 
            IVisionUser * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessMinimum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessMaximum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessDefault )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Contrast )( 
            IVisionUser * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Contrast )( 
            IVisionUser * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastMinimum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastMaximum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastDefault )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlackLevel )( 
            IVisionUser * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevel )( 
            IVisionUser * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelMinimum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelMaximum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelDefault )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Phase )( 
            IVisionUser * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Phase )( 
            IVisionUser * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseMinimum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseMaximum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseDefault )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SignalType )( 
            IVisionUser * This,
            /* [out] */ unsigned long *pSignalType);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Saturation )( 
            IVisionUser * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Saturation )( 
            IVisionUser * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationMinimum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationMaximum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationDefault )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hue )( 
            IVisionUser * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hue )( 
            IVisionUser * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueMinimum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueMaximum )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueDefault )( 
            IVisionUser * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VideoStandard )( 
            IVisionUser * This,
            /* [retval][out] */ USHORT *pValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCurrentVDIFAsUserDefined )( 
            IVisionUser * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DeleteCurrentVDIFAsUserDefined )( 
            IVisionUser * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetVDIFDescriptionStr )( 
            IVisionUser * This,
            /* [in] */ USHORT length,
            /* [out] */ WCHAR *pBuffer);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceDriver )( 
            IVisionUser * This,
            /* [out] */ DWORD *pMmajor,
            /* [out] */ DWORD *pMinor,
            /* [out] */ DWORD *pMicro,
            /* [out] */ DWORD *pRevision);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceFirmware )( 
            IVisionUser * This,
            /* [out] */ DWORD *pFirmware);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceVHDL )( 
            IVisionUser * This,
            /* [out] */ DWORD *pVHDL);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceIdentifier )( 
            IVisionUser * This,
            /* [out] */ DWORD *pLow,
            /* [out] */ DWORD *pHigh);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceLocation )( 
            IVisionUser * This,
            /* [out] */ DWORD *pBus,
            /* [out] */ DWORD *pDevice,
            /* [out] */ DWORD *pFunction);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VFlipCapture )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pVFlipCapture);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VFlipCapture )( 
            IVisionUser * This,
            /* [in] */ DWORD vFlipCapture);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalFrame )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pNoSignalFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalFrame )( 
            IVisionUser * This,
            /* [in] */ DWORD noSignalFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalSingleFrame )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pNoSignalSingleFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalSingleFrame )( 
            IVisionUser * This,
            /* [in] */ DWORD noSignalSingleFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalCounter )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pNoSignalCounter);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalCounter )( 
            IVisionUser * This,
            /* [in] */ DWORD noSignalCounter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetNoSignalText )( 
            IVisionUser * This,
            /* [in] */ USHORT length,
            /* [out] */ CHAR *pNoSignalText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetNoSignalText )( 
            IVisionUser * This,
            /* [in] */ CHAR *pNoSignalText);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalTextColour )( 
            IVisionUser * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalTextColour )( 
            IVisionUser * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalBackgroundColour )( 
            IVisionUser * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalBackgroundColour )( 
            IVisionUser * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalFrame )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pInvalidSignalFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalFrame )( 
            IVisionUser * This,
            /* [in] */ DWORD invalidSignalFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalSingleFrame )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pInvalidSignalSingleFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalSingleFrame )( 
            IVisionUser * This,
            /* [in] */ DWORD invalidSignalSingleFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalCounter )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pInvalidSignalCounter);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalCounter )( 
            IVisionUser * This,
            /* [in] */ DWORD invalidSignalCounter);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalTimings )( 
            IVisionUser * This,
            /* [retval][out] */ DWORD *pInvalidSignalTimings);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalTimings )( 
            IVisionUser * This,
            /* [in] */ DWORD invalidSignalTimings);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInvalidSignalText )( 
            IVisionUser * This,
            /* [in] */ USHORT length,
            /* [out] */ CHAR *pInvalidSignalText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetInvalidSignalText )( 
            IVisionUser * This,
            /* [in] */ CHAR *pInvalidSignalText);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalTextColour )( 
            IVisionUser * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalTextColour )( 
            IVisionUser * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalBackgroundColour )( 
            IVisionUser * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalBackgroundColour )( 
            IVisionUser * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInputFrameRate )( 
            IVisionUser * This,
            /* [in] */ unsigned long *pFrameRate);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ColourBalance )( 
            IVisionUser * This,
            /* [in] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalance )( 
            IVisionUser * This,
            /* [retval][out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalanceMinimum )( 
            IVisionUser * This,
            /* [out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalanceMaximum )( 
            IVisionUser * This,
            /* [out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalanceDefault )( 
            IVisionUser * This,
            /* [out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ColourDomain )( 
            IVisionUser * This,
            /* [in] */ CLRDOMAINDETECT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourDomain )( 
            IVisionUser * This,
            /* [retval][out] */ CLRDOMAINDETECT *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourDomainDefault )( 
            IVisionUser * This,
            /* [retval][out] */ CLRDOMAINDETECT *pValue);
        
        END_INTERFACE
    } IVisionUserVtbl;

    interface IVisionUser
    {
        CONST_VTBL struct IVisionUserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUser_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUser_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUser_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUser_put_HorPosition(This,value)	\
    ( (This)->lpVtbl -> put_HorPosition(This,value) ) 

#define IVisionUser_get_HorPosition(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPosition(This,pValue) ) 

#define IVisionUser_get_HorPositionMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionMinimum(This,pValue) ) 

#define IVisionUser_get_HorPositionMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionMaximum(This,pValue) ) 

#define IVisionUser_get_HorPositionDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionDefault(This,pValue) ) 

#define IVisionUser_put_VerPosition(This,value)	\
    ( (This)->lpVtbl -> put_VerPosition(This,value) ) 

#define IVisionUser_get_VerPosition(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPosition(This,pValue) ) 

#define IVisionUser_get_VerPositionMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionMinimum(This,pValue) ) 

#define IVisionUser_get_VerPositionMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionMaximum(This,pValue) ) 

#define IVisionUser_get_VerPositionDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionDefault(This,pValue) ) 

#define IVisionUser_put_HorScale(This,value)	\
    ( (This)->lpVtbl -> put_HorScale(This,value) ) 

#define IVisionUser_get_HorScale(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScale(This,pValue) ) 

#define IVisionUser_get_HorScaleMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleMinimum(This,pValue) ) 

#define IVisionUser_get_HorScaleMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleMaximum(This,pValue) ) 

#define IVisionUser_get_HorScaleDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleDefault(This,pValue) ) 

#define IVisionUser_put_CaptureWidth(This,value)	\
    ( (This)->lpVtbl -> put_CaptureWidth(This,value) ) 

#define IVisionUser_get_CaptureWidth(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidth(This,pValue) ) 

#define IVisionUser_get_CaptureWidthMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthMinimum(This,pValue) ) 

#define IVisionUser_get_CaptureWidthMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthMaximum(This,pValue) ) 

#define IVisionUser_get_CaptureWidthDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthDefault(This,pValue) ) 

#define IVisionUser_put_CaptureHeight(This,value)	\
    ( (This)->lpVtbl -> put_CaptureHeight(This,value) ) 

#define IVisionUser_get_CaptureHeight(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeight(This,pValue) ) 

#define IVisionUser_get_CaptureHeightMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightMinimum(This,pValue) ) 

#define IVisionUser_get_CaptureHeightMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightMaximum(This,pValue) ) 

#define IVisionUser_get_CaptureHeightDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightDefault(This,pValue) ) 

#define IVisionUser_put_Brightness(This,value)	\
    ( (This)->lpVtbl -> put_Brightness(This,value) ) 

#define IVisionUser_get_Brightness(This,pValue)	\
    ( (This)->lpVtbl -> get_Brightness(This,pValue) ) 

#define IVisionUser_get_BrightnessMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessMinimum(This,pValue) ) 

#define IVisionUser_get_BrightnessMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessMaximum(This,pValue) ) 

#define IVisionUser_get_BrightnessDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessDefault(This,pValue) ) 

#define IVisionUser_put_Contrast(This,value)	\
    ( (This)->lpVtbl -> put_Contrast(This,value) ) 

#define IVisionUser_get_Contrast(This,pValue)	\
    ( (This)->lpVtbl -> get_Contrast(This,pValue) ) 

#define IVisionUser_get_ContrastMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastMinimum(This,pValue) ) 

#define IVisionUser_get_ContrastMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastMaximum(This,pValue) ) 

#define IVisionUser_get_ContrastDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastDefault(This,pValue) ) 

#define IVisionUser_put_BlackLevel(This,value)	\
    ( (This)->lpVtbl -> put_BlackLevel(This,value) ) 

#define IVisionUser_get_BlackLevel(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevel(This,pValue) ) 

#define IVisionUser_get_BlackLevelMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelMinimum(This,pValue) ) 

#define IVisionUser_get_BlackLevelMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelMaximum(This,pValue) ) 

#define IVisionUser_get_BlackLevelDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelDefault(This,pValue) ) 

#define IVisionUser_put_Phase(This,value)	\
    ( (This)->lpVtbl -> put_Phase(This,value) ) 

#define IVisionUser_get_Phase(This,pValue)	\
    ( (This)->lpVtbl -> get_Phase(This,pValue) ) 

#define IVisionUser_get_PhaseMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseMinimum(This,pValue) ) 

#define IVisionUser_get_PhaseMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseMaximum(This,pValue) ) 

#define IVisionUser_get_PhaseDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseDefault(This,pValue) ) 

#define IVisionUser_get_SignalType(This,pSignalType)	\
    ( (This)->lpVtbl -> get_SignalType(This,pSignalType) ) 

#define IVisionUser_put_Saturation(This,value)	\
    ( (This)->lpVtbl -> put_Saturation(This,value) ) 

#define IVisionUser_get_Saturation(This,pValue)	\
    ( (This)->lpVtbl -> get_Saturation(This,pValue) ) 

#define IVisionUser_get_SaturationMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationMinimum(This,pValue) ) 

#define IVisionUser_get_SaturationMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationMaximum(This,pValue) ) 

#define IVisionUser_get_SaturationDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationDefault(This,pValue) ) 

#define IVisionUser_put_Hue(This,value)	\
    ( (This)->lpVtbl -> put_Hue(This,value) ) 

#define IVisionUser_get_Hue(This,pValue)	\
    ( (This)->lpVtbl -> get_Hue(This,pValue) ) 

#define IVisionUser_get_HueMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HueMinimum(This,pValue) ) 

#define IVisionUser_get_HueMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HueMaximum(This,pValue) ) 

#define IVisionUser_get_HueDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HueDefault(This,pValue) ) 

#define IVisionUser_get_VideoStandard(This,pValue)	\
    ( (This)->lpVtbl -> get_VideoStandard(This,pValue) ) 

#define IVisionUser_SetCurrentVDIFAsUserDefined(This)	\
    ( (This)->lpVtbl -> SetCurrentVDIFAsUserDefined(This) ) 

#define IVisionUser_DeleteCurrentVDIFAsUserDefined(This)	\
    ( (This)->lpVtbl -> DeleteCurrentVDIFAsUserDefined(This) ) 

#define IVisionUser_GetVDIFDescriptionStr(This,length,pBuffer)	\
    ( (This)->lpVtbl -> GetVDIFDescriptionStr(This,length,pBuffer) ) 

#define IVisionUser_GetDeviceDriver(This,pMmajor,pMinor,pMicro,pRevision)	\
    ( (This)->lpVtbl -> GetDeviceDriver(This,pMmajor,pMinor,pMicro,pRevision) ) 

#define IVisionUser_GetDeviceFirmware(This,pFirmware)	\
    ( (This)->lpVtbl -> GetDeviceFirmware(This,pFirmware) ) 

#define IVisionUser_GetDeviceVHDL(This,pVHDL)	\
    ( (This)->lpVtbl -> GetDeviceVHDL(This,pVHDL) ) 

#define IVisionUser_GetDeviceIdentifier(This,pLow,pHigh)	\
    ( (This)->lpVtbl -> GetDeviceIdentifier(This,pLow,pHigh) ) 

#define IVisionUser_GetDeviceLocation(This,pBus,pDevice,pFunction)	\
    ( (This)->lpVtbl -> GetDeviceLocation(This,pBus,pDevice,pFunction) ) 

#define IVisionUser_get_VFlipCapture(This,pVFlipCapture)	\
    ( (This)->lpVtbl -> get_VFlipCapture(This,pVFlipCapture) ) 

#define IVisionUser_put_VFlipCapture(This,vFlipCapture)	\
    ( (This)->lpVtbl -> put_VFlipCapture(This,vFlipCapture) ) 

#define IVisionUser_get_NoSignalFrame(This,pNoSignalFrame)	\
    ( (This)->lpVtbl -> get_NoSignalFrame(This,pNoSignalFrame) ) 

#define IVisionUser_put_NoSignalFrame(This,noSignalFrame)	\
    ( (This)->lpVtbl -> put_NoSignalFrame(This,noSignalFrame) ) 

#define IVisionUser_get_NoSignalSingleFrame(This,pNoSignalSingleFrame)	\
    ( (This)->lpVtbl -> get_NoSignalSingleFrame(This,pNoSignalSingleFrame) ) 

#define IVisionUser_put_NoSignalSingleFrame(This,noSignalSingleFrame)	\
    ( (This)->lpVtbl -> put_NoSignalSingleFrame(This,noSignalSingleFrame) ) 

#define IVisionUser_get_NoSignalCounter(This,pNoSignalCounter)	\
    ( (This)->lpVtbl -> get_NoSignalCounter(This,pNoSignalCounter) ) 

#define IVisionUser_put_NoSignalCounter(This,noSignalCounter)	\
    ( (This)->lpVtbl -> put_NoSignalCounter(This,noSignalCounter) ) 

#define IVisionUser_GetNoSignalText(This,length,pNoSignalText)	\
    ( (This)->lpVtbl -> GetNoSignalText(This,length,pNoSignalText) ) 

#define IVisionUser_SetNoSignalText(This,pNoSignalText)	\
    ( (This)->lpVtbl -> SetNoSignalText(This,pNoSignalText) ) 


#define IVisionUser_get_NoSignalTextColour(This,pColour)	\
    ( (This)->lpVtbl -> get_NoSignalTextColour(This,pColour) ) 

#define IVisionUser_put_NoSignalTextColour(This,colour)	\
    ( (This)->lpVtbl -> put_NoSignalTextColour(This,colour) ) 

#define IVisionUser_get_NoSignalBackgroundColour(This,pColour)	\
    ( (This)->lpVtbl -> get_NoSignalBackgroundColour(This,pColour) ) 

#define IVisionUser_put_NoSignalBackgroundColour(This,colour)	\
    ( (This)->lpVtbl -> put_NoSignalBackgroundColour(This,colour) ) 

#define IVisionUser_get_InvalidSignalFrame(This,pInvalidSignalFrame)	\
    ( (This)->lpVtbl -> get_InvalidSignalFrame(This,pInvalidSignalFrame) ) 

#define IVisionUser_put_InvalidSignalFrame(This,invalidSignalFrame)	\
    ( (This)->lpVtbl -> put_InvalidSignalFrame(This,invalidSignalFrame) ) 

#define IVisionUser_get_InvalidSignalSingleFrame(This,pInvalidSignalSingleFrame)	\
    ( (This)->lpVtbl -> get_InvalidSignalSingleFrame(This,pInvalidSignalSingleFrame) ) 

#define IVisionUser_put_InvalidSignalSingleFrame(This,invalidSignalSingleFrame)	\
    ( (This)->lpVtbl -> put_InvalidSignalSingleFrame(This,invalidSignalSingleFrame) ) 

#define IVisionUser_get_InvalidSignalCounter(This,pInvalidSignalCounter)	\
    ( (This)->lpVtbl -> get_InvalidSignalCounter(This,pInvalidSignalCounter) ) 

#define IVisionUser_put_InvalidSignalCounter(This,invalidSignalCounter)	\
    ( (This)->lpVtbl -> put_InvalidSignalCounter(This,invalidSignalCounter) ) 

#define IVisionUser_get_InvalidSignalTimings(This,pInvalidSignalTimings)	\
    ( (This)->lpVtbl -> get_InvalidSignalTimings(This,pInvalidSignalTimings) ) 

#define IVisionUser_put_InvalidSignalTimings(This,invalidSignalTimings)	\
    ( (This)->lpVtbl -> put_InvalidSignalTimings(This,invalidSignalTimings) ) 

#define IVisionUser_GetInvalidSignalText(This,length,pInvalidSignalText)	\
    ( (This)->lpVtbl -> GetInvalidSignalText(This,length,pInvalidSignalText) ) 

#define IVisionUser_SetInvalidSignalText(This,pInvalidSignalText)	\
    ( (This)->lpVtbl -> SetInvalidSignalText(This,pInvalidSignalText) ) 

#define IVisionUser_get_InvalidSignalTextColour(This,pColour)	\
    ( (This)->lpVtbl -> get_InvalidSignalTextColour(This,pColour) ) 

#define IVisionUser_put_InvalidSignalTextColour(This,colour)	\
    ( (This)->lpVtbl -> put_InvalidSignalTextColour(This,colour) ) 

#define IVisionUser_get_InvalidSignalBackgroundColour(This,pColour)	\
    ( (This)->lpVtbl -> get_InvalidSignalBackgroundColour(This,pColour) ) 

#define IVisionUser_put_InvalidSignalBackgroundColour(This,colour)	\
    ( (This)->lpVtbl -> put_InvalidSignalBackgroundColour(This,colour) ) 

#define IVisionUser_GetInputFrameRate(This,pFrameRate)	\
    ( (This)->lpVtbl -> GetInputFrameRate(This,pFrameRate) ) 

#define IVisionUser_put_ColourBalance(This,pValue)	\
    ( (This)->lpVtbl -> put_ColourBalance(This,pValue) ) 

#define IVisionUser_get_ColourBalance(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalance(This,pValue) ) 

#define IVisionUser_get_ColourBalanceMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalanceMinimum(This,pValue) ) 

#define IVisionUser_get_ColourBalanceMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalanceMaximum(This,pValue) ) 

#define IVisionUser_get_ColourBalanceDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalanceDefault(This,pValue) ) 

#define IVisionUser_put_ColourDomain(This,value)	\
    ( (This)->lpVtbl -> put_ColourDomain(This,value) ) 

#define IVisionUser_get_ColourDomain(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourDomain(This,pValue) ) 

#define IVisionUser_get_ColourDomainDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourDomainDefault(This,pValue) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUser_INTERFACE_DEFINED__ */


#ifndef __IDGC103ControlConfig_INTERFACE_DEFINED__
#define __IDGC103ControlConfig_INTERFACE_DEFINED__

/* interface IDGC103ControlConfig */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IDGC103ControlConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("620f2f08-85e6-4212-89e4-4db31036e090")
    IDGC103ControlConfig : public IUnknown
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Contrast( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Contrast( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxContrast( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultContrast( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Brightness( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Brightness( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxBrightness( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultBrightness( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Phase( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Phase( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxPhase( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultPhase( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlackLevel( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BlackLevel( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxBlackLevel( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultBlackLevel( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SampleRate( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SampleRate( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxSampleRate( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultSampleRate( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorOffset( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HorOffset( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxHorOffset( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultHorOffset( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VerOffset( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VerOffset( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxVerOffset( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultVerOffset( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorScale( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HorScale( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxHorScale( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultHorScale( 
            long *plDefault) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Channel( 
            /* [retval][out] */ long *plVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Channel( 
            /* [in] */ long lVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MinMaxChannel( 
            long *plMin,
            long *plMax) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultChannel( 
            long *plDefault) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDGC103ControlConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDGC103ControlConfig * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDGC103ControlConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDGC103ControlConfig * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Contrast )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Contrast )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxContrast )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultContrast )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Brightness )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Brightness )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxBrightness )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultBrightness )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Phase )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Phase )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxPhase )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultPhase )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevel )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlackLevel )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxBlackLevel )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultBlackLevel )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SampleRate )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SampleRate )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxSampleRate )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultSampleRate )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorOffset )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorOffset )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxHorOffset )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultHorOffset )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerOffset )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VerOffset )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxVerOffset )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultVerOffset )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScale )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorScale )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxHorScale )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultHorScale )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Channel )( 
            IDGC103ControlConfig * This,
            /* [retval][out] */ long *plVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Channel )( 
            IDGC103ControlConfig * This,
            /* [in] */ long lVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MinMaxChannel )( 
            IDGC103ControlConfig * This,
            long *plMin,
            long *plMax);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DefaultChannel )( 
            IDGC103ControlConfig * This,
            long *plDefault);
        
        END_INTERFACE
    } IDGC103ControlConfigVtbl;

    interface IDGC103ControlConfig
    {
        CONST_VTBL struct IDGC103ControlConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDGC103ControlConfig_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDGC103ControlConfig_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDGC103ControlConfig_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDGC103ControlConfig_get_Contrast(This,plVal)	\
    ( (This)->lpVtbl -> get_Contrast(This,plVal) ) 

#define IDGC103ControlConfig_put_Contrast(This,lVal)	\
    ( (This)->lpVtbl -> put_Contrast(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxContrast(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxContrast(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultContrast(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultContrast(This,plDefault) ) 

#define IDGC103ControlConfig_get_Brightness(This,plVal)	\
    ( (This)->lpVtbl -> get_Brightness(This,plVal) ) 

#define IDGC103ControlConfig_put_Brightness(This,lVal)	\
    ( (This)->lpVtbl -> put_Brightness(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxBrightness(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxBrightness(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultBrightness(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultBrightness(This,plDefault) ) 

#define IDGC103ControlConfig_get_Phase(This,plVal)	\
    ( (This)->lpVtbl -> get_Phase(This,plVal) ) 

#define IDGC103ControlConfig_put_Phase(This,lVal)	\
    ( (This)->lpVtbl -> put_Phase(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxPhase(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxPhase(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultPhase(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultPhase(This,plDefault) ) 

#define IDGC103ControlConfig_get_BlackLevel(This,plVal)	\
    ( (This)->lpVtbl -> get_BlackLevel(This,plVal) ) 

#define IDGC103ControlConfig_put_BlackLevel(This,lVal)	\
    ( (This)->lpVtbl -> put_BlackLevel(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxBlackLevel(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxBlackLevel(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultBlackLevel(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultBlackLevel(This,plDefault) ) 

#define IDGC103ControlConfig_get_SampleRate(This,plVal)	\
    ( (This)->lpVtbl -> get_SampleRate(This,plVal) ) 

#define IDGC103ControlConfig_put_SampleRate(This,lVal)	\
    ( (This)->lpVtbl -> put_SampleRate(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxSampleRate(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxSampleRate(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultSampleRate(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultSampleRate(This,plDefault) ) 

#define IDGC103ControlConfig_get_HorOffset(This,plVal)	\
    ( (This)->lpVtbl -> get_HorOffset(This,plVal) ) 

#define IDGC103ControlConfig_put_HorOffset(This,lVal)	\
    ( (This)->lpVtbl -> put_HorOffset(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxHorOffset(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxHorOffset(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultHorOffset(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultHorOffset(This,plDefault) ) 

#define IDGC103ControlConfig_get_VerOffset(This,plVal)	\
    ( (This)->lpVtbl -> get_VerOffset(This,plVal) ) 

#define IDGC103ControlConfig_put_VerOffset(This,lVal)	\
    ( (This)->lpVtbl -> put_VerOffset(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxVerOffset(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxVerOffset(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultVerOffset(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultVerOffset(This,plDefault) ) 

#define IDGC103ControlConfig_get_HorScale(This,plVal)	\
    ( (This)->lpVtbl -> get_HorScale(This,plVal) ) 

#define IDGC103ControlConfig_put_HorScale(This,lVal)	\
    ( (This)->lpVtbl -> put_HorScale(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxHorScale(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxHorScale(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultHorScale(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultHorScale(This,plDefault) ) 

#define IDGC103ControlConfig_get_Channel(This,plVal)	\
    ( (This)->lpVtbl -> get_Channel(This,plVal) ) 

#define IDGC103ControlConfig_put_Channel(This,lVal)	\
    ( (This)->lpVtbl -> put_Channel(This,lVal) ) 

#define IDGC103ControlConfig_get_MinMaxChannel(This,plMin,plMax)	\
    ( (This)->lpVtbl -> get_MinMaxChannel(This,plMin,plMax) ) 

#define IDGC103ControlConfig_get_DefaultChannel(This,plDefault)	\
    ( (This)->lpVtbl -> get_DefaultChannel(This,plDefault) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDGC103ControlConfig_INTERFACE_DEFINED__ */


#ifndef __IVisionUser2_INTERFACE_DEFINED__
#define __IVisionUser2_INTERFACE_DEFINED__

/* interface IVisionUser2 */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUser2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EA258966-4C3B-4699-97C4-DD41D3C34FD4")
    IVisionUser2 : public IVisionUser
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourDomainDetected( 
            /* [retval][out] */ CLRDOMAINDETECT *pValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionUser2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUser2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUser2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUser2 * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorPosition )( 
            IVisionUser2 * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPosition )( 
            IVisionUser2 * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionMinimum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionMaximum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorPositionDefault )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VerPosition )( 
            IVisionUser2 * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPosition )( 
            IVisionUser2 * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionMinimum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionMaximum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerPositionDefault )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_HorScale )( 
            IVisionUser2 * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScale )( 
            IVisionUser2 * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleMinimum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleMaximum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HorScaleDefault )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CaptureWidth )( 
            IVisionUser2 * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidth )( 
            IVisionUser2 * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthMinimum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthMaximum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureWidthDefault )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CaptureHeight )( 
            IVisionUser2 * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeight )( 
            IVisionUser2 * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightMinimum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightMaximum )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureHeightDefault )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Brightness )( 
            IVisionUser2 * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Brightness )( 
            IVisionUser2 * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessMinimum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessMaximum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BrightnessDefault )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Contrast )( 
            IVisionUser2 * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Contrast )( 
            IVisionUser2 * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastMinimum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastMaximum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ContrastDefault )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BlackLevel )( 
            IVisionUser2 * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevel )( 
            IVisionUser2 * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelMinimum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelMaximum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BlackLevelDefault )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Phase )( 
            IVisionUser2 * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Phase )( 
            IVisionUser2 * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseMinimum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseMaximum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PhaseDefault )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SignalType )( 
            IVisionUser2 * This,
            /* [out] */ unsigned long *pSignalType);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Saturation )( 
            IVisionUser2 * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Saturation )( 
            IVisionUser2 * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationMinimum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationMaximum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SaturationDefault )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hue )( 
            IVisionUser2 * This,
            /* [in] */ long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hue )( 
            IVisionUser2 * This,
            /* [retval][out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueMinimum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueMaximum )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_HueDefault )( 
            IVisionUser2 * This,
            /* [out] */ long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VideoStandard )( 
            IVisionUser2 * This,
            /* [retval][out] */ USHORT *pValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCurrentVDIFAsUserDefined )( 
            IVisionUser2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DeleteCurrentVDIFAsUserDefined )( 
            IVisionUser2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetVDIFDescriptionStr )( 
            IVisionUser2 * This,
            /* [in] */ USHORT length,
            /* [out] */ WCHAR *pBuffer);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceDriver )( 
            IVisionUser2 * This,
            /* [out] */ DWORD *pMmajor,
            /* [out] */ DWORD *pMinor,
            /* [out] */ DWORD *pMicro,
            /* [out] */ DWORD *pRevision);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceFirmware )( 
            IVisionUser2 * This,
            /* [out] */ DWORD *pFirmware);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceVHDL )( 
            IVisionUser2 * This,
            /* [out] */ DWORD *pVHDL);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceIdentifier )( 
            IVisionUser2 * This,
            /* [out] */ DWORD *pLow,
            /* [out] */ DWORD *pHigh);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceLocation )( 
            IVisionUser2 * This,
            /* [out] */ DWORD *pBus,
            /* [out] */ DWORD *pDevice,
            /* [out] */ DWORD *pFunction);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VFlipCapture )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pVFlipCapture);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VFlipCapture )( 
            IVisionUser2 * This,
            /* [in] */ DWORD vFlipCapture);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalFrame )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pNoSignalFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalFrame )( 
            IVisionUser2 * This,
            /* [in] */ DWORD noSignalFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalSingleFrame )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pNoSignalSingleFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalSingleFrame )( 
            IVisionUser2 * This,
            /* [in] */ DWORD noSignalSingleFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalCounter )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pNoSignalCounter);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalCounter )( 
            IVisionUser2 * This,
            /* [in] */ DWORD noSignalCounter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetNoSignalText )( 
            IVisionUser2 * This,
            /* [in] */ USHORT length,
            /* [out] */ CHAR *pNoSignalText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetNoSignalText )( 
            IVisionUser2 * This,
            /* [in] */ CHAR *pNoSignalText);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalTextColour )( 
            IVisionUser2 * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalTextColour )( 
            IVisionUser2 * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NoSignalBackgroundColour )( 
            IVisionUser2 * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NoSignalBackgroundColour )( 
            IVisionUser2 * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalFrame )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pInvalidSignalFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalFrame )( 
            IVisionUser2 * This,
            /* [in] */ DWORD invalidSignalFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalSingleFrame )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pInvalidSignalSingleFrame);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalSingleFrame )( 
            IVisionUser2 * This,
            /* [in] */ DWORD invalidSignalSingleFrame);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalCounter )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pInvalidSignalCounter);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalCounter )( 
            IVisionUser2 * This,
            /* [in] */ DWORD invalidSignalCounter);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalTimings )( 
            IVisionUser2 * This,
            /* [retval][out] */ DWORD *pInvalidSignalTimings);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalTimings )( 
            IVisionUser2 * This,
            /* [in] */ DWORD invalidSignalTimings);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInvalidSignalText )( 
            IVisionUser2 * This,
            /* [in] */ USHORT length,
            /* [out] */ CHAR *pInvalidSignalText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetInvalidSignalText )( 
            IVisionUser2 * This,
            /* [in] */ CHAR *pInvalidSignalText);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalTextColour )( 
            IVisionUser2 * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalTextColour )( 
            IVisionUser2 * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_InvalidSignalBackgroundColour )( 
            IVisionUser2 * This,
            /* [retval][out] */ COLOUR *pColour);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_InvalidSignalBackgroundColour )( 
            IVisionUser2 * This,
            /* [in] */ COLOUR colour);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInputFrameRate )( 
            IVisionUser2 * This,
            /* [in] */ unsigned long *pFrameRate);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ColourBalance )( 
            IVisionUser2 * This,
            /* [in] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalance )( 
            IVisionUser2 * This,
            /* [retval][out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalanceMinimum )( 
            IVisionUser2 * This,
            /* [out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalanceMaximum )( 
            IVisionUser2 * This,
            /* [out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourBalanceDefault )( 
            IVisionUser2 * This,
            /* [out] */ struct CLRBALANCE *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ColourDomain )( 
            IVisionUser2 * This,
            /* [in] */ CLRDOMAINDETECT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourDomain )( 
            IVisionUser2 * This,
            /* [retval][out] */ CLRDOMAINDETECT *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourDomainDefault )( 
            IVisionUser2 * This,
            /* [retval][out] */ CLRDOMAINDETECT *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ColourDomainDetected )( 
            IVisionUser2 * This,
            /* [retval][out] */ CLRDOMAINDETECT *pValue);
        
        END_INTERFACE
    } IVisionUser2Vtbl;

    interface IVisionUser2
    {
        CONST_VTBL struct IVisionUser2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUser2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUser2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUser2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUser2_put_HorPosition(This,value)	\
    ( (This)->lpVtbl -> put_HorPosition(This,value) ) 

#define IVisionUser2_get_HorPosition(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPosition(This,pValue) ) 

#define IVisionUser2_get_HorPositionMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionMinimum(This,pValue) ) 

#define IVisionUser2_get_HorPositionMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionMaximum(This,pValue) ) 

#define IVisionUser2_get_HorPositionDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HorPositionDefault(This,pValue) ) 

#define IVisionUser2_put_VerPosition(This,value)	\
    ( (This)->lpVtbl -> put_VerPosition(This,value) ) 

#define IVisionUser2_get_VerPosition(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPosition(This,pValue) ) 

#define IVisionUser2_get_VerPositionMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionMinimum(This,pValue) ) 

#define IVisionUser2_get_VerPositionMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionMaximum(This,pValue) ) 

#define IVisionUser2_get_VerPositionDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_VerPositionDefault(This,pValue) ) 

#define IVisionUser2_put_HorScale(This,value)	\
    ( (This)->lpVtbl -> put_HorScale(This,value) ) 

#define IVisionUser2_get_HorScale(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScale(This,pValue) ) 

#define IVisionUser2_get_HorScaleMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleMinimum(This,pValue) ) 

#define IVisionUser2_get_HorScaleMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleMaximum(This,pValue) ) 

#define IVisionUser2_get_HorScaleDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HorScaleDefault(This,pValue) ) 

#define IVisionUser2_put_CaptureWidth(This,value)	\
    ( (This)->lpVtbl -> put_CaptureWidth(This,value) ) 

#define IVisionUser2_get_CaptureWidth(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidth(This,pValue) ) 

#define IVisionUser2_get_CaptureWidthMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthMinimum(This,pValue) ) 

#define IVisionUser2_get_CaptureWidthMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthMaximum(This,pValue) ) 

#define IVisionUser2_get_CaptureWidthDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureWidthDefault(This,pValue) ) 

#define IVisionUser2_put_CaptureHeight(This,value)	\
    ( (This)->lpVtbl -> put_CaptureHeight(This,value) ) 

#define IVisionUser2_get_CaptureHeight(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeight(This,pValue) ) 

#define IVisionUser2_get_CaptureHeightMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightMinimum(This,pValue) ) 

#define IVisionUser2_get_CaptureHeightMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightMaximum(This,pValue) ) 

#define IVisionUser2_get_CaptureHeightDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_CaptureHeightDefault(This,pValue) ) 

#define IVisionUser2_put_Brightness(This,value)	\
    ( (This)->lpVtbl -> put_Brightness(This,value) ) 

#define IVisionUser2_get_Brightness(This,pValue)	\
    ( (This)->lpVtbl -> get_Brightness(This,pValue) ) 

#define IVisionUser2_get_BrightnessMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessMinimum(This,pValue) ) 

#define IVisionUser2_get_BrightnessMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessMaximum(This,pValue) ) 

#define IVisionUser2_get_BrightnessDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_BrightnessDefault(This,pValue) ) 

#define IVisionUser2_put_Contrast(This,value)	\
    ( (This)->lpVtbl -> put_Contrast(This,value) ) 

#define IVisionUser2_get_Contrast(This,pValue)	\
    ( (This)->lpVtbl -> get_Contrast(This,pValue) ) 

#define IVisionUser2_get_ContrastMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastMinimum(This,pValue) ) 

#define IVisionUser2_get_ContrastMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastMaximum(This,pValue) ) 

#define IVisionUser2_get_ContrastDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_ContrastDefault(This,pValue) ) 

#define IVisionUser2_put_BlackLevel(This,value)	\
    ( (This)->lpVtbl -> put_BlackLevel(This,value) ) 

#define IVisionUser2_get_BlackLevel(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevel(This,pValue) ) 

#define IVisionUser2_get_BlackLevelMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelMinimum(This,pValue) ) 

#define IVisionUser2_get_BlackLevelMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelMaximum(This,pValue) ) 

#define IVisionUser2_get_BlackLevelDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_BlackLevelDefault(This,pValue) ) 

#define IVisionUser2_put_Phase(This,value)	\
    ( (This)->lpVtbl -> put_Phase(This,value) ) 

#define IVisionUser2_get_Phase(This,pValue)	\
    ( (This)->lpVtbl -> get_Phase(This,pValue) ) 

#define IVisionUser2_get_PhaseMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseMinimum(This,pValue) ) 

#define IVisionUser2_get_PhaseMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseMaximum(This,pValue) ) 

#define IVisionUser2_get_PhaseDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_PhaseDefault(This,pValue) ) 

#define IVisionUser2_get_SignalType(This,pSignalType)	\
    ( (This)->lpVtbl -> get_SignalType(This,pSignalType) ) 

#define IVisionUser2_put_Saturation(This,value)	\
    ( (This)->lpVtbl -> put_Saturation(This,value) ) 

#define IVisionUser2_get_Saturation(This,pValue)	\
    ( (This)->lpVtbl -> get_Saturation(This,pValue) ) 

#define IVisionUser2_get_SaturationMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationMinimum(This,pValue) ) 

#define IVisionUser2_get_SaturationMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationMaximum(This,pValue) ) 

#define IVisionUser2_get_SaturationDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_SaturationDefault(This,pValue) ) 

#define IVisionUser2_put_Hue(This,value)	\
    ( (This)->lpVtbl -> put_Hue(This,value) ) 

#define IVisionUser2_get_Hue(This,pValue)	\
    ( (This)->lpVtbl -> get_Hue(This,pValue) ) 

#define IVisionUser2_get_HueMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_HueMinimum(This,pValue) ) 

#define IVisionUser2_get_HueMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_HueMaximum(This,pValue) ) 

#define IVisionUser2_get_HueDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_HueDefault(This,pValue) ) 

#define IVisionUser2_get_VideoStandard(This,pValue)	\
    ( (This)->lpVtbl -> get_VideoStandard(This,pValue) ) 

#define IVisionUser2_SetCurrentVDIFAsUserDefined(This)	\
    ( (This)->lpVtbl -> SetCurrentVDIFAsUserDefined(This) ) 

#define IVisionUser2_DeleteCurrentVDIFAsUserDefined(This)	\
    ( (This)->lpVtbl -> DeleteCurrentVDIFAsUserDefined(This) ) 

#define IVisionUser2_GetVDIFDescriptionStr(This,length,pBuffer)	\
    ( (This)->lpVtbl -> GetVDIFDescriptionStr(This,length,pBuffer) ) 

#define IVisionUser2_GetDeviceDriver(This,pMmajor,pMinor,pMicro,pRevision)	\
    ( (This)->lpVtbl -> GetDeviceDriver(This,pMmajor,pMinor,pMicro,pRevision) ) 

#define IVisionUser2_GetDeviceFirmware(This,pFirmware)	\
    ( (This)->lpVtbl -> GetDeviceFirmware(This,pFirmware) ) 

#define IVisionUser2_GetDeviceVHDL(This,pVHDL)	\
    ( (This)->lpVtbl -> GetDeviceVHDL(This,pVHDL) ) 

#define IVisionUser2_GetDeviceIdentifier(This,pLow,pHigh)	\
    ( (This)->lpVtbl -> GetDeviceIdentifier(This,pLow,pHigh) ) 

#define IVisionUser2_GetDeviceLocation(This,pBus,pDevice,pFunction)	\
    ( (This)->lpVtbl -> GetDeviceLocation(This,pBus,pDevice,pFunction) ) 

#define IVisionUser2_get_VFlipCapture(This,pVFlipCapture)	\
    ( (This)->lpVtbl -> get_VFlipCapture(This,pVFlipCapture) ) 

#define IVisionUser2_put_VFlipCapture(This,vFlipCapture)	\
    ( (This)->lpVtbl -> put_VFlipCapture(This,vFlipCapture) ) 

#define IVisionUser2_get_NoSignalFrame(This,pNoSignalFrame)	\
    ( (This)->lpVtbl -> get_NoSignalFrame(This,pNoSignalFrame) ) 

#define IVisionUser2_put_NoSignalFrame(This,noSignalFrame)	\
    ( (This)->lpVtbl -> put_NoSignalFrame(This,noSignalFrame) ) 

#define IVisionUser2_get_NoSignalSingleFrame(This,pNoSignalSingleFrame)	\
    ( (This)->lpVtbl -> get_NoSignalSingleFrame(This,pNoSignalSingleFrame) ) 

#define IVisionUser2_put_NoSignalSingleFrame(This,noSignalSingleFrame)	\
    ( (This)->lpVtbl -> put_NoSignalSingleFrame(This,noSignalSingleFrame) ) 

#define IVisionUser2_get_NoSignalCounter(This,pNoSignalCounter)	\
    ( (This)->lpVtbl -> get_NoSignalCounter(This,pNoSignalCounter) ) 

#define IVisionUser2_put_NoSignalCounter(This,noSignalCounter)	\
    ( (This)->lpVtbl -> put_NoSignalCounter(This,noSignalCounter) ) 

#define IVisionUser2_GetNoSignalText(This,length,pNoSignalText)	\
    ( (This)->lpVtbl -> GetNoSignalText(This,length,pNoSignalText) ) 

#define IVisionUser2_SetNoSignalText(This,pNoSignalText)	\
    ( (This)->lpVtbl -> SetNoSignalText(This,pNoSignalText) ) 


#define IVisionUser2_get_NoSignalTextColour(This,pColour)	\
    ( (This)->lpVtbl -> get_NoSignalTextColour(This,pColour) ) 

#define IVisionUser2_put_NoSignalTextColour(This,colour)	\
    ( (This)->lpVtbl -> put_NoSignalTextColour(This,colour) ) 

#define IVisionUser2_get_NoSignalBackgroundColour(This,pColour)	\
    ( (This)->lpVtbl -> get_NoSignalBackgroundColour(This,pColour) ) 

#define IVisionUser2_put_NoSignalBackgroundColour(This,colour)	\
    ( (This)->lpVtbl -> put_NoSignalBackgroundColour(This,colour) ) 

#define IVisionUser2_get_InvalidSignalFrame(This,pInvalidSignalFrame)	\
    ( (This)->lpVtbl -> get_InvalidSignalFrame(This,pInvalidSignalFrame) ) 

#define IVisionUser2_put_InvalidSignalFrame(This,invalidSignalFrame)	\
    ( (This)->lpVtbl -> put_InvalidSignalFrame(This,invalidSignalFrame) ) 

#define IVisionUser2_get_InvalidSignalSingleFrame(This,pInvalidSignalSingleFrame)	\
    ( (This)->lpVtbl -> get_InvalidSignalSingleFrame(This,pInvalidSignalSingleFrame) ) 

#define IVisionUser2_put_InvalidSignalSingleFrame(This,invalidSignalSingleFrame)	\
    ( (This)->lpVtbl -> put_InvalidSignalSingleFrame(This,invalidSignalSingleFrame) ) 

#define IVisionUser2_get_InvalidSignalCounter(This,pInvalidSignalCounter)	\
    ( (This)->lpVtbl -> get_InvalidSignalCounter(This,pInvalidSignalCounter) ) 

#define IVisionUser2_put_InvalidSignalCounter(This,invalidSignalCounter)	\
    ( (This)->lpVtbl -> put_InvalidSignalCounter(This,invalidSignalCounter) ) 

#define IVisionUser2_get_InvalidSignalTimings(This,pInvalidSignalTimings)	\
    ( (This)->lpVtbl -> get_InvalidSignalTimings(This,pInvalidSignalTimings) ) 

#define IVisionUser2_put_InvalidSignalTimings(This,invalidSignalTimings)	\
    ( (This)->lpVtbl -> put_InvalidSignalTimings(This,invalidSignalTimings) ) 

#define IVisionUser2_GetInvalidSignalText(This,length,pInvalidSignalText)	\
    ( (This)->lpVtbl -> GetInvalidSignalText(This,length,pInvalidSignalText) ) 

#define IVisionUser2_SetInvalidSignalText(This,pInvalidSignalText)	\
    ( (This)->lpVtbl -> SetInvalidSignalText(This,pInvalidSignalText) ) 

#define IVisionUser2_get_InvalidSignalTextColour(This,pColour)	\
    ( (This)->lpVtbl -> get_InvalidSignalTextColour(This,pColour) ) 

#define IVisionUser2_put_InvalidSignalTextColour(This,colour)	\
    ( (This)->lpVtbl -> put_InvalidSignalTextColour(This,colour) ) 

#define IVisionUser2_get_InvalidSignalBackgroundColour(This,pColour)	\
    ( (This)->lpVtbl -> get_InvalidSignalBackgroundColour(This,pColour) ) 

#define IVisionUser2_put_InvalidSignalBackgroundColour(This,colour)	\
    ( (This)->lpVtbl -> put_InvalidSignalBackgroundColour(This,colour) ) 

#define IVisionUser2_GetInputFrameRate(This,pFrameRate)	\
    ( (This)->lpVtbl -> GetInputFrameRate(This,pFrameRate) ) 

#define IVisionUser2_put_ColourBalance(This,pValue)	\
    ( (This)->lpVtbl -> put_ColourBalance(This,pValue) ) 

#define IVisionUser2_get_ColourBalance(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalance(This,pValue) ) 

#define IVisionUser2_get_ColourBalanceMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalanceMinimum(This,pValue) ) 

#define IVisionUser2_get_ColourBalanceMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalanceMaximum(This,pValue) ) 

#define IVisionUser2_get_ColourBalanceDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourBalanceDefault(This,pValue) ) 

#define IVisionUser2_put_ColourDomain(This,value)	\
    ( (This)->lpVtbl -> put_ColourDomain(This,value) ) 

#define IVisionUser2_get_ColourDomain(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourDomain(This,pValue) ) 

#define IVisionUser2_get_ColourDomainDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourDomainDefault(This,pValue) ) 


#define IVisionUser2_get_ColourDomainDetected(This,pValue)	\
    ( (This)->lpVtbl -> get_ColourDomainDetected(This,pValue) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUser2_INTERFACE_DEFINED__ */


#ifndef __IVisionUserGanging_INTERFACE_DEFINED__
#define __IVisionUserGanging_INTERFACE_DEFINED__

/* interface IVisionUserGanging */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUserGanging;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("12713C55-A015-41F6-8420-6FBD3BBEEF17")
    IVisionUserGanging : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsGangingSupported( 
            /* [retval][out] */ long *pBIsSupported) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GangingType( 
            /* [retval][out] */ GANG_TYPE *pType) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GangingType( 
            /* [in] */ GANG_TYPE type) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsGangingTypeSupported( 
            /* [in] */ GANG_TYPE type,
            /* [retval][out] */ long *pBIsSupported) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionUserGangingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUserGanging * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUserGanging * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUserGanging * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsGangingSupported )( 
            IVisionUserGanging * This,
            /* [retval][out] */ long *pBIsSupported);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_GangingType )( 
            IVisionUserGanging * This,
            /* [retval][out] */ GANG_TYPE *pType);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_GangingType )( 
            IVisionUserGanging * This,
            /* [in] */ GANG_TYPE type);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsGangingTypeSupported )( 
            IVisionUserGanging * This,
            /* [in] */ GANG_TYPE type,
            /* [retval][out] */ long *pBIsSupported);
        
        END_INTERFACE
    } IVisionUserGangingVtbl;

    interface IVisionUserGanging
    {
        CONST_VTBL struct IVisionUserGangingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUserGanging_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUserGanging_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUserGanging_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUserGanging_IsGangingSupported(This,pBIsSupported)	\
    ( (This)->lpVtbl -> IsGangingSupported(This,pBIsSupported) ) 

#define IVisionUserGanging_get_GangingType(This,pType)	\
    ( (This)->lpVtbl -> get_GangingType(This,pType) ) 

#define IVisionUserGanging_put_GangingType(This,type)	\
    ( (This)->lpVtbl -> put_GangingType(This,type) ) 

#define IVisionUserGanging_IsGangingTypeSupported(This,type,pBIsSupported)	\
    ( (This)->lpVtbl -> IsGangingTypeSupported(This,type,pBIsSupported) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUserGanging_INTERFACE_DEFINED__ */


#ifndef __IVisionUserOverlay_INTERFACE_DEFINED__
#define __IVisionUserOverlay_INTERFACE_DEFINED__

/* interface IVisionUserOverlay */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUserOverlay;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8B448ED4-C073-4B1E-82C8-B9686C9B25A3")
    IVisionUserOverlay : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsOverlaySupported( 
            /* [retval][out] */ long *pBIsSupported) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsOverlayEnabled( 
            /* [retval][out] */ long *pBIsEnabled) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OverlayEnable( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OverlayDisable( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OverlayKeepAlive( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverlayTimeoutMinimum( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverlayTimeoutMaximum( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverlayTimeoutDefault( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OverlayTimeout( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_OverlayTimeout( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetOverlayImage( 
            /* [in] */ unsigned long width,
            /* [in] */ unsigned long height,
            /* [in] */ unsigned long stride,
            /* [in] */ CLRDOMAIN colourdomain,
            /* [in] */ void *pBuffer,
            /* [in] */ unsigned long offsetX,
            /* [in] */ unsigned long offsetY) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionUserOverlayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUserOverlay * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUserOverlay * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUserOverlay * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsOverlaySupported )( 
            IVisionUserOverlay * This,
            /* [retval][out] */ long *pBIsSupported);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsOverlayEnabled )( 
            IVisionUserOverlay * This,
            /* [retval][out] */ long *pBIsEnabled);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OverlayEnable )( 
            IVisionUserOverlay * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OverlayDisable )( 
            IVisionUserOverlay * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OverlayKeepAlive )( 
            IVisionUserOverlay * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverlayTimeoutMinimum )( 
            IVisionUserOverlay * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverlayTimeoutMaximum )( 
            IVisionUserOverlay * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverlayTimeoutDefault )( 
            IVisionUserOverlay * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_OverlayTimeout )( 
            IVisionUserOverlay * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_OverlayTimeout )( 
            IVisionUserOverlay * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetOverlayImage )( 
            IVisionUserOverlay * This,
            /* [in] */ unsigned long width,
            /* [in] */ unsigned long height,
            /* [in] */ unsigned long stride,
            /* [in] */ CLRDOMAIN colourdomain,
            /* [in] */ void *pBuffer,
            /* [in] */ unsigned long offsetX,
            /* [in] */ unsigned long offsetY);
        
        END_INTERFACE
    } IVisionUserOverlayVtbl;

    interface IVisionUserOverlay
    {
        CONST_VTBL struct IVisionUserOverlayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUserOverlay_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUserOverlay_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUserOverlay_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUserOverlay_IsOverlaySupported(This,pBIsSupported)	\
    ( (This)->lpVtbl -> IsOverlaySupported(This,pBIsSupported) ) 

#define IVisionUserOverlay_IsOverlayEnabled(This,pBIsEnabled)	\
    ( (This)->lpVtbl -> IsOverlayEnabled(This,pBIsEnabled) ) 

#define IVisionUserOverlay_OverlayEnable(This)	\
    ( (This)->lpVtbl -> OverlayEnable(This) ) 

#define IVisionUserOverlay_OverlayDisable(This)	\
    ( (This)->lpVtbl -> OverlayDisable(This) ) 

#define IVisionUserOverlay_OverlayKeepAlive(This)	\
    ( (This)->lpVtbl -> OverlayKeepAlive(This) ) 

#define IVisionUserOverlay_get_OverlayTimeoutMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_OverlayTimeoutMinimum(This,pValue) ) 

#define IVisionUserOverlay_get_OverlayTimeoutMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_OverlayTimeoutMaximum(This,pValue) ) 

#define IVisionUserOverlay_get_OverlayTimeoutDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_OverlayTimeoutDefault(This,pValue) ) 

#define IVisionUserOverlay_get_OverlayTimeout(This,pValue)	\
    ( (This)->lpVtbl -> get_OverlayTimeout(This,pValue) ) 

#define IVisionUserOverlay_put_OverlayTimeout(This,value)	\
    ( (This)->lpVtbl -> put_OverlayTimeout(This,value) ) 

#define IVisionUserOverlay_SetOverlayImage(This,width,height,stride,colourdomain,pBuffer,offsetX,offsetY)	\
    ( (This)->lpVtbl -> SetOverlayImage(This,width,height,stride,colourdomain,pBuffer,offsetX,offsetY) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUserOverlay_INTERFACE_DEFINED__ */


#ifndef __IVisionCallBack_INTERFACE_DEFINED__
#define __IVisionCallBack_INTERFACE_DEFINED__

/* interface IVisionCallBack */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9AFD3590-30A9-4631-9851-7ECFDC746201")
    IVisionCallBack : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ClientCallback( 
            unsigned long eventFlag,
            long channel,
            PVOID pUserData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionCallBack * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionCallBack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionCallBack * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ClientCallback )( 
            IVisionCallBack * This,
            unsigned long eventFlag,
            long channel,
            PVOID pUserData);
        
        END_INTERFACE
    } IVisionCallBackVtbl;

    interface IVisionCallBack
    {
        CONST_VTBL struct IVisionCallBackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionCallBack_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionCallBack_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionCallBack_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionCallBack_ClientCallback(This,eventFlag,channel,pUserData)	\
    ( (This)->lpVtbl -> ClientCallback(This,eventFlag,channel,pUserData) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionCallBack_INTERFACE_DEFINED__ */


#ifndef __IVisionEvent_INTERFACE_DEFINED__
#define __IVisionEvent_INTERFACE_DEFINED__

/* interface IVisionEvent */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e952bb91-5aed-47a4-9e0d-1ff9db44f84c")
    IVisionEvent : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RegisterDeviceEvents( 
            /* [in] */ unsigned long eventFlag,
            IVisionCallBack *pVisionCallBack,
            /* [in] */ PVOID pUserData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveDeviceEvents( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionEvent * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionEvent * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RegisterDeviceEvents )( 
            IVisionEvent * This,
            /* [in] */ unsigned long eventFlag,
            IVisionCallBack *pVisionCallBack,
            /* [in] */ PVOID pUserData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RemoveDeviceEvents )( 
            IVisionEvent * This);
        
        END_INTERFACE
    } IVisionEventVtbl;

    interface IVisionEvent
    {
        CONST_VTBL struct IVisionEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionEvent_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionEvent_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionEvent_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionEvent_RegisterDeviceEvents(This,eventFlag,pVisionCallBack,pUserData)	\
    ( (This)->lpVtbl -> RegisterDeviceEvents(This,eventFlag,pVisionCallBack,pUserData) ) 

#define IVisionEvent_RemoveDeviceEvents(This)	\
    ( (This)->lpVtbl -> RemoveDeviceEvents(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionEvent_INTERFACE_DEFINED__ */


#ifndef __IVisionEvent2_INTERFACE_DEFINED__
#define __IVisionEvent2_INTERFACE_DEFINED__

/* interface IVisionEvent2 */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionEvent2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("477B1A96-F709-467c-9DA8-FF274B715E35")
    IVisionEvent2 : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RegisterDeviceEvents( 
            /* [in] */ unsigned long eventFlag,
            IVisionCallBack *pVisionCallBack,
            /* [in] */ PVOID pUserData,
            /* [out] */ ULONG_PTR *ppDeviceEventHandle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveDeviceEvents( 
            /* [in] */ ULONG_PTR pEventHandle) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionEvent2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionEvent2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionEvent2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionEvent2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RegisterDeviceEvents )( 
            IVisionEvent2 * This,
            /* [in] */ unsigned long eventFlag,
            IVisionCallBack *pVisionCallBack,
            /* [in] */ PVOID pUserData,
            /* [out] */ ULONG_PTR *ppDeviceEventHandle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RemoveDeviceEvents )( 
            IVisionEvent2 * This,
            /* [in] */ ULONG_PTR pEventHandle);
        
        END_INTERFACE
    } IVisionEvent2Vtbl;

    interface IVisionEvent2
    {
        CONST_VTBL struct IVisionEvent2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionEvent2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionEvent2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionEvent2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionEvent2_RegisterDeviceEvents(This,eventFlag,pVisionCallBack,pUserData,ppDeviceEventHandle)	\
    ( (This)->lpVtbl -> RegisterDeviceEvents(This,eventFlag,pVisionCallBack,pUserData,ppDeviceEventHandle) ) 

#define IVisionEvent2_RemoveDeviceEvents(This,pEventHandle)	\
    ( (This)->lpVtbl -> RemoveDeviceEvents(This,pEventHandle) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionEvent2_INTERFACE_DEFINED__ */


#ifndef __IVisionUserEvent_INTERFACE_DEFINED__
#define __IVisionUserEvent_INTERFACE_DEFINED__

/* interface IVisionUserEvent */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUserEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3CEF4B27-D1E1-4ddd-B5B7-AF5353C3B311")
    IVisionUserEvent : public IVisionEvent2
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IVisionUserEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUserEvent * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUserEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUserEvent * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RegisterDeviceEvents )( 
            IVisionUserEvent * This,
            /* [in] */ unsigned long eventFlag,
            IVisionCallBack *pVisionCallBack,
            /* [in] */ PVOID pUserData,
            /* [out] */ ULONG_PTR *ppDeviceEventHandle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RemoveDeviceEvents )( 
            IVisionUserEvent * This,
            /* [in] */ ULONG_PTR pEventHandle);
        
        END_INTERFACE
    } IVisionUserEventVtbl;

    interface IVisionUserEvent
    {
        CONST_VTBL struct IVisionUserEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUserEvent_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUserEvent_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUserEvent_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUserEvent_RegisterDeviceEvents(This,eventFlag,pVisionCallBack,pUserData,ppDeviceEventHandle)	\
    ( (This)->lpVtbl -> RegisterDeviceEvents(This,eventFlag,pVisionCallBack,pUserData,ppDeviceEventHandle) ) 

#define IVisionUserEvent_RemoveDeviceEvents(This,pEventHandle)	\
    ( (This)->lpVtbl -> RemoveDeviceEvents(This,pEventHandle) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUserEvent_INTERFACE_DEFINED__ */


#ifndef __IVisionPin_INTERFACE_DEFINED__
#define __IVisionPin_INTERFACE_DEFINED__

/* interface IVisionPin */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionPin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A916BB24-61C5-4580-A03D-E64E6C236051")
    IVisionPin : public IUnknown
    {
    public:
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CroppingEnabled( 
            /* [in] */ unsigned long bEnabled) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CroppingEnabled( 
            /* [retval][out] */ unsigned long *pBEnabled) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Cropping( 
            /* [in] */ struct CROP *pCrop) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Cropping( 
            /* [retval][out] */ struct CROP *pCrop) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CroppingOverscan( 
            /* [retval][out] */ struct CROP *pCrop) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CroppingMinimum( 
            /* [retval][out] */ struct CROP *pCrop) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CroppingMaximum( 
            /* [retval][out] */ struct CROP *pCrop) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CroppingDefault( 
            /* [retval][out] */ struct CROP *pCrop) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsSignalInterlaced( 
            /* [retval][out] */ long *pBInterlaced) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Deinterlace( 
            /* [retval][out] */ DEINTERLACE_METHOD *pDeinterlace) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Deinterlace( 
            /* [in] */ DEINTERLACE_METHOD deinterlace) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LiveStream( 
            /* [retval][out] */ LIVESTREAM_CLIENT *pLiveStreamClient) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LiveStream( 
            /* [in] */ LIVESTREAM_CLIENT LiveStreamClient) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionPinVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionPin * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionPin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionPin * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CroppingEnabled )( 
            IVisionPin * This,
            /* [in] */ unsigned long bEnabled);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingEnabled )( 
            IVisionPin * This,
            /* [retval][out] */ unsigned long *pBEnabled);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Cropping )( 
            IVisionPin * This,
            /* [in] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Cropping )( 
            IVisionPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingOverscan )( 
            IVisionPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingMinimum )( 
            IVisionPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingMaximum )( 
            IVisionPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingDefault )( 
            IVisionPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsSignalInterlaced )( 
            IVisionPin * This,
            /* [retval][out] */ long *pBInterlaced);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Deinterlace )( 
            IVisionPin * This,
            /* [retval][out] */ DEINTERLACE_METHOD *pDeinterlace);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Deinterlace )( 
            IVisionPin * This,
            /* [in] */ DEINTERLACE_METHOD deinterlace);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LiveStream )( 
            IVisionPin * This,
            /* [retval][out] */ LIVESTREAM_CLIENT *pLiveStreamClient);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LiveStream )( 
            IVisionPin * This,
            /* [in] */ LIVESTREAM_CLIENT LiveStreamClient);
        
        END_INTERFACE
    } IVisionPinVtbl;

    interface IVisionPin
    {
        CONST_VTBL struct IVisionPinVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionPin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionPin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionPin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionPin_put_CroppingEnabled(This,bEnabled)	\
    ( (This)->lpVtbl -> put_CroppingEnabled(This,bEnabled) ) 

#define IVisionPin_get_CroppingEnabled(This,pBEnabled)	\
    ( (This)->lpVtbl -> get_CroppingEnabled(This,pBEnabled) ) 

#define IVisionPin_put_Cropping(This,pCrop)	\
    ( (This)->lpVtbl -> put_Cropping(This,pCrop) ) 

#define IVisionPin_get_Cropping(This,pCrop)	\
    ( (This)->lpVtbl -> get_Cropping(This,pCrop) ) 

#define IVisionPin_get_CroppingOverscan(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingOverscan(This,pCrop) ) 

#define IVisionPin_get_CroppingMinimum(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingMinimum(This,pCrop) ) 

#define IVisionPin_get_CroppingMaximum(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingMaximum(This,pCrop) ) 

#define IVisionPin_get_CroppingDefault(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingDefault(This,pCrop) ) 

#define IVisionPin_IsSignalInterlaced(This,pBInterlaced)	\
    ( (This)->lpVtbl -> IsSignalInterlaced(This,pBInterlaced) ) 

#define IVisionPin_get_Deinterlace(This,pDeinterlace)	\
    ( (This)->lpVtbl -> get_Deinterlace(This,pDeinterlace) ) 

#define IVisionPin_put_Deinterlace(This,deinterlace)	\
    ( (This)->lpVtbl -> put_Deinterlace(This,deinterlace) ) 

#define IVisionPin_get_LiveStream(This,pLiveStreamClient)	\
    ( (This)->lpVtbl -> get_LiveStream(This,pLiveStreamClient) ) 

#define IVisionPin_put_LiveStream(This,LiveStreamClient)	\
    ( (This)->lpVtbl -> put_LiveStream(This,LiveStreamClient) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionPin_INTERFACE_DEFINED__ */


#ifndef __IVisionUserPin_INTERFACE_DEFINED__
#define __IVisionUserPin_INTERFACE_DEFINED__

/* interface IVisionUserPin */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUserPin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D3565213-DEF2-48ef-85E5-13585B86A9BD")
    IVisionUserPin : public IVisionPin
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsLiveStreamSupported( 
            /* [retval][out] */ long *pBSupported) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionUserPinVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUserPin * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUserPin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUserPin * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CroppingEnabled )( 
            IVisionUserPin * This,
            /* [in] */ unsigned long bEnabled);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingEnabled )( 
            IVisionUserPin * This,
            /* [retval][out] */ unsigned long *pBEnabled);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Cropping )( 
            IVisionUserPin * This,
            /* [in] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Cropping )( 
            IVisionUserPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingOverscan )( 
            IVisionUserPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingMinimum )( 
            IVisionUserPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingMaximum )( 
            IVisionUserPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingDefault )( 
            IVisionUserPin * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsSignalInterlaced )( 
            IVisionUserPin * This,
            /* [retval][out] */ long *pBInterlaced);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Deinterlace )( 
            IVisionUserPin * This,
            /* [retval][out] */ DEINTERLACE_METHOD *pDeinterlace);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Deinterlace )( 
            IVisionUserPin * This,
            /* [in] */ DEINTERLACE_METHOD deinterlace);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LiveStream )( 
            IVisionUserPin * This,
            /* [retval][out] */ LIVESTREAM_CLIENT *pLiveStreamClient);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LiveStream )( 
            IVisionUserPin * This,
            /* [in] */ LIVESTREAM_CLIENT LiveStreamClient);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsLiveStreamSupported )( 
            IVisionUserPin * This,
            /* [retval][out] */ long *pBSupported);
        
        END_INTERFACE
    } IVisionUserPinVtbl;

    interface IVisionUserPin
    {
        CONST_VTBL struct IVisionUserPinVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUserPin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUserPin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUserPin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUserPin_put_CroppingEnabled(This,bEnabled)	\
    ( (This)->lpVtbl -> put_CroppingEnabled(This,bEnabled) ) 

#define IVisionUserPin_get_CroppingEnabled(This,pBEnabled)	\
    ( (This)->lpVtbl -> get_CroppingEnabled(This,pBEnabled) ) 

#define IVisionUserPin_put_Cropping(This,pCrop)	\
    ( (This)->lpVtbl -> put_Cropping(This,pCrop) ) 

#define IVisionUserPin_get_Cropping(This,pCrop)	\
    ( (This)->lpVtbl -> get_Cropping(This,pCrop) ) 

#define IVisionUserPin_get_CroppingOverscan(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingOverscan(This,pCrop) ) 

#define IVisionUserPin_get_CroppingMinimum(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingMinimum(This,pCrop) ) 

#define IVisionUserPin_get_CroppingMaximum(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingMaximum(This,pCrop) ) 

#define IVisionUserPin_get_CroppingDefault(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingDefault(This,pCrop) ) 

#define IVisionUserPin_IsSignalInterlaced(This,pBInterlaced)	\
    ( (This)->lpVtbl -> IsSignalInterlaced(This,pBInterlaced) ) 

#define IVisionUserPin_get_Deinterlace(This,pDeinterlace)	\
    ( (This)->lpVtbl -> get_Deinterlace(This,pDeinterlace) ) 

#define IVisionUserPin_put_Deinterlace(This,deinterlace)	\
    ( (This)->lpVtbl -> put_Deinterlace(This,deinterlace) ) 

#define IVisionUserPin_get_LiveStream(This,pLiveStreamClient)	\
    ( (This)->lpVtbl -> get_LiveStream(This,pLiveStreamClient) ) 

#define IVisionUserPin_put_LiveStream(This,LiveStreamClient)	\
    ( (This)->lpVtbl -> put_LiveStream(This,LiveStreamClient) ) 


#define IVisionUserPin_IsLiveStreamSupported(This,pBSupported)	\
    ( (This)->lpVtbl -> IsLiveStreamSupported(This,pBSupported) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUserPin_INTERFACE_DEFINED__ */


#ifndef __IVisionUserPin2_INTERFACE_DEFINED__
#define __IVisionUserPin2_INTERFACE_DEFINED__

/* interface IVisionUserPin2 */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUserPin2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EB3D84A1-2A7E-423D-9E25-9C6C4518D979")
    IVisionUserPin2 : public IVisionUserPin
    {
    public:
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EncodeBitRate( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeBitRate( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeBitRateMinimum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeBitRateMaximum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeBitRateDefault( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EncodeKeyFrameInterval( 
            /* [in] */ unsigned long value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeKeyFrameInterval( 
            /* [retval][out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeKeyFrameIntervalMinimum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeKeyFrameIntervalMaximum( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeKeyFrameIntervalDefault( 
            /* [out] */ unsigned long *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsEncodeProfileSupported( 
            /* [in] */ ENCPROFILE value,
            /* [retval][out] */ long *pBSupported) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EncodeProfile( 
            /* [in] */ ENCPROFILE value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeProfile( 
            /* [retval][out] */ ENCPROFILE *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeProfileDefault( 
            /* [out] */ ENCPROFILE *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsEncodeLevelSupported( 
            /* [in] */ ENCLEVEL value,
            /* [retval][out] */ long *pBSupported) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EncodeLevel( 
            /* [in] */ ENCLEVEL value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeLevel( 
            /* [retval][out] */ ENCLEVEL *pValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodeLevelDefault( 
            /* [out] */ ENCLEVEL *pValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionUserPin2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUserPin2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUserPin2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUserPin2 * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CroppingEnabled )( 
            IVisionUserPin2 * This,
            /* [in] */ unsigned long bEnabled);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingEnabled )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ unsigned long *pBEnabled);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Cropping )( 
            IVisionUserPin2 * This,
            /* [in] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Cropping )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingOverscan )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingMinimum )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingMaximum )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CroppingDefault )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ struct CROP *pCrop);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsSignalInterlaced )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ long *pBInterlaced);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Deinterlace )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ DEINTERLACE_METHOD *pDeinterlace);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Deinterlace )( 
            IVisionUserPin2 * This,
            /* [in] */ DEINTERLACE_METHOD deinterlace);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LiveStream )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ LIVESTREAM_CLIENT *pLiveStreamClient);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LiveStream )( 
            IVisionUserPin2 * This,
            /* [in] */ LIVESTREAM_CLIENT LiveStreamClient);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsLiveStreamSupported )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ long *pBSupported);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EncodeBitRate )( 
            IVisionUserPin2 * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeBitRate )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeBitRateMinimum )( 
            IVisionUserPin2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeBitRateMaximum )( 
            IVisionUserPin2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeBitRateDefault )( 
            IVisionUserPin2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EncodeKeyFrameInterval )( 
            IVisionUserPin2 * This,
            /* [in] */ unsigned long value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeKeyFrameInterval )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeKeyFrameIntervalMinimum )( 
            IVisionUserPin2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeKeyFrameIntervalMaximum )( 
            IVisionUserPin2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeKeyFrameIntervalDefault )( 
            IVisionUserPin2 * This,
            /* [out] */ unsigned long *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsEncodeProfileSupported )( 
            IVisionUserPin2 * This,
            /* [in] */ ENCPROFILE value,
            /* [retval][out] */ long *pBSupported);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EncodeProfile )( 
            IVisionUserPin2 * This,
            /* [in] */ ENCPROFILE value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeProfile )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ ENCPROFILE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeProfileDefault )( 
            IVisionUserPin2 * This,
            /* [out] */ ENCPROFILE *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsEncodeLevelSupported )( 
            IVisionUserPin2 * This,
            /* [in] */ ENCLEVEL value,
            /* [retval][out] */ long *pBSupported);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EncodeLevel )( 
            IVisionUserPin2 * This,
            /* [in] */ ENCLEVEL value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeLevel )( 
            IVisionUserPin2 * This,
            /* [retval][out] */ ENCLEVEL *pValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncodeLevelDefault )( 
            IVisionUserPin2 * This,
            /* [out] */ ENCLEVEL *pValue);
        
        END_INTERFACE
    } IVisionUserPin2Vtbl;

    interface IVisionUserPin2
    {
        CONST_VTBL struct IVisionUserPin2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUserPin2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUserPin2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUserPin2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUserPin2_put_CroppingEnabled(This,bEnabled)	\
    ( (This)->lpVtbl -> put_CroppingEnabled(This,bEnabled) ) 

#define IVisionUserPin2_get_CroppingEnabled(This,pBEnabled)	\
    ( (This)->lpVtbl -> get_CroppingEnabled(This,pBEnabled) ) 

#define IVisionUserPin2_put_Cropping(This,pCrop)	\
    ( (This)->lpVtbl -> put_Cropping(This,pCrop) ) 

#define IVisionUserPin2_get_Cropping(This,pCrop)	\
    ( (This)->lpVtbl -> get_Cropping(This,pCrop) ) 

#define IVisionUserPin2_get_CroppingOverscan(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingOverscan(This,pCrop) ) 

#define IVisionUserPin2_get_CroppingMinimum(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingMinimum(This,pCrop) ) 

#define IVisionUserPin2_get_CroppingMaximum(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingMaximum(This,pCrop) ) 

#define IVisionUserPin2_get_CroppingDefault(This,pCrop)	\
    ( (This)->lpVtbl -> get_CroppingDefault(This,pCrop) ) 

#define IVisionUserPin2_IsSignalInterlaced(This,pBInterlaced)	\
    ( (This)->lpVtbl -> IsSignalInterlaced(This,pBInterlaced) ) 

#define IVisionUserPin2_get_Deinterlace(This,pDeinterlace)	\
    ( (This)->lpVtbl -> get_Deinterlace(This,pDeinterlace) ) 

#define IVisionUserPin2_put_Deinterlace(This,deinterlace)	\
    ( (This)->lpVtbl -> put_Deinterlace(This,deinterlace) ) 

#define IVisionUserPin2_get_LiveStream(This,pLiveStreamClient)	\
    ( (This)->lpVtbl -> get_LiveStream(This,pLiveStreamClient) ) 

#define IVisionUserPin2_put_LiveStream(This,LiveStreamClient)	\
    ( (This)->lpVtbl -> put_LiveStream(This,LiveStreamClient) ) 


#define IVisionUserPin2_IsLiveStreamSupported(This,pBSupported)	\
    ( (This)->lpVtbl -> IsLiveStreamSupported(This,pBSupported) ) 


#define IVisionUserPin2_put_EncodeBitRate(This,value)	\
    ( (This)->lpVtbl -> put_EncodeBitRate(This,value) ) 

#define IVisionUserPin2_get_EncodeBitRate(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeBitRate(This,pValue) ) 

#define IVisionUserPin2_get_EncodeBitRateMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeBitRateMinimum(This,pValue) ) 

#define IVisionUserPin2_get_EncodeBitRateMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeBitRateMaximum(This,pValue) ) 

#define IVisionUserPin2_get_EncodeBitRateDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeBitRateDefault(This,pValue) ) 

#define IVisionUserPin2_put_EncodeKeyFrameInterval(This,value)	\
    ( (This)->lpVtbl -> put_EncodeKeyFrameInterval(This,value) ) 

#define IVisionUserPin2_get_EncodeKeyFrameInterval(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeKeyFrameInterval(This,pValue) ) 

#define IVisionUserPin2_get_EncodeKeyFrameIntervalMinimum(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeKeyFrameIntervalMinimum(This,pValue) ) 

#define IVisionUserPin2_get_EncodeKeyFrameIntervalMaximum(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeKeyFrameIntervalMaximum(This,pValue) ) 

#define IVisionUserPin2_get_EncodeKeyFrameIntervalDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeKeyFrameIntervalDefault(This,pValue) ) 

#define IVisionUserPin2_get_IsEncodeProfileSupported(This,value,pBSupported)	\
    ( (This)->lpVtbl -> get_IsEncodeProfileSupported(This,value,pBSupported) ) 

#define IVisionUserPin2_put_EncodeProfile(This,value)	\
    ( (This)->lpVtbl -> put_EncodeProfile(This,value) ) 

#define IVisionUserPin2_get_EncodeProfile(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeProfile(This,pValue) ) 

#define IVisionUserPin2_get_EncodeProfileDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeProfileDefault(This,pValue) ) 

#define IVisionUserPin2_get_IsEncodeLevelSupported(This,value,pBSupported)	\
    ( (This)->lpVtbl -> get_IsEncodeLevelSupported(This,value,pBSupported) ) 

#define IVisionUserPin2_put_EncodeLevel(This,value)	\
    ( (This)->lpVtbl -> put_EncodeLevel(This,value) ) 

#define IVisionUserPin2_get_EncodeLevel(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeLevel(This,pValue) ) 

#define IVisionUserPin2_get_EncodeLevelDefault(This,pValue)	\
    ( (This)->lpVtbl -> get_EncodeLevelDefault(This,pValue) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUserPin2_INTERFACE_DEFINED__ */


#ifndef __IVisionUserPinGamut_INTERFACE_DEFINED__
#define __IVisionUserPinGamut_INTERFACE_DEFINED__

/* interface IVisionUserPinGamut */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IVisionUserPinGamut;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AC488D2C-9651-4946-935D-DBA0552C4F11")
    IVisionUserPinGamut : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsPixelGamutSupported( 
            /* [retval][out] */ long *pBSupported) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PixelGamut( 
            /* [in] */ PIXEL_GAMUT value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PixelGamut( 
            /* [retval][out] */ PIXEL_GAMUT *pValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsPixelRangeSupported( 
            /* [retval][out] */ long *pBSupported) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PixelRange( 
            /* [in] */ PIXEL_RANGE value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PixelRange( 
            /* [retval][out] */ PIXEL_RANGE *pValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVisionUserPinGamutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVisionUserPinGamut * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVisionUserPinGamut * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVisionUserPinGamut * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsPixelGamutSupported )( 
            IVisionUserPinGamut * This,
            /* [retval][out] */ long *pBSupported);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PixelGamut )( 
            IVisionUserPinGamut * This,
            /* [in] */ PIXEL_GAMUT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PixelGamut )( 
            IVisionUserPinGamut * This,
            /* [retval][out] */ PIXEL_GAMUT *pValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsPixelRangeSupported )( 
            IVisionUserPinGamut * This,
            /* [retval][out] */ long *pBSupported);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PixelRange )( 
            IVisionUserPinGamut * This,
            /* [in] */ PIXEL_RANGE value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PixelRange )( 
            IVisionUserPinGamut * This,
            /* [retval][out] */ PIXEL_RANGE *pValue);
        
        END_INTERFACE
    } IVisionUserPinGamutVtbl;

    interface IVisionUserPinGamut
    {
        CONST_VTBL struct IVisionUserPinGamutVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVisionUserPinGamut_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVisionUserPinGamut_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVisionUserPinGamut_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVisionUserPinGamut_IsPixelGamutSupported(This,pBSupported)	\
    ( (This)->lpVtbl -> IsPixelGamutSupported(This,pBSupported) ) 

#define IVisionUserPinGamut_put_PixelGamut(This,value)	\
    ( (This)->lpVtbl -> put_PixelGamut(This,value) ) 

#define IVisionUserPinGamut_get_PixelGamut(This,pValue)	\
    ( (This)->lpVtbl -> get_PixelGamut(This,pValue) ) 

#define IVisionUserPinGamut_IsPixelRangeSupported(This,pBSupported)	\
    ( (This)->lpVtbl -> IsPixelRangeSupported(This,pBSupported) ) 

#define IVisionUserPinGamut_put_PixelRange(This,value)	\
    ( (This)->lpVtbl -> put_PixelRange(This,value) ) 

#define IVisionUserPinGamut_get_PixelRange(This,pValue)	\
    ( (This)->lpVtbl -> get_PixelRange(This,pValue) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVisionUserPinGamut_INTERFACE_DEFINED__ */

#endif /* __VisionStreaming_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


