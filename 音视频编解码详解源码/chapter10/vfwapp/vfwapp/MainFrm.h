// MainFrm.h : CMainFrame 类的接口
//

#pragma once
/////////////////////////////////////////////////////////////////////////////////////////
#include "vfw.h"

typedef struct{
	int headsize;						//bitmap headsize
	char buffer[1300000];				//bitmap head and data
}DIBINFO, *PDIBINFO;

LRESULT CALLBACK EXPORT ErrorCallbackProc(HWND hWnd, int nErrID, LPSTR lpErrorText);
LRESULT FAR PASCAL StatusCallbackProc(HWND hWnd, int nID, LPSTR lpStatusText);
LRESULT FAR PASCAL VideoCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr);
/////////////////////////////////////////////////////////////////////////////////////////

class CMainFrame : public CFrameWnd
{
	
protected: // 仅从序列化创建
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 属性
public:
	
	DIBINFO m_dibinfo;
	//int test;
	CAPDRIVERCAPS m_caps;
	PAVIFILE       m_pFile;            //AVI文件
	AVISTREAMINFO  strhdr;             //AVI流信息
	PAVISTREAM     ps;                 //AVI流指针
	DWORD          m_Frame;            //记录帧数

// 操作
public:

	void InitAVIWriteOpt();
// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnVfwInitvfw();
public:
	afx_msg void OnVfwVideoformat();
public:
	//afx_msg void OnVfwCompressor();
public:
	//afx_msg void OnVfwCapturevideo();
public:
	afx_msg void OnClose();
public:
	afx_msg void OnVfwPreviewvideo();
public:
	afx_msg void OnVfwCodec();
};


