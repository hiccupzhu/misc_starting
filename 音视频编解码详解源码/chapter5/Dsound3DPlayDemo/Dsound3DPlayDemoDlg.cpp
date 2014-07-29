// Dsound3DPlayDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <math.h>

#include "Dsound3DPlayDemo.h"
#include "Dsound3DPlayDemoDlg.h"
#include "waveFile.h"
#include ".\dsound3dplaydemodlg.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define   WAVECHANNEL   2
#define   WAVESAMPLEPERSEC  44100
#define   WAVEBITSPERSAMPLE 16
#define   MAX_AUDIO_BUF 3
#define   BUFFERNOTIFYSIZE  7680

#define IDT_MOVEMENT_TIMER      1
#define ORBIT_MAX_RADIUS        5.0f

LPDIRECTSOUND8          g_pDsd               = NULL;   //Directsound 对象指针
LPDIRECTSOUNDBUFFER     g_pDSBuffer     = NULL;        //辅助缓冲区指针
LPDIRECTSOUND3DBUFFER   g_pDS3DBuffer         = NULL;   // 3D 声源对象指针
LPDIRECTSOUND3DLISTENER g_pDSListener         = NULL;   // 3D 听者之中呢
DS3DBUFFER              g_dsBufferParams;               // 3D buffer properties
DS3DLISTENER            g_dsListenerParams;             // Listener properties
CWaveFile              *g_pWaveFile= NULL;              //读写wave文件用到的一个类对象
BOOL                   g_bPlaying = FALSE;              //是否正在播放


HBITMAP                 g_hGrid               = NULL;   // Grid Bitmap object
INT                     g_nGridW, g_nGridH;             // and dimensions


LPDIRECTSOUNDNOTIFY8       g_pDSNotify = NULL;	
DSBPOSITIONNOTIFY          g_aPosNotify[MAX_AUDIO_BUF];//设置通知标志的数组
HANDLE g_event[3];
DWORD g_dwNextWriteOffset = 0;

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


// CDsound3DPlayDemoDlg 对话框



CDsound3DPlayDemoDlg::CDsound3DPlayDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDsound3DPlayDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDsound3DPlayDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDsound3DPlayDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnSelectFile)
	ON_BN_CLICKED(IDC_PLAY, OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP, OnBnClickedStop)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDsound3DPlayDemoDlg 消息处理程序

