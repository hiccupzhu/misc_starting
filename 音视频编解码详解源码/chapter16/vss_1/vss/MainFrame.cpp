// MainFrame.cpp : 实现文件
//

#include "stdafx.h"
#include "vss.h"
#include "MainFrame.h"


// CMainFrame 对话框

IMPLEMENT_DYNAMIC(CMainFrame, CBkDialogST)

CMainFrame::CMainFrame(CWnd* pParent /*=NULL*/)
	: CBkDialogST(CMainFrame::IDD, pParent)
{

}

CMainFrame::~CMainFrame()
{
}

void CMainFrame::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_ONE_PIC, m_BtnOnePic);
	DDX_Control(pDX, IDC_BUTTON_FOURE_PIC, m_BtnFourePic);
	DDX_Control(pDX, IDC_BUTTON_EXIT_APP, m_BtnExitApp);
	DDX_Control(pDX, IDC_BUTTON_CHAN_L, m_BtnChan1);
	DDX_Control(pDX, IDC_BUTTON_CHAN_M, m_BtnChan2);
	DDX_Control(pDX, IDC_BUTTON_CHAN_R, m_BtnChan3);
	DDX_Control(pDX, IDC_BUTTON_MINI, m_BtnMini);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_BtnClose);
	DDX_Control(pDX, IDC_BUTTON_PLAY_ALL, m_BtnPlayAll);
}


BEGIN_MESSAGE_MAP(CMainFrame, CBkDialogST)
	ON_BN_CLICKED(IDC_BUTTON_ONE_PIC, &CMainFrame::OnBnClickedButtonOnePic)
	ON_BN_CLICKED(IDC_BUTTON_FOURE_PIC, &CMainFrame::OnBnClickedButtonFourePic)
	ON_BN_CLICKED(IDC_BUTTON_EXIT_APP, &CMainFrame::OnBnClickedButtonExitApp)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CMainFrame::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_MINI, &CMainFrame::OnBnClickedButtonMini)
	ON_BN_CLICKED(IDC_BUTTON_CHAN_L, &CMainFrame::OnBnClickedButtonChanL)
	ON_BN_CLICKED(IDC_BUTTON_CHAN_M, &CMainFrame::OnBnClickedButtonChanM)
	ON_BN_CLICKED(IDC_BUTTON_CHAN_R, &CMainFrame::OnBnClickedButtonChanR)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphNotify)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON_PLAY_ALL, &CMainFrame::OnBnClickedButtonPlayAll)
END_MESSAGE_MAP()


// CMainFrame 消息处理程序

