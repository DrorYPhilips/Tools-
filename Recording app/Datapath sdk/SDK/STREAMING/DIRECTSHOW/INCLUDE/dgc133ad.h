

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Feb 16 12:37:57 2022
 */
/* Compiler settings for dgc133ad.idl:
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


#ifndef __dgc133ad_h__
#define __dgc133ad_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IAudioUser_FWD_DEFINED__
#define __IAudioUser_FWD_DEFINED__
typedef interface IAudioUser IAudioUser;
#endif 	/* __IAudioUser_FWD_DEFINED__ */


#ifndef __IAudioUser2_FWD_DEFINED__
#define __IAudioUser2_FWD_DEFINED__
typedef interface IAudioUser2 IAudioUser2;
#endif 	/* __IAudioUser2_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __VisionUserStreaming_LIBRARY_DEFINED__
#define __VisionUserStreaming_LIBRARY_DEFINED__

/* library VisionUserStreaming */
/* [control][helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_VisionUserStreaming;

#ifndef __IAudioUser_INTERFACE_DEFINED__
#define __IAudioUser_INTERFACE_DEFINED__

/* interface IAudioUser */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IAudioUser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18AE65DC-E8B0-4e4f-A72A-108A151EB507")
    IAudioUser : public IUnknown
    {
    public:
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DigitalChannelPair( 
            /* [in] */ unsigned long channel) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DigitalChannelPair( 
            /* [retval][out] */ unsigned long *pChannel) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DigitalMute( 
            /* [in] */ unsigned long digitalMute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DigitalMute( 
            /* [retval][out] */ unsigned long *pDigitalMute) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnalogueBalancedGain( 
            /* [in] */ long gain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueBalancedGain( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueBalancedGainMinimum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueBalancedGainMaximum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueBalancedGainDefault( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnalogueBalancedGainBoost( 
            /* [in] */ unsigned long boost) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueBalancedGainBoost( 
            /* [retval][out] */ unsigned long *pboost) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnalogueBalancedMute( 
            /* [in] */ unsigned long mute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueBalancedMute( 
            /* [retval][out] */ unsigned long *pMute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueBalancedGainScale( 
            /* [retval][out] */ unsigned long *pScale) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnalogueUnbalancedGain( 
            /* [in] */ long gain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueUnbalancedGain( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueUnbalancedGainMinimum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueUnbalancedGainMaximum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueUnbalancedGainDefault( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnalogueUnbalancedMute( 
            /* [in] */ unsigned long mute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueUnbalancedMute( 
            /* [retval][out] */ unsigned long *pMute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnalogueUnbalancedGainScale( 
            /* [retval][out] */ unsigned long *pScale) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ADCGain( 
            /* [in] */ long gain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ADCGain( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ADCGainMinimum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ADCGainMaximum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ADCGainDefault( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ADCMute( 
            /* [in] */ unsigned long mute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ADCMute( 
            /* [retval][out] */ unsigned long *pMute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ADCGainScale( 
            /* [retval][out] */ unsigned long *pScale) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LineOutSource( 
            /* [in] */ unsigned long source) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LineOutSource( 
            /* [retval][out] */ unsigned long *pSource) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LineOutGain( 
            /* [in] */ long gain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LineOutGain( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LineOutGainMinimum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LineOutGainMaximum( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LineOutGainDefault( 
            /* [retval][out] */ long *pGain) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LineOutMute( 
            /* [in] */ unsigned long mute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LineOutMute( 
            /* [retval][out] */ unsigned long *pMute) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LineOutGainScale( 
            /* [retval][out] */ unsigned long *pScale) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MediaTypeCount( 
            /* [retval][out] */ unsigned long *pCount) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MediaType( 
            /* [in] */ unsigned long index,
            /* [retval][out] */ AM_MEDIA_TYPE *pMT) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCurrentParametersAsUserDefined( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ResetCurrentParametersAsUserDefined( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAudioUserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAudioUser * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAudioUser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAudioUser * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DigitalChannelPair )( 
            IAudioUser * This,
            /* [in] */ unsigned long channel);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DigitalChannelPair )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pChannel);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DigitalMute )( 
            IAudioUser * This,
            /* [in] */ unsigned long digitalMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DigitalMute )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pDigitalMute);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueBalancedGain )( 
            IAudioUser * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGain )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainMinimum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainMaximum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainDefault )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueBalancedGainBoost )( 
            IAudioUser * This,
            /* [in] */ unsigned long boost);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainBoost )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pboost);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueBalancedMute )( 
            IAudioUser * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedMute )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainScale )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueUnbalancedGain )( 
            IAudioUser * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGain )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainMinimum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainMaximum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainDefault )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueUnbalancedMute )( 
            IAudioUser * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedMute )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainScale )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ADCGain )( 
            IAudioUser * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGain )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainMinimum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainMaximum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainDefault )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ADCMute )( 
            IAudioUser * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCMute )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainScale )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LineOutSource )( 
            IAudioUser * This,
            /* [in] */ unsigned long source);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutSource )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pSource);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LineOutGain )( 
            IAudioUser * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGain )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainMinimum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainMaximum )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainDefault )( 
            IAudioUser * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LineOutMute )( 
            IAudioUser * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutMute )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainScale )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MediaTypeCount )( 
            IAudioUser * This,
            /* [retval][out] */ unsigned long *pCount);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            IAudioUser * This,
            /* [in] */ unsigned long index,
            /* [retval][out] */ AM_MEDIA_TYPE *pMT);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCurrentParametersAsUserDefined )( 
            IAudioUser * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ResetCurrentParametersAsUserDefined )( 
            IAudioUser * This);
        
        END_INTERFACE
    } IAudioUserVtbl;

    interface IAudioUser
    {
        CONST_VTBL struct IAudioUserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAudioUser_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAudioUser_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAudioUser_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAudioUser_put_DigitalChannelPair(This,channel)	\
    ( (This)->lpVtbl -> put_DigitalChannelPair(This,channel) ) 

#define IAudioUser_get_DigitalChannelPair(This,pChannel)	\
    ( (This)->lpVtbl -> get_DigitalChannelPair(This,pChannel) ) 

#define IAudioUser_put_DigitalMute(This,digitalMute)	\
    ( (This)->lpVtbl -> put_DigitalMute(This,digitalMute) ) 

#define IAudioUser_get_DigitalMute(This,pDigitalMute)	\
    ( (This)->lpVtbl -> get_DigitalMute(This,pDigitalMute) ) 

#define IAudioUser_put_AnalogueBalancedGain(This,gain)	\
    ( (This)->lpVtbl -> put_AnalogueBalancedGain(This,gain) ) 

#define IAudioUser_get_AnalogueBalancedGain(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGain(This,pGain) ) 

#define IAudioUser_get_AnalogueBalancedGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainMinimum(This,pGain) ) 

#define IAudioUser_get_AnalogueBalancedGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainMaximum(This,pGain) ) 

#define IAudioUser_get_AnalogueBalancedGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainDefault(This,pGain) ) 

#define IAudioUser_put_AnalogueBalancedGainBoost(This,boost)	\
    ( (This)->lpVtbl -> put_AnalogueBalancedGainBoost(This,boost) ) 

#define IAudioUser_get_AnalogueBalancedGainBoost(This,pboost)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainBoost(This,pboost) ) 

#define IAudioUser_put_AnalogueBalancedMute(This,mute)	\
    ( (This)->lpVtbl -> put_AnalogueBalancedMute(This,mute) ) 

#define IAudioUser_get_AnalogueBalancedMute(This,pMute)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedMute(This,pMute) ) 

#define IAudioUser_get_AnalogueBalancedGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainScale(This,pScale) ) 

#define IAudioUser_put_AnalogueUnbalancedGain(This,gain)	\
    ( (This)->lpVtbl -> put_AnalogueUnbalancedGain(This,gain) ) 

#define IAudioUser_get_AnalogueUnbalancedGain(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGain(This,pGain) ) 

#define IAudioUser_get_AnalogueUnbalancedGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainMinimum(This,pGain) ) 

#define IAudioUser_get_AnalogueUnbalancedGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainMaximum(This,pGain) ) 

#define IAudioUser_get_AnalogueUnbalancedGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainDefault(This,pGain) ) 

#define IAudioUser_put_AnalogueUnbalancedMute(This,mute)	\
    ( (This)->lpVtbl -> put_AnalogueUnbalancedMute(This,mute) ) 

#define IAudioUser_get_AnalogueUnbalancedMute(This,pMute)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedMute(This,pMute) ) 

#define IAudioUser_get_AnalogueUnbalancedGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainScale(This,pScale) ) 

#define IAudioUser_put_ADCGain(This,gain)	\
    ( (This)->lpVtbl -> put_ADCGain(This,gain) ) 

#define IAudioUser_get_ADCGain(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGain(This,pGain) ) 

#define IAudioUser_get_ADCGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGainMinimum(This,pGain) ) 

#define IAudioUser_get_ADCGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGainMaximum(This,pGain) ) 

#define IAudioUser_get_ADCGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGainDefault(This,pGain) ) 

#define IAudioUser_put_ADCMute(This,mute)	\
    ( (This)->lpVtbl -> put_ADCMute(This,mute) ) 

#define IAudioUser_get_ADCMute(This,pMute)	\
    ( (This)->lpVtbl -> get_ADCMute(This,pMute) ) 

#define IAudioUser_get_ADCGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_ADCGainScale(This,pScale) ) 

#define IAudioUser_put_LineOutSource(This,source)	\
    ( (This)->lpVtbl -> put_LineOutSource(This,source) ) 

#define IAudioUser_get_LineOutSource(This,pSource)	\
    ( (This)->lpVtbl -> get_LineOutSource(This,pSource) ) 

#define IAudioUser_put_LineOutGain(This,gain)	\
    ( (This)->lpVtbl -> put_LineOutGain(This,gain) ) 

#define IAudioUser_get_LineOutGain(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGain(This,pGain) ) 

#define IAudioUser_get_LineOutGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGainMinimum(This,pGain) ) 

#define IAudioUser_get_LineOutGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGainMaximum(This,pGain) ) 

#define IAudioUser_get_LineOutGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGainDefault(This,pGain) ) 

#define IAudioUser_put_LineOutMute(This,mute)	\
    ( (This)->lpVtbl -> put_LineOutMute(This,mute) ) 

#define IAudioUser_get_LineOutMute(This,pMute)	\
    ( (This)->lpVtbl -> get_LineOutMute(This,pMute) ) 

#define IAudioUser_get_LineOutGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_LineOutGainScale(This,pScale) ) 

#define IAudioUser_get_MediaTypeCount(This,pCount)	\
    ( (This)->lpVtbl -> get_MediaTypeCount(This,pCount) ) 

#define IAudioUser_get_MediaType(This,index,pMT)	\
    ( (This)->lpVtbl -> get_MediaType(This,index,pMT) ) 

#define IAudioUser_SetCurrentParametersAsUserDefined(This)	\
    ( (This)->lpVtbl -> SetCurrentParametersAsUserDefined(This) ) 

#define IAudioUser_ResetCurrentParametersAsUserDefined(This)	\
    ( (This)->lpVtbl -> ResetCurrentParametersAsUserDefined(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAudioUser_INTERFACE_DEFINED__ */


#ifndef __IAudioUser2_INTERFACE_DEFINED__
#define __IAudioUser2_INTERFACE_DEFINED__

/* interface IAudioUser2 */
/* [version][unique][uuid][object] */ 


EXTERN_C const IID IID_IAudioUser2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("633E1573-7103-4C56-A87B-137FA56FFCD5")
    IAudioUser2 : public IAudioUser
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Source( 
            /* [retval][out] */ unsigned long *pSource) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CaptureType( 
            /* [in] */ unsigned long type) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptureType( 
            /* [retval][out] */ unsigned long *pType) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAudioUser2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAudioUser2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAudioUser2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAudioUser2 * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DigitalChannelPair )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long channel);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DigitalChannelPair )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pChannel);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DigitalMute )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long digitalMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DigitalMute )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pDigitalMute);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueBalancedGain )( 
            IAudioUser2 * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGain )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainMinimum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainMaximum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainDefault )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueBalancedGainBoost )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long boost);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainBoost )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pboost);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueBalancedMute )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedMute )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueBalancedGainScale )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueUnbalancedGain )( 
            IAudioUser2 * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGain )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainMinimum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainMaximum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainDefault )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_AnalogueUnbalancedMute )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedMute )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_AnalogueUnbalancedGainScale )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ADCGain )( 
            IAudioUser2 * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGain )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainMinimum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainMaximum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainDefault )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ADCMute )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCMute )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ADCGainScale )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LineOutSource )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long source);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutSource )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pSource);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LineOutGain )( 
            IAudioUser2 * This,
            /* [in] */ long gain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGain )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainMinimum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainMaximum )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainDefault )( 
            IAudioUser2 * This,
            /* [retval][out] */ long *pGain);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_LineOutMute )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long mute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutMute )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pMute);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LineOutGainScale )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pScale);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MediaTypeCount )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pCount);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long index,
            /* [retval][out] */ AM_MEDIA_TYPE *pMT);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCurrentParametersAsUserDefined )( 
            IAudioUser2 * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ResetCurrentParametersAsUserDefined )( 
            IAudioUser2 * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pSource);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_CaptureType )( 
            IAudioUser2 * This,
            /* [in] */ unsigned long type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CaptureType )( 
            IAudioUser2 * This,
            /* [retval][out] */ unsigned long *pType);
        
        END_INTERFACE
    } IAudioUser2Vtbl;

    interface IAudioUser2
    {
        CONST_VTBL struct IAudioUser2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAudioUser2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAudioUser2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAudioUser2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAudioUser2_put_DigitalChannelPair(This,channel)	\
    ( (This)->lpVtbl -> put_DigitalChannelPair(This,channel) ) 

#define IAudioUser2_get_DigitalChannelPair(This,pChannel)	\
    ( (This)->lpVtbl -> get_DigitalChannelPair(This,pChannel) ) 

#define IAudioUser2_put_DigitalMute(This,digitalMute)	\
    ( (This)->lpVtbl -> put_DigitalMute(This,digitalMute) ) 

#define IAudioUser2_get_DigitalMute(This,pDigitalMute)	\
    ( (This)->lpVtbl -> get_DigitalMute(This,pDigitalMute) ) 

#define IAudioUser2_put_AnalogueBalancedGain(This,gain)	\
    ( (This)->lpVtbl -> put_AnalogueBalancedGain(This,gain) ) 

#define IAudioUser2_get_AnalogueBalancedGain(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGain(This,pGain) ) 

#define IAudioUser2_get_AnalogueBalancedGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainMinimum(This,pGain) ) 

#define IAudioUser2_get_AnalogueBalancedGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainMaximum(This,pGain) ) 

#define IAudioUser2_get_AnalogueBalancedGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainDefault(This,pGain) ) 

#define IAudioUser2_put_AnalogueBalancedGainBoost(This,boost)	\
    ( (This)->lpVtbl -> put_AnalogueBalancedGainBoost(This,boost) ) 

#define IAudioUser2_get_AnalogueBalancedGainBoost(This,pboost)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainBoost(This,pboost) ) 

#define IAudioUser2_put_AnalogueBalancedMute(This,mute)	\
    ( (This)->lpVtbl -> put_AnalogueBalancedMute(This,mute) ) 

#define IAudioUser2_get_AnalogueBalancedMute(This,pMute)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedMute(This,pMute) ) 

#define IAudioUser2_get_AnalogueBalancedGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_AnalogueBalancedGainScale(This,pScale) ) 

#define IAudioUser2_put_AnalogueUnbalancedGain(This,gain)	\
    ( (This)->lpVtbl -> put_AnalogueUnbalancedGain(This,gain) ) 

#define IAudioUser2_get_AnalogueUnbalancedGain(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGain(This,pGain) ) 

#define IAudioUser2_get_AnalogueUnbalancedGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainMinimum(This,pGain) ) 

#define IAudioUser2_get_AnalogueUnbalancedGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainMaximum(This,pGain) ) 

#define IAudioUser2_get_AnalogueUnbalancedGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainDefault(This,pGain) ) 

#define IAudioUser2_put_AnalogueUnbalancedMute(This,mute)	\
    ( (This)->lpVtbl -> put_AnalogueUnbalancedMute(This,mute) ) 

#define IAudioUser2_get_AnalogueUnbalancedMute(This,pMute)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedMute(This,pMute) ) 

#define IAudioUser2_get_AnalogueUnbalancedGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_AnalogueUnbalancedGainScale(This,pScale) ) 

#define IAudioUser2_put_ADCGain(This,gain)	\
    ( (This)->lpVtbl -> put_ADCGain(This,gain) ) 

#define IAudioUser2_get_ADCGain(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGain(This,pGain) ) 

#define IAudioUser2_get_ADCGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGainMinimum(This,pGain) ) 

#define IAudioUser2_get_ADCGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGainMaximum(This,pGain) ) 

#define IAudioUser2_get_ADCGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_ADCGainDefault(This,pGain) ) 

#define IAudioUser2_put_ADCMute(This,mute)	\
    ( (This)->lpVtbl -> put_ADCMute(This,mute) ) 

#define IAudioUser2_get_ADCMute(This,pMute)	\
    ( (This)->lpVtbl -> get_ADCMute(This,pMute) ) 

#define IAudioUser2_get_ADCGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_ADCGainScale(This,pScale) ) 

#define IAudioUser2_put_LineOutSource(This,source)	\
    ( (This)->lpVtbl -> put_LineOutSource(This,source) ) 

#define IAudioUser2_get_LineOutSource(This,pSource)	\
    ( (This)->lpVtbl -> get_LineOutSource(This,pSource) ) 

#define IAudioUser2_put_LineOutGain(This,gain)	\
    ( (This)->lpVtbl -> put_LineOutGain(This,gain) ) 

#define IAudioUser2_get_LineOutGain(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGain(This,pGain) ) 

#define IAudioUser2_get_LineOutGainMinimum(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGainMinimum(This,pGain) ) 

#define IAudioUser2_get_LineOutGainMaximum(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGainMaximum(This,pGain) ) 

#define IAudioUser2_get_LineOutGainDefault(This,pGain)	\
    ( (This)->lpVtbl -> get_LineOutGainDefault(This,pGain) ) 

#define IAudioUser2_put_LineOutMute(This,mute)	\
    ( (This)->lpVtbl -> put_LineOutMute(This,mute) ) 

#define IAudioUser2_get_LineOutMute(This,pMute)	\
    ( (This)->lpVtbl -> get_LineOutMute(This,pMute) ) 

#define IAudioUser2_get_LineOutGainScale(This,pScale)	\
    ( (This)->lpVtbl -> get_LineOutGainScale(This,pScale) ) 

#define IAudioUser2_get_MediaTypeCount(This,pCount)	\
    ( (This)->lpVtbl -> get_MediaTypeCount(This,pCount) ) 

#define IAudioUser2_get_MediaType(This,index,pMT)	\
    ( (This)->lpVtbl -> get_MediaType(This,index,pMT) ) 

#define IAudioUser2_SetCurrentParametersAsUserDefined(This)	\
    ( (This)->lpVtbl -> SetCurrentParametersAsUserDefined(This) ) 

#define IAudioUser2_ResetCurrentParametersAsUserDefined(This)	\
    ( (This)->lpVtbl -> ResetCurrentParametersAsUserDefined(This) ) 


#define IAudioUser2_get_Source(This,pSource)	\
    ( (This)->lpVtbl -> get_Source(This,pSource) ) 

#define IAudioUser2_put_CaptureType(This,type)	\
    ( (This)->lpVtbl -> put_CaptureType(This,type) ) 

#define IAudioUser2_get_CaptureType(This,pType)	\
    ( (This)->lpVtbl -> get_CaptureType(This,pType) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAudioUser2_INTERFACE_DEFINED__ */

#endif /* __VisionUserStreaming_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


