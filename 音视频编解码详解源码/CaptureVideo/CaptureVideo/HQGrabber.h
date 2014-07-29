// HQGrabber.h: interface for the CHQGrabber class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HQGRABBER_H__3EFD6478_8811_400C_BDBE_C7C329803C94__INCLUDED_)
#define AFX_HQGRABBER_H__3EFD6478_8811_400C_BDBE_C7C329803C94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BaseFilter.h"
#include "IImageGrabber.h"

class CHQGrabber  : public CBaseFilter
{
public:
	CHQGrabber(IGraphBuilder *inGraph);
	virtual ~CHQGrabber();

public:
	IImageGrabber  *m_pGrabber;

public:
	virtual BOOL CreateFilter();
	void Initialization();
	long GetBitCount();
	void Snapshot(BYTE * outBuffer,BOOL inIsSyncmode);
	void GetBitmapInfoHead(BITMAPINFOHEADER * outBitMapHead);
	long GetImageSize();
	long GetHeight();
	long GetWidth();
	IPin * GetOutputPin();
	IPin * GetInputPin();


};

#endif // !defined(AFX_HQGRABBER_H__3EFD6478_8811_400C_BDBE_C7C329803C94__INCLUDED_)
