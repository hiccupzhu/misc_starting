// CaptureVideoDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "CaptureClass.h"

// CCaptureVideoDlg 对话框
class CCaptureVideoDlg : public CDialog
{
// 构造
public:
	CCaptureVideoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CAPTUREVIDEO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);
// user data
	CCaptureClass m_cap;
	CToolTipCtrl m_tooltip;
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
	// 显示捕获的图像
	CStatic m_videoWindow;
public:
	afx_msg void OnBnClickedPreview();
public:
	afx_msg void OnBnClickedCapture();
public:
	afx_msg void OnBnClickedVideoFormat();
public:
	afx_msg void OnBnClickedImageParameter();
public:
	afx_msg void OnBnClickedSavegraph();
public:
	afx_msg void OnBnClickedExit();
public:
	// 组合框列表，显示设备名称
	CComboBox m_listCtrl;
public:
	CStatic m_show2;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
