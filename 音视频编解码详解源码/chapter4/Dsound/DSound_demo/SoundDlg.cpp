// SoundDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sound.h"
#include "SoundDlg.h"
#include "process.h"
#include "DirectSound.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

CSoundDlg::CSoundDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSoundDlg, CDialog)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_Sound_Play, OnSound2)
	ON_BN_CLICKED(IDC_Sound_stop, OnSound1)
	ON_BN_CLICKED(IDC_sound_pause, OnBothSounds)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_Slow, OnBUTTONSlow)
	ON_BN_CLICKED(IDC_BUTTON_Fast, OnBUTTONFast)
	ON_BN_CLICKED(IDC_BUTTON_Normal, OnBUTTONNormal)
	//}}AFX_MSG_MAP
	//ON_BN_CLICKED(IDC_Sound_stop2, &CSoundDlg::OnBnClickedSoundstop2)
	ON_BN_CLICKED(IDC_Sound_open, &CSoundDlg::OnBnClickedSoundopen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg message handlers

BOOL CSoundDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	m_hThread = 0;
	m_bStop = 0;
	m_bContine = TRUE;
	m_sndSound1 = new CDirectSound;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSoundDlg::OnPaint() 
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

HCURSOR CSoundDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

unsigned int WINAPI FileReadProc(LPVOID pOwner)
{

	CSoundDlg* pThis = (CSoundDlg*)pOwner;
	pThis->ReadFileProc();

	
	return 1;
}

void CSoundDlg::ReadFileProc()
{


	//BYTE buf[1025];
	int n = 0;
	while(1)
	{

	

		BYTE buf[1025];
//		Sleep(0);
		int nlen;
		if(m_bStop)
			break;
 		nlen = fread(buf,1,1024,fp);
		
		
		if(!nlen)
			break;

		while(m_sndSound1->WriteDataToBuf(buf,nlen)==-1)
		{
			Sleep(100);
		}
		
		
	}
	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}
	

}

typedef struct _WAVE_FORMAT
{
    WORD  AudioFormat;
    WORD  NumChannels;
    DWORD SampleRate;
    DWORD ByteRate;
    WORD  BlockAlign;
    WORD  BitsPerSample;
}WAVE_FORMAT,*PWAVE_FORMAT;

void CSoundDlg::OnSound2() 
{

//	m_sndSound1 = new CDirectSound();
	AUDIO_CONFIG WaveHead;
	WaveHead.wFormatTag = 1;
	WaveHead.nChannels = 2;
	WaveHead.nSamplesPerSec = 44100;
	WaveHead.nAvgBytesPerSec = 176400;
	WaveHead.nBlockAlign = 4;
	WaveHead.wBitsPerSample = 16;
//	WaveHead.cbSize = 24932;

	DWORD fdwSound =0;
	if(m_soundfile.IsEmpty())
	
		MessageBox("请选中播放文件！");
	
	m_bStop = 0;
//	m_sndSound1->SetHWnd(AfxGetMainWnd()->m_hWnd);

	unsigned int dwReadID;

	m_bContine = 1;

	 fp = fopen((LPCSTR)m_soundfile,"rb");

    fseek(fp,20,0); //Skip previous 20 bytes RIFF header

    WAVE_FORMAT waveFormat;
    int nLen=0;
    nLen=fread(&waveFormat,1,sizeof(WAVE_FORMAT),fp);
    
    WaveHead.nBlockAlign=waveFormat.BlockAlign;
    WaveHead.nChannels=waveFormat.NumChannels;
    WaveHead.nSamplesPerSec=waveFormat.SampleRate;
    WaveHead.wBitsPerSample=waveFormat.BitsPerSample;
    WaveHead.nAvgBytesPerSec=waveFormat.ByteRate;
    //WaveHead.nSamplesPerSec=0;
 
    
    m_sndSound1->CreateDSound(&WaveHead,8000);

    fseek(fp,20+sizeof(WAVE_FORMAT),0);
	
    m_hThread = (HANDLE)_beginthreadex(0,0,FileReadProc,this,0,&dwReadID);

	Sleep(200);
	
	m_sndSound1->Play();

	GetDlgItem(IDC_Sound_open)->EnableWindow(FALSE);	
	GetDlgItem(IDC_Sound_Play)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_Fast)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_Slow)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_Normal)->EnableWindow(TRUE);
    GetDlgItem(IDC_Sound_stop)->EnableWindow(TRUE);
    GetDlgItem(IDC_sound_pause)->EnableWindow(TRUE);
	
	//ReadFileProc();
	

}

void CSoundDlg::OnSound1() 
{
//	m_sndSound2.Play();
	m_bStop = 1;

	if(m_hThread)
	{
		WaitForSingleObject(m_hThread,2000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	m_sndSound1->Stop();

	GetDlgItem(IDC_Sound_open)->EnableWindow(TRUE);	
	GetDlgItem(IDC_Sound_Play)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_Fast)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_Slow)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_Normal)->EnableWindow(FALSE);
    GetDlgItem(IDC_Sound_stop)->EnableWindow(FALSE);
    GetDlgItem(IDC_sound_pause)->EnableWindow(FALSE);

//	delete m_sndSound1;
	
//	if(fp)
//		fclose(fp);

}

void CSoundDlg::OnBothSounds() 
{
//	m_sndSound1.Play();
//	m_sndSound2.Play();
	if(m_bContine)
	{
		m_sndSound1->Pause();
		m_bContine = 0;
	}
	else
	{
		m_sndSound1->Continue();
		m_bContine = 1;
	}
}

void CSoundDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here


	m_bStop = 1;

	
	if(m_hThread)
	{
		WaitForSingleObject(m_hThread,2000);
		CloseHandle(m_hThread);
		m_hThread = 0;
	}
	OnSound1();

	delete m_sndSound1 ;

	
}

void CSoundDlg::OnBUTTONSlow() 
{
	// TODO: Add your control notification handler code here
	m_sndSound1->SetPlaySpeed(0.5);
}

void CSoundDlg::OnBUTTONFast() 
{
	// TODO: Add your control notification handler code here
	m_sndSound1->SetPlaySpeed(2.0);
}

void CSoundDlg::OnBUTTONNormal() 
{
	// TODO: Add your control notification handler code here
	m_sndSound1->SetPlaySpeed(1.0);
}

void CSoundDlg::OnBnClickedSoundstop2()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CSoundDlg::OnBnClickedSoundopen()
{
	// TODO: 在此添加控件通知处理程序代码
    LPCTSTR lpszFilter ="Wave File(*.wav)|*.wav|All Files|*.*||";
	CFileDialog dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,lpszFilter);
	if(dlg.DoModal()==IDOK)
	{
		m_soundfile = dlg.GetPathName();
	}
	GetDlgItem(IDC_Sound_open)->EnableWindow(FALSE);	
	GetDlgItem(IDC_Sound_Play)->EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_Fast)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_Slow)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_Normal)->EnableWindow(FALSE);
    GetDlgItem(IDC_Sound_stop)->EnableWindow(FALSE);
    GetDlgItem(IDC_sound_pause)->EnableWindow(FALSE);
}
