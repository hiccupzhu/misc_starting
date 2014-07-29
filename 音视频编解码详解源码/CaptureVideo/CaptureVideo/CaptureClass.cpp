//-------------------------------------------------------------------
// CCaptureClass视频捕捉类实现文件CaptureVideo.cpp
//-------------------------------------------------------------------
// CaptureVideo.cpp: implementation of the CCaptureClass class.
//
/////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CaptureClass.h"
#include "SmartTee.h"
#include "HQGrabber.h"
#include "NullRenderer.h"
#include "AVIDecompressor.h"
#include "VideoRenderer.h"
#include "Resource.h"

#include "stdint.h"
#include "x264.h"
#include "avcodec.h"

#pragma comment(lib,"..\\..\\chapter15\\x264-060308\\build\\win32\\bin\\libx264")
#pragma comment(lib,"..\\..\\chapter15\\ffmpeg_h264_vc\\Debug\\H264Dec.lib")
#pragma comment(lib,"cscc")

class ColorSpaceConversions {

public:

	ColorSpaceConversions();

	void RGB24_to_YV12(unsigned char * in, unsigned char * out,int w, int h);
	void YV12_to_RGB24(unsigned char *src0,unsigned char *src1,unsigned char *src2,unsigned char *dst_ori,int width,int height);
	void YVU9_to_YV12(unsigned char * in,unsigned char * out, int w, int h);
	void YUY2_to_YV12(unsigned char * in,unsigned char * out, int w, int h);
	void YV12_to_YVU9(unsigned char * in,unsigned char * out, int w, int h);
	void YV12_to_YUY2(unsigned char * in,unsigned char * out, int w, int h);
};

#define CLIP(color) (unsigned char)((color>0xFF)?0xff:((color<0)?0:color))


extern "C" void		x264_param_default( x264_param_t * );
extern "C" int		x264_nal_encode( void *, int *, int b_annexeb, x264_nal_t *nal );
extern "C" int		x264_nal_decode( x264_nal_t *nal, void *, int );
extern "C" x264_t*	x264_encoder_open   ( x264_param_t * );
extern "C" int		x264_encoder_reconfig( x264_t *, x264_param_t * );
extern "C" int		x264_encoder_headers( x264_t *, x264_nal_t **, int * );
extern "C" int		x264_encoder_encode ( x264_t *, x264_nal_t **, int *, x264_picture_t *, x264_picture_t * );
extern "C" void		x264_encoder_close  ( x264_t * );
extern "C" void		x264_picture_alloc( x264_picture_t *pic, int i_csp, int i_width, int i_height );
extern "C" void		x264_picture_clean( x264_picture_t *pic );


void avcodec_init(void);
void avcodec_register_all(void);
AVCodec *avcodec_find_decoder(enum CodecID id);
AVCodecContext *avcodec_alloc_context(void);

int avcodec_open(AVCodecContext *avctx, AVCodec *codec);
AVFrame *avcodec_alloc_frame(void);
int avcodec_decode_video(AVCodecContext *avctx, AVFrame *picture, 
						 int *got_picture_ptr,
						 uint8_t *buf, int buf_size);
int avcodec_close(AVCodecContext *avctx);
void av_free(void *ptr);

FILE *outf;


#ifndef safedel
#define safedel(x) \
	if((x)) \
	{ \
	delete (x);\
	(x) = NULL;\
	}
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


x264_param_t param;

