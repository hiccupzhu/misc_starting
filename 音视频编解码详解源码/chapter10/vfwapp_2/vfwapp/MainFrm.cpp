// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "vfwapp.h"

#include "MainFrm.h"
#include "xvid_codec.h"
//#include "convert.h"

//#pragma comment(lib,"cscc.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#define XDIM 704				//图像宽度，预设的足够大
#define YDIM 576				//图像高度，预设的足够大

unsigned char bufo[XDIM*YDIM*3+40];//预显示的图像，RGB格式

//the parameters used by VCM
HIC             hic1;			//编码器句柄
HIC             hic2;			//解码器句柄
LPBITMAPINFO    lpbiIn;			//输入格式
LPBITMAPINFO	lpbiTmp;		//the output format of compressor
LPBITMAPINFO    lpbiOut;		//输出格式
COMPVARS        pc;				//编码设置结构体
BOOL            IsKeyFrame;		//
BOOL			bSaveAVI=FALSE;	//
long            FrameSize;		//
int				CurrentID;		//
//UINT			uiVideoX=0;		//显示图像的宽度
//UINT			uiVideoY=0;		//显示图像的高度
BOOL			bPreview=FALSE;	//
int				nStreamLength;	//
int				nOstreamSize;	//

CFrameWnd	m_wndSource;		//创建的新窗口
HWND		m_hWndCap;			//VFW设备窗口
CRect		disRect;			//显示窗口的客户区域
CMainFrame *pMainFrame=NULL;	//MainFrame 指针
enum		VFW_STATE{PREVIEW,ENCDEC};
VFW_STATE	m_vfwState = PREVIEW;

///////////////////////////////////////////////////////////////////////////////////////////////////


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_VFW_INITVFW, &CMainFrame::OnVfwInitvfw)
	ON_COMMAND(ID_VFW_VIDEOFORMAT, &CMainFrame::OnVfwVideoformat)
	ON_WM_CLOSE()
	ON_COMMAND(ID_VFW_PREVIEWVIDEO, &CMainFrame::OnVfwPreviewvideo)
	ON_COMMAND(ID_VFW_CODEC, &CMainFrame::OnVfwCodec)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO: 在此添加成员初始化代码
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//不需要状态栏和工具栏
#if 0
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("未能创建工具栏\n");
		return -1;      // 未能创建
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}

	// TODO: 如果不需要工具栏可停靠，则删除这三行
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
#endif
	
	lpbiIn  = new BITMAPINFO;  
	lpbiTmp = new BITMAPINFO;
	lpbiOut = new BITMAPINFO; 
	pMainFrame=this;
	::GetClientRect(m_wndSource.m_hWnd,&disRect);
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	cs.cx = 352+8;
	cs.cy = 288+54;
	cs.lpszName=_T("VFW采集");
	cs.style&=~FWS_ADDTOTITLE;

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


/****************************************************************************
 *	VFW callback routines
 ***************************************************************************/

// Function name	: EXPORT ErrorCallbackProc
// Description	    : 
// Return type		: LRESULT CALLBACK 
// Argument         : HWND hWnd
// Argument         : int nErrID
// Argument         : LPSTR lpErrorText
LRESULT CALLBACK EXPORT ErrorCallbackProc(HWND hWnd, int nErrID, LPSTR lpErrorText)
{
	if(nErrID==0)
		return TRUE;
	MessageBox(NULL,(LPCWSTR)lpErrorText,_T(""),MB_OK);
	return TRUE;
}


// Function name	: PASCAL StatusCallbackProc
// Description	    : 
// Return type		: LRESULT FAR 
// Argument         : HWND hWnd
// Argument         : int nID
// Argument         : LPSTR lpStatusText
LRESULT FAR PASCAL StatusCallbackProc(HWND hWnd, int nID, LPSTR lpStatusText)
{
	if(nID==IDS_CAP_END){
		if((CurrentID==IDS_CAP_STAT_VIDEOAUDIO)||(CurrentID==IDS_CAP_STAT_VIDEOONLY))
			return TRUE;
	}
	CurrentID = nID;
	return (LRESULT) TRUE;
}


