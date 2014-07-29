// BaseFilter.h: interface for the CBaseFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEFILTER_H__118ED901_331B_4567_8ECF_DAEBF5689150__INCLUDED_)
#define AFX_BASEFILTER_H__118ED901_331B_4567_8ECF_DAEBF5689150__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <DShow.h>
//#include <qedit.h>
//#include <afxtempl.h>

class CBaseFilter  
{
public:
	CBaseFilter(IGraphBuilder * inGraph, GUID inClsid, const char * inName);
	virtual ~CBaseFilter();

public:
	IGraphBuilder	*m_pGraph;
	GUID			m_guid;
	WCHAR			m_name[256];
	IBaseFilter		*m_pBaseFilter;

public:
	virtual BOOL CreateFilter();
	void ReleaseFilter();
	IPin * FindPin(BOOL inPinDir, const char *inPartialName=NULL, IBaseFilter *inFilter=NULL);
	IBaseFilter * GetNextFilter();
	IBaseFilter * FindFilter(char * inFilterName,IGraphBuilder *inGraph=NULL);
	IBaseFilter * GetFilter();

};

#endif // !defined(AFX_BASEFILTER_H__118ED901_331B_4567_8ECF_DAEBF5689150__INCLUDED_)