x264_t *h=NULL;
x264_picture_t pic;
AVCodec *codec;			  // Codec
AVCodecContext *c;		  // Codec Context
AVFrame *picture;		  // Frame




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCaptureClass::CCaptureClass()
{
	// COM 库初始化
	CoInitialize(NULL);
	m_hWnd = NULL;
	m_pVW = NULL;
	m_pMC = NULL;
	m_pGraph = NULL;
	m_pBF = NULL;
	m_pCapture = NULL; 

	m_pSmartTee=NULL;
	m_pHQGrabber=NULL;
	m_pNullRenderer=NULL;
	m_pAVIDecompressor=NULL;
	m_pVideoRenderer=NULL;

	x264_param_default(&param);
	param.i_width=640;
	param.i_height=480;
	h=x264_encoder_open(&param);
	x264_picture_alloc(&pic,X264_CSP_I420,param.i_width,param.i_height);


	fp=fopen("c:\\demo.264","wb");
	outf=fopen("c:\\demo.yuv","wb");

	avcodec_init(); 
	avcodec_register_all(); 
	codec = avcodec_find_decoder(CODEC_ID_H264);

	c = avcodec_alloc_context(); 
	avcodec_open(c, codec);
	picture   = avcodec_alloc_frame();
}
CCaptureClass::~CCaptureClass()
{
	//
	if(m_pMC)
		m_pMC->Stop();
	if(m_pVW)
	{
		m_pVW->put_Visible(OAFALSE);
		m_pVW->put_Owner(NULL);
	}

	safedel(m_pVideoRenderer);
	safedel(m_pAVIDecompressor);
	safedel(m_pNullRenderer);
	safedel(m_pHQGrabber);
	safedel(m_pSmartTee);

	srelease(m_pCapture);
	srelease(m_pMC);
	srelease(m_pGraph);
	srelease(m_pBF);

	fclose(fp);
	fclose(outf);

	CoUninitialize();
}

/*
系统设备枚举器为我们按类型枚举已注册在系统中的Fitler提供了统一的方法。而且它能够区分不同的硬件设备，即便是同一个Filter支持它们。这对那些使用Windows驱动模型和KSProxy Filter的设备来说是非常有用的。系统设备枚举器对它们按不同的设备实例进行对待（译注：虽然它们支持相同Filter）。
　　当我们利用系统设备枚举器查询设备的时候，系统设备枚举器为特定类型的设备（如，音频捕获和视频压缩）生成了一张枚举表（Enumerator）。类型枚举器（Category enumerator）为每个这种类型的设备返回一个Moniker，类型枚举器自动把每一种即插即用的设备包含在内。
　　按如下的步骤使用系统设备枚举器：
　　1． 调用方法CoCreateInstance生成系统设备枚举器。类标识（CLSID）为CLSID_SystemDeviceEnum。
　　2． 调用ICreateDevEnum::CreateClassEnumerator方法生成类型枚举器，参数为你想要得到的类型的CLSID，该方法返回一个IEnumMoniker接口指针，如果指定的类型（是空的）或不存在，函数ICreateDevEnum::CreateClassEnumerator将返回S_FALSE而不是错误代码，同时IEnumMoniker指针（译注：通过参数返回）也是空的，这就要求我们在调用CreateClassEnumerator的时候明确用S_OK进行比较而不是使用宏SUCCEEDED。
　　3． 使用IEnumMoniker::Next方法依次得到IEnumMoniker指针中的每个moniker。该方法返回一个IMoniker接口指针。当Next到达枚举的底部，它的返回值仍然是S_FALSE，这里我们仍需要用S_OK来进行检验。
　　4． 想要得到该设备较为友好的名称（例如想要在用户界面中进行显示），调用IMoniker::BindToStorage方法。
　　5． 如果想要生成并初始化管理该设备的Filter调用3返回指针的IMonitor::BindToObject方法，接下来调用IFilterGraph::AddFilter把该Filter添加到视图中。
*/

int CCaptureClass::EnumDevices(HWND hList)
{
	if (!hList)	return -1;
	int id = 0;

	//枚举捕获设备
	ICreateDevEnum *pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
						CLSCTX_INPROC_SERVER,
						IID_ICreateDevEnum,
						(void**)&pCreateDevEnum);

	if (hr != NOERROR) 	return -1;
	
	IEnumMoniker *pEm;
	//创建视频类型枚举器
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
	//创建音频捕获类
	//hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,&pEm, 0);

	if (hr != NOERROR) return -1;
	//类型枚举器复位
	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		IPropertyBag *pBag;
		//设备属性页
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				id++;
				::SendMessage(hList, CB_ADDSTRING, 0,(LPARAM)var.bstrVal);
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
	}
	return id;
}

void CCaptureClass::Create()
{
	m_pSmartTee=new CSmartTee(m_pGraph);
	m_pHQGrabber=new CHQGrabber(m_pGraph);
	m_pNullRenderer=new CNullRenderer(m_pGraph);
	m_pAVIDecompressor=new CAVIDecompressor(m_pGraph);
	m_pVideoRenderer=new CVideoRenderer(m_pGraph);

	m_pSmartTee->CreateFilter();
	m_pHQGrabber->CreateFilter();
	m_pNullRenderer->CreateFilter();
	m_pAVIDecompressor->CreateFilter();
	m_pVideoRenderer->CreateFilter();
}

