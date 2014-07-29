// SoundDlg.h : header file
//

#if !defined(AFX_SOUNDDLG_H__BDDA38F5_613D_11D2_A7A3_000000000000__INCLUDED_)
#define AFX_SOUNDDLG_H__BDDA38F5_613D_11D2_A7A3_000000000000__INCLUDED_

#include "DirectSound.h"	// Added by ClassView
#include <dsound.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

class CSoundDlg : public CDialog
{
// Construction
public:
	CString	     m_soundfile;
	CString		 m_filename;

	int          m_nState;
	BOOL         m_bStop;
	HANDLE       m_hThread;
	FILE*        fp;
	BOOL         m_bContine;

	void         ReadFileProc();
	
	CDirectSound* m_sndSound1;
	
	CSoundDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSoundDlg)
	enum { IDD = IDD_SOUND_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSoundDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSound2();
	afx_msg void OnSound1();
	afx_msg void OnBothSounds();
	afx_msg void OnDestroy();
	afx_msg void OnBUTTONSlow();
	afx_msg void OnBUTTONFast();
	afx_msg void OnBUTTONNormal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	
//	CDirectSound m_sndSound2;
	afx_msg void OnBnClickedSoundstop2();
	afx_msg void OnBnClickedSoundopen();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDDLG_H__BDDA38F5_613D_11D2_A7A3_000000000000__INCLUDED_)
