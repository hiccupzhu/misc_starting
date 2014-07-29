// MediaPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MediaPlayer.h"
#include "MediaPlayerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMediaPlayerDlg 对话框


CMediaPlayerDlg::CMediaPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	//初始化
	m_pFilterGraph = NULL;
	m_sourceFile = "";
	m_playerTimer = 0;
}

void CMediaPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_VIDEO_WINDOW, fdgfdgfgfdg);
	DDX_Control(pDX, IDC_VIDEO_WINDOW, m_videoWindow);
	DDX_Control(pDX, IDC_SLIDER_PLAY, m_sliderPlayer);


	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_sliderVolume);
}

BEGIN_MESSAGE_MAP(CMediaPlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CMediaPlayerDlg::OnBnClickedButtonOpen)
	ON_WM_ERASEBKGND()
//	ON_WM_ASKCBFORMATNAME()
ON_BN_CLICKED(IDC_BUTTON_PLAY, &CMediaPlayerDlg::OnBnClickedButtonPlay)
ON_BN_CLICKED(IDC_BUTTON_EXIT, &CMediaPlayerDlg::OnBnClickedButtonExit)
ON_BN_CLICKED(IDC_BUTTON_STOP, &CMediaPlayerDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphNotify)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CMediaPlayerDlg::OnBnClickedButtonPause)
	ON_COMMAND(ID_MENU_OPENFILE, &CMediaPlayerDlg::OnMenuOpenfile)
	ON_COMMAND(ID_MENU_HALFRATE, &CMediaPlayerDlg::OnMenuHalfrate)
	ON_COMMAND(ID_MENU_NORMALRATE, &CMediaPlayerDlg::OnMenuNormalrate)
	ON_COMMAND(ID_MENU_DOUBLERATE, &CMediaPlayerDlg::OnMenuDoublerate)
	ON_COMMAND(ID_MENU_FULLSCREEN, &CMediaPlayerDlg::OnMenuFullscreen)
	ON_COMMAND(ID_MENU_ALWAYSONTOP, &CMediaPlayerDlg::OnMenuAlwaysontop)
	ON_WM_HSCROLL()
	ON_COMMAND(ID_MENU_MUTE, &CMediaPlayerDlg::OnMenuMute)
	ON_BN_CLICKED(IDC_BUTTON_GRASP, &CMediaPlayerDlg::OnBnClickedButtonGrasp)
	ON_WM_TIMER()
	ON_COMMAND(ID_MENU_CLOSEFILE, &CMediaPlayerDlg::OnMenuClosefile)
	ON_COMMAND(ID_MENU_PLAY, &CMediaPlayerDlg::OnMenuPlay)
	ON_COMMAND(ID_MENU_STOP, &CMediaPlayerDlg::OnMenuStop)
	ON_COMMAND(ID_MENU_GRABIMAGE, &CMediaPlayerDlg::OnMenuGrabimage)
	ON_COMMAND(ID_MENU_SAVEGRAPH, &CMediaPlayerDlg::OnMenuSavegraph)
	ON_COMMAND(ID_MENU_EXIT, &CMediaPlayerDlg::OnMenuExit)
END_MESSAGE_MAP()


// CMediaPlayerDlg 消息处理程序

BOOL CMediaPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_pFilterGraph = NULL;
	//m_videoWindow.ModifyStyle(0, WS_CLIPCHILDREN);

	m_sliderPlayer.SetRange(0, 1000);
	m_sliderPlayer.SetPos(0);

	m_sliderVolume.SetRange(50,100);
	m_sliderVolume.SetPos(50);
	m_volume = 100;

	m_tooltip.Create(this);
	m_tooltip.Activate(TRUE);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_OPEN), _T("Open Media File"));
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_PLAY), _T("Play Media File"));
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_PAUSE), _T("Pause Media File"));
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_STOP), _T("Stop Media File"));
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_GRASP),_T("Grasp Image"));
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_EXIT),_T("Exit the App"));
	m_tooltip.AddTool(GetDlgItem(IDC_SLIDER_PLAY),_T("Slider of Player"));
	m_tooltip.AddTool(GetDlgItem(IDC_SLIDER_VOLUME),_T("Slider of Volume"));
	m_tooltip.AddTool(GetDlgItem(IDC_VIDEO_WINDOW),_T("Display Pictures"));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMediaPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMediaPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CMediaPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