/*开始预览视频数据*/
HRESULT CCaptureClass::PreviewImages(int iDeviceID, HWND hWnd)
{
	HRESULT hr;
	
	// 初始化视频捕获滤波器链表管理器
	hr = InitCaptureGraphBuilder();
	if (FAILED(hr))
	{
		AfxMessageBox(_T("Failed to get video interfaces!"));
		return hr;
	}

	// 把指定采集设备与滤波器捆绑
	if(!BindFilter(iDeviceID, &m_pBF))
		return S_FALSE;
	// 把滤波器添加到滤波器链表中
	hr = m_pGraph->AddFilter(m_pBF, L"Capture Filter");
	
	// 渲染媒体，把链表中滤波器连接起来
    hr = m_pCapture->RenderStream( &PIN_CATEGORY_PREVIEW, 
									0, 
									m_pBF, 
									NULL, 
									NULL );

	if( FAILED( hr ) )
	{
		AfxMessageBox(_T("Can’t build the graph"));
		return hr;
	}

	NukeDownstream(m_pBF);

	Create();

	IPin *pPin=FindPin(m_pBF,"捕获");

	IPin * pInputPinU1=m_pSmartTee->GetInputPin();
	pPin->Connect(pInputPinU1,0);

	m_pSmartTee->GetPreviewPin()->Connect(m_pAVIDecompressor->GetInputPin(),0);
	m_pAVIDecompressor->GetOutputPin()->Connect(m_pHQGrabber->GetInputPin(),0);
	m_pHQGrabber->GetOutputPin()->Connect(m_pVideoRenderer->GetInputPin(),0);



	//设置视频显示窗口
	m_hWnd = hWnd; 
	SetupVideoWindow();
	//test for config
	hr = m_pMC->Run();
	if(FAILED(hr))
	{
		AfxMessageBox(_T("Couldn’t run the graph!"));
		return hr;
	}
	return S_OK;
}


IPin * CCaptureClass::FindPin(IBaseFilter *inFilter, char *inFilterName)
{
	HRESULT hr;
	IPin *pin=NULL;
	IPin * pFoundPin=NULL;
	IEnumPins *pEnumPins=NULL;
	hr=inFilter->EnumPins(&pEnumPins);
	if(FAILED(hr))
		return pFoundPin;
	ULONG fetched=0;
	while(SUCCEEDED(pEnumPins->Next(1,&pin,&fetched)) && fetched)
	{
		PIN_INFO pinfo;
		pin->QueryPinInfo(&pinfo);
		pinfo.pFilter->Release();
		CString str(pinfo.achName);
		if(str==CString(inFilterName))
		{
			pFoundPin=pin;
			pin->Release();
			break;
		}
		pin->Release();
	}
	pEnumPins->Release();
	return pFoundPin;
}

void CCaptureClass::NukeDownstream(IBaseFilter * inFilter)
{
	if(!m_pGraph || !inFilter)
		return ;
	IEnumPins *pPinEnum=NULL;

	if(SUCCEEDED(inFilter->EnumPins(&pPinEnum)))
	{
		pPinEnum->Reset();

		IPin * pin=NULL;
		unsigned long ulFetched=0;

		while(SUCCEEDED(pPinEnum->Next(1,&pin,&ulFetched)) && ulFetched)
		{
			if(pin)
			{
				IPin * pConnectedPin=NULL;
				pin->ConnectedTo(&pConnectedPin);
				if(pConnectedPin)
				{
					PIN_INFO pininfo;
					if(SUCCEEDED(pConnectedPin->QueryPinInfo(&pininfo)))
					{
						pininfo.pFilter->Release();
						if(pininfo.dir==PINDIR_INPUT)
						{
							NukeDownstream(pininfo.pFilter);
							m_pGraph->Disconnect(pConnectedPin);
							m_pGraph->Disconnect(pin);
							m_pGraph->RemoveFilter(pininfo.pFilter);
						}
					}
					pConnectedPin->Release();
				}
				pin->Release();
			}
		}
		pPinEnum->Release();
	}
}