// Function name	: PASCAL VideoCallbackProc
// Description	    : Encode the captured frame
// Return type		: LRESULT FAR 
// Argument         : HWND hWnd
// Argument         : LPVIDEOHDR lpVHdr
LRESULT FAR PASCAL VideoCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	unsigned char *bufi, *buf;
	int type=0;
	int quant=0;
	int declen=0;
	int enclen=0;
	bufi = new unsigned char[lpVHdr->dwBytesUsed+40];	//original image
	buf = new unsigned char[lpVHdr->dwBytesUsed];		//coded stream

	memcpy((void *)(bufi), lpVHdr->lpData, lpVHdr->dwBytesUsed);
	
	unsigned char *buf1;
	buf1 = buf;
	

	if (m_vfwState==ENCDEC) {
   //Encode
		buf1 = (unsigned char*)ICSeqCompressFrame(&pc,0,bufi, &IsKeyFrame,&FrameSize);
		//enc_main(bufi, buf, (int *)&FrameSize, &IsKeyFrame, -1);
		////////////////////////////////
		if (bSaveAVI){
			AVIStreamSetFormat(pMainFrame->ps,pMainFrame->m_Frame++,lpbiTmp,sizeof(BITMAPINFO));
			AVIStreamWrite(pMainFrame->ps,pMainFrame->m_Frame, 1, (LPBYTE)buf1,
						 lpbiTmp->bmiHeader.biSizeImage,AVIIF_KEYFRAME,NULL,NULL);
		}
		////////////////////////////////
   //Decode
		ICDecompress(hic2,0,&lpbiTmp->bmiHeader,buf1,&lpbiOut->bmiHeader,&bufo[40]);
	} else {
		enc_main(bufi,buf, &IsKeyFrame,&type,&quant,&enclen);
		declen = dec_main(buf, bufi, enclen,lpbiIn->bmiHeader.biWidth);
		pMainFrame->conv.YV12_to_RGB24(bufi,
									   bufi+(lpbiIn->bmiHeader.biWidth*lpbiIn->bmiHeader.biHeight),
									   bufi+(lpbiIn->bmiHeader.biWidth*lpbiIn->bmiHeader.biHeight*5/4),
									   &bufo[40],
									   lpbiIn->bmiHeader.biWidth,
									   lpbiIn->bmiHeader.biHeight);
	}

	pMainFrame->GetActiveView()->InvalidateRect(NULL,FALSE);											

	delete bufi;
	delete buf;

	return (LRESULT) TRUE;
}


// CMainFrame 消息处理程序

/*初始化VFW设备*/ 
void CMainFrame::OnVfwInitvfw()
{
	// TODO: 在此添加命令处理程序代码
	DWORD fsize;

	// 创建视频窗口
	if(!m_wndSource.CreateEx(WS_EX_TOPMOST,NULL,
							_T("Source"),WS_OVERLAPPED|WS_CAPTION,
							CRect(0,0,352,288),NULL,0))
		return;
	
	m_hWndCap = capCreateCaptureWindow(_T("Capture Window"),WS_CHILD|WS_VISIBLE,
									  0,0,352,288,
									  m_wndSource.m_hWnd,0);

	//m_wndSource.ShowWindow(SW_SHOW);
	// 注册回调函数
	capSetCallbackOnError(m_hWndCap,(FARPROC)ErrorCallbackProc);
	capSetCallbackOnStatus(m_hWndCap,(FARPROC)StatusCallbackProc);
	capSetCallbackOnVideoStream(m_hWndCap,(FARPROC)VideoCallbackProc);

	// 连接视频设备
	capDriverConnect(m_hWndCap,0);	//(HWND m_hWndCap, int index);//index : 0--9
	// 获取驱动器的性能参数
	capDriverGetCaps(m_hWndCap,&m_caps,sizeof(CAPDRIVERCAPS));
	if (m_caps.fHasOverlay)
		capOverlay(m_hWndCap,TRUE);
	// 设置预览速率开始预览
	capPreviewRate(m_hWndCap,1000/25);
	capPreview(m_hWndCap,bPreview);


	fsize = capGetVideoFormatSize(m_hWndCap);
	capGetVideoFormat(m_hWndCap, lpbiIn, fsize);
	
	AfxMessageBox(_T("初始化成功！"));
}

