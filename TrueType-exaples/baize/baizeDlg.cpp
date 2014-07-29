
// baizeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "baize.h"
#include "baizeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CbaizeDlg 对话框




CbaizeDlg::CbaizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CbaizeDlg::IDD, pParent),
	m_pdata(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CbaizeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SHOW, m_show);
}

BEGIN_MESSAGE_MAP(CbaizeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDOK, &CbaizeDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CbaizeDlg 消息处理程序

BOOL CbaizeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CbaizeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CbaizeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int set_point(char* pdata,int x,int y,int width,int height)
{
    char* p = (pdata + (( y) * width + x) * 3);
    *(p + 0) = 0;
    *(p + 1) = 0;
    *(p + 2) = 0;
    return 0;
}

class MPoint{
public:
    int x;
    int y;
    
public:
    MPoint(int _x,int _y){
        x = _x;
        y = _y;
    }
    
};

MPoint operator *(int t,MPoint point){
    point.x *= t;
    point.y *= t;
    return point;
}

MPoint operator +(MPoint p1,MPoint p2){
    p1.x += p2.x;
    p1.y += p2.y;
    return p1;
}

void CbaizeDlg::OnBnClickedOk()
{
    RECT rc = {0};
    m_show.GetWindowRect(&rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    
    BITMAPINFOHEADER info = {0};
    info.biBitCount = 24;
    info.biCompression = 0;
    info.biHeight = height;
    info.biWidth = width;
    info.biSize = sizeof(BITMAPINFOHEADER);
    info.biSizeImage = width * height * info.biBitCount / 8;
    info.biPlanes = 1;
    
    if(m_pdata == NULL){
        m_pdata = (char*)malloc(width * height * 3);
        memset(m_pdata, 0xff, width * height * 3);
    }
    
    MPoint point1(0,0);
    MPoint point2(width,height);
    
    int x = 0;
    int y = 0;
    const int factor = 1000;
    for(int t = 0;t <= factor; t++){
        MPoint tmp = (factor - t) * point1 + t * point2;
        x = tmp.x / factor;
        y = tmp.y / factor;
        x = max(0,min(x,width - 1));
        y = max(0,min(y,height - 1));
        set_point(m_pdata,x,y,width,height);
    }
    
    
    StretchDIBits(m_show.GetDC()->GetSafeHdc(),
                  0,0,width,height,
                  0,0,width,height,
                  m_pdata,(BITMAPINFO*)&info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}
