/*******************************************************************************

Copyright Datapath Ltd. 2009, 2016.

File:    CVisionCallBack.cpp

Purpose: Implementation of a callback class.

History: 30 MAR 09    RL   Created.
         26 APR 12    JL   Added support for User-Mode filters.
         15 DEC 16    JL   Removed need for compiling as User or Kernel Mode.

*******************************************************************************/

#include <streams.h>

#include "DGC133ST.h"

#include "CVisionCallBack.h"

/******************************************************************************/
/* Constructor */
CVisionCallBack::CVisionCallBack( PCALLBACKFN pCallBackFn ) :
   CUnknown("CVisionCallBack", NULL ),
   m_pCallBackFn ( pCallBackFn )
{
}

/******************************************************************************/
/* Destructor */
CVisionCallBack::~CVisionCallBack ( )
{
}

/******************************************************************************/

HRESULT 
CVisionCallBack::ClientCallback ( ULONG eventFlag, long channel, PVOID pUserdata )
{
   if ( m_pCallBackFn )
   {
      m_pCallBackFn ( eventFlag, channel, pUserdata );
   }

   return S_OK;
}

/******************************************************************************/