uint8_t temp;
/*设置捕获视频的文件，开始捕捉视频数据写文件*/
HRESULT CCaptureClass::CaptureImages(CString inFileName,HDC hdc,RECT rc)
{
	HRESULT hr=0;

	BITMAPINFOHEADER bitInfoHeader;
	m_pHQGrabber->GetBitmapInfoHead(&bitInfoHeader);

	int nWidth=bitInfoHeader.biWidth;
	int nHeight=bitInfoHeader.biHeight;
	int nBiCount=bitInfoHeader.biBitCount;

	BYTE * pImageRGB32=new BYTE[nWidth*nHeight*4];
	BYTE * pImageRGB24=new BYTE[nWidth*nHeight*3];

	m_pHQGrabber->Snapshot(pImageRGB32,TRUE);

	for(int i=0;i<nHeight;i++)
	{
		for(int j=0;j<nWidth;j++)
		{
			LPBYTE pim=pImageRGB24+(i*nWidth+j)*3;
			LPBYTE pim2=pImageRGB32+(i*nWidth+j)*nBiCount/8;
			memcpy(pim,pim2,3);
		}
	}

	BYTE upreColor=0;
	BYTE vpreColor=0;

	BYTE *py=pic.img.plane[0];
	BYTE *pu=pic.img.plane[1];
	BYTE *pv=pic.img.plane[2];
	for(int i=nHeight-1;i>=0;i--)
	{
		for(int j=0;j<nWidth;j++)
		{
			BYTE *rColor=pImageRGB32 + i*nWidth*nBiCount/8 + j*nBiCount/8 + 0;
			BYTE *gColor=rColor+1;
			BYTE *bColor=rColor+2;

			BYTE yColor=(BYTE)(*rColor*0.299 + *gColor*0.587 + *bColor*0.114+16);
			BYTE uColor=(BYTE)(*rColor*0.168 - *gColor*0.331 + *bColor*0.500+128);
			BYTE vColor=(BYTE)(*rColor*0.500 - *gColor*0.4187 - *bColor*0.0813+128);


			*py++=yColor;
			if(j%2==0)
			{
				if(j%4==0)
					*pu++=(upreColor>>4)<<4 | vColor>>4 ;
				else
					*pv++=(vpreColor & 0xf0) | uColor>>4 ;
			}

			upreColor=uColor;
			vpreColor=vColor;

		}
	}
	ColorSpaceConversions space;
//	space.RGB24_to_YV12(pImageRGB24,(BYTE *)pic.img.plane[0],nWidth,nHeight);

	x264_picture_t pic_out;
	x264_nal_t *nal=NULL;

	int i_nal=0;
	pic.i_type = X264_TYPE_AUTO;
	pic.i_qpplus1 = 0;

	if(x264_encoder_encode(h,&nal,&i_nal,&pic,&pic_out)<0)
	{
		TRACE("failed x264_encoder_encode\n");
	}

	uint8_t *pYUV=new uint8_t[nWidth*nHeight*3/2];
	uint8_t *ppYUV[4];
	ppYUV[0]=pYUV;
	ppYUV[1]=pYUV+nWidth*nHeight;
	ppYUV[2]=ppYUV[1]+nWidth*nHeight/4;
	uint8_t *data=new uint8_t[3000000];
	for(int i = 0; i < i_nal; i++ )
	{
		int i_size;
		int i_data;

		i_data = 3000000;
		if( ( i_size = x264_nal_encode( data, &i_data, 1, &nal[i] ) ) > 0 )
		{
			fwrite(data,i_size,1,fp);
		


			int  got_picture=0;
			int	 consumed_bytes=0; 
			int  nalLen=i_size;
			
			uint8_t *Buf = (unsigned char*)calloc ( 1000000, sizeof(uint8_t));
			consumed_bytes= avcodec_decode_video(c, picture, &got_picture, data, nalLen);

			int icount=0;
			if(consumed_bytes > 0)
			{
// 				memcpy(ppYUV[0],picture->data[0],nWidth*nHeight);
// 				memcpy(ppYUV[1],picture->data[1],nWidth*nHeight/2);
// 				memcpy(ppYUV[2],picture->data[2],nWidth*nHeight/2);
				for(icount=0; icount<c->height; icount++)
				{
					//fwrite(picture->data[0] + icount * picture->linesize[0], 1, c->width, outf);
					memcpy(ppYUV[0]+icount*nWidth,picture->data[0] + icount * picture->linesize[0],c->width);
				}
				for(icount=0; icount<c->height/2; icount++)
				{
					//fwrite(picture->data[1] + icount * picture->linesize[1], 1, c->width/2, outf);
					memcpy(ppYUV[1]+icount*nWidth/2,picture->data[1] + icount * picture->linesize[1],c->width/2);
				}
				for(icount=0; icount<c->height/2; icount++)
				{
					//fwrite(picture->data[2] + icount * picture->linesize[2], 1, c->width/2, outf);
					memcpy(ppYUV[2]+icount*nWidth/2,picture->data[2] + icount * picture->linesize[2],c->width/2);
				}
			}

		}
		else if( i_size < 0 )
		{
			fprintf( stderr, "need to increase buffer size (size=%d)\n", -i_size );
		}
	}

	
//	convert_yuv420_rgb(pYUV,pImageRGB,nWidth,nHeight,1,2);
	
	space.YV12_to_RGB24(ppYUV[0],ppYUV[1],ppYUV[2],pImageRGB24,nWidth,nHeight);


	bitInfoHeader.biBitCount=24;
	bitInfoHeader.biSizeImage=3*nWidth*nHeight;
	StretchDIBits(hdc,
		0,0,rc.right-rc.left,rc.bottom-rc.top,
		0,0,nWidth,nHeight,
		pImageRGB24,(BITMAPINFO *)&bitInfoHeader,
		DIB_RGB_COLORS,SRCCOPY);


	delete []pImageRGB32;
	delete []pImageRGB24;
	delete []pYUV;
//	delete []data;
//	x264_picture_clean(&pic);

//	fclose(fp);

	return hr;
}

