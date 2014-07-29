// capaudioDlg.h : 头文件
//
#include <dshow.h>

#pragma once


// CcapaudioDlg 对话框
class CcapaudioDlg : public CDialog
{
// 构造
public:
	CcapaudioDlg(CWnd* pParent = NULL);	// 标准构造函数
	
	//myself
	IGraphBuilder *pGraph;
	ICaptureGraphBuilder2 * pBuilder;
	IMediaControl *pMC;
	IMoniker *pMoniker;
	IBaseFilter *pSrc,*pWaveDest,*pWriter;
	IFileSinkFilter2 *pSink;
	IPin * FindPin(IBaseFilter *pFilter, PIN_DIRECTION dir);
	//~myself

// 对话框数据
	enum { IDD = IDD_CAPAUDIO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


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
	afx_msg void OnBnClickedRecord();
public:
	afx_msg void OnBnClickedCancel();
public:
	afx_msg void OnBnClickedStop();
public:
	afx_msg void OnBnClickedButtonFileName();
public:
	// 保存文件名
	CString m_PathName;
public:
//	afx_msg void OnDestroy();
public:
	afx_msg void OnClose();
};