BOOL CMainFrame::OnInitDialog()
{
	CBkDialogST::OnInitDialog();

/////////////////////////////////////////////////////
	HWND hDesktop;
	HDC hdc; 
	DWORD w, h; 
	hDesktop = ::GetDesktopWindow ();
	hdc = ::GetDC(hDesktop); 
	ScrWidth  = w = GetDeviceCaps(hdc, HORZRES); 
	ScrHeight = h = GetDeviceCaps(hdc, VERTRES);

	bChanged = FALSE;

	if((w!=1024)&&(h!=768))
	{
		DEVMODE   lpDevMode;
		lpDevMode.dmBitsPerPel	= 32;
		lpDevMode.dmPelsWidth	= 1024;
		lpDevMode.dmPelsHeight	= 768;
		lpDevMode.dmDisplayFrequency=75;
		lpDevMode.dmSize = sizeof(lpDevMode);
		lpDevMode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
		LONG result = ChangeDisplaySettings(&lpDevMode,0);
		if(result == DISP_CHANGE_SUCCESSFUL)
		{
			ChangeDisplaySettings(&lpDevMode,CDS_UPDATEREGISTRY);
			bChanged = TRUE;
			//使用CDS_UPDATEREGISTRY表示次修改是持久的,
			//并在注册表中写入了相关的数据 
		}
		else
		{
			ChangeDisplaySettings(NULL,0);
		} 
	}
////////////////////////////////////////////////////////
	m_pFilterGraph[0] = NULL;
	m_pFilterGraph[1] = NULL;
	m_pFilterGraph[2] = NULL;
	m_pFilterGraph[3] = NULL;

////////////////////////////////////////////////////////
	ModifyStyle(WS_CAPTION,0,0);
	this->ShowWindow(SW_SHOWMAXIMIZED);

	InitInterface();

	//SetBitmap(IDB_1_PIC );
	SetBitmap(IDB_4_PIC);
	
	SetTimer(0,1000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CMainFrame::InitInterface()
{
	// 获取对象指针
	pBtnOnePic	= GetDlgItem(IDC_BUTTON_ONE_PIC);
	pBtnFourePic= GetDlgItem(IDC_BUTTON_FOURE_PIC);
	pBtnExitApp = GetDlgItem(IDC_BUTTON_EXIT_APP);
	pBtnMini	= GetDlgItem(IDC_BUTTON_MINI);
	pBtnClose	= GetDlgItem(IDC_BUTTON_CLOSE);
	pBtnChan1	= GetDlgItem(IDC_BUTTON_CHAN_L);
	pBtnChan2   = GetDlgItem(IDC_BUTTON_CHAN_M);
	pBtnChan3	= GetDlgItem(IDC_BUTTON_CHAN_R);

	pDisSysTime = GetDlgItem(IDC_STATIC_TIME);
	pDisSysTime->MoveWindow(850,54,130,80);
	
	pStaticVideo[0]= GetDlgItem(IDC_VIDEO0);
	pStaticVideo[1]= GetDlgItem(IDC_VIDEO1);
	pStaticVideo[2]= GetDlgItem(IDC_VIDEO2);
	pStaticVideo[3]= GetDlgItem(IDC_VIDEO3);

	pStaticVideo[0]->MoveWindow(26,53,394,311);
	pStaticVideo[1]->MoveWindow(26+395,53,392,311);
	pStaticVideo[2]->MoveWindow(26,53+311+2,394,307);
	pStaticVideo[3]->MoveWindow(26+395,53+311+2,392,307);

	pStaticVideo[0]->ShowWindow(SW_HIDE);
	pStaticVideo[1]->ShowWindow(SW_HIDE);
	pStaticVideo[2]->ShowWindow(SW_HIDE);
	pStaticVideo[3]->ShowWindow(SW_HIDE);

	pStaticVideo[0]->ModifyStyle(0, WS_CLIPCHILDREN);
	pStaticVideo[1]->ModifyStyle(0, WS_CLIPCHILDREN);
	pStaticVideo[2]->ModifyStyle(0, WS_CLIPCHILDREN);
	pStaticVideo[3]->ModifyStyle(0, WS_CLIPCHILDREN);
	
	pBtnPlayAll = GetDlgItem(IDC_BUTTON_PLAY_ALL);

	// 移动到指定位置
	pBtnOnePic->MoveWindow(16,702,37,37);
	pBtnFourePic->MoveWindow(16+75,702,37,37);
	pBtnMini->MoveWindow(880,2,35,35);
	pBtnClose->MoveWindow(965,3,35,35);
	pBtnChan1->MoveWindow(	876,	401,26,17);
	pBtnChan2->MoveWindow(	876+26,	402,25,16);
	pBtnChan3->MoveWindow(	876+51,	402,26,16);
	pBtnExitApp->MoveWindow(920,684,46,48);
	pBtnPlayAll->MoveWindow(167,704,37,37);
	// 设置位图
	m_BtnOnePic.SetSkin(IDB_BTN_N,IDB_BTN_D,IDB_BTN_N,IDB_BTN_N,0,0,0,0,0);
	m_BtnFourePic.SetSkin(IDB_BTN_N,IDB_BTN_D,IDB_BTN_N,IDB_BTN_N,0,0,0,0,0);
	m_BtnMini.SetSkin(IDB_MINI,IDB_MINI,IDB_MINI,IDB_MINI,0,0,0,0,0);
	m_BtnClose.SetSkin(IDB_CLOSE,IDB_CLOSE,IDB_CLOSE,IDB_CLOSE,0,0,0,0,0);
	m_BtnChan1.SetSkin(IDB_1_n,IDB_1_d,IDB_1_n,IDB_1_n,0,0,0,0,0);
	m_BtnChan2.SetSkin(IDB_2_n,IDB_2_d,IDB_2_n,IDB_2_n,0,0,0,0,0);
	m_BtnChan3.SetSkin(IDB_3_n,IDB_3_d,IDB_3_n,IDB_3_n,0,0,0,0,0);
	m_BtnExitApp.SetSkin( IDB_EXIT,IDB_EXIT, IDB_EXIT, IDB_EXIT,0,0,0,0,0);
	m_BtnPlayAll.SetSkin(IDB_ALL_N,IDB_ALL_D,IDB_ALL_N,IDB_ALL_N,0,0,0,0,0);
	// 添加在线提示
	m_BtnOnePic.SetToolTipText(_T("单画面显示"));
	m_BtnFourePic.SetToolTipText(_T("四画面显示"));
	m_BtnMini.SetToolTipText(_T("最小化"));
	m_BtnClose.SetToolTipText(_T("关闭"));
	m_BtnChan1.SetToolTipText(_T("第1通道"));
	m_BtnChan2.SetToolTipText(_T("第2通道"));
	m_BtnChan3.SetToolTipText(_T("第3通道"));

}
void CMainFrame::OnBnClickedButtonOnePic()
{
	// TODO: 在此添加控件通知处理程序代码
	SetBitmap(IDB_1_PIC);
	pStaticVideo[0]->MoveWindow(26,53,811-24,672-52);
}

void CMainFrame::OnBnClickedButtonFourePic()
{
	// TODO: 在此添加控件通知处理程序代码

	SetBitmap(IDB_4_PIC);
	pStaticVideo[0]->MoveWindow(26,53,394,311);
}

void CMainFrame::OnBnClickedButtonExitApp()
{
	// TODO: 在此添加控件通知处理程序代码
	// 释放资源
	if (MessageBox(_T("确定要退出该账户吗？"),_T("退出系统"),MB_OKCANCEL|MB_ICONQUESTION)==IDCANCEL)
		return ;
	else 
	{
		KillTimer(0);
		if (bChanged)
		{
			DEVMODE   lpDevMode;
			lpDevMode.dmBitsPerPel	= 32;
			lpDevMode.dmPelsWidth	= ScrWidth;
			lpDevMode.dmPelsHeight	= ScrHeight;
			lpDevMode.dmDisplayFrequency=60;
			lpDevMode.dmSize = sizeof(lpDevMode);
			lpDevMode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
			LONG result = ChangeDisplaySettings(&lpDevMode,0);
			if(result == DISP_CHANGE_SUCCESSFUL)
			{
				ChangeDisplaySettings(&lpDevMode,CDS_UPDATEREGISTRY);
				bChanged = TRUE;
			}
			else
			{
				ChangeDisplaySettings(NULL,0);
			} 
		}

		//
		int i;
		for (i=0; i<4; i++)
		{
			DestroyGraph(i);
		}
		//

		SendMessage(WM_SYSCOMMAND, SC_CLOSE, NULL);
	}
}

void CMainFrame::OnBnClickedButtonClose()
{
	// TODO: 在此添加控件通知处理程序代码
	// 释放其他资源
	if (bChanged)
	{
		DEVMODE   lpDevMode;
		lpDevMode.dmBitsPerPel	= 32;
		lpDevMode.dmPelsWidth	= ScrWidth;
		lpDevMode.dmPelsHeight	= ScrHeight;
		lpDevMode.dmDisplayFrequency=60;
		lpDevMode.dmSize = sizeof(lpDevMode);
		lpDevMode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
		LONG result = ChangeDisplaySettings(&lpDevMode,0);
		if(result == DISP_CHANGE_SUCCESSFUL)
		{
			ChangeDisplaySettings(&lpDevMode,CDS_UPDATEREGISTRY);
		}
		else
		{
			ChangeDisplaySettings(NULL,0);
		} 
	}

	SendMessage(WM_SYSCOMMAND, SC_CLOSE, NULL);
}

void CMainFrame::OnBnClickedButtonMini()
{
	// TODO: 在此添加控件通知处理程序代码
	KillTimer(0);
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, NULL);
}

void CMainFrame::OnBnClickedButtonChanL()
{
	iCurChan = 3;
	OpenFilePlay(0);
}

void CMainFrame::OnBnClickedButtonChanM()
{
	OpenFilePlay(1);
}

void CMainFrame::OnBnClickedButtonChanR()
{
	OpenFilePlay(2);
}

void CMainFrame::OpenFilePlay(int chan)
{
	CString strFilter = _T("AVI File (*.avi) | *.avi|");
	strFilter += "MPEG File (*.mpg; *.mpeg) | *.mpg; *.mpeg|";
	strFilter += "MP3 File (*.mp3) | *.mp3|";
	strFilter += "WMA File (*.wma) | *.wma|";
	strFilter += "All File (*.*) | *.*|";

	CFileDialog dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, strFilter, this);
	if (dlg.DoModal() == IDOK)
	{
		m_sourceFile[chan] = dlg.GetPathName();
		//m_mediaFileName[chan] =GetFileTitleFromFileName(m_sourceFile[chan],1);
		this->pStaticVideo[chan]->ShowWindow(SW_SHOWNORMAL);
		CreateGraph(chan);
		//m_pFilterGraph[chan]->Run();
		//this->pStaticVideo[chan]->UpdateWindow();
		//pStaticVideo[chan]->Invalidate();
		//iCurChan = chan;
	}
}


