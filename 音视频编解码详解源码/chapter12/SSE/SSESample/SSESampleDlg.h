// SSESampleDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include <xmmintrin.h>      // __m128 data type and SSE functions


#define ARRAY_SIZE 100000

#pragma warning(disable : 4324)

// CSSESampleDlg dialog
class CSSESampleDlg : public CDialog
{
// Construction
public:
	CSSESampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SSESAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

    // Arrays processed by SSE should have 16 bytes alignment:
    __declspec(align(16)) float m_fInitialArray[ARRAY_SIZE];
    __declspec(align(16)) float m_fResultArray[ARRAY_SIZE];


    // minimum and maximum values in the result array
    float m_fMin;
    float m_fMax;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonCplusplus();
    afx_msg void OnBnClickedButtonReset();
    afx_msg void OnBnClickedButtonSseAssembly();
    afx_msg void OnBnClickedButtonSseC();

    CStatic m_static_minimum;
    CStatic m_static_time;
    CStatic m_static_maximum;
    CListBox m_list_box;

protected:
    void InitArray();
    void ShowMinMax();
    void AddString(LPCTSTR s);
    void ClearList();
    void ShowArray(float* pArray);
    void ConvertFloatToString(CString& s, float f);
    void ShowTime(__int64 nTime);

};

#pragma warning(default : 4324)
