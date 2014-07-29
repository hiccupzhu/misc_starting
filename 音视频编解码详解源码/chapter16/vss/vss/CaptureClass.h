
/////////////////////////////////////////////////////////////////////
// CaptureVideo.h : header file
/////////////////////////////////////////////////////////////////////
#ifndef _CAPCLASS_HEAD_
#define _CAPCLASS_HEAD_

#include <streams.h>

#ifndef srelease
#define srelease(x) \
if ( NULL != x ) \
{ \
  x->Release( ); \
  x = NULL; \
}
#endif


class CCaptureClass
{

public:
    CCaptureClass();
    virtual ~CCaptureClass();
	
	int EnumDevices(HWND hList);

	void SaveGraph(TCHAR *wFileName);
	void ConfigCameraPin(HWND hwndParent);
	void ConfigCameraFilter(HWND hwndParent);
	HRESULT CaptureImages(CString inFileName);
	HRESULT PreviewImages(int iDeviceID, HWND hWnd);

private:

    HWND m_hWnd;
    IGraphBuilder			*m_pGB;
    ICaptureGraphBuilder2	*m_pCapture;
    IBaseFilter				*m_pBF;
    IMediaControl			*m_pMC;
    IVideoWindow			*m_pVW;
	IBaseFilter				*pMux;
	
protected:

	bool BindFilter(int deviceId, IBaseFilter **pFilter);
	void ResizeVideoWindow();
	HRESULT SetupVideoWindow();
	HRESULT InitCaptureGraphBuilder();
};

#endif

