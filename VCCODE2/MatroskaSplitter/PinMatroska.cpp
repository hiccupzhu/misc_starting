#include <streams.h>
#include "FilterMatroska.h"
#include "PinMatroska.h"

CPinMatroska::CPinMatroska(HRESULT *phr, CFilterMatroska * pFilter, LPCWSTR pPinName):
CSourceStream(NAME("matroska_stream"), phr, pFilter, pPinName),
CSourceSeeking("matroska_seek", NULL, phr, pFilter->pStateLock())
{
}

CPinMatroska::~CPinMatroska(void)
{
}

STDMETHODIMP CPinMatroska::QueryId(LPWSTR * Id)
{
    return CBaseOutputPin::QueryId(Id);
}

STDMETHODIMP CPinMatroska::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
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

HRESULT CPinMatroska::FillBuffer(IMediaSample * pSample)
{
    return S_OK;
}

HRESULT CPinMatroska::DecideBufferSize(IMemAllocator *pAlloc, 
                                    ALLOCATOR_PROPERTIES *pProperties)
{
    return NOERROR;
}

HRESULT CPinMatroska::Active(void)
{
    return CSourceStream::Active();
}

HRESULT CPinMatroska::CheckMediaType(const CMediaType * inMediatype)
{
    return E_FAIL;
}

HRESULT CPinMatroska::GetMediaType(int iPosition, CMediaType *pmt)
{
    return NOERROR;
}

// Quality control
STDMETHODIMP CPinMatroska::Notify(IBaseFilter * pSender, Quality q)
{
    return NOERROR;
}

HRESULT CPinMatroska::ChangeStart()
{
    
    return NOERROR;
}

HRESULT CPinMatroska::ChangeStop()
{
    return NOERROR;
}

HRESULT CPinMatroska::ChangeRate()
{
    return NOERROR;
}

HRESULT CPinMatroska::OnThreadCreate(void)
{
    return NOERROR;
}

HRESULT CPinMatroska::OnThreadStartPlay(void)
{
    return NOERROR;
}