BOOL CDsound3DPlayDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

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

    g_hGrid = LoadBitmap( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDB_BITMAP1 ) );
	  BITMAP bmp;
    GetObject( g_hGrid, sizeof(BITMAP), &bmp );
    g_nGridW = bmp.bmWidth;
    g_nGridH = bmp.bmHeight;
    //初始化Dsound
	HRESULT hr;
	if(FAILED(hr = DirectSoundCreate8(NULL,&g_pDsd,NULL)))
		return FALSE;
	if(FAILED(hr = g_pDsd->SetCooperativeLevel(m_hWnd,DSSCL_PRIORITY)))
		return FALSE;
    //初始化Directsound 的主缓冲区，并设置格式
	LPDIRECTSOUNDBUFFER  pDSBPrimary = NULL;
    DSBUFFERDESC   dsbdesc ;
	ZeroMemory(&dsbdesc,sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	if(FAILED(hr = g_pDsd->CreateSoundBuffer(&dsbdesc,&pDSBPrimary ,NULL)))
		return FALSE;
	
   if( FAILED( hr = pDSBPrimary->QueryInterface( IID_IDirectSound3DListener, (VOID**)&g_pDSListener ) ) )
	   return FALSE;

    WAVEFORMATEX wfx;
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM; 
    wfx.nChannels       = WAVECHANNEL   ; 
    wfx.nSamplesPerSec  = WAVESAMPLEPERSEC  ; 
    wfx.wBitsPerSample  = WAVEBITSPERSAMPLE ; 
    wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
    wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);
    if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) )
        return FALSE;


	//
    for(int i =0; i< MAX_AUDIO_BUF; i++)
		g_event[i] = CreateEvent(NULL, FALSE, FALSE, NULL);


	 CSliderCtrl *pDopplerSlider = (CSliderCtrl*)GetDlgItem(IDC_DOPPLER_SLIDER);
     pDopplerSlider->SetRange(0.0f,10.0f);
	 pDopplerSlider->SetPos(1);
	 
	 CSliderCtrl *pRolloffSlider= (CSliderCtrl*)GetDlgItem(IDC_ROLLOFF_SLIDER);
	 pRolloffSlider->SetRange(0.0f,10.0f);
	 pRolloffSlider->SetPos(1);

	 CSliderCtrl *pMinDistSlider  = (CSliderCtrl*)GetDlgItem(IDC_MINDISTANCE_SLIDER );
	 pMinDistSlider->SetRange(1,40);
	 pMinDistSlider->SetPos(4);
	 CSliderCtrl *pMaxDistSlider  = (CSliderCtrl*)GetDlgItem(IDC_MAXDISTANCE_SLIDER );
	 pMaxDistSlider->SetRange(1,40);
	 pMaxDistSlider->SetPos(4);

	 CSliderCtrl *pVerslider = (CSliderCtrl*)GetDlgItem(IDC_VERTICAL_SLIDER );
	 pVerslider->SetRange(-100,100);
	 pVerslider->SetPos(100);
	 CSliderCtrl *pHorslider = (CSliderCtrl*)GetDlgItem(IDC_HORIZONTAL_SLIDER );
	 pHorslider ->SetRange(-100,100);
	 pHorslider ->SetPos(100);


	 SetTimer( IDT_MOVEMENT_TIMER, 0, NULL ); 

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CDsound3DPlayDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDsound3DPlayDemoDlg::OnPaint() 
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
HCURSOR CDsound3DPlayDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