/*配置XVID CODEC视频编解码算法*/
void CMainFrame::Config_XVIECODEC()
{
	////////////////////////////////////////////////////////////////
	ENC_PARAM m_Param;
	m_Param.width = lpbiIn->bmiHeader.biWidth;	//视频图像宽度
	m_Param.height= lpbiIn->bmiHeader.biHeight;	//视频图像高度
	m_Param.bitrate = 800000;					//码流大小，使用码流控制
	m_Param.max_key_interval = 100;				//关键帧间隔
	m_Param.framerate = 25;						//帧率
	m_Param.quant = 0;							//量化步长，0表示使用码流控制

	enc_init(&m_Param);							//创建xvid codec编码器
	dec_init(m_Param.width,m_Param.height);		//创建xvid codec解码器
	////////////////////////////////////////////////////////////////
}

/* 配置视频格式：分辨率和视频格式：RGB/I420 */
void CMainFrame::OnVfwVideoformat()
{
	// TODO: 在此添加命令处理程序代码
	DWORD fsize;
	if(m_caps.fHasDlgVideoFormat){
		capDlgVideoFormat(m_hWndCap);
		fsize = capGetVideoFormatSize(m_hWndCap);
		capGetVideoFormat(m_hWndCap, lpbiIn, fsize);
		Config_XVIECODEC();
	}
}

void CMainFrame::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	capCaptureAbort(m_hWndCap);
	capDriverDisconnect(m_hWndCap);
	Sleep(100);
	capSetCallbackOnError(m_hWndCap,NULL);
	capSetCallbackOnStatus(m_hWndCap,NULL);
	capSetCallbackOnVideoStream(m_hWndCap,NULL);
	delete lpbiIn;
	delete lpbiTmp;
	delete lpbiOut;
	if (m_vfwState==ENCDEC){
		ICDecompressEnd(hic2);
		ICClose(hic2);
		ICSeqCompressFrameEnd(&pc);
		ICCompressEnd(hic1);
		ICClose(hic1);
		AVIStreamClose(ps);
		if(m_pFile != NULL)
			AVIFileRelease(m_pFile);
	}

	enc_stop();
	dec_stop();

	Sleep(100);
	CFrameWnd::OnClose();
}

/* 开始捕获预览视频 */
void CMainFrame::OnVfwPreviewvideo()
{
	// TODO: 在此添加命令处理程序代码
	CAPTUREPARMS CapParms;

	bPreview =! bPreview;
	if(bPreview){
		capCaptureGetSetup(m_hWndCap,&CapParms,sizeof(CAPTUREPARMS));
		CapParms.dwIndexSize=324000;
		CapParms.fMakeUserHitOKToCapture=!CapParms.fMCIControl;
		CapParms.wPercentDropForError=100;
		CapParms.wNumVideoRequested=5;
		CapParms.wChunkGranularity=0;
		CapParms.fYield=TRUE;
		CapParms.fCaptureAudio=FALSE;
		CapParms.vKeyAbort=0;
		CapParms.fAbortLeftMouse=CapParms.fAbortRightMouse=FALSE;
		CapParms.dwRequestMicroSecPerFrame=1000000/25;
		capSetCallbackOnYield(m_hWndCap,NULL);
		capCaptureSetSetup(m_hWndCap,&CapParms,sizeof(CAPTUREPARMS));

		capCaptureSequenceNoFile(m_hWndCap);
		m_vfwState  = PREVIEW;
	}else{
		capCaptureAbort(m_hWndCap);
	}
}

