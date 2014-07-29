// CaptureVideoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CaptureVideo.h"
#include "CaptureVideoDlg.h"
#include <d3d9.h>
#include "IDTSurface.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

template<class T> void Release(T t)
{
	if( t )
	{
		t->Release();
		t = 0;
	}
}

template<class T> void Delete(T t)
{
	if( t )
	{
		delete t;
		t = 0;
	}
}

typedef struct 
{
	HWND hwnd;
}THREADPARAM;

THREADPARAM threadParam;
LPDIRECT3DDEVICE9       pD3DDevice = NULL;	// Our rendering device


CCaptureVideoDlg::CCaptureVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureVideoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pData=NULL;
	m_hThread=NULL;
}

CCaptureVideoDlg::~CCaptureVideoDlg()
{
	Delete(m_pData);
	Release(pD3DDevice);
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
	ON_BN_CLICKED(IDC_BUTTON2, &CCaptureVideoDlg::OnBnClickedButton2)
	ON_MESSAGE(MY_THREADMSG,OnMyThreadMsg)
	ON_WM_CREATE() 
END_MESSAGE_MAP()


BOOL CCaptureVideoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	
	
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


	return TRUE;
}

int CCaptureVideoDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CCaptureVideoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}


void CCaptureVideoDlg::OnPaint()
{	
}



HCURSOR CCaptureVideoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CCaptureVideoDlg::PreTranslateMessage(MSG* pMsg) 
{
	// CG: The following block was added by the ToolTips component.
	{
		m_tooltip.RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CCaptureVideoDlg::OnBnClickedPreview()
{
	HWND hVWindow = m_videoWindow.GetSafeHwnd();
	int id = m_listCtrl.GetCurSel();

	m_cap.PreviewImages(id , hVWindow);

	BITMAPINFOHEADER infoheader;
	m_cap.GetSampleInfo(m_param);
	m_cap.GetSampleInfoHeader(infoheader);
	m_videoWindow.MoveWindow(0,0,m_param.nWidth / 2,m_param.nHeight / 2 - 10);
	m_cap.SetupVideoWindow();

#pragma pack(push) 
#pragma pack(4)
	m_pData=new BYTE[m_param.nWidth*m_param.nHeight*m_param.nBiCount/8];
#pragma pack(pop)

	GetDlgItem(ID_PREVIEW)->EnableWindow(FALSE);

//	m_DX9LogoImage.Initialize(pD3DDevice,m_param.nWidth,m_param.nHeight);
	
	CRect rc;
	rc.left=m_param.nWidth / 2 + 10;
	rc.top=0;
	rc.right=rc.left+infoheader.biWidth;
	rc.bottom=rc.top+infoheader.biHeight;
	m_show2.MoveWindow(&rc);
	m_d3dRender.InitD3D(&pD3DDevice,infoheader,m_show2.m_hWnd);
	m_d3dRender.Setup();
	m_imageHandle.Init(infoheader);

	threadParam.hwnd=m_hWnd;
}

void CCaptureVideoDlg::OnBnClickedCapture()
{
	CString strFilter = _T("AVI File (*.avi) | *.avi|");
	strFilter += "All File (*.*) | *.*|";

	CFileDialog dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, strFilter, this);
	if (dlg.DoModal() == IDOK)
	{
		CString m_sourceFile = dlg.GetPathName();	
		m_cap.CaptureImages(m_sourceFile);
	}
}

void CCaptureVideoDlg::OnBnClickedVideoFormat()
{
	m_cap.ConfigCameraPin(this->m_hWnd);
}

void CCaptureVideoDlg::OnBnClickedImageParameter()
{
	m_cap.ConfigCameraFilter(this->m_hWnd);	
}

void CCaptureVideoDlg::OnBnClickedSavegraph()
{
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
	CDialog::OnOK();
}


DWORD ThreadProc(LPVOID lpParameter)
{
	THREADPARAM* pThreadParam=(THREADPARAM*)lpParameter;
	while(true)
	{
		SendMessage(pThreadParam->hwnd,MY_THREADMSG,0,0);

		Sleep(20);
	}
	return 0;
}


void CCaptureVideoDlg::OnBnClickedButton2()
{
	CString str;
	GetDlgItem(IDC_BUTTON2)->GetWindowText(str);
	if(!str.Compare("图像处理 开启"))
	{
		if(m_hThread==NULL)
		{
			m_hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProc,(LPVOID)&threadParam,0,0);
			GetDlgItem(IDC_BUTTON2)->SetWindowText("图像处理 关闭");
		}
	}
	else
	{
		TerminateThread(m_hThread,0);
		m_hThread=NULL;
		GetDlgItem(IDC_BUTTON2)->SetWindowText("图像处理 开启");
	}
}


LRESULT CCaptureVideoDlg::OnMyThreadMsg(WPARAM wParam,LPARAM lParam)
{
// 	CClientDC dc(this);
// 
// 	CRect rc;
// 	m_videoWindow.GetWindowRect(&rc);	
 	m_cap.GetSampleData(m_pData);
// 	m_imageHandle.ImageHandle(m_pData);
// 
// 	RECT srcRect={rc.Width()+10,0,rc.Width()+10 + m_param.nWidth,m_param.nHeight};
// 	D3DRECT drc;
// 	drc.x1=srcRect.left;
// 	drc.y1=srcRect.top;
// 	drc.x2=srcRect.right;
// 	drc.y2=srcRect.bottom;

//	pD3DDevice->Clear(1,&drc, 	D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,0xff000000, 1.0f, 0);
//	pD3DDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

// 	LPDIRECT3DSURFACE9 surf;
// 	if (SUCCEEDED(pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surf)))
// 	{
// 		RECT rect; 
// 		rect.left = 0; 
// 		rect.top=0; 
// 		rect.right=rect.left + m_param.nWidth; 
// 		rect.bottom=rect.top + m_param.nHeight;
// 
// 		int ret = m_DX9LogoImage.BltFast(surf, m_pData, 0, 0, srcRect);
// 		if (ret != DT_OK)
// 		{
// 			return S_FALSE;
// 		}
// 		surf->Release();
// 	}

	m_d3dRender.DrawD3D(m_pData);

//	pD3DDevice->Present(&srcRect, &srcRect, NULL, NULL);
//	pD3DDevice->Present(0, 0, NULL, NULL);

	return S_OK;
}