LRESULT CMediaPlayerDlg::OnGraphNotify(WPARAM inWParam, LPARAM inLParam)
{
	IMediaEventEx *pEvent = NULL;
	if ((m_pFilterGraph!=NULL) && (pEvent = m_pFilterGraph->GetEventHandle()))
	{
		LONG eventCode = 0;
		LONG eventParam1  = 0;
		LONG eventParam2 = 0;
		while (SUCCEEDED(pEvent->GetEvent(&eventCode, &eventParam1, &eventParam2, 0)))
		{
			pEvent->FreeEventParams(eventCode, eventParam1, eventParam2);
			switch (eventCode)
			{
			case EC_COMPLETE:
				OnBnClickedButtonPause();
				m_pFilterGraph->SetCurrentPosition(0);
				break;
			case EC_USERABORT:
			case EC_ERRORABORT:
				OnBnClickedButtonStop();
				break;
			default:
				break;
			}
		}
	}
	return 0;
}


void CMediaPlayerDlg::CreateGraph()
{
	DestroyGraph();                  //销毁滤波器链表图
	m_pFilterGraph = new CDXGraph(); //创建CDXGraph对象
	if (m_pFilterGraph->Create())    //创建滤波器链表管理器
	{
		//if (!m_pFilterGraph->RenderFile(ch))//渲染媒体文件，构建滤波器链表
		TCHAR *ch1 = m_sourceFile.GetBuffer(m_sourceFile.GetLength());
		
		if (!m_pFilterGraph->RenderFile(ch1))//渲染媒体文件，构建滤波器链表
		{
			MessageBox(_T("无法渲染此媒体文件！请确认是否安装相关解码器插件！\n 或者此媒体文件已损坏！"),_T("系统提示"),MB_ICONWARNING); 
			return;
		}
		m_sourceFile.ReleaseBuffer();
         //设置图像显示窗口
		m_pFilterGraph->SetDisplayWindow(m_videoWindow.GetSafeHwnd());
         //设置窗口消息通知
		m_pFilterGraph->SetNotifyWindow(this->GetSafeHwnd());
		//显示第一帧图像
		m_pFilterGraph->Pause();

	}
}


void CMediaPlayerDlg::DestroyGraph()
{
	if (m_pFilterGraph != NULL)
	{
		m_pFilterGraph->Stop();
		m_pFilterGraph->SetNotifyWindow(NULL);

		delete m_pFilterGraph;
		m_pFilterGraph = NULL;
	}
}


CString CMediaPlayerDlg::GetFileTitleFromFileName(CString FileName, BOOL Ext)   
{   
    int Where;   
    Where = FileName.ReverseFind('\\');  
    if (Where == -1)  
        Where = FileName.ReverseFind('/');  
    CString FileTitle = FileName.Right(FileName.GetLength() - 1 - Where);  
    if (!Ext)  
    {  
        int Which = FileTitle.ReverseFind('.');   
        if (Which != -1)   
            FileTitle = FileTitle.Left(Which);   
    }   
    return FileTitle;   
}  

void CMediaPlayerDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
#if 1
	CString strFilter = _T("AVI File (*.avi) | *.avi|");
	strFilter += "MPEG File (*.mpg; *.mpeg) | *.mpg; *.mpeg|";
	strFilter += "MP3 File (*.mp3) | *.mp3|";
	strFilter += "WMA File (*.wma) | *.wma|";
	strFilter += "All File (*.*) | *.*|";
#else
	CString strFilter = _T("AVI File (*.avi)|*.avi|MPEG File (*.mpg)|*.mpg|MP3 File (*.mp3)|*.mp3|All Files (*.*)|*.*||"); 
#endif


	CFileDialog dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, strFilter, this);
	if (dlg.DoModal() == IDOK)
	{
		m_sourceFile = dlg.GetPathName();
		m_mediaFileName =GetFileTitleFromFileName(m_sourceFile,1);

		CreateGraph();
	}
}

