// CaptureVideoDlg.h : Í·ÎÄ¼þ
//

#pragma once
#include "afxwin.h"
#include "CaptureClass.h"
#include "ImageHandle.h"
#include <d3d9types.h>
#include "D3DRender.h"

#define MY_THREADMSG WM_USER+1


class CCaptureVideoDlg : public CDialog
{

public:
	CCaptureVideoDlg(CWnd* pParent = NULL);	
	~CCaptureVideoDlg();

	enum { IDD = IDD_CAPTUREVIDEO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	
	virtual BOOL PreTranslateMessage(MSG* pMsg);	
	CToolTipCtrl m_tooltip;

protected:
	HICON m_hIcon;	
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	
public:
	afx_msg void OnBnClickedPreview();
	afx_msg void OnBnClickedCapture();
	afx_msg void OnBnClickedVideoFormat();
	afx_msg void OnBnClickedImageParameter();
	afx_msg void OnBnClickedSavegraph();
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedButton2();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnMyThreadMsg(WPARAM,LPARAM);

public:
	CComboBox m_listCtrl;

public:
	CCaptureClass m_cap;
	CStatic m_videoWindow;
	MYPARAM m_param;
	D3DDISPLAYMODE m_d3ddm;
	BYTE *m_pData;
	CImageHandle m_imageHandle;
	HANDLE m_hThread;
	bool m_isThreadActive;
	CD3DRender m_d3dRender;

	CStatic m_show2;
};
