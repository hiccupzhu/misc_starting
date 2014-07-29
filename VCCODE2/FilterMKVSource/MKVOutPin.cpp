#include <stdio.h>
#include "FilterMKVSource.h"
#include "MKVOutPin.h"


size_t url_read(void *handle, void *buf, size_t size, int full_read)
{
	return fread(buf,1,size,(FILE*)handle);
}

int64_t url_seek(void *handle, int64_t offset, int origin)
{
	return fseek((FILE*)handle,offset,origin);
}

int64_t url_tell(void *handle)
{
	return ftell((FILE*)handle);
}





CMKVOutPin::CMKVOutPin(HRESULT *phr,CFilterMKVSource* pFilter,LPCWSTR pPinName) : 
CSourceStream(NAME("MKV Stream"), phr, pFilter, pPinName),
CSourceSeeking("MediaSeeking", NULL , phr , pFilter->pStateLock())
{
	mFilter = pFilter;
	memset(&mSource,0,sizeof(source_access_callbacks_t));
}

CMKVOutPin::~CMKVOutPin(void)
{
	CAutoLock lock(&mSharedState);
	if(mSource.m_pHandle){
		fclose((FILE*)mSource.m_pHandle);
		mSource.m_pHandle = NULL;
	}
}



BOOL CMKVOutPin::SetFileSource(const char * inFile)
{
	CAutoLock lock(&mSharedState);
	strcpy(mFilePath,inFile);
	if(mSource.m_pHandle){
		fclose((FILE*)mSource.m_pHandle);
		mSource.m_pHandle = NULL;
	}
	
	mSource.m_pHandle = fopen(mFilePath,"rb");
	mSource.m_Read = url_read;
	mSource.m_Seek = url_seek;
	mSource.m_Tell = url_tell;
	
	mMkv.load(&mSource);
	mPreferred.SetType(&MEDIATYPE_Video);
	mPreferred.SetFormatType(&FORMAT_VideoInfo);
	mPreferred.SetSubtype(&MEDIASUBTYPE_MPEG2_VIDEO);
	mPreferred.SetTemporalCompression(true);

	return TRUE;
}
void CMKVOutPin::GetFileSource(char * outFile)
{
	strcpy(outFile,mFilePath);
}



// Basic COM - used here to reveal our own interfaces
STDMETHODIMP CMKVOutPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv,E_POINTER);

	if (riid == IID_IMediaSeeking) 
	{
		return GetInterface((IMediaSeeking *) this , ppv);
	}
	else
	{
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
	}
}


STDMETHODIMP CMKVOutPin::QueryId(LPWSTR * Id)
{
	return CBaseOutputPin::QueryId(Id);
}
// Pure methods
HRESULT CMKVOutPin::FillBuffer(IMediaSample * pSample)
{
	CAutoLock lock(&mSharedState);
	
	uint8_t* pData = NULL;
	pSample->GetPointer(&pData);
	
	return S_OK;
}

HRESULT CMKVOutPin::DecideBufferSize(
				IMemAllocator *pAlloc, 
				ALLOCATOR_PROPERTIES *pProperties)
{
	CAutoLock  filterLock(m_pFilter->pStateLock());
	ASSERT(pAlloc);
	ASSERT(pProperties);
	HRESULT hr = NOERROR;

	VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
	pProperties->cBuffers = 1;
	pProperties->cbBuffer = 1024 * 1024 * 1;

	ASSERT(pProperties->cbBuffer);

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pProperties, &Actual);
	if (FAILED(hr)) {
		return hr;
	}

	if (Actual.cbBuffer < pProperties->cbBuffer) {
		return E_FAIL;
	}
	
	ASSERT( Actual.cBuffers == 1 );

	return NOERROR;
}
HRESULT CMKVOutPin::CheckMediaType(const CMediaType * inMediatype)
{
	CAutoLock lockit(m_pFilter->pStateLock());
	if(mPreferred == *inMediatype){
        return S_OK;
	}
	
	return E_FAIL;
}

HRESULT CMKVOutPin::Active(void)
{
	return CSourceStream::Active();
}
HRESULT CMKVOutPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock lockit(m_pFilter->pStateLock());
    
    if(iPosition != 0)
    {
        return E_FAIL;
    }
    *pmt = mPreferred;
    return NOERROR;
}

// Quality control notifications sent to us
STDMETHODIMP CMKVOutPin::Notify(IBaseFilter * pSender, Quality q)
{
    if(q.Late > 0){
        //do something
    }else{
        //do something
    }
    return NOERROR;
}

// IMediaSeeking
HRESULT CMKVOutPin::ChangeStart()
{
	UpdateFromSeek();
	return NOERROR;
}
HRESULT CMKVOutPin::ChangeStop()
{
	return NOERROR;
}
HRESULT CMKVOutPin::ChangeRate()
{
	return NOERROR;
}
HRESULT CMKVOutPin::OnThreadCreate(void)
{
	CAutoLock   lock(&mSharedState);
	mLastSampleTime = 0;
	return NOERROR;
}
HRESULT CMKVOutPin::OnThreadStartPlay(void)
{
	return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}
void CMKVOutPin::UpdateFromSeek(void)
{
	if (ThreadExists()) 
	{
		DeliverBeginFlush();
		Stop();
		DeliverEndFlush();
		Run();
	}
}