BOOL CMediaPlayerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// CG: The following block was added by the ToolTips component.
	{
		// Let the ToolTip process this message.
		m_tooltip.RelayEvent(pMsg);
	}
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || VK_ESCAPE)
		{
			//RestoreFromFullScreen();
			if (m_pFilterGraph != NULL)
			{
				if (m_pFilterGraph->GetFullScreen())
				{
					m_pFilterGraph->SetFullScreen(FALSE);
				}
			}

			return TRUE;
		}
	}else if (WM_RBUTTONDOWN == pMsg->message)
	{
		CPoint point;	
		//HMENU hmenu;
		CMenu hmenu; 
		HMENU hmenuTrackPopup;  
		hmenu.LoadMenu(IDR_MENU_MAIN);

		GetCursorPos(&point);
		hmenuTrackPopup = GetSubMenu(hmenu, 0); 
		TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, this->m_hWnd, NULL); 
		DestroyMenu(hmenu);

	}


	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CMediaPlayerDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
#if 1
	CRect rect; 
	m_videoWindow.GetClientRect(&rect);
	pDC-> ExcludeClipRect(&rect);
#endif

	return CDialog::OnEraseBkgnd(pDC);
}

void CMediaPlayerDlg::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_pFilterGraph)
	{
		SetWindowText(_T("1倍速播放 ") + m_mediaFileName);
		m_pFilterGraph->Run();
		//m_volume = m_pFilterGraph->GetAudioVolume();
		//m_sliderAudio.SetPos(m_volume);
		//m_volume = 100;
		m_pFilterGraph->ChangeAudioVolume(m_volume);
		m_sliderVolume.SetPos(m_volume);

		if (m_playerTimer == 0)
		{
			m_playerTimer = SetTimer(SLIDER_TIMER, 100, NULL);
		}
	}


}

void CMediaPlayerDlg::OnBnClickedButtonExit()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedButtonStop();
	CoUninitialize();

	CDialog::OnOK();
}

void CMediaPlayerDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pFilterGraph != NULL)
	{
		m_pFilterGraph->Stop();
	}
		
}

void CMediaPlayerDlg::OnBnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pFilterGraph != NULL)
	{
		m_pFilterGraph->Pause();
	}
}

void CMediaPlayerDlg::OnMenuOpenfile()
{
	// TODO: 在此添加命令处理程序代码
	OnBnClickedButtonOpen();
}

void CMediaPlayerDlg::OnMenuHalfrate()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFilterGraph)
	{
		m_pFilterGraph->SetPlaybackRate(0.5);
		SetWindowText(_T("1/2倍速播放 ") + m_mediaFileName);
	}
}

void CMediaPlayerDlg::OnMenuNormalrate()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFilterGraph)
	{
		m_pFilterGraph->SetPlaybackRate(1.0);
		SetWindowText(_T("1倍速播放 ") + m_mediaFileName);
	}
}

void CMediaPlayerDlg::OnMenuDoublerate()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFilterGraph)
	{
		m_pFilterGraph->SetPlaybackRate(2.0);
		SetWindowText(_T("2倍速播放 ") + m_mediaFileName);
	}
}

void CMediaPlayerDlg::OnMenuFullscreen()
{
	// TODO: 在此添加命令处理程序代码
	static int flag=0; 
	if (m_pFilterGraph != NULL)
	{
		if (!flag){
			m_pFilterGraph->SetFullScreen(TRUE);
			flag = 1;
		}else{
			m_pFilterGraph->SetFullScreen(FALSE);
			flag = 0;
		}
	}
}

void CMediaPlayerDlg::OnMenuAlwaysontop()
{
	// TODO: 在此添加命令处理程序代码
	static int flag=0; 
	if (!flag)
	{
		::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
		flag = 1;
	}
	else 
	{
		::SetWindowPos(m_hWnd,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		flag = 0;
	}
}

void CMediaPlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (pScrollBar->GetSafeHwnd() == m_sliderPlayer.GetSafeHwnd())
	{
		if (m_pFilterGraph != NULL)
		{
			double duration =1.0;
			m_pFilterGraph->GetDuration(&duration);
			double pos = duration * m_sliderPlayer.GetPos()/1000.0;
			m_pFilterGraph->SetCurrentPosition(pos);
		}
	}else if (pScrollBar->GetSafeHwnd() == m_sliderVolume.GetSafeHwnd())
	{
		if (m_pFilterGraph != NULL)
		{
			//m_volume = m_sliderAudio.GetPos();
			m_volume = m_sliderVolume.GetPos();
			m_pFilterGraph->ChangeAudioVolume(m_volume);
		}
	} else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}

