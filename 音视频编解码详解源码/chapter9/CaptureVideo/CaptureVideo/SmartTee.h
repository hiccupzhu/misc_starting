// SmartTee.h: interface for the CSmartTee class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMARTTEE_H__9AC56D20_EED4_4DF0_90B4_1B0BF0890BF4__INCLUDED_)
#define AFX_SMARTTEE_H__9AC56D20_EED4_4DF0_90B4_1B0BF0890BF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BaseFilter.h"

class CSmartTee  :  public CBaseFilter
{
public:
	CSmartTee(IGraphBuilder *inGraph);
	virtual ~CSmartTee();

public:
	IPin * GetInputPin();
	IPin * GetCapturePin();
	IPin * GetPreviewPin();

};

#endif // !defined(AFX_SMARTTEE_H__9AC56D20_EED4_4DF0_90B4_1B0BF0890BF4__INCLUDED_)
