/*******************************************************************************

Copyright Datapath Ltd. 2009.

File:    CVisionCallBack.h

Purpose: Declarations for the Vision callback interface.

History: 18 MAR 09    RL   Created.

*******************************************************************************/

typedef void (__cdecl CALLBACKFN) ( ULONG eventFlag, long channel, PVOID pUserdata );

typedef CALLBACKFN *PCALLBACKFN;

class CVisionCallBack : public CUnknown, public IVisionCallBack
{
public:
   /* Implement IUnknown. */
   DECLARE_IUNKNOWN
      
   CVisionCallBack( PCALLBACKFN pCallBackFn );
   ~CVisionCallBack();
   STDMETHODIMP ClientCallback ( ULONG eventFlag, long channel, PVOID pUserdata );

private:
   PCALLBACKFN   m_pCallBackFn;
};

/******************************************************************************/