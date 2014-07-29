// RecorderDlg.cpp : implementation file
//

#include "stdafx.h"


#include "Recorder.h"
#include "RecorderDlg.h"
#include "CaptureDevices.h"
#include "audioformat.h"
#include <afxdlgs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


VOID    ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName );

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecorderDlg dialog

CRecorderDlg::CRecorderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecorderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRecorderDlg)
	m_strFileName = _T("");
    m_pAudioFormat=NULL;
    m_pCapDevices=NULL;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRecorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecorderDlg)
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRecorderDlg, CDialog)
	//{{AFX_MSG_MAP(CRecorderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_HELP, OnButtonHelp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SELFILE, OnButtonSelfile)
	ON_BN_CLICKED(IDC_RECORD, OnRecord)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecorderDlg message handlers

BOOL CRecorderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

    

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
    InitDevice();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRecorderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRecorderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRecorderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CRecorderDlg::OnButtonHelp() 
{
	DXUtil_LaunchReadme(m_hWnd);
	
}

void CRecorderDlg::OnButtonRecord() 
{
	// TODO: Add your control notification handler code here
	
}

void CRecorderDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	UnInit();

    CDialog::OnClose();
}

int CRecorderDlg::InitDevice()
{
    m_pCapDevices=new CCaptureDevices();
    if(m_pCapDevices==NULL)
    {
        return false;
    }
    if(m_pCapDevices->DoModal()==IDCANCEL)
    {
        UnInit();
        return false;
    }
    
    theApp.InitDirectSound();

    m_pAudioFormat=new CAudioFormat();
    if(m_pAudioFormat==NULL)
    {
        UnInit();
        return false;
    }
    if(m_pAudioFormat->DoModal()==IDCANCEL)
    {
        UnInit();
        return false;
    }

    WAVEFORMATEX wfxInput;
    TCHAR        strInputFormat[255];
    HWND         hInputFormatText;
    
    hInputFormatText = GetDlgItem( IDC_MAIN_INPUTFORMAT_TEXT)->GetSafeHwnd();
    
    ZeroMemory( &wfxInput, sizeof(wfxInput));
    theApp.m_pDSBCapture->GetFormat( &wfxInput, sizeof(wfxInput), NULL );
    ConvertWaveFormatToString( &wfxInput, strInputFormat );   
    
    ::SetWindowText( hInputFormatText, strInputFormat );
    
    theApp.m_bRecording = FALSE;
    

    return 0;
}

int CRecorderDlg::UnInit()
{
    if(m_pAudioFormat)
    {
        delete m_pAudioFormat;
        m_pAudioFormat=NULL;
    }
    if(m_pCapDevices)
    {
        delete m_pCapDevices;
        m_pCapDevices=NULL;        
    }
    return 0;
}

void CRecorderDlg::OnButtonSelfile() 
{
    
    HRESULT hr;
    static char BASED_CODE szFilter[] = "Wave Filter(*.wav)| *.wav|All Files (*.*)|*.*||";
 
    CFileDialog *pSaveDlg=new CFileDialog(false,NULL,NULL,OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | 
        OFN_HIDEREADONLY    | OFN_NOREADONLYRETURN,szFilter,NULL);

    if( theApp.m_bRecording )
    {
        // Stop the capture and read any data that 
        // was not caught by a notification
        theApp.StartOrStopRecord( FALSE );
        theApp.m_bRecording = FALSE;
    }
    
    // Update the UI controls to show the sound as loading a file
    GetDlgItem(IDC_RECORD )->EnableWindow(false);
    SetDlgItemText( IDC_FILENAME, TEXT("Saving file...") );
    
    pSaveDlg->DoModal();
    m_strFileName=pSaveDlg->GetPathName();

    SetDlgItemText( IDC_FILENAME, TEXT("") );
    
    SAFE_DELETE(theApp.m_pWaveFile );
    theApp.m_pWaveFile = new CWaveFile;
    if( NULL == theApp.m_pWaveFile )
        return;
    
    // Get the format of the capture buffer in g_wfxCaptureWaveFormat
    WAVEFORMATEX wfxCaptureWaveFormat;
    ZeroMemory( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX) );
    theApp.m_pDSBCapture->GetFormat( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX), NULL );
    
    // Load the wave file
    if( FAILED( hr =theApp.m_pWaveFile->Open((char*)(LPCTSTR )m_strFileName , &wfxCaptureWaveFormat, WAVEFILE_WRITE ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("Open"), hr );
        SetDlgItemText( IDC_FILENAME, TEXT("Can not create wave file.") );
        return;
    }
    
    // Update the UI controls to show the sound as the file is loaded
    GetDlgItem(IDC_RECORD )->EnableWindow(TRUE );
    
    UpdateData(false);

    delete pSaveDlg;
    pSaveDlg=NULL;
}

void CRecorderDlg::OnRecord() 
{
    HRESULT hr;
    theApp.m_bRecording = !theApp.m_bRecording;
    if( FAILED( hr =theApp.StartOrStopRecord( theApp.m_bRecording ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("StartOrStopRecord"), hr );
        MessageBox( "Error with DirectSoundCapture buffer."                            
            "Sample will now exit.", "DirectSound Sample", 
            MB_OK | MB_ICONERROR );        
    }
    
    if( !theApp.m_bRecording )
    {
        GetDlgItem(IDC_RECORD )->EnableWindow( FALSE );
        SetWindowText(TEXT("Â¼ÖÆÒôÆµ"));
    }
    else
    {
        SetWindowText(TEXT("ÒôÆµÂ¼ÖÆÖÐ..."));
        GetDlgItem(IDC_RECORD)->EnableWindow(TRUE);
    }
}
