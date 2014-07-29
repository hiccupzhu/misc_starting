#pragma once
#include <streams.h>
#include <stdio.h>
#include "matroska_reader_t.h"

class CFilterMKVSource;
class CMKVOutPin	: public CSourceStream
					, public CSourceSeeking
{

public:
	BOOL SetFileSource(const char * inFile);
	void GetFileSource(char * outFile);
	
	
public:
	CMKVOutPin(HRESULT *phr,CFilterMKVSource* pFilter,LPCWSTR pPinName);
	~CMKVOutPin(void);

public:
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (IID_IMediaSeeking == riid)
		{
			return CSourceSeeking::QueryInterface(riid, ppv);
		}
		else 
		{
			return CSourceStream::QueryInterface(riid,ppv);
		}
	};                                                          
	STDMETHODIMP_(ULONG) AddRef() 
	{                             
		return CSourceStream::AddRef();                            
	};                                                              
	STDMETHODIMP_(ULONG) Release() 
	{                            
		return CSourceStream::Release();                               
	};
	// Basic COM - used here to reveal our own interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);


	STDMETHODIMP QueryId(LPWSTR * Id);
	// Pure methods
	virtual HRESULT FillBuffer(IMediaSample * pSample);
	virtual HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
	virtual HRESULT CheckMediaType(const CMediaType * inMediatype);

	virtual HRESULT Active(void);
	HRESULT GetMediaType(int iPosition, CMediaType *pmt);

	// Quality control notifications sent to us
	STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

	// IMediaSeeking
	virtual HRESULT ChangeStart();
	virtual HRESULT ChangeStop();
	virtual HRESULT ChangeRate();
	virtual HRESULT OnThreadCreate(void);
	virtual HRESULT OnThreadStartPlay(void);
	void UpdateFromSeek(void);


private:
	CCritSec				mSharedState;
	source_access_callbacks_t mSource;
	char					mFilePath[MAX_PATH];
	REFERENCE_TIME			mLastSampleTime;
	CFilterMKVSource *		mFilter;
	CMediaType				mPreferred;
	matroska_reader_t		mMkv;
};
