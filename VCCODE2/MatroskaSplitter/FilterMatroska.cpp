#include <streams.h>          // quartz, includes windows
// Eliminate two expected level 4 warnings from the Microsoft compiler.
// The class does not have an assignment or copy operator, and so cannot
// be passed by value.  This is normal.  This file compiles clean at the
// highest (most picky) warning level (-W4).
#pragma warning(disable: 4511 4512)

#include <measure.h>          // performance measurement (MSR_)
#include <initguid.h>

#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

#include "MatroskaGuid.h"
#include "FilterMatroska.h"

#pragma comment(lib,"strmbasd.lib")
#pragma comment(lib,"msvcrtd.lib")
#pragma comment(lib,"quartz.lib")
#pragma comment(lib,"vfw32.lib") 
#pragma comment(lib,"winmm.lib") 
#pragma comment(lib,"version.lib") 
#pragma comment(lib,"comctl32.lib") 
#pragma comment(lib,"olepro32.lib")


const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,            // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
    { 
        L"Video",          // String pin name
            FALSE,              // Is it rendered
            TRUE,               // Is it an output
            FALSE,              // Allowed none
            FALSE,              // Allowed many
            &CLSID_NULL,        // Connects to filter
            L"Input",           // Connects to pin
            1,                  // Number of types
            &sudPinTypes        // The pin details
    }
};


const AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_MATROSKA_FILTER,          // Filter CLSID
    L"MatroskaFilter",             // Filter name
    MERIT_DO_NOT_USE,         // Its merit
    1,                        // Number of pins
    psudPins                  // Pin details
};

CFactoryTemplate g_Templates[] = 
{
    { 
        L"Matroska Demux",
            &CLSID_MATROSKA_FILTER,
            CFilterMatroska::CreateInstance,
            NULL,
            &sudFilter 
    }    
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


#include "PinMatroska.h"

CFilterMatroska::CFilterMatroska(LPUNKNOWN lpunk, HRESULT *phr):
CSource(NAME("Matroska Filter"), lpunk, CLSID_MATROSKA_FILTER)
{
    CPinMatroska* pin = new CPinMatroska(phr, this, L"Video");
    if(pin == NULL){
        *phr = E_OUTOFMEMORY;
    }
}

CFilterMatroska::~CFilterMatroska(void)
{
}

CUnknown * WINAPI CFilterMatroska::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr){
    CUnknown* pukn = new CFilterMatroska(lpunk, phr);
    if(pukn == NULL){
        *phr = E_OUTOFMEMORY;
    }
    return pukn;
}

HRESULT STDMETHODCALLTYPE CFilterMatroska::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt){
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CFilterMatroska::GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt){
    return S_OK;
}

int CFilterMatroska::GetPinCount(void){
    return 1;
}

CBasePin *CFilterMatroska::GetPin(int n){
    if(n == 0){
        return m_paStreams[0];
    }
    return NULL;
}





//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

STDAPI DllRegisterServer(){
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer(){
    return AMovieDllRegisterServer2( FALSE );
}

#pragma warning( disable:4514)