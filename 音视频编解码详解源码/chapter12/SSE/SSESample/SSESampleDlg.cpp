// SSESampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SSESample.h"
#include "SSESampleDlg.h"
#include ".\ssesampledlg.h"
#include <float.h>
#include <math.h>
#include "TimeCounterEx.h"
#include "MMX_SSESupport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#define TIME_START CTimeCounter* pT = new CTimeCounter()
#define TIME_END   ShowTime(pT->GetExecutionTime())


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


// CSSESampleDlg dialog



CSSESampleDlg::CSSESampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSSESampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CSSESampleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_MINIMUM, m_static_minimum);
    DDX_Control(pDX, IDC_STATIC_MAXIMUM, m_static_maximum);
    DDX_Control(pDX, IDC_LIST1, m_list_box);
    DDX_Control(pDX, IDC_STATIC_TIME, m_static_time);
}

BEGIN_MESSAGE_MAP(CSSESampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_CPLUSPLUS, OnBnClickedButtonCplusplus)
    ON_BN_CLICKED(IDC_BUTTON_RESET, OnBnClickedButtonReset)
    ON_BN_CLICKED(IDC_BUTTON_SSE_ASSEMBLY, OnBnClickedButtonSseAssembly)
    ON_BN_CLICKED(IDC_BUTTON_SSE_C, OnBnClickedButtonSseC)
END_MESSAGE_MAP()


// CSSESampleDlg message handlers

BOOL CSSESampleDlg::OnInitDialog()
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

    // Test SSE support
    bool bMMX, bSSE;
    TestFeatures(&bMMX, &bSSE);

    if ( ! bSSE )
    {
        // Disable SSE buttons
        GetDlgItem(IDC_BUTTON_SSE_ASSEMBLY)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_SSE_C)->EnableWindow(FALSE);
    }

    // Fill and show initial array
    OnBnClickedButtonReset();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSSESampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSSESampleDlg::OnPaint() 
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
HCURSOR CSSESampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



// Fill initial array
void CSSESampleDlg::InitArray()
{
    m_fMin = FLT_MAX;
    m_fMax = FLT_MIN;

    float f;
    int i;


    for ( i = 0; i < ARRAY_SIZE; i++ )
    {
        // Fill array with one sin cycle and ensure that all values are positive
        // (to use sqrt in conversion)
        f = (float) sin(((double)i * 6.29 / ARRAY_SIZE)) + 2.0f;

        if ( f < m_fMin )
            m_fMin = f;

        if ( f > m_fMax )
            m_fMax = f;

        m_fInitialArray[i] = f;
    }
}

// Show min/max value in array
void CSSESampleDlg::ShowMinMax()
{
    CString s;

    ConvertFloatToString(s, m_fMin);
    m_static_minimum.SetWindowText(s);

    ConvertFloatToString(s, m_fMax);
    m_static_maximum.SetWindowText(s);
}

void CSSESampleDlg::AddString(LPCTSTR s)
{
    m_list_box.AddString(s);
}

void CSSESampleDlg::ClearList()
{
    m_list_box.ResetContent();
}

// Show array in the listbox.
// Only one of 100 array elements is shown
// (array are too large to show them in the listbox).
void CSSESampleDlg::ShowArray(float* pArray)
{
    ClearList();
    CString s;
    float* p = pArray;

    for ( int i = 0; i < ARRAY_SIZE; i+=100 )
    {
        ConvertFloatToString(s, p[i]);
        AddString(s);
    }

    ShowMinMax();
}

void CSSESampleDlg::ConvertFloatToString(CString& s, float f)
{
    s.Format(_T("%8.3f"), f);
}

// Show execution time (ms)
void CSSESampleDlg::ShowTime(__int64 nTime)
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

// Fill initial array and show it in the listbox
void CSSESampleDlg::OnBnClickedButtonReset()
{
    InitArray();
    ShowArray(m_fInitialArray);
    ShowTime(0);
}