// 把指定采集设备与滤波器捆绑
bool CCaptureClass::BindFilter(int deviceId, IBaseFilter **pFilter)
{
	if (deviceId < 0) return false;

	// enumerate all video capture devices
	
	ICreateDevEnum *pCreateDevEnum;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
					CLSCTX_INPROC_SERVER,
					IID_ICreateDevEnum,
					(void**)&pCreateDevEnum);
	if (hr != NOERROR) return false;
	
	IEnumMoniker *pEm;

	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
	if (hr != NOERROR) return false;
	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	int index = 0;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= deviceId)
	{
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				if (index == deviceId)
				{
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				}
				SysFreeString(var.bstrVal);
			}			
			pBag->Release();
		}
		pM->Release();
		index++;
	}
	return true;
}

/* 创建滤波器链表管理器，查询其各种控制接口 */
HRESULT CCaptureClass::InitCaptureGraphBuilder()
{
	HRESULT hr;

	// 创建IGraphBuilder接口
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, 
						CLSCTX_INPROC_SERVER,
						IID_IGraphBuilder, (void **)&m_pGraph);
	if (FAILED(hr)) return hr;
	
	// 创建ICaptureGraphBuilder2接口
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2 , NULL,
						CLSCTX_INPROC,
						IID_ICaptureGraphBuilder2, (void **)&m_pCapture);
	if (FAILED(hr)) return hr;

	// 初始化滤波器链表管理器IGraphBuilder
	m_pCapture->SetFiltergraph(m_pGraph);
	
	// 查询媒体控制接口
	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMC);
	if (FAILED(hr)) return hr;
	// 查询视频窗口接口
	hr = m_pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *) &m_pVW);
	if (FAILED(hr)) return hr;

	return hr;

}

/* 设置视频显示窗口的特性 */ 
HRESULT CCaptureClass::SetupVideoWindow()
{
	HRESULT hr;
	//ljz 
	hr = m_pVW->put_Visible(OAFALSE);
	
	hr = m_pVW->put_Owner((OAHWND)m_hWnd);
	if (FAILED(hr)) return hr;

	hr = m_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr)) return hr;

	ResizeVideoWindow();
	hr = m_pVW->put_Visible(OATRUE);
	return hr;
}

/* 更改视频窗口大小 */
void CCaptureClass::ResizeVideoWindow()
{
	if (m_pVW)
	{
		//让图像充满整个窗口
		CRect rc;
		::GetClientRect(m_hWnd,&rc);
		m_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
	} 
}