INT_PTR CALLBACK AlgorithmDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	// Default is DS3DALG_NO_VIRTUALIZATION for fastest performance
	static int nDefaultRadio = IDC_NO_VIRT_RADIO;

	switch( msg ) 
	{
	case WM_INITDIALOG:
		// Default is DS3DALG_NO_VIRTUALIZATION for fastest performance
		CheckRadioButton( hDlg, IDC_NO_VIRT_RADIO, IDC_LIGHT_VIRT_RADIO, nDefaultRadio );
		return TRUE; // Message handled 

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDCANCEL:
			EndDialog( hDlg, -1 );
			return TRUE; // Message handled 

		case IDOK:
			if( IsDlgButtonChecked( hDlg, IDC_NO_VIRT_RADIO )    == BST_CHECKED )
			{
				nDefaultRadio = IDC_NO_VIRT_RADIO;
				EndDialog( hDlg, 0 );
			}

			if( IsDlgButtonChecked( hDlg, IDC_HIGH_VIRT_RADIO )  == BST_CHECKED )
			{               
				nDefaultRadio = IDC_HIGH_VIRT_RADIO;
				EndDialog( hDlg, 1 );
			}

			if( IsDlgButtonChecked( hDlg, IDC_LIGHT_VIRT_RADIO ) == BST_CHECKED )
			{               
				nDefaultRadio = IDC_LIGHT_VIRT_RADIO;
				EndDialog( hDlg, 2 );
			}

			return TRUE; // Message handled 
		}
		break;
	}

	return FALSE; // Message not handled 
}
void CDsound3DPlayDemoDlg::OnTimer(UINT nIDEvent )
{
	FLOAT fXScale;
	FLOAT fYScale;

	fXScale = (((CSliderCtrl*)GetDlgItem(IDC_HORIZONTAL_SLIDER ))->GetPos())/100.0f;

	fYScale = (((CSliderCtrl*)GetDlgItem(IDC_VERTICAL_SLIDER ))->GetPos())/100.0f;
	FLOAT t = timeGetTime()/1000.0f;

	// Move the sound object around the listener. The maximum radius of the
	// orbit is 27.5 units.
	D3DVECTOR vPosition;
	vPosition.x = ORBIT_MAX_RADIUS * fXScale * (FLOAT)sin(t);
	vPosition.y = 0.0f;
	vPosition.z = ORBIT_MAX_RADIUS * fYScale * (FLOAT)cos(t);

	D3DVECTOR vVelocity;
	vVelocity.x = ORBIT_MAX_RADIUS * fXScale * (FLOAT)sin(t+0.05f);
	vVelocity.y = 0.0f;
	vVelocity.z = ORBIT_MAX_RADIUS * fYScale * (FLOAT)cos(t+0.05f); 

	memcpy( &g_dsBufferParams.vPosition, &vPosition, sizeof(D3DVECTOR) );
	memcpy( &g_dsBufferParams.vVelocity, &vVelocity, sizeof(D3DVECTOR) );
	
	UpdateGrid(  vPosition.x, vPosition.z );

	if( g_pDS3DBuffer )
		g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, DS3D_IMMEDIATE );

}
void CDsound3DPlayDemoDlg::UpdateGrid(  FLOAT x, FLOAT y )
{
  static LONG s_lX = 0;
    static LONG s_lY = 0;

    // Don't update the grid if a WM_PAINT will be called soon
	if( ::GetUpdateRect( m_hWnd,NULL,FALSE ) )
        return;

    // Get access to the needed device contexts
	HWND hWndGrid = ::GetDlgItem( m_hWnd, IDC_RENDER_WINDOW ); 
	HDC  hDC      = ::GetDC( hWndGrid );
	HDC  hDCbmp   = ::CreateCompatibleDC( hDC );
    
    // Select the grid bitmap into the off-screen DC
	::SelectObject( hDCbmp, g_hGrid );

    // Restrict painting to the grid area
	::IntersectClipRect( hDC, 0, 0, g_nGridW, g_nGridH );

    // Restore the area of the grid previously drawn upon
    BitBlt( hDC, s_lX-1, s_lY-1, 3, 3, hDCbmp, s_lX-1, s_lY-1, SRCCOPY ); 

    // Convert the world space x,y coordinates to pixel coordinates
    RECT rc;
	::GetClientRect( hWndGrid, &rc );
    s_lX = (LONG)( ( x/ORBIT_MAX_RADIUS + 1 ) * ( rc.left + rc.right ) / 2 );
    s_lY = (LONG)( (-y/ORBIT_MAX_RADIUS + 1 ) * ( rc.top + rc.bottom ) / 2 );

    // Draw a crosshair object in red pixels
    SetPixel( hDC, s_lX-1, s_lY+0, 0x000000ff );
    SetPixel( hDC, s_lX+0, s_lY-1, 0x000000ff );
    SetPixel( hDC, s_lX+0, s_lY+0, 0x000000ff );
    SetPixel( hDC, s_lX+0, s_lY+1, 0x000000ff );
    SetPixel( hDC, s_lX+1, s_lY+0, 0x000000ff );

	::ReleaseDC( hWndGrid, hDC );
	::DeleteDC( hDCbmp );
}


