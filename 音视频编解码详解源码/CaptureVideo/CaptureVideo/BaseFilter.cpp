// BaseFilter.cpp: implementation of the CBaseFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseFilter::CBaseFilter(IGraphBuilder * inGraph, GUID inClsid, const char * inName)
{
	m_pGraph=inGraph;
	m_guid=inClsid;
	m_pBaseFilter=NULL;
	if (inName)
	{
		MultiByteToWideChar(CP_ACP, 0, inName, -1, m_name, 256);
	}
	else
	{
		wcscpy(m_name, L"");
	}
}

CBaseFilter::~CBaseFilter()
{
	ReleaseFilter();
}

void CBaseFilter::ReleaseFilter()
{
	if (m_pBaseFilter)
	{
		if (m_pGraph)
		{
			m_pGraph->RemoveFilter(m_pBaseFilter);
		}
		m_pBaseFilter->Release();
		m_pBaseFilter = NULL;
	}	
}

BOOL CBaseFilter::CreateFilter()
{
	if (!m_pBaseFilter && m_pGraph)
	{
		if (SUCCEEDED(CoCreateInstance(m_guid, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void **)&m_pBaseFilter)))
		{
			if (SUCCEEDED(m_pGraph->AddFilter(m_pBaseFilter, m_name)))
			{
				return TRUE;
			}
		}
	}
	ReleaseFilter();
	return FALSE;
}

IBaseFilter * CBaseFilter::GetFilter()
{
	return m_pBaseFilter;
}

IBaseFilter * CBaseFilter::FindFilter(char *inFilterName,IGraphBuilder *inGraph )
{
	HRESULT hr=NULL;
	
	IEnumFilters * pEnumFilters=NULL;
	IBaseFilter * pFilter=NULL;
	IBaseFilter * pFoundFilter=NULL;
	
	if(inGraph==NULL)
	{
		inGraph=m_pGraph;
	}

	hr=inGraph->EnumFilters(&pEnumFilters);
	if(FAILED(hr))
		return NULL;
	unsigned long fetched=0;
	while(SUCCEEDED(pEnumFilters->Next(1,&pFilter,&fetched)) && fetched)
	{
		FILTER_INFO filterinfo;
		hr=pFilter->QueryFilterInfo(&filterinfo);
		if(FAILED(hr))
		{
			pFilter->Release();
			pEnumFilters->Release();
			return NULL;
		}
		
		CString str(filterinfo.achName);		
		
        filterinfo.pGraph->Release();		
		pFilter->Release();
		
		if(str==CString(inFilterName))
		{
			pFoundFilter=pFilter;
			break;
		}
	}
	pEnumFilters->Release();
	
	return pFoundFilter;
}

IBaseFilter * CBaseFilter::GetNextFilter()
{
	IPin * pPin=FindPin(FALSE);
	IPin * pOutPin=NULL;
	IBaseFilter * pFoundFilter=NULL;
	PIN_INFO	pininfo;

	if(SUCCEEDED(pPin->ConnectedTo(&pOutPin)))
	{
		if(SUCCEEDED(pOutPin->QueryPinInfo(&pininfo)))
		{
			pFoundFilter=pininfo.pFilter;
			pininfo.pFilter->Release();
		}
	}
	return pFoundFilter;
}

IPin * CBaseFilter::FindPin(BOOL inPinDir, const char *inPartialName, IBaseFilter *inFilter)
{
	PIN_DIRECTION direction = inPinDir ? PINDIR_INPUT : PINDIR_OUTPUT;
	IPin * foundPin = NULL;

	if(inFilter==NULL)
	{
		inFilter=m_pBaseFilter;
	}
	
	if (inFilter)
	{
		IEnumPins * pinEnum = NULL;
		if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();
			
			IPin * pin = NULL;
			ULONG fetchCount = 0;
			while (!foundPin && SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && fetchCount)
			{
				if (pin)
				{
					PIN_INFO pinInfo;
					if (SUCCEEDED(pin->QueryPinInfo(&pinInfo)))
					{
						if (pinInfo.dir == direction)
						{
							// Ignore the pin name
							if (!inPartialName)
							{
								pin->AddRef();
								foundPin = pin;
							}
							else
							{
								char pinName[128];
								::WideCharToMultiByte(CP_ACP, 0, pinInfo.achName, 
									-1,	pinName, 128, NULL, NULL);
								if (::strstr(pinName, inPartialName))
								{
									pin->AddRef();
									foundPin = pin;
								}
							}
						}
						pinInfo.pFilter->Release();
					}
					pin->Release();
				}
			}
			pinEnum->Release();
		}
	}
	
	if (foundPin)
	{
		foundPin->Release();
	}
	return foundPin;
}