void CMediaPlayerDlg::OnMenuMute()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFilterGraph != NULL)
	{
		static int flag=0;
		if (!flag)
		{
			m_pFilterGraph->Mute();
			flag = 1;
		}else
		{
			m_pFilterGraph->UnMute();
			flag = 0;
		}
	}
}

void CMediaPlayerDlg::OnBnClickedButtonGrasp()
{
	// TODO: 在此添加控件通知处理程序代码
	static int c = 0;
	TCHAR szFilename[MAX_PATH]; 
	DWORD   dwPathLen  = 0;   
	if((dwPathLen = ::GetModuleFileName(::AfxGetInstanceHandle(),szFilename, MAX_PATH ))== 0)
	{   
		return;   
	}   
	for( int i=dwPathLen-1;  i>=0; i--)
	{   
		if(('\\' == szFilename[i])  ||  ('/' == szFilename[i]))
		{   
			break;   
		} else {   
			szFilename[i]   =   '\0';   
		}   
	}   
	
	CString str;
	str.Format(_T("%s"),szFilename);

	CString strTemp;
	strTemp.Format(_T("%d"),c);
	str += strTemp + _T(".bmp");
	c++;
	
	TCHAR *p=str.GetBuffer(str.GetLength());
	str.ReleaseBuffer();

	if (m_pFilterGraph != NULL)
	{
		if (m_pFilterGraph->SnapshotBitmap(p))
		{
		}else
			MessageBox(_T("抓图失败!"));
	}
}

void CMediaPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == m_playerTimer && m_pFilterGraph != NULL)
	{
		double pos = 0;
		double duration = 1.0;
		m_pFilterGraph->GetCurrentPosition(&pos);
		m_pFilterGraph->GetDuration(&duration);
		int newPos = int(pos*1000/duration);
		if (m_sliderPlayer.GetPos() != newPos)
		{
			m_sliderPlayer.SetPos(newPos);
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CMediaPlayerDlg::OnMenuClosefile()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFilterGraph != NULL)
	{
		m_pFilterGraph->Stop();
	}
}

void CMediaPlayerDlg::OnMenuPlay()
{
	// TODO: 在此添加命令处理程序代码
    if (m_pFilterGraph)	 
	{
		if (m_pFilterGraph->IsPaused())
			OnBnClickedButtonPlay();
		else 
			OnBnClickedButtonPause();
	}
}

void CMediaPlayerDlg::OnMenuStop()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pFilterGraph != NULL)
	{
		m_pFilterGraph->Stop();
	}
}

void CMediaPlayerDlg::OnMenuGrabimage()
{
	// TODO: 在此添加命令处理程序代码
	OnBnClickedButtonGrasp();
}

void CMediaPlayerDlg::OnMenuSavegraph()
{
	// TODO: 在此添加命令处理程序代码
	HRESULT hr;
	CFileDialog dlg(TRUE);
	if (dlg.DoModal()==IDOK)
	{
		 TCHAR *wFileName;
		 CString str;
		 str = dlg.GetPathName();
		 wFileName = str.GetBuffer(str.GetLength());
		IStorage* pStorage=NULL;
		// First, create a document file that will hold the GRF file
		hr = ::StgCreateDocfile(
			wFileName,
			STGM_CREATE|STGM_TRANSACTED|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
			0, &pStorage);
		if (FAILED(hr))
		{
			AfxMessageBox(TEXT("Can not create a document"));
			return;
		}
		// Next, create a stream to store.
		WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
		IStream *pStream;
		hr = pStorage->CreateStream(
			wszStreamName,
			STGM_WRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE,
			0, 0, &pStream);
		if(FAILED(hr))
		{
			AfxMessageBox(TEXT("Can not create a stream"));
			pStorage->Release();
			return;
		}
		// The IpersistStream::Save method converts a stream
        // into a persistent object.
		IPersistStream *pPersist = NULL;
		m_pFilterGraph->pGraph->QueryInterface(IID_IPersistStream, 
			reinterpret_cast<void**>(&pPersist));
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

void CMediaPlayerDlg::OnMenuExit()
{
	// TODO: 在此添加命令处理程序代码
	OnBnClickedButtonStop();

	CDialog::OnOK();
}
