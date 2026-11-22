
/*******************************************************************************

Copyright Datapath Ltd. 2009, 2010.

File:    VisionUtil.h

Purpose: 

History:
         16 FEB 09    RL   Created.
         06 OCT 09    RL   Added DisconnectAndRemoveFilter.
         01 JUL 10    RL   Added RemoveGraphFilters.
                           Added GetGraphFilter.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Declarations ***************************************************************/

void 
ErrorMessage(
   HWND     hWnd,
   LPCTSTR  pTitle,
   TCHAR    *pFormat,
   HRESULT  hr);

HRESULT 
AddGraphToRot(
   IUnknown *pUnkGraph, 
   DWORD    *pdwRegister) ;

void 
RemoveGraphFromRot(
   DWORD pdwRegister);

void
DisconnectFilter(
   IGraphBuilder  *pGraphBuilder,
   IBaseFilter    *pSrcFilter);

void
DisconnectAndRemoveFilter(
   IGraphBuilder  *pGraphBuilder,
   IBaseFilter    *pSrcFilter);

HRESULT
RemoveGraphFilters(
   IFilterGraph *pGraph);

HRESULT
RemoveGraphFiltersIfNotConnected (
   IFilterGraph *pGraph );

HRESULT 
GetGraphFilter(
   IFilterGraph *pGraph,
   LPCTSTR      pFilterName,
   IBaseFilter  **ppSrcFilter);

HRESULT
LoadRendererQualPropInterface(
   ICaptureGraphBuilder2   *pBuilder,
   IBaseFilter             *pSrcFilter,
   IQualProp               **ppIQP);

/******************************************************************************/