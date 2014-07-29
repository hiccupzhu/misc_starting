//-------------------------------------------------------------------
// CCaptureClass视频捕捉类实现文件CaptureVideo.cpp
//-------------------------------------------------------------------
// CaptureVideo.cpp: implementation of the CCaptureClass class.
//
/////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CaptureClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	m_pGB = NULL;
	m_pBF = NULL;
	m_pCapture = NULL; 
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
	srelease(m_pCapture);
	srelease(m_pMC);
	srelease(m_pGB);
	srelease(m_pBF);
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
	hr = m_pGB->AddFilter(m_pBF, L"Capture Filter");
	
	// 渲染媒体，把链表中滤波器连接起来
    hr = m_pCapture->RenderStream( &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, m_pBF, NULL, NULL );

	if( FAILED( hr ) )
	{
		AfxMessageBox(_T("Can’t build the graph"));
		return hr;
	}

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

/*设置捕获视频的文件，开始捕捉视频数据写文件*/
HRESULT CCaptureClass::CaptureImages(CString inFileName)
{
	HRESULT hr=0;
	
	// 先停止视频
	m_pMC->Stop();
	// 设置文件名，注意该函数的第二个参数的类型
	hr = m_pCapture->SetOutputFileName(&MEDIASUBTYPE_Avi, inFileName.AllocSysString(), &pMux, NULL );
    // 渲染媒体，连接所有滤波器
	hr = m_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pBF, NULL, pMux );
	pMux->Release();
	// 回复视频
	m_pMC->Run();
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
						IID_IGraphBuilder, (void **)&m_pGB);
	if (FAILED(hr)) return hr;
	
	// 创建ICaptureGraphBuilder2接口
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2 , NULL,
						CLSCTX_INPROC,
						IID_ICaptureGraphBuilder2, (void **)&m_pCapture);
	if (FAILED(hr)) return hr;

	// 初始化滤波器链表管理器IGraphBuilder
	m_pCapture->SetFiltergraph(m_pGB);
	
	// 查询媒体控制接口
	hr = m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC);
	if (FAILED(hr)) return hr;
	// 查询视频窗口接口
	hr = m_pGB->QueryInterface(IID_IVideoWindow, (LPVOID *) &m_pVW);
	if (FAILED(hr)) return hr;

	return hr;

}

/* 设置视频显示窗口的特性 */ 
HRESULT CCaptureClass::SetupVideoWindow()
{
	HRESULT hr;
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
	//CFileDialog dlg(TRUE);
	
	//if (dlg.DoModal()==IDOK)
	{
		//WCHAR wFileName[MAX_PATH];
		//MultiByteToWideChar(CP_ACP, 0, dlg.GetPathName(), -1, wFileName, MAX_PATH);
			
		IStorage* pStorage=NULL;
		
		// First, create a document file that will hold the GRF file
		hr = ::StgCreateDocfile(wFileName, STGM_CREATE|STGM_TRANSACTED|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,	0, &pStorage);
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

		m_pGB->QueryInterface(IID_IPersistStream,  reinterpret_cast<void**>(&pPersist));
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

