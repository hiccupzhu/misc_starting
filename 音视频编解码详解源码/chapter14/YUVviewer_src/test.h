#if !defined(AFX_TEST_H__5B41059F_E9C3_458C_A8B9_B661F58FBA22__INCLUDED_)
#define AFX_TEST_H__5B41059F_E9C3_458C_A8B9_B661F58FBA22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// test.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// test frame

class test : public CFrameWnd
{
	DECLARE_DYNCREATE(test)
protected:
	test();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(test)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~test();

	// Generated message map functions
	//{{AFX_MSG(test)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEST_H__5B41059F_E9C3_458C_A8B9_B661F58FBA22__INCLUDED_)
