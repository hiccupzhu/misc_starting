// VideoRenderer.h: interface for the CVideoRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEORENDERER_H__ADEDB380_2701_47D0_8DBD_BC30DA4908FA__INCLUDED_)
#define AFX_VIDEORENDERER_H__ADEDB380_2701_47D0_8DBD_BC30DA4908FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BaseFilter.h"

class CVideoRenderer  : public CBaseFilter
{
public:
	CVideoRenderer(IGraphBuilder *inGraph);
	virtual ~CVideoRenderer();

public:
	IPin * GetInputPin();

};

#endif // !defined(AFX_VIDEORENDERER_H__ADEDB380_2701_47D0_8DBD_BC30DA4908FA__INCLUDED_)
