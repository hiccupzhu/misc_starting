#include <streams.h>
#include "FilterMKVSource.h"

//#pragma warning(disable: 4511 4512)

#include <measure.h>          // performance measurement (MSR_)
#include <initguid.h>
#include "MKVSourceGuids.h"

#pragma comment(lib,"strmbasd.lib")
#pragma comment(lib,"winmm.lib")

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_NULL,            // Major type
	&CLSID_AVC          // Minor type
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
	&CLSID_MKVSource,          // Filter CLSID
	L"MKV Source",             // Filter name
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
		L"MKV Source",
			&CLSID_MKVSource,
			CFilterMKVSource::CreateInstance,
			NULL,
			&sudFilter 
	}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);






CFilterMKVSource::CFilterMKVSource(LPUNKNOWN lpunk, HRESULT *phr) : 
CSource(NAME("MKV Source"), lpunk, CLSID_MKVSource)
{
	CMKVOutPin* outStream = new CMKVOutPin(phr,this,L"OutPut");
	if(outStream == NULL)
	{
		*phr = E_OUTOFMEMORY;
	}
}

CFilterMKVSource::~CFilterMKVSource(void)
{
}

CUnknown* WINAPI CFilterMKVSource::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	CFilterMKVSource *pNewObject = new CFilterMKVSource(lpunk,phr);
	if(pNewObject == NULL){
		*phr = E_OUTOFMEMORY;
	}
	return pNewObject;
}


STDMETHODIMP CFilterMKVSource::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv, E_POINTER);

    if (riid == IID_IFileSourceFilter)
    {
        return GetInterface((IFileSourceFilter *) this, ppv);
    }
    else if (riid == IID_IMKVSource)
    {
        return GetInterface((IMKVSource *) this, ppv);
    }
    else
    {
        return CSource::NonDelegatingQueryInterface(riid, ppv);
    }
}
int CFilterMKVSource::GetPinCount()
{
	return 1;
}
CBasePin *CFilterMKVSource::GetPin(int n)
{
	if( n == 0 ){
		return m_paStreams[0];
	}
	return NULL;
}

// --- IFileSourceFilter methods ---
STDMETHODIMP CFilterMKVSource::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt)
{
	char szAnsi[MAX_PATH];
	WideCharToMultiByte(CP_ACP,0,pszFileName,-1,szAnsi,MAX_PATH,NULL,NULL);
	if(OutPin()->SetFileSource(szAnsi))
	{
		return NOERROR;
	}
	return E_FAIL;
}
STDMETHODIMP CFilterMKVSource::GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt)
{
    char szAnsi[MAX_PATH];
    OutPin()->GetFileSource(szAnsi);

    DWORD n = sizeof(WCHAR) * (1 + strlen(szAnsi));
    *ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
    if (*ppszFileName != NULL) 
    {
        WCHAR   szwFile[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, szwFile, MAX_PATH);
        CopyMemory(*ppszFileName, szwFile, n);
    }
    return NOERROR;
}

// --- IQQSource methods ---
STDMETHODIMP CFilterMKVSource::CanChangeSource(void)
{
    if (IsStopped() && OutPin()->IsConnected() == FALSE)
    {
        return S_OK;
    }
    return S_FALSE;
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

/******************************Public Routine******************************\
* exported entry points for registration and
* unregistration (in this case they only call
* through to default implmentations).
*
*
*
* History:
*
\**************************************************************************/
STDAPI DllRegisterServer()
{
	// Register the ".QQ" file extension
	DWORD	dwDisposition;
	DWORD	dwReserved = 0;
	HKEY  	hTempKey   = (HKEY)0;
	TCHAR   szKey[]    = "Media Type\\Extensions\\.MKV1";
	TCHAR   szValue[]  = "Source Filter";
	TCHAR   szData[100];
	
	WCHAR * pClsid = NULL;
	StringFromCLSID(CLSID_MKVSource, &pClsid);
	WideCharToMultiByte(CP_ACP, 0, pClsid, -1, szData, 100, NULL, NULL);
	CoTaskMemFree(pClsid);
	
	DWORD	dwBufferLength = lstrlen(szData) * sizeof(TCHAR);
	if (ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_CLASSES_ROOT, szKey, dwReserved,
		(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,
		&hTempKey, &dwDisposition))
	{	
		// dwBufferLength must include size of terminating nul 
		// character when using REG_SZ with RegSetValueEx function
		dwBufferLength += sizeof(TCHAR);	
		::RegSetValueEx(hTempKey, (LPTSTR)szValue, dwReserved, REG_SZ, 
			(LPBYTE)szData, dwBufferLength);
		::RegCloseKey(hTempKey);
	}
	
	return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
	// Remove ".MKV" file extension registry
	HKEY   hKey = NULL;
	DWORD  dw   = 0;
	char  szKey[] = "Media Type\\Extensions";
	if (ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_CLASSES_ROOT, szKey, 0L, 
		NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw))
	{
		EliminateSubKey(hKey, ".MKV");
		::RegCloseKey(hKey);
	}	

	return AMovieDllRegisterServer2( FALSE );
}
