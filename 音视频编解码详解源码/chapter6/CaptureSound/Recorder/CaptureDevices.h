#if !defined(AFX_CAPTUREDEVICES_H__D514BE70_70D8_412F_B3FA_5DAD2D8A7FDE__INCLUDED_)
#define AFX_CAPTUREDEVICES_H__D514BE70_70D8_412F_B3FA_5DAD2D8A7FDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CaptureDevices.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCaptureDevices dialog

class CCaptureDevices : public CDialog
{
// Construction
public:
	int m_nDevIdx;
    GUID * m_pCaptureGUID;
	CCaptureDevices(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCaptureDevices)
	enum { IDD = IDD_DEVICE };
	CComboBox	m_ctlDevices;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaptureDevices)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCaptureDevices)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAPTUREDEVICES_H__D514BE70_70D8_412F_B3FA_5DAD2D8A7FDE__INCLUDED_)