/* 保存整个滤波器链表到文件，以便于使用GraphEdit软件查看*/
void CCaptureClass::SaveGraph(TCHAR *wFileName)
{
	HRESULT hr;
	USES_CONVERSION;

	//CFileDialog dlg(TRUE);
	
	
	//if (dlg.DoModal()==IDOK)
	{
		//WCHAR wFileName[MAX_PATH];
		//MultiByteToWideChar(CP_ACP, 0, dlg.GetPathName(), -1, wFileName, MAX_PATH);
			
		IStorage* pStorage=NULL;
		
		// First, create a document file that will hold the GRF file
		hr = ::StgCreateDocfile(A2OLE(wFileName), STGM_CREATE|STGM_TRANSACTED|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,	0, &pStorage);
		if (FAILED(hr))
		{
			AfxMessageBox(TEXT("Can not create a document"));
			return;
		}

		// Next, create a stream to store.
		WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
		IStream *pStream;

		hr = pStorage->CreateStream(wszStreamName,STGM_WRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE, 0, 0, &pStream);

		if(FAILED(hr))
		{
			AfxMessageBox(TEXT("Can not create a stream"));
			pStorage->Release();
			return;
		}

		// The IpersistStream::Save method converts a stream
		// into a persistent object.
		IPersistStream *pPersist = NULL;

		m_pGraph->QueryInterface(IID_IPersistStream,  reinterpret_cast<void**>(&pPersist));
		hr = pPersist->Save(pStream, TRUE);
		pStream->Release();
		pPersist->Release();

		if(SUCCEEDED(hr))
		{
			hr = pStorage->Commit(STGC_DEFAULT);
			if (FAILED(hr))
			{
			AfxMessageBox(TEXT("can not store it"));
			}
		}
		pStorage->Release();
	}

}

/*配置摄像头数据源格式：分辨率、RGB/I420等*/
void CCaptureClass::ConfigCameraPin(HWND hwndParent)
{
    HRESULT hr;
	IAMStreamConfig *pSC;
    ISpecifyPropertyPages *pSpec;

	//只有停止后，才能进行pin属性的设置
	m_pMC->Stop();

	hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Video, m_pBF,
                        IID_IAMStreamConfig, (void **)&pSC);

    CAUUID cauuid;

	hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
	if(hr == S_OK)
	{
		hr = pSpec->GetPages(&cauuid);
		//显示属性页
		hr = OleCreatePropertyFrame(hwndParent,	30, 30, NULL, 1, 
						(IUnknown **)&pSC, cauuid.cElems,
						(GUID *)cauuid.pElems, 0, 0, NULL);
		
		//释放内存、资源
  		CoTaskMemFree(cauuid.pElems);
		pSpec->Release();
		pSC->Release();
	}
	//回复运行
	m_pMC->Run();

}

/*配置图像参数：亮度、色度、饱和度等*/
void CCaptureClass::ConfigCameraFilter(HWND hwndParent)
{
	HRESULT hr=0;

	ISpecifyPropertyPages *pProp;

	hr = m_pBF->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);

	if (SUCCEEDED(hr)) 
	{
		// 获取滤波器名称和IUnknown接口指针
		FILTER_INFO FilterInfo;
		hr = m_pBF->QueryFilterInfo(&FilterInfo); 
		IUnknown *pFilterUnk;
		m_pBF->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

		// 显示该页
		CAUUID caGUID;
		pProp->GetPages(&caGUID);

		OleCreatePropertyFrame(
			hwndParent,				// 父窗口
			0, 0,                   // Reserved
			FilterInfo.achName,     // 对话框标题
			1,                      // 该滤波器的目标数目
			&pFilterUnk,            // 目标指针数组 
			caGUID.cElems,          // 属性页数目
			caGUID.pElems,          // 属性页的CLSID数组
			0,                      // 本地标识
			0, NULL                 // Reserved
		);

		// 释放内存、资源
		CoTaskMemFree(caGUID.pElems);
		pFilterUnk->Release();
		FilterInfo.pGraph->Release(); 
		pProp->Release();
	}
	//m_pMC->Run();
}


