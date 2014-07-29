// NullRenderer.cpp: implementation of the CNullRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NullRenderer.h"

#include <DShow.h>
#include <qedit.h>
#include <afxtempl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNullRenderer::CNullRenderer(IGraphBuilder * inGraph)
:CBaseFilter(inGraph,CLSID_NullRenderer,"Null Renderer")
{

}

CNullRenderer::~CNullRenderer()
{

}

IPin * CNullRenderer::GetInputPin()
{
	return CBaseFilter::FindPin(TRUE);
}
