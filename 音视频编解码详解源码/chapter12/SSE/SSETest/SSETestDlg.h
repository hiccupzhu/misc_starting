// SSETestDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "Chart.h"


#define ARRAY_SIZE 30000
#define GRAPH_STEP 100           // one of 100 points is shown in the graph





// CSSETestDlg dialog
class CSSETestDlg : public CDialog
{
// Construction
public:
	CSSETestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SSETEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    CChart m_chart;


    void CreateChartControl();
    void InitChartControl();
    void UpdateChartControl();
    void TestSSESupport();
    void ShowTime(__int64 nTime);
    void FillSourceArrays();
    void ShowSourceArrays();
    void ShowResultArray();
    void ShowArray(float* pArray, int nSeries);


    typedef void (CSSETestDlg::* COMPUTE_FUNCTION)(float* pArray1, float* pArray2, float* pResult, int nSize);
    void ComputeResultArray(COMPUTE_FUNCTION pFunction);
    void ComputeArrayCPlusPlus(float* pArray1, float* pArray2, float* pResult, int nSize);
    void ComputeArrayCPlusPlusSSE(float* pArray1, float* pArray2, float* pResult, int nSize);
    void ComputeArrayAssemblySSE(float* pArray1, float* pArray2, float* pResult, int nSize);

    // Arrays processed by SSE should have 16 bytes alignment:
    __declspec(align(16)) float m_fArray1[ARRAY_SIZE];
    __declspec(align(16)) float m_fArray2[ARRAY_SIZE];

    // This array is dynamically allocated to test _aligned_malloc function:
    float* m_fArrayResult;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    CStatic m_static_time;
    afx_msg void OnBnClickedButtonReset();
    afx_msg void OnBnClickedButtonCplusplus();
    afx_msg void OnBnClickedButtonSse();
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedButtonAssembly();
};
