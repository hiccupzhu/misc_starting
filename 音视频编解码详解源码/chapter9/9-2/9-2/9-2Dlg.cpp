// 9-2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "9-2.h"
#include "9-2Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID 1
#define TIMER_DELAY 40
#define CAMERA_WIDTH  352
#define CAMERA_HEIGHT 288

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


// CMy92Dlg 对话框




CMy92Dlg::CMy92Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMy92Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy92Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIDEO_WINDOW, m_videoWindow);
	DDX_Control(pDX, IDC_LISTDEVICE, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CMy92Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_PREVIEW, &CMy92Dlg::OnBnClickedPreview)
	ON_BN_CLICKED(ID_CAPTURE, &CMy92Dlg::OnBnClickedCapture)
	ON_BN_CLICKED(ID_PAUSEPLAY, &CMy92Dlg::OnBnClickedPauseplay)
	ON_BN_CLICKED(ID_SAVEGRAPH, &CMy92Dlg::OnBnClickedSavegraph)
	ON_BN_CLICKED(ID_EXITAPP, &CMy92Dlg::OnBnClickedExitapp)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOPCAP, &CMy92Dlg::OnBnClickedStopcap)
END_MESSAGE_MAP()


// CMy92Dlg 消息处理程序

BOOL CMy92Dlg::OnInitDialog()
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
	this->m_VMRCap.EnumDevices(this->m_listCtrl);
	this->m_listCtrl.SetCurSel(0);
	m_yuvFileName = _T("");
	m_fileState = FALSE;
	m_imageWidth = CAMERA_WIDTH;
	m_imageHeight= CAMERA_HEIGHT;
	p_yuv420 = NULL;

	m_tooltip.Create(this);
	m_tooltip.Activate(TRUE);
	m_tooltip.AddTool(GetDlgItem(ID_PREVIEW), _T("预览视频"));
	m_tooltip.AddTool(GetDlgItem(ID_PAUSEPLAY), _T("暂停/开始预览"));
	m_tooltip.AddTool(GetDlgItem(ID_CAPTURE), _T("图像捕捉"));
	m_tooltip.AddTool(GetDlgItem(ID_STOPCAP), _T("终止捕捉"));
	m_tooltip.AddTool(GetDlgItem(ID_SAVEGRAPH), _T("保存链表"));
	m_tooltip.AddTool(GetDlgItem(ID_EXITAPP), _T("退出应用程序"));
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMy92Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMy92Dlg::OnPaint()
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
HCURSOR CMy92Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CMy92Dlg::PreTranslateMessage(MSG* pMsg) 
{
	// CG: The following block was added by the ToolTips component.
	{
		// Let the ToolTip process this message.
		m_tooltip.RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CMy92Dlg::OnBnClickedPreview()
{
	// TODO: 在此添加控件通知处理程序代码
	HWND hwnd = this->m_videoWindow.GetSafeHwnd();
	int id = this->m_listCtrl.GetCurSel();
	HRESULT hr = m_VMRCap.Init(id,hwnd,m_imageWidth,m_imageHeight);
	
	if (FAILED(hr)) 
		AfxMessageBox(_T("无法创建滤波器链表！"));
}

void CMy92Dlg::OnBnClickedCapture()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(FALSE);

	if (dlg.DoModal()==IDOK)
	{
		m_yuvFileName = dlg.GetPathName();
		BOOL ret = m_pFile.Open(m_yuvFileName,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		if (!ret)
		{
			AfxMessageBox(_T("创建YUV文件失败！"));
			return ;
		}
		else
			m_fileState = ret;
		if (!p_yuv420)
			p_yuv420 = new BYTE [m_imageWidth*m_imageHeight*3/2];
		this->KillTimer(TIMER_ID);
		this->SetTimer(TIMER_ID,TIMER_DELAY,NULL);
	}
}

void CMy92Dlg::OnBnClickedStopcap()
{
	// TODO: 在此添加控件通知处理程序代码
	KillTimer(TIMER_ID);
	if (p_yuv420)
	{
		delete [] p_yuv420;
		p_yuv420 = NULL;
	}
	if (m_fileState)
	{
		m_pFile.Close();
		m_fileState = FALSE;
	}
}

void CMy92Dlg::OnBnClickedPauseplay()
{
	// TODO: 在此添加控件通知处理程序代码
	static int c=0;
	if (!c) {
		GetDlgItem(ID_PAUSEPLAY)->SetWindowTextW(_T("开始预览"));
		c = 1;
	} else {
		GetDlgItem(ID_PAUSEPLAY)->SetWindowTextW(_T("暂停预览"));
		c = 0;
	}
	m_VMRCap.Pause();
}

void CMy92Dlg::OnBnClickedSavegraph()
{
	// TODO: 在此添加控件通知处理程序代码
	HRESULT hr;
	CFileDialog dlg(FALSE);
	
	if (dlg.DoModal()==IDOK)
	{
		CString inFileName = dlg.GetPathName();
		m_VMRCap.SaveGraph(inFileName);
	}
}

void CMy92Dlg::OnBnClickedExitapp()
{
	// TODO: 在此添加控件通知处理程序代码
	KillTimer(TIMER_ID);
	if (p_yuv420)
		delete [] p_yuv420;
	if (m_fileState)
		m_pFile.Close();
	CDialog::OnOK();
}

void CMy92Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent==TIMER_ID)
	{	//定时读取获取图像帧，转换颜色空间，写文件
		if (m_fileState)
		{
			DWORD dwSize;
			dwSize = this->m_VMRCap.GrabFrame();
			if (dwSize >0 )
			{
				BYTE *pImage;
				this->m_VMRCap.GetFrame(&pImage);
				conv.RGB24_to_YV12(pImage,this->p_yuv420,m_imageWidth,m_imageHeight);
				m_pFile.Write(p_yuv420,m_imageWidth*m_imageHeight*3/2);
			}
		}
	}

	CDialog::OnTimer(nIDEvent);
}


