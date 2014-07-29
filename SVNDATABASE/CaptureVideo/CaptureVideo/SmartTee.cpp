// SmartTee.cpp: implementation of the CSmartTee class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SmartTee.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSmartTee::CSmartTee(IGraphBuilder *inGraph)
:CBaseFilter(inGraph,CLSID_SmartTee,"Smart Tee")
{

}

CSmartTee::~CSmartTee()
{

}

IPin * CSmartTee::GetPreviewPin()
{
	return CBaseFilter::FindPin(FALSE,"Preview");
}

IPin * CSmartTee::GetCapturePin()
{
	return CBaseFilter::FindPin(FALSE,"Capture");
}

IPin * CSmartTee::GetInputPin()
{
	return CBaseFilter::FindPin(TRUE);
}
