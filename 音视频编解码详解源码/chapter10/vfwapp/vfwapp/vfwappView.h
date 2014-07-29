// vfwappView.h : CvfwappView 类的接口
//


#pragma once
#include <vfw.h>

class CvfwappView : public CView
{
protected: // 仅从序列化创建
	CvfwappView();
	DECLARE_DYNCREATE(CvfwappView)

// 属性
public:
	CvfwappDoc* GetDocument() const;

// 操作
public:
	HDRAWDIB m_hdd;
// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 实现
public:
	virtual ~CvfwappView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // vfwappView.cpp 中的调试版本
inline CvfwappDoc* CvfwappView::GetDocument() const
   { return reinterpret_cast<CvfwappDoc*>(m_pDocument); }
#endif

