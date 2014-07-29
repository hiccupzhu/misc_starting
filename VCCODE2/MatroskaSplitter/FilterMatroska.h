#ifndef __FILTERMATROSKA_H__
#define __FILTERMATROSKA_H__


class CFilterMatroska : public CSource,
                        public IFileSourceFilter
{
public:
    CFilterMatroska(LPUNKNOWN lpunk, HRESULT *phr);
    ~CFilterMatroska(void);

    DECLARE_IUNKNOWN;

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    virtual HRESULT STDMETHODCALLTYPE Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt);
    virtual HRESULT STDMETHODCALLTYPE GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt);

    int       GetPinCount(void);
    CBasePin *GetPin(int n);
};


#endif