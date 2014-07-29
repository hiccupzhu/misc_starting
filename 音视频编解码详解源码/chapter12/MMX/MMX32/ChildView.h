// ChildView.h : interface of the CChildView class
//


#pragma once

#include <atlimage.h>


#include "Img32Operations.h"

// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

protected:
    void InitImage();
    void TestMMX();
    void ShowTime(__int64 nTime);
    void InvertImage(CImg32Operations::INVERT_IMAGE pFunction);
    void ComputeColors(CImg32Operations::COMPUTE_COLORS pFunction);

    CImg32Operations m_Operations;

    BOOL m_bMMX;            // TRUE if MMX is supported

    CImage m_Image;
    BYTE* m_pImageBits;     // points to start of the image bits in m_Image

    BYTE* m_pInitialImage;  // initial array of image bits


	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnColorsC();
    afx_msg void OnColorsCMmx();
    afx_msg void OnUpdateMmx(CCmdUI *pCmdUI);
    afx_msg void OnColorsAssemblyMmx();
    afx_msg void OnInvertcolorsC();
    afx_msg void OnInvertcolorsAssemblyMmx();
    afx_msg void OnInvertcolorsCMmx();
    afx_msg void OnOperationsReset();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