void CDsound3DPlayDemoDlg::SafeRealse()
{
   if(g_pWaveFile)
   {
	   g_pWaveFile->Close();
	   delete g_pWaveFile;
	   g_pWaveFile = NULL;
   }
   if(g_pDSBuffer)
   {
	   g_pDSBuffer->Release();
	   g_pDSBuffer = NULL;
   }

   if(g_pDS3DBuffer)
   {
	   g_pDS3DBuffer->Release();
	   g_pDS3DBuffer = NULL;
   }
}
//选择文件
void CDsound3DPlayDemoDlg::OnSelectFile()
{

	GUID  guid3DAlgorithm  =GUID_NULL;
	int  nResult;
	HRESULT hr;
	static char strFileName[MAX_PATH] ="";
	static char strPath[MAX_PATH] = "";

	OPENFILENAME ofn = { sizeof(OPENFILENAME), m_hWnd, NULL,
		TEXT("Wave Files\0*.wav\0All Files\0*.*\0\0"), NULL,
		0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
		TEXT("Open Sound File"),
		OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
		TEXT(".wav"), 0, NULL, NULL };
	if(strPath[0] == '\0')
	{
		if( strcmp( &strPath[strlen(strPath)], TEXT("\\") ) )
			strcat( strPath, TEXT("\\") );
		strcat( strPath, TEXT("MEDIA") );

	}

	if(GetOpenFileName(&ofn) != TRUE)
		return ;

	//如果正在播放，停止 
	g_bPlaying = FALSE; 
	SafeRealse(); 

	g_pWaveFile = new CWaveFile;
	g_pWaveFile->Open(strFileName,NULL,WAVEFILE_READ);
	WAVEFORMATEX* pwfx  = g_pWaveFile->GetFormat();
	if(pwfx == NULL)
		return ;
	if(pwfx->nChannels > 1)
	{
		AfxMessageBox("only sourport wave file with one channel ");
		return ;
	}
	if( pwfx->wFormatTag != WAVE_FORMAT_PCM )
	{
         AfxMessageBox("not wave file ");
		 return;
	}

	GetDlgItem(IDC_EDIT1)->SetWindowText(strFileName);

	nResult = (int)DialogBox( NULL, MAKEINTRESOURCE(IDD_3D_ALGORITHM), 
		NULL, AlgorithmDlgProc );
	switch( nResult )
	{
	case -1: // User canceled dialog box 
		return;

	case 0: // User selected DS3DALG_NO_VIRTUALIZATION  
		guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
		break;

	case 1: // User selected DS3DALG_HRTF_FULL  
		guid3DAlgorithm = DS3DALG_HRTF_FULL;
		break;

	case 2: // User selected DS3DALG_HRTF_LIGHT
		guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
		break;
	}

	///创建辅助缓冲区，通过辅助缓冲区获取3dbuffer指针
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize          = sizeof(DSBUFFERDESC);

	dsbd.dwFlags         = DSBCAPS_CTRL3D| DSBCAPS_GLOBALFOCUS | 	DSBCAPS_CTRLPOSITIONNOTIFY |DSBCAPS_GETCURRENTPOSITION2;
	//dsbd.dwBufferBytes   =MAX_AUDIO_BUF * BUFFERNOTIFYSIZE  ;//如果采用流buffer，请将此句放开
	dsbd.dwBufferBytes   =g_pWaveFile->GetSize(); // 如果采用流buffer，请屏蔽掉此句
	dsbd.guid3DAlgorithm = guid3DAlgorithm;
	dsbd.lpwfxFormat     = g_pWaveFile->m_pwfx;
	if(FAILED(hr = g_pDsd->CreateSoundBuffer(&dsbd,&g_pDSBuffer,NULL)))
		return ;
	for(int i =0; i< MAX_AUDIO_BUF;i++)
	{
        g_aPosNotify[i].dwOffset = i* BUFFERNOTIFYSIZE  ;
		g_aPosNotify[i].hEventNotify = g_event[i];
	}

	if(FAILED(hr = g_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer, (VOID**)&g_pDS3DBuffer )))
		return ;
    
	g_dsBufferParams.dwSize = sizeof(DS3DBUFFER);
	g_pDS3DBuffer->GetAllParameters( &g_dsBufferParams );

	// Set new 3D buffer parameters
	g_dsBufferParams.dwMode = DS3DMODE_HEADRELATIVE;
	g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, DS3D_IMMEDIATE );

    if(FAILED(hr = g_pDSBuffer->QueryInterface(IID_IDirectSoundNotify,(LPVOID *) &g_pDSNotify )))
		return ;
	g_pDSNotify->SetNotificationPositions(MAX_AUDIO_BUF,g_aPosNotify);
	g_pDSNotify->Release();

	FLOAT fDopplerFactor;
	FLOAT fRolloffFactor;
	FLOAT fMinDistance; 
	FLOAT fMaxDistance;

	CSliderCtrl *pDopplerSlider = (CSliderCtrl*)GetDlgItem(IDC_DOPPLER_SLIDER); 
	fDopplerFactor = pDopplerSlider->GetPos();

	CSliderCtrl *pRolloffSlider= (CSliderCtrl*)GetDlgItem(IDC_ROLLOFF_SLIDER); 
	fRolloffFactor = pRolloffSlider->GetPos();

	CSliderCtrl *pMinDistSlider  = (CSliderCtrl*)GetDlgItem(IDC_MINDISTANCE_SLIDER ); 
	fMinDistance = pMinDistSlider->GetPos();
	CSliderCtrl *pMaxDistSlider  = (CSliderCtrl*)GetDlgItem(IDC_MAXDISTANCE_SLIDER ); 
	fMaxDistance = pMaxDistSlider->GetPos();

	g_dsListenerParams.flDopplerFactor = fDopplerFactor;
	g_dsListenerParams.flRolloffFactor = fRolloffFactor;

	if( g_pDSListener )
	{
		g_pDSListener->SetAllParameters( &g_dsListenerParams, DS3D_DEFERRED );
	    g_pDSListener->CommitDeferredSettings();
	}

	g_dsBufferParams.flMinDistance = fMinDistance;
	g_dsBufferParams.flMaxDistance = fMaxDistance;

	if( g_pDS3DBuffer )
		g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, DS3D_DEFERRED );

	GetDlgItem( IDC_PLAY )->EnableWindow( TRUE );
	GetDlgItem( IDC_STOP )->EnableWindow( FALSE );

	
     
}

