
// compatibledcDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CcompatibledcDlg 对话框
class CcompatibledcDlg : public CDialog
{
// 构造
public:
	CcompatibledcDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_COMPATIBLEDC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    CStatic m_show;
    CButton m_ok;
    CButton m_cancle;
};
