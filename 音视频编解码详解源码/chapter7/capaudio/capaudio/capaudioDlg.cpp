// capaudioDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "capaudio.h"
#include "capaudioDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const GUID CLSID_WavDest = {0x3C78B8E2,0x6C4D,0x11D1,{0xAD,0xE2,0x0,0x0,0xF8,0x75,0x4B,0x99}};

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


// CcapaudioDlg 对话框




CcapaudioDlg::CcapaudioDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CcapaudioDlg::IDD, pParent)
	, m_PathName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcapaudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PATH, m_PathName);
}

BEGIN_MESSAGE_MAP(CcapaudioDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_RECORD, &CcapaudioDlg::OnBnClickedRecord)
	
	ON_BN_CLICKED(ID_STOP, &CcapaudioDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BUTTON_FILE_NAME, &CcapaudioDlg::OnBnClickedButtonFileName)
//	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CcapaudioDlg 消息处理程序

BOOL CcapaudioDlg::OnInitDialog()
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
	//GetDlgItem(ID_RECORD)->EnableWindow(FALSE);
	//GetDlgItem(ID_STOP)->EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CcapaudioDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CcapaudioDlg::OnPaint()
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
HCURSOR CcapaudioDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CcapaudioDlg::OnBnClickedRecord()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strName;
	GetDlgItem(IDC_EDIT_PATH)->GetWindowText(strName);
	
	if(strName.IsEmpty())
	{
		MessageBox(_T("请先设置文件名!"));
		return;
	}
	GetDlgItem(ID_STOP)->EnableWindow(true);
	
	
	pGraph = NULL;
	pMC = NULL;
	pBuilder = NULL;

	CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,
							CLSCTX_INPROC_SERVER,
							IID_ICaptureGraphBuilder2,
							(void**)&pBuilder);

	CoCreateInstance(CLSID_FilterGraph, NULL, 
					CLSCTX_INPROC_SERVER, 
                    IID_IGraphBuilder, 
					(void **)&pGraph);

	pBuilder->SetFiltergraph(pGraph);

	pGraph->QueryInterface(IID_IMediaControl,(void**)&pMC);

	ICreateDevEnum *pDevEnum = NULL;
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
					CLSCTX_INPROC, 
					IID_ICreateDevEnum,
					(void **)&pDevEnum);

	IEnumMoniker *pClassEnum = NULL;
	pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pClassEnum, 0);
	ULONG cFetched;
	if (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)     
	{
		pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
		pMoniker->Release();      
	}
	pClassEnum->Release();

	CoCreateInstance(CLSID_WavDest, NULL, CLSCTX_ALL, 
                     IID_IBaseFilter, (void **)&pWaveDest);

	CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_ALL, 
                     IID_IBaseFilter, (void **)&pWriter);

	pGraph->AddFilter(pSrc,L"Wav");
	pGraph->AddFilter(pWaveDest,L"WavDest");
	pGraph->AddFilter(pWriter,L"FileWriter");

	pWriter->QueryInterface(IID_IFileSinkFilter2,(void**)&pSink);

	pSink->SetFileName(strName.AllocSysString(),NULL);

	IPin* pOutpin = FindPin(pSrc,PINDIR_OUTPUT);

	IPin* pInpin,*pOut;	
	
	pOut= FindPin(pWaveDest,PINDIR_OUTPUT);

	AM_MEDIA_TYPE type;
	type.majortype = MEDIATYPE_Stream;
	type.subtype =MEDIASUBTYPE_WAVE;
	type.formattype = FORMAT_None;
	type.bFixedSizeSamples = FALSE;
	type.bTemporalCompression = FALSE;
	type.pUnk = NULL;
	
	//查找滤波器引脚
	pInpin = FindPin(pWaveDest,PINDIR_INPUT);
	IPin* pInpin2= FindPin(pWriter,PINDIR_INPUT);	

	//连接滤波器的引脚
	pGraph->ConnectDirect(pOutpin,pInpin,NULL);
	pGraph->ConnectDirect(pOut,pInpin2,NULL);

	pMC->Run();
	GetDlgItem(ID_RECORD)->EnableWindow(false);
	GetDlgItem(ID_STOP)->EnableWindow(true);

}

IPin* CcapaudioDlg::FindPin(IBaseFilter *pFilter, PIN_DIRECTION dir)
{
	IEnumPins* pEnumPins;
	IPin* pOutpin;
	PIN_DIRECTION pDir;
	pFilter->EnumPins(&pEnumPins);

	while (pEnumPins->Next(1,&pOutpin,NULL)==S_OK)
	{
		pOutpin->QueryDirection(&pDir);
		if (pDir==dir)
		{
			return pOutpin;
		}
	}
	return 0;
}


void CcapaudioDlg::OnBnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	pMC->Stop();
	pMC->Release();
	pGraph->Release();
	pWaveDest->Release();
	pWriter->Release();


	MessageBox(_T("录制完成"));
	GetDlgItem(IDC_EDIT_PATH)->SetWindowText(_T(""));
	GetDlgItem(ID_STOP)->EnableWindow(false);
}

void CcapaudioDlg::OnBnClickedButtonFileName()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFilter = _T("Wav File (*.wav) | *.wav|");
	strFilter += "All File (*.*) | *.*|";
	CFileDialog dlg(FALSE, NULL, NULL, OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, strFilter, this);

	if (dlg.DoModal()==IDOK)
	{
		CString strName = dlg.GetPathName();
		strName += ".wav";
		this->m_PathName = strName;
		UpdateData(FALSE);
		GetDlgItem(ID_RECORD)->EnableWindow(TRUE);
		GetDlgItem(ID_STOP)->EnableWindow(FALSE);
	}
}

//void CcapaudioDlg::OnDestroy()
//{
//	CDialog::OnDestroy();
//
//
//	// TODO: 在此处添加消息处理程序代码
//}

void CcapaudioDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	OnBnClickedStop();
	CoUninitialize();
	CDialog::OnClose();
}
