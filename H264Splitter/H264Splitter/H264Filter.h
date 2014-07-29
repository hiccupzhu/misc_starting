#pragma once

#include <streams.h>

class CH264Filter : public CSource
{
public:
    CH264Filter(LPUNKNOWN lpunk, HRESULT *phr);
    ~CH264Filter(void);

public:
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
};
