
// baizeDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CbaizeDlg 对话框
class CbaizeDlg : public CDialog
{
// 构造
public:
	CbaizeDlg(CWnd* pParent = NULL);	// 标准构造函数
    ~CbaizeDlg(){ 
        if(m_pdata){
            free(m_pdata);
            m_pdata = NULL;
        }
    }

// 对话框数据
	enum { IDD = IDD_BAIZE_DIALOG };

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
    CStatic m_show;
    afx_msg void OnBnClickedOk();
    
    char*   m_pdata;
};
