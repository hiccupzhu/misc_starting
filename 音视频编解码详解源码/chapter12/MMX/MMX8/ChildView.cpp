// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MMX8.h"
#include "ChildView.h"
#include ".\childview.h"
#include "MMX_SSESupport.h"
#include "TimeCounterEx.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define TIME_START CTimeCounter* pT = new CTimeCounter()
#define TIME_END   ShowTime(pT->GetExecutionTime())


const int c_nNumberOfCalculations = 10;

const int c_nBrightnessChange = -40;

// CChildView

CChildView::CChildView()
{
    m_pInitialImage = NULL;
    m_pImageBits = NULL;

    m_bMMX = FALSE;
}

CChildView::~CChildView()
{
    if ( m_pInitialImage )
        delete[] m_pInitialImage;
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
    ON_WM_CREATE()
    ON_COMMAND(ID_OPERATIONS_RESET, OnOperationsReset)
    ON_COMMAND(ID_INVERTCOLORS_ASSEMBLY_MMX, OnInvertcolorsAssemblyMmx)
    ON_UPDATE_COMMAND_UI(ID_INVERTCOLORS_ASSEMBLY_MMX, OnUpdateMmx)
    ON_COMMAND(ID_INVERTCOLORS_C, OnInvertcolorsC)
    ON_COMMAND(ID_INVERTCOLORS_C_MMX, OnInvertcolorsC3Mmx)
    ON_UPDATE_COMMAND_UI(ID_INVERTCOLORS_C_MMX, OnUpdateMmx)
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_CHANGEBRIGHTNESS_C, OnChangebrightnessC)
    ON_COMMAND(ID_CHANGEBRIGHTNESS_ASSEMBLY_MMX, OnChangebrightnessAssemblyMmx)
    ON_COMMAND(ID_CHANGEBRIGHTNESS_C_MMX, OnChangebrightnessCMmx)
    ON_UPDATE_COMMAND_UI(ID_CHANGEBRIGHTNESS_ASSEMBLY_MMX, OnUpdateMmx)
    ON_UPDATE_COMMAND_UI(ID_CHANGEBRIGHTNESS_C_MMX, OnUpdateMmx)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

// prevent flickering
BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
//    return CWnd::OnEraseBkgnd(pDC);

    CRect rect;
    GetClientRect(&rect);

    if ( ! m_Image.IsNull() )
        pDC->ExcludeClipRect(0, 0, m_Image.GetWidth() - 1, m_Image.GetHeight() - 1);

    pDC->FillSolidRect(&rect, RGB(255, 255, 255));

    pDC->SelectClipRgn(NULL);

    return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
    if ( ! m_Image.IsNull() )
    {
        m_Image.BitBlt(
            dc.m_hDC,
            0,
            0);
    }
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    TestMMX();

    InitImage();

    return 0;
}


// Test MMX support
void CChildView::TestMMX()
{
    bool bMMX, bSSE;
    TestFeatures(&bMMX, &bSSE);

    if ( bMMX )
        m_bMMX = TRUE;
}


// Initialization
void CChildView::InitImage()
{
    CImage image;
    image.LoadFromResource(AfxGetInstanceHandle(), IDB_BITMAP_SAMPLE);

    if ( image.IsNull() )
        return;

    // create 8 bpp image
    m_Image.Create(
        image.GetWidth(),
        image.GetHeight(),
        8);

    int nPitch = m_Image.GetPitch();
    int nWidth = m_Image.GetWidth();
    int nHeight = m_Image.GetHeight();

    ASSERT( m_Image.IsDIBSection() );
    ASSERT( abs(nPitch) == nWidth );

    // fill gray-scale color table
    RGBQUAD colorTable[256];
    int i;

    for ( i = 0; i < 256; i++ )
    {
        colorTable[i].rgbBlue = colorTable[i].rgbGreen = 
            colorTable[i].rgbRed = (BYTE) i;
        colorTable[i].rgbReserved = 0;
    }

    m_Image.SetColorTable(0, 256, colorTable);

    // copy from image to m_Image
    image.BitBlt(
        m_Image.GetDC(),
        0, 
        0);

    m_Image.ReleaseDC();


    // get pointer to start of the image in memory
    if ( nPitch > 0 )
        m_pImageBits = (BYTE*) m_Image.GetBits();
    else
        m_pImageBits = (BYTE*) m_Image.GetBits() - (nHeight - 1) * nWidth;

    // keep initial image state in m_pInitialImage array
    m_pInitialImage = new BYTE[nWidth * nHeight];
    memcpy(m_pInitialImage, m_pImageBits, nWidth * nHeight);
}

// Show execution time
void CChildView::ShowTime(__int64 nTime)
{
    CString s;
    s.Format(_T("%d ms"), nTime);

    ((CMainFrame*)AfxGetMainWnd())->ShowText(s);
}

// All MMX menu items are disabled if MMX is not supported
void CChildView::OnUpdateMmx(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_bMMX);
}

// Reset the original image
void CChildView::OnOperationsReset()
{
    if ( m_pInitialImage && m_pImageBits )
    {
        memcpy(m_pImageBits, m_pInitialImage, m_Image.GetWidth() * m_Image.GetHeight());

        Invalidate();
        UpdateWindow();
    }
}

// Change brightness using one of possible ways
// (C++, C++ and MMX, Assembly and MMX)
void CChildView::ChangeBrightness(CImg8Operations::CHANGE_BRIGHTNESS pFunction)
{
    TIME_START;

    // make operation number of times to see the difference
    for ( int i = 0; i < c_nNumberOfCalculations; i++ )
    {
        (m_Operations.*pFunction)(
            m_pInitialImage,
            m_pImageBits,
            m_Image.GetWidth() * m_Image.GetHeight(),
            c_nBrightnessChange);
    }

    TIME_END;

    Invalidate();
    UpdateWindow();
}


// Invert image using one of possible ways
// (C++, C++ and MMX, Assembly and MMX)
void CChildView::InvertImage(CImg8Operations::INVERT_IMAGE pFunction)
{
    TIME_START;

    // make operation number of times to see the difference
    for ( int i = 0; i < c_nNumberOfCalculations; i++ )
    {
        (m_Operations.*pFunction)(
            m_pInitialImage,
            m_pImageBits,
            m_Image.GetWidth() * m_Image.GetHeight());
    }

    TIME_END;

    Invalidate();
    UpdateWindow();
}

// Invert image using C++
void CChildView::OnInvertcolorsC()
{
    InvertImage(&CImg8Operations::InvertImageCPlusPlus);
}

// Invert image using C++ with MMX 
void CChildView::OnInvertcolorsC3Mmx()
{
    InvertImage(&CImg8Operations::InvertImageC_MMX);
}

// Invert image using Assembly 
void CChildView::OnInvertcolorsAssemblyMmx()
{
    InvertImage(&CImg8Operations::InvertImageC_MMX);
}

// Change brightness using C++
void CChildView::OnChangebrightnessC()
{
    ChangeBrightness(&CImg8Operations::ChangeBrightnessCPlusPlus);
}

// Change brightness using C++ with MMX
void CChildView::OnChangebrightnessCMmx()
{
    ChangeBrightness(&CImg8Operations::ChangeBrightnessC_MMX);
}

// Change brightness using Assembly with MMX
void CChildView::OnChangebrightnessAssemblyMmx()
{
    ChangeBrightness(&CImg8Operations::ChangeBrightnessAssembly_MMX);
}