// Make conversion using C++ code
//
// Each initial array member is converted to result array member
// using some formula (just to demonstrate SSE features).
// Minimum and maximum result values are calculated and shown.
//
// Function also calculates and shows conversion time (ms).
//
void CSSESampleDlg::OnBnClickedButtonCplusplus()
{
    TIME_START;

    m_fMin = FLT_MAX;
    m_fMax = FLT_MIN;

    int i;

    for ( i = 0; i < ARRAY_SIZE; i++ )
    {
        m_fResultArray[i] = sqrt(m_fInitialArray[i]  * 2.8f);

        if ( m_fResultArray[i] < m_fMin )
            m_fMin = m_fResultArray[i];

        if ( m_fResultArray[i] > m_fMax )
            m_fMax = m_fResultArray[i];
    }


    TIME_END;

    ShowArray(m_fResultArray);
}


// Make conversion using C++ code with inline Assembly
//
void CSSESampleDlg::OnBnClickedButtonSseAssembly()
{
    TIME_START;

    float* pIn = m_fInitialArray;
    float* pOut = m_fResultArray;

    float f = 2.8f;
    float flt_min = FLT_MIN;
    float flt_max = FLT_MAX;

    __m128 min128;
    __m128 max128;

    // using additional registers:
    // xmm2 - multiplication coefficient
    // xmm3 - minimum
    // xmm4 - maximum

    _asm
    {
        movss   xmm2, f                         // xmm2[0] = 2.8
        shufps  xmm2, xmm2, 0                   // xmm2[1, 2, 3] = xmm2[0]

        movss   xmm3, flt_max                   // xmm3 = FLT_MAX
        shufps  xmm3, xmm3, 0                   // xmm3[1, 2, 3] = xmm3[0]

        movss   xmm4, flt_min                   // xmm4 = FLT_MIN
        shufps  xmm4, xmm4, 0                   // xmm3[1, 2, 3] = xmm3[0]

        mov         esi, pIn                    // input pointer
        mov         edi, pOut                   // output pointer
        mov         ecx, ARRAY_SIZE/4           // loop counter

start_loop:
        movaps      xmm1, [esi]                 // xmm1 = [esi]
        mulps       xmm1, xmm2                  // xmm1 = xmm1 * xmm2
        sqrtps      xmm1, xmm1                  // xmm1 = sqrt(xmm1)
        movaps      [edi], xmm1                 // [edi] = xmm1

        minps       xmm3, xmm1
        maxps       xmm4, xmm1

        add         esi, 16
        add         edi, 16

        dec         ecx
        jnz         start_loop


        movaps      min128, xmm3
        movaps      max128, xmm4
    }

    // extract minimum and maximum values from min128 and max128
    union u
    {
        __m128 m;
        float f[4];
    } x;

    x.m = min128;
    m_fMin = min(x.f[0], min(x.f[1], min(x.f[2], x.f[3])));

    x.m = max128;
    m_fMax = max(x.f[0], max(x.f[1], max(x.f[2], x.f[3])));


    TIME_END;

    ShowArray(m_fResultArray);
}


// Make conversion using C++ code with SSE Intrinsics
//
void CSSESampleDlg::OnBnClickedButtonSseC()
{
    TIME_START;

    __m128 coeff = _mm_set_ps1(2.8f);      // coeff[0, 1, 2, 3] = 2.8
    __m128 tmp;

    __m128 min128 = _mm_set_ps1(FLT_MAX);  // min128[0, 1, 2, 3] = FLT_MAX
    __m128 max128 = _mm_set_ps1(FLT_MIN);  // max128[0, 1, 2, 3] = FLT_MIN

    __m128* pSource = (__m128*) m_fInitialArray;
    __m128* pDest = (__m128*) m_fResultArray;

    for ( int i = 0; i < ARRAY_SIZE/4; i++ )
    {
        tmp = _mm_mul_ps(*pSource, coeff);      // tmp = *pSource * coeff
        *pDest = _mm_sqrt_ps(tmp);              // *pDest = sqrt(tmp)

        min128 =  _mm_min_ps(*pDest, min128);
        max128 =  _mm_max_ps(*pDest, max128);

        pSource++;
        pDest++;
    }

    // extract minimum and maximum values from min128 and max128
    union u
    {
        __m128 m;
        float f[4];
    } x;

    x.m = min128;
    m_fMin = min(x.f[0], min(x.f[1], min(x.f[2], x.f[3])));

    x.m = max128;
    m_fMax = max(x.f[0], max(x.f[1], max(x.f[2], x.f[3])));


    TIME_END;

    ShowArray(m_fResultArray);
}