/* 对捕获的视频做编码、解码处理 */
void CMainFrame::OnVfwCodec()
{
	// TODO: 在此添加命令处理程序代码
	DWORD fsize;
	/* VCM initialization */
	hic1 = ICOpen(mmioFOURCC('v','i','d','c'),  mmioFOURCC('X','V','I','D'),  ICMODE_COMPRESS);
	if (hic1 == 0) 
		AfxMessageBox(_T("打开编码器失败!"));

	hic2 = ICOpen(mmioFOURCC('v','i','d','c'),  mmioFOURCC('X','V','I','D'),  ICMODE_DECOMPRESS);
	if (hic2 == 0) 
		AfxMessageBox(_T("打开解码器失败!"));

	fsize = capGetVideoFormatSize(m_hWndCap);
	capGetVideoFormat(m_hWndCap, lpbiIn, fsize);
	
	InitAVIWriteOpt();

	lpbiOut->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	lpbiOut->bmiHeader.biWidth         = lpbiIn->bmiHeader.biWidth;
	lpbiOut->bmiHeader.biHeight        = lpbiIn->bmiHeader.biHeight;
	lpbiOut->bmiHeader.biPlanes        = 1;
	lpbiOut->bmiHeader.biBitCount      = 24;
	lpbiOut->bmiHeader.biCompression   = BI_RGB;
	lpbiOut->bmiHeader.biSizeImage     = lpbiIn->bmiHeader.biWidth*lpbiIn->bmiHeader.biHeight*3;
	lpbiOut->bmiHeader.biXPelsPerMeter = 0;
	lpbiOut->bmiHeader.biYPelsPerMeter = 0;
	lpbiOut->bmiHeader.biClrUsed       = 0;
	lpbiOut->bmiHeader.biClrImportant  = 0;

//	get the format of the input video
	if (ICCompressGetFormat(hic1,lpbiIn,lpbiTmp)!=ICERR_OK) 
		AfxMessageBox(_T("编码器不能读取输出格式！"));
	if (ICCompressQuery(hic1,lpbiIn,lpbiTmp) != ICERR_OK)   
		AfxMessageBox(_T("不能处理编码器输入输出格式！"));

//	set the parameters of the CODEC
	pc.cbSize         = sizeof(COMPVARS);			//结构体大小
	pc.dwFlags        = ICMF_COMPVARS_VALID;
	pc.hic            = hic1;						//编码器句柄
	pc.fccType        = mmioFOURCC('v','i','d','c');
	pc.fccHandler     = mmioFOURCC('X','V','I','D');
	pc.lpbiOut        = lpbiTmp;					//输出格式
	pc.lKey           = 100;						//key帧频率
	pc.lQ             = 10000;						//图像质量

	if(!ICSeqCompressFrameStart(&pc, lpbiIn))
		return;
	ICDecompressBegin(hic2,lpbiTmp,lpbiOut);
	m_vfwState  = ENCDEC;
}

void CMainFrame::InitAVIWriteOpt()
{
	CString filename;
	CFileDialog FileDlg(FALSE,_T("avi"));
	if (FileDlg.DoModal()==IDOK)
	{
		filename = FileDlg.GetPathName();
		//capGetVideoFormat(m_hWndCap,&m_InInfo,sizeof(m_InInfo));
		m_Frame = 0 ;
		//AVI文件初始化
		AVIFileInit() ;
		bSaveAVI = TRUE;
		
		//打开文件
		AVIFileOpen(&m_pFile,filename,OF_WRITE | OF_CREATE,NULL);
		memset(&strhdr, 0, sizeof(strhdr)) ;
		strhdr.fccType    = streamtypeVIDEO; 
		strhdr.fccHandler = 0  ;
		strhdr.dwScale    = 1  ;
		strhdr.dwRate     = 25 ; 
		strhdr.dwSuggestedBufferSize = lpbiIn->bmiHeader.biSizeImage;
		SetRect(&strhdr.rcFrame, 0, 0, lpbiIn->bmiHeader.biWidth, lpbiIn->bmiHeader.biHeight);
		ps = NULL;
		//文件文件流
		AVIFileCreateStream(m_pFile,&ps,&strhdr); 
		
		//开始捕捉
		capCaptureSequenceNoFile(m_hWndCap);
	}
}