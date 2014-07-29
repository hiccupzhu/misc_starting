#include "H264Pin.h"
#include "H264Filter.h"
#include "H264Guid.h"

CH264Pin::CH264Pin(HRESULT *phr, CH264Filter * pFilter, LPCWSTR pPinName):
CSourceStream(NAME("h264pin_obj"), phr, pFilter, pPinName)
{
    m_mediaType.SetType(&MEDIATYPE_Video);
    m_mediaType.SetSubtype(&MEDIATYPE_AVC1);
    m_mediaType.SetFormatType(&FORMAT_MPEG2_VIDEO);
    m_mediaType.bFixedSizeSamples = false;
    m_mediaType.bTemporalCompression = false;
    
}

CH264Pin::~CH264Pin(void)
{
}

// STDMETHODIMP CH264Pin::QueryId(LPWSTR * Id)
// {
//     return CBaseOutputPin::QueryId(Id);
// }
// 
// STDMETHODIMP CH264Pin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
// {
//     CheckPointer(ppv,E_POINTER);
// 
//     if (riid == IID_IMediaSeeking) 
//     {
//         return GetInterface((IMediaSeeking *) this , ppv);
//     }
//     else
//     {
//         return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
//     }
// }

HRESULT CH264Pin::FillBuffer(IMediaSample *pSamp){
    return S_OK;
}

HRESULT CH264Pin::DecideBufferSize(IMemAllocator *pAlloc, 
                                    ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock  filterLock(m_pFilter->pStateLock());
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cbBuffer = 1;
    pProperties->cBuffers = 0x100000;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties, &Actual);
    if (FAILED(hr)) {
        return hr;
    }
    // Is this allocator unsuitable
    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }
    ASSERT( Actual.cBuffers == 1 );

    return NOERROR;
}
// 
// HRESULT CH264Pin::Active(void)
// {
//     return CSourceStream::Active();
// }
// 
HRESULT CH264Pin::CheckMediaType(const CMediaType * inMediatype)
{
    HRESULT res = E_FAIL;
    CAutoLock  lockit(m_pFilter->pStateLock());

    if(inMediatype->Subtype() == m_mediaType.Subtype()){
        res = S_OK;
    }
    return res;
}

HRESULT CH264Pin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock  lockit(m_pFilter->pStateLock());

    if (iPosition != 0)
    {
        return E_FAIL;
    }

    *pmt = m_mediaType;
    return NOERROR;
}

STDMETHODIMP CH264Pin::Notify(IBaseFilter * pSender, Quality q)
{
    
    return NOERROR;
}

// HRESULT CH264Pin::ChangeStart()
// {
//     return NOERROR;
// }
// 
// HRESULT CH264Pin::ChangeStop()
// {
//     return NOERROR;
// }
// 
// HRESULT CH264Pin::ChangeRate()
// {
//     return NOERROR;
// }
// 
// HRESULT CH264Pin::OnThreadCreate(void)
// {
//     return NOERROR;
// }
// 
// HRESULT CH264Pin::OnThreadStartPlay(void)
// {
//     return NOERROR;
// }