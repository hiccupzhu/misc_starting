// 9-2Dlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "VMR_Capture.h"
#include "Convert.h"

// CMy92Dlg 对话框
class CMy92Dlg : public CDialog
{
// 构造
public:
	CMy92Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MY92_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);
// user dta
	//CToolTipCtrl m_toolTip;
	CToolTipCtrl m_tooltip;

	CVMR_Capture m_VMRCap;
	ColorSpaceConversions conv;
	UINT m_timerID;
	CString m_yuvFileName;
	CFile m_pFile;
	BOOL m_fileState;
	int m_imageWidth;
	int m_imageHeight;
	BYTE *p_yuv420;

//

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// display video data
	CStatic m_videoWindow;
public:
	CComboBox m_listCtrl;
public:
	afx_msg void OnBnClickedPreview();
public:
	afx_msg void OnBnClickedCapture();
public:
	afx_msg void OnBnClickedPauseplay();
public:
	afx_msg void OnBnClickedSavegraph();
public:
	afx_msg void OnBnClickedExitapp();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg void OnBnClickedStopcap();
};
