#pragma once
#include <streams.h>

class CH264Filter;
class CH264Pin : public CSourceStream
{
public:
    CH264Pin(HRESULT *phr, CH264Filter * pFilter, LPCWSTR pPinName);
    ~CH264Pin(void);
    
    // Basic COM - used here to reveal our own interfaces
//     STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
// 
// 
//     STDMETHODIMP QueryId(LPWSTR * Id);
    // Pure methods
    virtual HRESULT FillBuffer(IMediaSample * pSample);
    virtual HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
    virtual HRESULT CheckMediaType(const CMediaType * inMediatype);

//    virtual HRESULT Active(void);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    // Quality control notifications sent to us
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    // IMediaSeeking
//     virtual HRESULT ChangeStart();
//     virtual HRESULT ChangeStop();
//     virtual HRESULT ChangeRate();
//     virtual HRESULT OnThreadCreate(void);
//     virtual HRESULT OnThreadStartPlay(void);;


private:
    CMediaType      m_mediaType;

};
