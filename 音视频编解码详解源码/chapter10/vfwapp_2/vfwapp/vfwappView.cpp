// vfwappView.cpp : CvfwappView 类的实现
//

#include "stdafx.h"
#include "vfwapp.h"

#include "vfwappDoc.h"
#include "vfwappView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern unsigned char bufo[];
extern int nStreamLength;
extern int nOstreamSize;
extern LPBITMAPINFO lpbiIn;


// CvfwappView

IMPLEMENT_DYNCREATE(CvfwappView, CView)

BEGIN_MESSAGE_MAP(CvfwappView, CView)
END_MESSAGE_MAP()

// CvfwappView 构造/析构

CvfwappView::CvfwappView()
{
	// TODO: 在此处添加构造代码
	m_hdd=DrawDibOpen();
}

CvfwappView::~CvfwappView()
{
	DrawDibClose(m_hdd);
}

BOOL CvfwappView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CvfwappView 绘制

void CvfwappView::OnDraw(CDC * pDC)
{
	CvfwappDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	// TODO: 在此处为本机数据添加绘制代码
	RECT rect;
	GetClientRect(&rect);
#if 0
	memcpy(bufo,lpbiIn,sizeof(BITMAPINFO));
#else
/*****************Add the bitmap information.********************************************/
	((LPBITMAPINFOHEADER)bufo)->biSize = sizeof(BITMAPINFOHEADER);
	((LPBITMAPINFOHEADER)bufo)->biWidth = lpbiIn->bmiHeader.biWidth;
	((LPBITMAPINFOHEADER)bufo)->biHeight = lpbiIn->bmiHeader.biHeight;
	((LPBITMAPINFOHEADER)bufo)->biPlanes = 1;
	((LPBITMAPINFOHEADER)bufo)->biBitCount = 24;
	((LPBITMAPINFOHEADER)bufo)->biCompression = BI_RGB;
	((LPBITMAPINFOHEADER)bufo)->biSizeImage = lpbiIn->bmiHeader.biWidth*lpbiIn->bmiHeader.biHeight*3;
	((LPBITMAPINFOHEADER)bufo)->biClrUsed = 0;
	((LPBITMAPINFOHEADER)bufo)->biXPelsPerMeter = 0;
	((LPBITMAPINFOHEADER)bufo)->biYPelsPerMeter = 0;
	((LPBITMAPINFOHEADER)bufo)->biClrImportant = 0;
/*****************Add the bitmap information.********************************************/
#endif	
	DrawDibDraw(m_hdd,
				pDC->GetSafeHdc(),
				rect.left,rect.top,	rect.right,	rect.bottom,
				(LPBITMAPINFOHEADER)bufo, &bufo[40],
				0,0, -1,-1,
				0);
}


// CvfwappView 诊断

#ifdef _DEBUG
void CvfwappView::AssertValid() const
{
	CView::AssertValid();
}

void CvfwappView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CvfwappDoc* CvfwappView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CvfwappDoc)));
	return (CvfwappDoc*)m_pDocument;
}
#endif //_DEBUG


// CvfwappView 消息处理程序