void CMainFrame::OnPaint()
{
	CPaintDC pDC(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
#if 1
	CFont NewFont; 
	NewFont.CreateFont(16,      // nHeight 字体高度
					0,			// nWidth  字体宽度
					0,			// nEscapement 字体显示的角度
					0,			// nOrientation  字体的角度
					FW_NORMAL,	//FW_NORMAL,//FW_BOLD,	// nWeight 字体的磅数
					            //FW_MEDIUM
					FALSE,		// bItalic    斜体字体
					FALSE,		// bUnderline带下划线的字体
					0,			// cStrikeOut 带删除线的字体
					GB2312_CHARSET,		//ANSI_CHARSET,	// nCharSet 所需的字符集
					OUT_DEFAULT_PRECIS,	// nOutPrecision  输出的精度
					CLIP_DEFAULT_PRECIS,	// nClipPrecision  裁减的精度
					DEFAULT_QUALITY,	// nQuality逻辑字体与输出设备的实际  
                                                          //字体之间的精度  
					DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 字体间距和字体集  
					_T("幼圆")            	    //Arial 字体名称
					); 
	CFont *pOldFont = pDC.SelectObject(&NewFont);
	pDC.SetBkMode(TRANSPARENT);		//选进设备描述表
	pDC.SetTextColor(RGB(255,0,0));         //设置字体颜色，这里是红色
	pDC.TextOut(100,10,_T("西南科技大学视频监控系统"));
	pDC.SelectObject(&pOldFont);            // 回复到旧字体
	NewFont.DeleteObject();                 // 删除新创建的字体
	// 不为绘图消息调用CBkDialogST::OnPaint()
#endif

}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent==0)
	{
		CTime tTime = CTime::GetCurrentTime();
		CString strTime =tTime.Format("\n           %Y-%m-%d  \n\n              %H:%M:%S");
		this->pDisSysTime->SetWindowText(strTime);
	}
	CBkDialogST::OnTimer(nIDEvent);
}


