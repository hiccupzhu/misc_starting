// CaptureVideoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CaptureVideo.h"
#include "CaptureVideoDlg.h"
#include <d3d9.h>
#include "camera.h"
#include "terrain.h"


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
	CCaptureVideoDlg *pDlg;
}THREADPARAM;

THREADPARAM threadParam;
LPDIRECT3DDEVICE9       pD3DDevice = NULL;	// Our rendering device
Camera g_Camera(Camera::LANDOBJECT);
extern Terrain* g_pTerrain;


CCaptureVideoDlg::CCaptureVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureVideoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pData=NULL;
	m_hThread=NULL;
	m_isThreadActive=false;

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
	static float anglex=0.0f;
	static float angley=0.0f;
	static float zDistance=5.0f;
	static float xDistance=0.0f;
	static float xzAngle=0.0f;
	static float r=5.0f;

	const float conAngle=0.01f;	

	if (WM_KEYDOWN == pMsg->message)
	{
		float timeDelta=1.0f;
		float conDistance=5.0f;
		float camIncre=0.05f;
		if(m_isThreadActive)
		{
			DWORD result=SuspendThread(m_hThread);
			if(0xffffffff==result)
			{
				MessageBox("error SuspendThread");
				return FALSE;
			}
			m_isThreadActive=false;
		}
		switch(pMsg->wParam)
		{			
		case VK_UP:
//			r -= conDistance*timeDelta;
			g_Camera.walk(conDistance);
			TRACE("up is pressed.\n");
			break;
		case VK_DOWN:		
//			r += conDistance*timeDelta;
			g_Camera.walk(-conDistance);
			TRACE("down is pressed.\n");
			break;
		case VK_RIGHT:			
//			xzAngle += conAngle*timeDelta;
			g_Camera.strafe(conDistance);
			TRACE("right is pressed.\n");
			break;
		case VK_LEFT:
//			xzAngle -= conAngle*timeDelta;
			g_Camera.strafe(-conDistance);
			TRACE("left is pressed.\n");
			break;
		case 'W':
			anglex -= conAngle*timeDelta;
			TRACE("W is pressed.\n");
			break;
		case 'S':
			anglex += conAngle*timeDelta;
			TRACE("S is pressed.\n");
			break;
		case 'A':
			angley += conAngle*timeDelta;
			TRACE("A is pressed.\n");
			break;
		case 'D':
			angley -= conAngle*timeDelta;
			TRACE("D is pressed.\n");
			break;
		case 'I':
			g_Camera.pitch(-camIncre);
			TRACE("I is pressed.\n");
			break;
		case 'K':
			g_Camera.pitch(camIncre);
			TRACE("K is pressed.\n");
			break;
		case 'J':
			g_Camera.yaw(-camIncre);
			TRACE("J is pressed.\n");
			break;
		case 'L':
			g_Camera.yaw(camIncre);
			TRACE("L is pressed.\n");
			break;
		default:
			break;
		}
		D3DXVECTOR3 pos;
		g_Camera.getPosition(&pos);
		float height = g_pTerrain->getHeight( pos.x, pos.z );
		pos.y = height + 5.0f; // add height because we're standing up 
		g_Camera.setPosition(&pos);

		D3DXMATRIX V;
		g_Camera.getViewMatrix(&V);
		m_d3dRender.SetTransform(D3DTS_VIEW, &V);

		if(angley>=6.28f || angley<=-6.28){	angley=0.0f;}
		if(anglex>=6.28f || anglex<=-6.28){	anglex=0.0f;}
		if(xzAngle>=6.28 || xzAngle<=-6.28){xzAngle=0.0f;}

		xDistance=r*sinf(xzAngle);
		zDistance=r*cosf(xzAngle);

		m_d3dRender.SetWorld(anglex,angley);
		PostMessage(MY_THREADMSG);

		return true;
	}
	else if (WM_KEYUP==pMsg->message)
	{
		if(!m_isThreadActive)
		{
			DWORD result=ResumeThread(m_hThread);
			if(0xffffffff==result)
			{
				MessageBox("error ResumeThread");
				return FALSE;
			}
			m_isThreadActive=true;
		}
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

	m_show2.SetFocus();

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
// 	while(true)
// 	{
// 		SendMessage(pThreadParam->hwnd,MY_THREADMSG,0,0);
// 
// 		//Sleep(20);
// 	}
    //////////////////////////////////////////////////////////////////////////
	MSG mssg;                // message from queue
	LONGLONG cur_time;       // current time
	DWORD  time_count=40;    // ms per frame, default if no performance counter
	LONGLONG perf_cnt;       // performance timer frequency
	BOOL perf_flag=FALSE;    // flag determining which timer to use
	LONGLONG next_time=0;    // time to render next frame
	BOOL move_flag=TRUE;     // flag noting if we have moved yet

	// is there a performance counter available?

	if (QueryPerformanceFrequency((LARGE_INTEGER *) &perf_cnt)) {

		// yes, set time_count and timer choice flag

		perf_flag=TRUE;
		time_count=perf_cnt/25;        // calculate time per frame based on frequency
		QueryPerformanceCounter((LARGE_INTEGER *) &next_time);

	} else {

		// no performance counter, read in using timeGetTime

		next_time=timeGetTime();

	}

	while(TRUE)
	{
		// use the appropriate method to get time

		if (perf_flag)

			QueryPerformanceCounter((LARGE_INTEGER *) &cur_time);

		else

			cur_time=timeGetTime();

		// is it time to render the frame?

		if (cur_time>next_time) {

			// yes, render the frame

			{
				CCaptureVideoDlg* pDlg=pThreadParam->pDlg;
				pDlg->m_cap.GetSampleData(pDlg->m_pData);
				pDlg->m_imageHandle.ImageHandle(pDlg->m_pData);
				pDlg->m_d3dRender.DrawD3D(pDlg->m_pData);
			}

			// set time for next frame

			next_time += time_count;

			// If we get more than a frame ahead, allow us to drop one
			// Otherwise, we will never catch up if we let the error
			// accumulate, and message handling will suffer

			if (next_time < cur_time)
				next_time = cur_time + time_count;

			// flag that we need to move objects again

			move_flag=TRUE;

		}
	}

	//////////////////////////////////////////////////////////////////////////

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
			threadParam.pDlg=this;
			m_hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProc,(LPVOID)&threadParam,0,0);
			GetDlgItem(IDC_BUTTON2)->SetWindowText("图像处理 关闭");
			m_isThreadActive=true;
		}
	}
	else
	{
		TerminateThread(m_hThread,0);
		m_hThread=NULL;
		m_isThreadActive=false;
		GetDlgItem(IDC_BUTTON2)->SetWindowText("图像处理 开启");
	}
}


LRESULT CCaptureVideoDlg::OnMyThreadMsg(WPARAM wParam,LPARAM lParam)
{	
 	m_cap.GetSampleData(m_pData);
 	m_imageHandle.ImageHandle(m_pData);
	m_d3dRender.DrawD3D(m_pData);
	return S_OK;
}
