// ChildView.h : interface of the CChildView class
//


#pragma once

#include <atlimage.h>

#include "Img8Operations.h"

// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	//CImg8Operations m_Img8Operation;
// Operations
public:

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    void TestMMX();
    void InitImage();
    void ShowTime(__int64 nTime);
    void InvertImage(CImg8Operations::INVERT_IMAGE pFunction);
    void ChangeBrightness(CImg8Operations::CHANGE_BRIGHTNESS pFunction);

    CImg8Operations m_Operations;

    BOOL m_bMMX;            // TRUE if MMX is supported

    CImage m_Image;
    BYTE* m_pImageBits;     // points to start of the image bits in m_Image

    BYTE* m_pInitialImage;  // initial array of image bits


// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnOperationsReset();
    afx_msg void OnInvertcolorsAssemblyMmx();
    afx_msg void OnUpdateMmx(CCmdUI *pCmdUI);
    afx_msg void OnInvertcolorsC();
    afx_msg void OnInvertcolorsC3Mmx();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnChangebrightnessC();
    afx_msg void OnChangebrightnessAssemblyMmx();
    afx_msg void OnChangebrightnessCMmx();
};