void CCaptureClass::convert_yuv420_rgb(unsigned char * src, unsigned char * dst,
									   int width, int height, int flipUV, int ColSpace)
{
	unsigned char *Y, *U, *V ;
	int y1, y2, u, v ;
	int v1, v2, u1, u2 ;
	unsigned char *pty1, *pty2 ;
	int i, j ;
	unsigned char *RGB1, *RGB2 ;
	int r, g, b ;

	//Initialization
	Y = src;
	V = Y + width * height;
	U = Y + width * height + width * height / 4;

	pty1 = Y;
	pty2 = pty1 + width;
	RGB1 = dst;
	RGB2 = RGB1 + 3 * width;
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 2) {
			if (flipUV) {
				u = (*V++) - 128;
				v = (*U++) - 128;
			} else {
				v = (*V++) - 128;
				u = (*U++) - 128;
			}

			switch (ColSpace) {
		  case 0:
			  {
				  // M$ color space
				  v1 = ((v << 10) + (v << 9) + (v << 6) + (v << 5)) >> 10;    // 1.593
				  u1 = ((u << 8) + (u << 7) + (u << 4)) >> 10;    //         0.390
				  v2 = ((v << 9) + (v << 4)) >> 10;    //                0.515
				  u2 = ((u << 11) + (u << 4)) >> 10;    //               2.015
			  }
			  break;
			  // PAL specific
		  case 1:
			  {
				  v1 = ((v << 10) + (v << 7) + (v << 4)) >> 10;    //      1.1406
				  u1 = ((u << 8) + (u << 7) + (u << 4) + (u << 3)) >> 10;    // 0.3984
				  v2 = ((v << 9) + (v << 6) + (v << 4) + (v << 1)) >> 10;    // 0.5800
				  u2 = ((u << 11) + (u << 5)) >> 10;    //              2.0312
			  }
			  break;
			  // V4l2
		  case 2:
			  {
				  v1 = ((v << 10) + (v << 8) + (v << 7) + (v << 5)) >> 10;    //       1.406
				  u1 = ((u << 8) + (u << 6) + (u << 5)) >> 10;    //                0.343
				  v2 = ((v << 9) + (v << 7) + (v << 6) + (v << 5)) >> 10;    //        0.718
				  u2 = ((u << 10) + (u << 9) + (u << 8) + (u << 4) + (u << 3)) >> 10;    // 1.773
			  }
			  break;
		  case 3:
			  {
				  v1 = u1 = v2 = u2 = 0;
			  }
			  break;
		  default:
			  break;
			} // end switch 
			//up-left

			y1 = (*pty1++);
			if (y1 > 0) {
				r = y1 + (v1);
				g = y1 - (u1) - (v2);
				b = y1 + (u2);

				r = CLIP (r);
				g = CLIP (g);
				b = CLIP (b);
			} else {
				r = g = b = 0;
			}
			/*       *RGB1++ = r; */
			/*       *RGB1++ = g; */
			/*       *RGB1++ = b; */
			*RGB1++ = b;
			*RGB1++ = g;
			*RGB1++ = r;

			//down-left
			y2 = (*pty2++);
			if (y2 > 0) {
				r = y2 + (v1);
				g = y2 - (u1) - (v2);
				b = y2 + (u2);

				r = CLIP (r);
				g = CLIP (g);
				b = CLIP (b);


			} else {
				r = b = g = 0;
			}
			/*       *RGB2++ = r; */
			/*       *RGB2++ = g; */
			/*       *RGB2++ = b; */
			*RGB2++ = b;
			*RGB2++ = g;
			*RGB2++ = r;

			//up-right
			y1 = (*pty1++);
			if (y1 > 0) {
				r = y1 + (v1);
				g = y1 - (u1) - (v2);
				b = y1 + (u2);

				r = CLIP (r);
				g = CLIP (g);
				b = CLIP (b);
			} else {
				r = g = b = 0;
			}
			*RGB1++ = b;
			*RGB1++ = g;
			*RGB1++ = r;

			/*       *RGB1++ = r; */
			/*       *RGB1++ = g; */
			/*       *RGB1++ = b; */
			//down-right
			y2 = (*pty2++);
			if (y2 > 0) {
				r = y2 + (v1);
				g = y2 - (u1) - (v2);
				b = y2 + (u2);

				r = CLIP (r);
				g = CLIP (g);
				b = CLIP (b);
			} else {
				r = b = g = 0;
			}

			/*       *RGB2++ = r; */
			/*       *RGB2++ = g; */
			/*       *RGB2++ = b; */

			*RGB2++ = b;
			*RGB2++ = g;
			*RGB2++ = r;

		}
		RGB1 += 3 * width;
		RGB2 += 3 * width;
		pty1 += width;
		pty2 += width;
	}

}
