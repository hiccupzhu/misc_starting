// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MMX32.h"
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

const float c_fRedCoefficient = 1.5f;
const float c_fGreenCoefficient = 1.5f;
const float c_fBlueCoefficient = 0.5f;


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
    ON_COMMAND(ID_COLORS_C, OnColorsC)
    ON_COMMAND(ID_COLORS_C_MMX, OnColorsCMmx)
    ON_UPDATE_COMMAND_UI(ID_COLORS_C_MMX, OnUpdateMmx)
    ON_UPDATE_COMMAND_UI(ID_COLORS_ASSEMBLY_MMX, OnUpdateMmx)
    ON_COMMAND(ID_COLORS_ASSEMBLY_MMX, OnColorsAssemblyMmx)
    ON_COMMAND(ID_INVERTCOLORS_C, OnInvertcolorsC)
    ON_COMMAND(ID_INVERTCOLORS_ASSEMBLY_MMX, OnInvertcolorsAssemblyMmx)
    ON_UPDATE_COMMAND_UI(ID_INVERTCOLORS_ASSEMBLY_MMX, OnUpdateMmx)
    ON_COMMAND(ID_INVERTCOLORS_C_MMX, OnInvertcolorsCMmx)
    ON_UPDATE_COMMAND_UI(ID_INVERTCOLORS_C_MMX, OnUpdateMmx)
    ON_COMMAND(ID_OPERATIONS_RESET, OnOperationsReset)
    ON_WM_ERASEBKGND()
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

// Initialization
void CChildView::InitImage()
{
    CImage image;
    image.LoadFromResource(AfxGetInstanceHandle(), IDB_BITMAP_SAMPLE);

    if ( image.IsNull() )
        return;

    // create 32 bpp image
    m_Image.Create(
        image.GetWidth(),
        image.GetHeight(),
        32);

    int nPitch = m_Image.GetPitch();
    int nWidth = m_Image.GetWidth();
    int nHeight = m_Image.GetHeight();

    ASSERT( m_Image.IsDIBSection() );
    ASSERT( abs(nPitch) == nWidth * 4 );

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
        m_pImageBits = (BYTE*) m_Image.GetBits() - (nHeight - 1) * nWidth * 4;

    // keep initial image state in m_pInitialImage array
    m_pInitialImage = new BYTE[nWidth * nHeight * 4];
    memcpy(m_pInitialImage, m_pImageBits, nWidth * nHeight * 4);

}

// Test MMX support
void CChildView::TestMMX()
{
    bool bMMX, bSSE;
    TestFeatures(&bMMX, &bSSE);

    if ( bMMX )
        m_bMMX = TRUE;
}

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
        memcpy(m_pImageBits, m_pInitialImage, m_Image.GetWidth() * m_Image.GetHeight() * 4);

        Invalidate();
        UpdateWindow();
    }
}



// Convert image colors using C++ code 
void CChildView::OnColorsC()
{
    ComputeColors(&CImg32Operations::ColorsCPlusPlus);
}

// Convert image colors using C++ code with MMX 
void CChildView::OnColorsCMmx()
{
    ComputeColors(&CImg32Operations::ColorsC_MMX);
}

// Convert image colors using Assembly with MMX 
void CChildView::OnColorsAssemblyMmx()
{
    ComputeColors(&CImg32Operations::ColorsAssembly_MMX);
}

// Make colors transformation using one of possible ways
// (C++, C++ and MMX, Assembly and MMX)
void CChildView::ComputeColors(CImg32Operations::COMPUTE_COLORS pFunction)
{
    TIME_START;

    // make operation number of times to see the difference
    for ( int i = 0; i < c_nNumberOfCalculations; i++ )
    {
        (m_Operations.*pFunction)(
            m_pInitialImage,
            m_pImageBits,
            m_Image.GetWidth() * m_Image.GetHeight(),
            c_fRedCoefficient,
            c_fGreenCoefficient,
            c_fBlueCoefficient);
    }

    TIME_END;

    Invalidate();
    UpdateWindow();
}




// Invert image using C++ 
void CChildView::OnInvertcolorsC()
{
    InvertImage(&CImg32Operations::InvertImageCPlusPlus);
}

// Invert image using Assembly 
void CChildView::OnInvertcolorsAssemblyMmx()
{
    InvertImage(&CImg32Operations::InvertImageAssembly_MMX);
}

// Invert image using C++ with MMX 
void CChildView::OnInvertcolorsCMmx()
{
    InvertImage(&CImg32Operations::InvertImageC_MMX);
}

// Invert image using one of possible ways
// (C++, C++ and MMX, Assembly and MMX)
void CChildView::InvertImage(CImg32Operations::INVERT_IMAGE pFunction)
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




