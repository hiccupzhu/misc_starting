// NullRenderer.h: interface for the CNullRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NULLRENDERER_H__8B74DC4D_0354_470E_860F_8D1C6DB477BA__INCLUDED_)
#define AFX_NULLRENDERER_H__8B74DC4D_0354_470E_860F_8D1C6DB477BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BaseFilter.h"

class CNullRenderer  : public CBaseFilter
{
public:
	CNullRenderer(IGraphBuilder * inGraph);
	virtual ~CNullRenderer();

public:
	IPin * GetInputPin();

};

#endif // !defined(AFX_NULLRENDERER_H__8B74DC4D_0354_470E_860F_8D1C6DB477BA__INCLUDED_)
