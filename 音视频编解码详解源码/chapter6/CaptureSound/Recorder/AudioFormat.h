#if !defined(AFX_AUDIOFORMAT_H__B510BAB1_19D4_4DD1_ACB6_FA64CB60E217__INCLUDED_)
#define AFX_AUDIOFORMAT_H__B510BAB1_19D4_4DD1_ACB6_FA64CB60E217__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioFormat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAudioFormat dialog

class CAudioFormat : public CDialog
{
// Construction
public:
	int ScanAvailableInputFormats();
	CAudioFormat(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAudioFormat)
	enum { IDD = IDD_FORMATS };
	CListBox	m_ctlFormats;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioFormat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAudioFormat)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeFormatsInputListbox();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOFORMAT_H__B510BAB1_19D4_4DD1_ACB6_FA64CB60E217__INCLUDED_)
