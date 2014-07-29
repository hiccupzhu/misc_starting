// CaptureVideoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CaptureVideo.h"
#include "CaptureVideoDlg.h"

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


// CCaptureVideoDlg 对话框




CCaptureVideoDlg::CCaptureVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureVideoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCaptureVideoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIDEO_WINDOW, m_videoWindow);
	DDX_Control(pDX, IDC_COMBO1, m_listCtrl);
	DDX_Control(pDX, IDC_SHOW2, m_show2);
}

BEGIN_MESSAGE_MAP(CCaptureVideoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_PREVIEW, &CCaptureVideoDlg::OnBnClickedPreview)
	ON_BN_CLICKED(ID_CAPTURE, &CCaptureVideoDlg::OnBnClickedCapture)
	ON_BN_CLICKED(ID_VIDEO_FORMAT, &CCaptureVideoDlg::OnBnClickedVideoFormat)
	ON_BN_CLICKED(ID_IMAGE_PARAMETER, &CCaptureVideoDlg::OnBnClickedImageParameter)
	ON_BN_CLICKED(ID_SAVEGRAPH, &CCaptureVideoDlg::OnBnClickedSavegraph)
	ON_BN_CLICKED(ID_EXIT, &CCaptureVideoDlg::OnBnClickedExit)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CCaptureVideoDlg 消息处理程序

BOOL CCaptureVideoDlg::OnInitDialog()
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

	//ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码
	m_cap.EnumDevices(m_listCtrl.GetSafeHwnd());
	m_listCtrl.SetCurSel (0); 
	
	m_tooltip.Create(this);
	m_tooltip.Activate(TRUE);
	m_tooltip.AddTool(GetDlgItem(ID_PREVIEW), _T("开始预览视频"));
	m_tooltip.AddTool(GetDlgItem(ID_CAPTURE), _T("开始捕获、保存视频"));
	m_tooltip.AddTool(GetDlgItem(ID_VIDEO_FORMAT), _T("视频格式"));
	m_tooltip.AddTool(GetDlgItem(ID_IMAGE_PARAMETER), _T("图像参数"));
	m_tooltip.AddTool(GetDlgItem(ID_SAVEGRAPH), _T("保存链表"));
	m_tooltip.AddTool(GetDlgItem(ID_EXIT), _T("退出应用程序"));


	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCaptureVideoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCaptureVideoDlg::OnPaint()
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
HCURSOR CCaptureVideoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CCaptureVideoDlg::PreTranslateMessage(MSG* pMsg) 
{
	// CG: The following block was added by the ToolTips component.
	{
		// Let the ToolTip process this message.
		m_tooltip.RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CCaptureVideoDlg::OnBnClickedPreview()
{
	// TODO: 在此添加控件通知处理程序代码
	HWND hVWindow = m_videoWindow.GetSafeHwnd();
	int id = m_listCtrl.GetCurSel();

	m_cap.PreviewImages(id , hVWindow);
}

void CCaptureVideoDlg::OnBnClickedCapture()
{
	// TODO: 在此添加控件通知处理程序代码
// 	CString strFilter = _T("AVI File (*.avi) | *.avi|");
// 	strFilter += "All File (*.*) | *.*|";
// 
// 	CFileDialog dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, strFilter, this);
// 	if (dlg.DoModal() == IDOK)
// 	{
// 		CString m_sourceFile = dlg.GetPathName();	
//		m_cap.CaptureImages(m_sourceFile);
//	}

	
// 	RECT rc={0};
// 	CClientDC dc(GetDlgItem(IDC_SHOW2));
// 	GetDlgItem(IDC_SHOW2)->GetClientRect(&rc);
// 	m_cap.CaptureImages("c:\\temp.bmp",dc.GetSafeHdc(),rc);

	SetTimer(1,200,NULL);
}

void CCaptureVideoDlg::OnBnClickedVideoFormat()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cap.ConfigCameraPin(this->m_hWnd);
}

void CCaptureVideoDlg::OnBnClickedImageParameter()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cap.ConfigCameraFilter(this->m_hWnd);	
}

void CCaptureVideoDlg::OnBnClickedSavegraph()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE);
	if (dlg.DoModal()==IDOK)
	{
		CString str=dlg.GetPathName();
		TCHAR *inFileName = str.GetBuffer(str.GetLength());
		str.ReleaseBuffer();
		m_cap.SaveGraph(inFileName);
	}
}

void CCaptureVideoDlg::OnBnClickedExit()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::OnOK();
}

void CCaptureVideoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

// 	RECT rc={0};
// 	CClientDC dc(GetDlgItem(IDC_SHOW2));
// 	GetDlgItem(IDC_SHOW2)->GetClientRect(&rc);
// 	HDC hdc=dc.GetSafeHdc();
	m_cap.CaptureImages(this);

	//CDialog::OnTimer(nIDEvent);
}