LRESULT CMainFrame::OnGraphNotify(WPARAM inWParam, LPARAM inLParam)
{
	IMediaEventEx *pEvent = NULL;
#if 0
	if ((m_pFilterGraph[iCurChan]!=NULL) && (pEvent = m_pFilterGraph[iCurChan]->GetEventHandle()))
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
				m_pFilterGraph[iCurChan]->SetCurrentPosition(0);
				break;
			case EC_USERABORT:
			case EC_ERRORABORT:
				break;
			default:
				break;
			}
		}
	}
#endif

	return 0;
}


void CMainFrame::CreateGraph(int chan)
{
	BOOL ret;
	DestroyGraph(chan);                  //销毁滤波器链表图
	m_pFilterGraph[chan] = new CDXGraph(); //创建CDXGraph对象
	if (m_pFilterGraph[chan]->Create())    //创建滤波器链表管理器
	{
		//if (!m_pFilterGraph->RenderFile(ch))//渲染媒体文件，构建滤波器链表
		TCHAR *ch1 = m_sourceFile[chan].GetBuffer(m_sourceFile[chan].GetLength());
		
		if (!m_pFilterGraph[chan]->RenderFile(ch1))//渲染媒体文件，构建滤波器链表
		{
			MessageBox(_T("无法渲染此媒体文件！请确认是否安装相关解码器插件！\n 或者此媒体文件已损坏！"),_T("系统提示"),MB_ICONWARNING); 
			return;
		}
		m_sourceFile[chan].ReleaseBuffer();
         //设置图像显示窗口
		ret = m_pFilterGraph[chan]->SetDisplayWindow(pStaticVideo[chan]->GetSafeHwnd());
         //设置窗口消息通知
		ret = m_pFilterGraph[chan]->SetNotifyWindow(this->GetSafeHwnd());
		//显示第一帧图像
		ret = m_pFilterGraph[chan]->Pause();

	}
}


void CMainFrame::DestroyGraph(int chan)
{
	if (m_pFilterGraph[chan] != NULL)
	{
		m_pFilterGraph[chan]->Stop();
		m_pFilterGraph[chan]->SetNotifyWindow(NULL);

		delete m_pFilterGraph[chan];
		m_pFilterGraph[chan] = NULL;
	}
}
BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CBkDialogST::OnEraseBkgnd(pDC);
}

void CMainFrame::OnBnClickedButtonPlayAll()
{
	// TODO: 在此添加控件通知处理程序代码
	int i;
	for (i=0; i<4; i++)
	{
		if (m_pFilterGraph[i])
		{
			m_pFilterGraph[i]->Run();
		}
	}
}
