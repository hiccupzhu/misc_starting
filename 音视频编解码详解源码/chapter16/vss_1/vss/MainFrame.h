#pragma once

#include "bkdialogst.h"
#include "xskinbutton.h"
#include "afxwin.h"
#include "CaptureClass.h" // ljz
#include "CDXGraph.h"     // ljz

#define CHAN_SUM 4

// CMainFrame 对话框

class CMainFrame : public CBkDialogST//CDialog
{
	DECLARE_DYNAMIC(CMainFrame)

public:
	CMainFrame(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainFrame();
	CWnd *pBtnOnePic,*pBtnFourePic;
	CWnd *pBtnExitApp;
	CWnd *pBtnChan1,*pBtnChan2,*pBtnChan3;
	CWnd *pBtnMini,*pBtnClose;
	CWnd *pStaticVideo[4];
	CWnd *pDisSysTime;
	CWnd *pBtnPlayAll;

	int ScrHeight;
	int ScrWidth;
	BOOL bChanged;
	
	// capture
	CCaptureClass m_cap;
	// play
	CDXGraph *m_pFilterGraph[CHAN_SUM];
	CString	m_sourceFile[CHAN_SUM];
	CString m_mediaFileName[CHAN_SUM];
	void CreateGraph(int chan);
	void DestroyGraph(int chan);
	void OpenFilePlay(int chan);
	int iCurChan;

	BOOL bInit;

	//my function
	void InitInterface();
// 对话框数据
	enum { IDD = IDD_DIALOG_MAINFACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CxSkinButton m_BtnOnePic;
public:
	CxSkinButton m_BtnFourePic;
public:
	CxSkinButton m_BtnExitApp;
public:
	CxSkinButton m_BtnChan1;
public:
	CxSkinButton m_BtnChan2;
public:
	CxSkinButton m_BtnChan3;
public:
	CxSkinButton m_BtnMini;
public:
	CxSkinButton m_BtnClose;
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedButtonOnePic();
public:
	afx_msg void OnBnClickedButtonFourePic();
public:
	afx_msg void OnBnClickedButtonExitApp();
public:
	afx_msg void OnBnClickedButtonClose();
public:
	afx_msg void OnBnClickedButtonMini();
public:
	afx_msg void OnBnClickedButtonChanL();
public:
	afx_msg void OnBnClickedButtonChanM();
public:
	afx_msg void OnBnClickedButtonChanR();
public:
	afx_msg void OnPaint();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
//public:
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	// 显示视频
	//CStatic m_StaticVideo;
	afx_msg LRESULT OnGraphNotify(WPARAM inWParam, LPARAM inLParam);
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	CxSkinButton m_BtnPlayAll;
public:
	afx_msg void OnBnClickedButtonPlayAll();
};
