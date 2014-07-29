#pragma once
#include "MKVOutPin.h"
#include "IMKVSource.h"

class CFilterMKVSource	: public CSource
						, public IFileSourceFilter
						, public IMKVSource
{
public:
	CFilterMKVSource(LPUNKNOWN lpunk, HRESULT *phr);
	~CFilterMKVSource(void);
	
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
	DECLARE_IUNKNOWN;
	
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	virtual int GetPinCount();
	virtual CBasePin *GetPin(int n);
	
	// --- IFileSourceFilter methods ---
	STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt);	

	// --- IQQSource methods ---
	STDMETHODIMP CanChangeSource(void);
	
protected:
	CMKVOutPin * OutPin() {return (CMKVOutPin *)m_paStreams[0];}
};
