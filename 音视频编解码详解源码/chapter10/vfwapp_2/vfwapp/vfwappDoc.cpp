// vfwappDoc.cpp : CvfwappDoc 类的实现
//

#include "stdafx.h"
#include "vfwapp.h"

#include "vfwappDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CvfwappDoc

IMPLEMENT_DYNCREATE(CvfwappDoc, CDocument)

BEGIN_MESSAGE_MAP(CvfwappDoc, CDocument)
END_MESSAGE_MAP()


// CvfwappDoc 构造/析构

CvfwappDoc::CvfwappDoc()
{
	// TODO: 在此添加一次性构造代码

}

CvfwappDoc::~CvfwappDoc()
{
}

BOOL CvfwappDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CvfwappDoc 序列化

void CvfwappDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}


// CvfwappDoc 诊断

#ifdef _DEBUG
void CvfwappDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CvfwappDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CvfwappDoc 命令
