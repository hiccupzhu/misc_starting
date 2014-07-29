
/////////////////////////////////////////////////////////////////////
// CaptureVideo.h : header file
/////////////////////////////////////////////////////////////////////
#ifndef _CAPCLASS_HEAD_
#define _CAPCLASS_HEAD_



#ifndef srelease
#define srelease(x) \
if ( NULL != x ) \
{ \
  x->Release( ); \
  x = NULL; \
}
#endif

class 	CSmartTee;
class 	CAVIDecompressor0001;
class 	CHQGrabber;
class 	CNullRenderer;
class 	CAVIDecompressor;
class 	CVideoRenderer;

class CCaptureClass
{

public:
    CCaptureClass();
    virtual ~CCaptureClass();
	
	int EnumDevices(HWND hList);

	void SaveGraph(TCHAR *wFileName);
	void ConfigCameraPin(HWND hwndParent);
	void ConfigCameraFilter(HWND hwndParent);
	HRESULT CaptureImages(CString inFileName,HDC hdc,RECT rc);
	HRESULT PreviewImages(int iDeviceID, HWND hWnd);

private:

    HWND m_hWnd;
    IGraphBuilder			*m_pGraph;
    ICaptureGraphBuilder2	*m_pCapture;
    IBaseFilter				*m_pBF;
    IMediaControl			*m_pMC;
    IVideoWindow			*m_pVW;
	IBaseFilter				*pMux;

private:
	CSmartTee				*m_pSmartTee;
	CHQGrabber				*m_pHQGrabber;
	CNullRenderer			*m_pNullRenderer;
	CAVIDecompressor		*m_pAVIDecompressor;
	CVideoRenderer			*m_pVideoRenderer;
	
protected:
	FILE *fp;
	bool BindFilter(int deviceId, IBaseFilter **pFilter);
	void ResizeVideoWindow();
	HRESULT SetupVideoWindow();
	HRESULT InitCaptureGraphBuilder();
	void NukeDownstream(IBaseFilter * inFilter);
	void Create();
	IPin * FindPin(IBaseFilter *inFilter, char *inFilterName);
public:
	void convert_yuv420_rgb(unsigned char * src, unsigned char * dst,
							int width, int height, int flipUV, int ColSpace);
};

#endif

