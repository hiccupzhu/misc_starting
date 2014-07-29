// RecorderDlg.h : header file
//

#if !defined(AFX_RECORDERDLG_H__C4CF0FF1_847B_4508_89AD_4DD68D474738__INCLUDED_)
#define AFX_RECORDERDLG_H__C4CF0FF1_847B_4508_89AD_4DD68D474738__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCaptureDevices;
class CAudioFormat;
/////////////////////////////////////////////////////////////////////////////
// CRecorderDlg dialog
extern CRecorderApp theApp;


class CRecorderDlg : public CDialog
{
// Construction
public:
	int UnInit();
	int InitDevice();
	CRecorderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CRecorderDlg)
	enum { IDD = IDD_RECORDER_DIALOG };
	CString	m_strFileName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecorderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CRecorderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonHelp();
	afx_msg void OnButtonRecord();
	afx_msg void OnClose();
	afx_msg void OnButtonSelfile();
	afx_msg void OnRecord();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    CAudioFormat * m_pAudioFormat;
    CCaptureDevices * m_pCapDevices;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDERDLG_H__C4CF0FF1_847B_4508_89AD_4DD68D474738__INCLUDED_)
