// SSETestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SSETest.h"
#include "SSETestDlg.h"
#include <float.h>
#include <math.h>
#include "TimeCounterEx.h"
#include "MMX_SSESupport.h"
#include ".\ssetestdlg.h"


#include <xmmintrin.h>      // __m128 data type and SSE functions


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define PI 3.141592
#define NUMBER_OF_COMPUTATIONS 10

#define TIME_START CTimeCounter* pT = new CTimeCounter()
#define TIME_END   ShowTime(pT->GetExecutionTime())


namespace
{
    const COLORREF cBackColor = GetSysColor(COLOR_3DFACE);
};



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CSSETestDlg dialog



CSSETestDlg::CSSETestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSSETestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    // allocate float array with 16-bytes alignemeent
    m_fArrayResult = (float*) _aligned_malloc(ARRAY_SIZE * sizeof(float), 16);
}

void CSSETestDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // free array allocated by _aligned_malloc
    _aligned_free(m_fArrayResult);
}


void CSSETestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_TIME, m_static_time);
}

BEGIN_MESSAGE_MAP(CSSETestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_RESET, OnBnClickedButtonReset)
    ON_BN_CLICKED(IDC_BUTTON_CPLUSPLUS, OnBnClickedButtonCplusplus)
    ON_BN_CLICKED(IDC_BUTTON_SSE, OnBnClickedButtonSse)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BUTTON_ASSEMBLY, OnBnClickedButtonAssembly)
END_MESSAGE_MAP()


// CSSETestDlg message handlers

BOOL CSSETestDlg::OnInitDialog()
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

    CreateChartControl();
    InitChartControl();
    TestSSESupport();
    FillSourceArrays();
    ShowSourceArrays();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSSETestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSSETestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSSETestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Create chart control
void CSSETestDlg::CreateChartControl()
{
    CRect rect;
    GetDlgItem(IDC_STATIC_CHART_PLACE)->GetWindowRect(&rect);
    ScreenToClient(&rect);

	m_chart.Create(
        WS_CHILD|WS_VISIBLE,
        rect,
        this,
        IDC_CHART);
}

// Initialize chart control
void CSSETestDlg::InitChartControl()
{
    m_chart.SetChartTitle(_T(""));
    m_chart.SetChartLabel(_T(""), _T(""));          // axis labels
    m_chart.SetAxisStyle(1);                        // double quadrant
    m_chart.SetGridXYNumber(1, 1);                  // grid lines
    m_chart.m_BGColor = cBackColor;                 // background color

    m_chart.SetRange(
        0.0,                   // min X
        7.0,                   // max X
        -2.0,                  // min Y
        2.0);                  // max Y

    m_chart.nSerieCount = 3;                        // number of series
    m_chart.AllocSerie(ARRAY_SIZE/GRAPH_STEP);      // max number of points in each series

    // set colors
    m_chart.mpSerie[0].m_plotColor = RGB(255, 0, 0);
    m_chart.mpSerie[1].m_plotColor = RGB(0, 0, 255);
    m_chart.mpSerie[2].m_plotColor = cBackColor;    // hidden

}

// Update chart control
void CSSETestDlg::UpdateChartControl()
{
    m_chart.Invalidate();
    m_chart.UpdateWindow();
}


// Show execution time (ms)
void CSSETestDlg::ShowTime(__int64 nTime)
{
    if ( nTime == 0 )
        m_static_time.SetWindowText(_T(""));
    else
    {
        CString s;
        s.Format(_T("%I64d"), nTime);
        m_static_time.SetWindowText(s);
    }
}


// Test SSE support
void CSSETestDlg::TestSSESupport()
{
    bool bMMX, bSSE;
    TestFeatures(&bMMX, &bSSE);

    if ( ! bSSE )
    {
        // Disable SSE buttons
        GetDlgItem(IDC_BUTTON_SSE)->EnableWindow(FALSE);
    }
}

// Fill source arrays m_fArray1 and m_fArray2
void CSSETestDlg::FillSourceArrays()
{
    int i;

    for ( i = 0; i < ARRAY_SIZE; i++ )
    {
        m_fArray1[i] = (float)sin(((double)i * PI * 2 / ARRAY_SIZE));

        m_fArray2[i] = (float)sin(((double)i * PI * 2 * 2 / ARRAY_SIZE));
    }
}

// Show source arrays in the graph
void CSSETestDlg::ShowSourceArrays()
{
    ShowArray(m_fArray1, 0);
    ShowArray(m_fArray2, 1);
}

// Show result array in the graph
void CSSETestDlg::ShowResultArray()
{
    ShowArray(m_fArrayResult, 2);
    m_chart.mpSerie[2].m_plotColor = RGB(255, 255, 255);

    UpdateChartControl();
}


// Show array in the graph
void CSSETestDlg::ShowArray(float* pArray, int nSeries)
{
    float fMax = (float) (PI * 2);
    int nNumberOfSteps = ARRAY_SIZE/GRAPH_STEP;
    int i, j;

    for ( i = 0, j = 0; i < nNumberOfSteps; i++, j+=GRAPH_STEP )
    {
        m_chart.SetXYValue(fMax*i/nNumberOfSteps, pArray[j], i, nSeries);
    }
}


