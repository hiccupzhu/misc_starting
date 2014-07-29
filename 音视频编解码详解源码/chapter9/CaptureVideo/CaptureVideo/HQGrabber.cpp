// HQGrabber.cpp: implementation of the CHQGrabber class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HQGrabber.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHQGrabber::CHQGrabber(IGraphBuilder * inGraph)
:CBaseFilter(inGraph,CLSID_HQImageGrabber,"HQ Grabber")
{
	m_pGrabber=NULL;
}

CHQGrabber::~CHQGrabber()
{
	if(m_pGrabber)
	{
		m_pGrabber->Release();
		m_pGrabber=NULL;
	}
}

void CHQGrabber::Initialization()
{
	HRESULT hr=m_pBaseFilter->QueryInterface(IID_IImageGrabber,(void **)&m_pGrabber);
	if(FAILED(hr)) 
		return;
}

IPin * CHQGrabber::GetInputPin()
{
	return CBaseFilter::FindPin(TRUE);
}

IPin * CHQGrabber::GetOutputPin()
{
	return CBaseFilter::FindPin(FALSE);
}

long CHQGrabber::GetWidth()
{
	long lWidth=0;
	long lHeight=0;
	long lBitCount=0;
	m_pGrabber->get_ImageSize(&lWidth,&lHeight,&lBitCount);
	return lWidth;
}

long CHQGrabber::GetHeight()
{
	long lWidth=0;
	long lHeight=0;
	long lBitCount=0;
	m_pGrabber->get_ImageSize(&lWidth,&lHeight,&lBitCount);
	return lHeight;
}

long CHQGrabber::GetBitCount()
{
	long lWidth=0;
	long lHeight=0;
	long lBitCount=0;
	m_pGrabber->get_ImageSize(&lWidth,&lHeight,&lBitCount);
	return lBitCount;
}

long CHQGrabber::GetImageSize()
{
	long size=0;
	m_pGrabber->get_FrameSize(&size);
	return size;
}


void CHQGrabber::GetBitmapInfoHead(BITMAPINFOHEADER *outBitMapHead)
{
	m_pGrabber->get_BitmapInfoHeader(outBitMapHead);
}

void CHQGrabber::Snapshot(BYTE *outBuffer, BOOL inIsSyncmode)
{
	m_pGrabber->Snapshot(outBuffer,inIsSyncmode);
}


BOOL CHQGrabber::CreateFilter()
{
	if (!m_pBaseFilter && m_pGraph)
	{
		if (SUCCEEDED(CoCreateInstance(m_guid, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void **)&m_pBaseFilter)))
		{
			if (SUCCEEDED(m_pGraph->AddFilter(m_pBaseFilter, m_name)))
			{
				Initialization();
				return TRUE;
			}
		}
	}
	ReleaseFilter();
	return FALSE;
}