void ProcessBuffer()
{
	DWORD   dwBytesWrittenToBuffer = 0;

	VOID*   pDSLockedBuffer = NULL;
	VOID*   pDSLockedBuffer2 = NULL;
	DWORD   dwDSLockedBufferSize;
	DWORD   dwDSLockedBufferSize2;
	HRESULT hr; 
    
	hr = g_pDSBuffer->Lock(g_dwNextWriteOffset,BUFFERNOTIFYSIZE,&pDSLockedBuffer,&dwDSLockedBufferSize,
		      &pDSLockedBuffer2,&dwDSLockedBufferSize2,0);
	if(hr == DSERR_BUFFERLOST)
	{
		g_pDSBuffer->Restore();
		g_pDSBuffer->Lock(g_dwNextWriteOffset,BUFFERNOTIFYSIZE,&pDSLockedBuffer,&dwDSLockedBufferSize,
			                &pDSLockedBuffer2,&dwDSLockedBufferSize2,0);
	}
	if(SUCCEEDED(hr))
	{//write
		g_pWaveFile->Read((BYTE*)pDSLockedBuffer,dwDSLockedBufferSize,&dwBytesWrittenToBuffer);
        g_dwNextWriteOffset += dwBytesWrittenToBuffer;

		if (NULL != pDSLockedBuffer2) 
		{ 
			g_pWaveFile->Read((BYTE*)pDSLockedBuffer2,dwDSLockedBufferSize2,&dwBytesWrittenToBuffer);
			g_dwNextWriteOffset += dwBytesWrittenToBuffer;
			
		} 
		g_dwNextWriteOffset %= (BUFFERNOTIFYSIZE * MAX_AUDIO_BUF);

		if(dwBytesWrittenToBuffer <BUFFERNOTIFYSIZE )
		{

			FillMemory( (BYTE*) pDSLockedBuffer + dwBytesWrittenToBuffer, 
				       BUFFERNOTIFYSIZE - dwBytesWrittenToBuffer, 
				       (BYTE)(g_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
		  
          g_bPlaying = FALSE; 

    	}
		hr = g_pDSBuffer->Unlock(pDSLockedBuffer,dwDSLockedBufferSize,
			                  pDSLockedBuffer2,dwDSLockedBufferSize2);

	}
	
}
DWORD WINAPI PlayThread(LPVOID lpParame)
{
	DWORD res;
    LPVOID  lplockbuf;
	DWORD   len;
	DWORD  dwWrite;
	CDsound3DPlayDemoDlg * pDlg = (CDsound3DPlayDemoDlg*)lpParame;
	g_pDSBuffer->Lock(0,0,&lplockbuf,&len,NULL,NULL,DSBLOCK_ENTIREBUFFER);
	g_pWaveFile->Read((BYTE*)lplockbuf,len,&dwWrite);
	g_pDSBuffer->Unlock(lplockbuf,len,NULL,0);
	g_pDSBuffer->SetCurrentPosition(0);
	g_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
	g_dwNextWriteOffset = 0;
	while(g_bPlaying)
	{
        res = WaitForMultipleObjects (MAX_AUDIO_BUF, g_event, FALSE, INFINITE);
		if(res > WAIT_OBJECT_0) 
		  ProcessBuffer();
	}

	pDlg->GetDlgItem( IDC_PLAY )->EnableWindow( TRUE );
	pDlg->GetDlgItem( IDC_STOP )->EnableWindow( FALSE );

	return 0;
}
//开始播放
void CDsound3DPlayDemoDlg::OnBnClickedPlay()
{
	g_bPlaying =TRUE;
 //set d3d buffer param
	FLOAT fDopplerFactor;
	FLOAT fRolloffFactor;
	FLOAT fMinDistance; 
	FLOAT fMaxDistance;

	CSliderCtrl *pDopplerSlider = (CSliderCtrl*)GetDlgItem(IDC_DOPPLER_SLIDER); 
	fDopplerFactor = pDopplerSlider->GetPos();

	CSliderCtrl *pRolloffSlider= (CSliderCtrl*)GetDlgItem(IDC_ROLLOFF_SLIDER); 
	fRolloffFactor = pRolloffSlider->GetPos();

	CSliderCtrl *pMinDistSlider  = (CSliderCtrl*)GetDlgItem(IDC_MINDISTANCE_SLIDER ); 
	fMinDistance = pMinDistSlider->GetPos();
	CSliderCtrl *pMaxDistSlider  = (CSliderCtrl*)GetDlgItem(IDC_MAXDISTANCE_SLIDER ); 
	fMaxDistance = pMaxDistSlider->GetPos();

	g_dsListenerParams.flDopplerFactor = fDopplerFactor;
	g_dsListenerParams.flRolloffFactor = fRolloffFactor;

	if( g_pDSListener )
	{
		g_pDSListener->SetAllParameters( &g_dsListenerParams, DS3D_DEFERRED );
		g_pDSListener->CommitDeferredSettings();
	}

	g_dsBufferParams.flMinDistance = fMinDistance;
	g_dsBufferParams.flMaxDistance = fMaxDistance;

	if( g_pDS3DBuffer )
		g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, DS3D_DEFERRED );

	GetDlgItem( IDC_PLAY )->EnableWindow( FALSE );
	GetDlgItem( IDC_STOP )->EnableWindow( TRUE );
  //  CreateThread(0,0,PlayThread,this,NULL,NULL); //如果采用流 buffer，请将此句放开，将下面的句子屏蔽

	  DWORD res;
    LPVOID  lplockbuf;
	DWORD   len;
	DWORD  dwWrite;
	
	g_pDSBuffer->Lock(0,0,&lplockbuf,&len,NULL,NULL,DSBLOCK_ENTIREBUFFER);
	g_pWaveFile->Read((BYTE*)lplockbuf,len,&dwWrite);
	g_pDSBuffer->Unlock(lplockbuf,len,NULL,0);
	g_pDSBuffer->SetCurrentPosition(0);
	g_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
}
//停止
void CDsound3DPlayDemoDlg::OnBnClickedStop()
{
     g_bPlaying =FALSE;
	 Sleep(500);
	 g_pDSBuffer->Stop();
	 
	GetDlgItem( IDC_PLAY )->EnableWindow( TRUE);
	GetDlgItem( IDC_STOP )->EnableWindow( FALSE );
}
//退出
void CDsound3DPlayDemoDlg::OnBnClickedOk()
{
	SafeRealse();
	if(g_pDSListener)
		g_pDSListener->Release();

	if(g_pDsd)
		g_pDsd->Release();
	OnOK();
}
