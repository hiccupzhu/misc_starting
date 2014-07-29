// VideoRenderer.cpp: implementation of the CVideoRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoRenderer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoRenderer::CVideoRenderer(IGraphBuilder * inGraph)
:CBaseFilter(inGraph,CLSID_VideoRenderer,"Video Renderer")
{

}

CVideoRenderer::~CVideoRenderer()
{

}

IPin * CVideoRenderer::GetInputPin()
{
	return CBaseFilter::FindPin(TRUE);
}
