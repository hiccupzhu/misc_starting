#include <streams.h>
#include "H264Guid.h"
#include "H264Filter.h"
#include "H264Pin.h"

//#pragma comment(lib, "strmbasd.lib")
// #pragma comment(lib, "quartz.lib")
// #pragma comment(lib, "winmm.lib")

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,            // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
    { 
        L"Output",          // String pin name
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
    &CLSID_H264_FILTER,          // Filter CLSID
    L"H264 Filter",             // Filter name
    MERIT_DO_NOT_USE,         // Its merit
    1,                        // Number of pins
    psudPins                  // Pin details
};


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance
CFactoryTemplate g_Templates[] = 
{
    { 
        L"H264 Filter",
            &CLSID_H264_FILTER,
            CH264Filter::CreateInstance,
            NULL,
            &sudFilter 
    }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


CH264Filter::CH264Filter(LPUNKNOWN lpunk, HRESULT *phr) :
CSource(NAME("MH264Splitter"), lpunk, CLSID_H264_FILTER)
{
    CH264Pin* pnew = new CH264Pin(phr, this, L"video");
    if(pnew == NULL){
        *phr = E_OUTOFMEMORY;
    }
}

CH264Filter::~CH264Filter(void)
{
}

CUnknown* CH264Filter::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr){
    CUnknown* pnew = new CH264Filter(lpunk, phr);
    if(pnew == NULL){
        *phr = E_OUTOFMEMORY;
    }
    return pnew;
}





//////////////////////////////////////////////////////////////////////////
///////////////////////  COM's Interface  start  /////////////////////////

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );
}



//////////////////////////////////////////////////////////////////////////