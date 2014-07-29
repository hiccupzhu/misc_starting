#ifndef __PINMATROSKA_H__
#define __PINMATROSKA_H__

class CFilterMatroska;
class CPinMatroska : public CSourceStream
                   , public CSourceSeeking
{
public:
    CPinMatroska(HRESULT *phr, CFilterMatroska * pFilter, LPCWSTR pPinName);
    ~CPinMatroska(void);

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
};


#endif