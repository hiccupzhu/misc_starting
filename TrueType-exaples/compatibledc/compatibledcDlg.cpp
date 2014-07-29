
// compatibledcDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "compatibledc.h"
#include "compatibledcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CcompatibledcDlg::CcompatibledcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CcompatibledcDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcompatibledcDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SHOW, m_show);
    DDX_Control(pDX, IDOK, m_ok);
    DDX_Control(pDX, IDCANCEL, m_cancle);
}

BEGIN_MESSAGE_MAP(CcompatibledcDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDOK, &CcompatibledcDlg::OnBnClickedOk)
END_MESSAGE_MAP()



BOOL CcompatibledcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	
	int width   = 640;
	int height  = 540;
	int scw = GetSystemMetrics(SM_CXSCREEN);
	int sch = GetSystemMetrics(SM_CYSCREEN);
	int x   = scw / 2 - width / 2;
	int y   = sch / 2 - height / 2;
    this->MoveWindow(x, y, width, height);
    
    CRect rc(x, y, 640, 480);
    //ScreenToClient(rc);
    
    m_show.MoveWindow(0, 0, 640, 480);
    
//     m_ok.GetWindowRect(rc);
//     m_ok.MoveWindow(rc.left, height - rc.Height(), rc.Width(),rc.Height());
//     
//     m_cancle.GetWindowRect(rc);
//     m_cancle.MoveWindow(rc.left, height - rc.Height(), rc.Width(),rc.Height());
    
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CcompatibledcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CcompatibledcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CcompatibledcDlg::OnBnClickedOk()
{
    CRect rc;
    m_show.GetClientRect(rc);
    CDC* pdc = m_show.GetDC();
    pdc->SetMapMode(MM_ANISOTROPIC);
    CDC memdc;
    memdc.CreateCompatibleDC(pdc);
    
    CBitmap membitmap;
    membitmap.CreateCompatibleBitmap(pdc,rc.Width(),rc.Height());
    membitmap.LoadBitmap("f:\\back.bmp");
    
    
    CBitmap* poldbitmap = memdc.SelectObject(&membitmap);
    
    
    //memdc.FillSolidRect(0,0,rc.Width(),rc.Height(),RGB(122,122,122));
    memdc.MoveTo(0,0);
    memdc.LineTo(rc.Width(),rc.Height());
    
    
    pdc->BitBlt(0,0,rc.Width(),rc.Height(),&memdc,0,0,SRCCOPY);
    
    
    pdc->SelectObject(poldbitmap);
    
}