// Hide result series if it is shown
void CSSETestDlg::OnBnClickedButtonReset()
{
    m_chart.mpSerie[2].m_plotColor = cBackColor;     // hide series
    ShowSourceArrays();
    UpdateChartControl();

    ShowTime(0);
}

// Compute the result array and show it in the graph control
// using C++
void CSSETestDlg::OnBnClickedButtonCplusplus()
{
    ComputeResultArray(&CSSETestDlg::ComputeArrayCPlusPlus);
}


// Compute the result array and show it in the graph control
// using C++ code with SSE Intrinsics
void CSSETestDlg::OnBnClickedButtonSse()
{
    ComputeResultArray(&CSSETestDlg::ComputeArrayCPlusPlusSSE);
}

// Compute the result array and show it in the graph control
// using Assembly code with SSE Intrinsics
void CSSETestDlg::OnBnClickedButtonAssembly()
{
    ComputeResultArray(&CSSETestDlg::ComputeArrayAssemblySSE);
}


// Compute the result array using C++ or SSE
void CSSETestDlg::ComputeResultArray(COMPUTE_FUNCTION pFunction)  // pointer to function
{
    TIME_START;

    // Make computation number of times to get considerable execution time
    for ( int i = 0; i < NUMBER_OF_COMPUTATIONS; i++ )
    {
        (this->*pFunction)(m_fArray1, m_fArray2, m_fArrayResult, ARRAY_SIZE);
    }

    TIME_END;           // show execution time

    ShowResultArray();
}

// Compute the result array using C++
void CSSETestDlg::ComputeArrayCPlusPlus(
          float* pArray1,                   // [in] first source array
          float* pArray2,                   // [in] second source array
          float* pResult,                   // [out] result array
          int nSize)                        // [in] size of all arrays
{

    int i;

    float* pSource1 = pArray1;
    float* pSource2 = pArray2;
    float* pDest = pResult;

    for ( i = 0; i < nSize; i++ )
    {
        *pDest = (float)sqrt((*pSource1) * (*pSource1) + (*pSource2) * (*pSource2)) + 0.5f;

        pSource1++;
        pSource2++;
        pDest++;
    }
}

// Compute the result array using C++ code with SSE Intrinsics
void CSSETestDlg::ComputeArrayCPlusPlusSSE(
          float* pArray1,                   // [in] first source array
          float* pArray2,                   // [in] second source array
          float* pResult,                   // [out] result array
          int nSize)                        // [in] size of all arrays
{
    int nLoop = nSize/ 4;

    __m128 m1, m2, m3, m4;

    __m128* pSrc1 = (__m128*) pArray1;
    __m128* pSrc2 = (__m128*) pArray2;
    __m128* pDest = (__m128*) pResult;


    __m128 m0_5 = _mm_set_ps1(0.5f);        // m0_5[0, 1, 2, 3] = 0.5

    for ( int i = 0; i < nLoop; i++ )
    {
#if 1
        // This code is easy to understand:
        m1 = _mm_mul_ps(*pSrc1, *pSrc1);        // m1 = *pSrc1 * *pSrc1
        m2 = _mm_mul_ps(*pSrc2, *pSrc2);        // m2 = *pSrc2 * *pSrc2
        m3 = _mm_add_ps(m1, m2);                // m3 = m1 + m2
        m4 = _mm_sqrt_ps(m3);                   // m4 = sqrt(m3)
        *pDest = _mm_add_ps(m4, m0_5);          // *pDest = m4 + 0.5
#else
        // And this line is for guys who love complicated expressions
        *pDest = _mm_add_ps(_mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(*pSrc1, *pSrc1), _mm_mul_ps(*pSrc2, *pSrc2))), m0_5);

        // Both versions give the same execution time in the Release configuration.
#endif

        pSrc1++;
        pSrc2++;
        pDest++;
    }
}

// Compute the result array using Assembly code with SSE Intrinsics
void CSSETestDlg::ComputeArrayAssemblySSE(
          float* pArray1,                   // [in] first source array
          float* pArray2,                   // [in] second source array
          float* pResult,                   // [out] result array
          int nSize)                        // [in] size of all arrays
{
    int nLoop = nSize/4;
    float f = 0.5f;

    _asm
    {
        movss   xmm2, f                         // xmm2[0] = 0.5
        shufps  xmm2, xmm2, 0                   // xmm2[1, 2, 3] = xmm2[0]

        mov         esi, pArray1                // input array 1
        mov         edx, pArray2                // input array 2
        mov         edi, pResult                // output pointer
        mov         ecx, nLoop                  // loop counter

start_loop:
        movaps      xmm0, [esi]                 // xmm0 = [esi]
        mulps       xmm0, xmm0                  // xmm0 = xmm0 * xmm0

        movaps      xmm1, [edx]                 // xmm1 = [edx]
        mulps       xmm1, xmm1                  // xmm1 = xmm1 * xmm1

        addps       xmm0, xmm1                  // xmm0 = xmm0 + xmm1
        sqrtps      xmm0, xmm0                  // xmm0 = sqrt(xmm0)

        addps       xmm0, xmm2                  // xmm0 = xmm1 + xmm2

        movaps      [edi], xmm0                 // [edi] = xmm0

        add         esi, 16                     // esi += 16
        add         edx, 16                     // edx += 16
        add         edi, 16                     // edi += 16

        dec         ecx                         // ecx--
        jnz         start_loop                  // if not zero go to start_loop
    }
}