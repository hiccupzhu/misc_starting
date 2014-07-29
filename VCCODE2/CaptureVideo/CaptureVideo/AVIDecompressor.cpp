// AVIDecompressor.cpp: implementation of the CAVIDecompressor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AVIDecompressor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAVIDecompressor::CAVIDecompressor(IGraphBuilder * inGraph)
:CBaseFilter(inGraph,CLSID_AVIDec,"AVI Decompressor")
{

}

CAVIDecompressor::~CAVIDecompressor()
{

}

IPin * CAVIDecompressor::GetInputPin()
{
	return CBaseFilter::FindPin(TRUE);
}

IPin * CAVIDecompressor::GetOutputPin()
{
	return CBaseFilter::FindPin(FALSE);
}
