// MMX8.h : main header file for the MMX8 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CMMX8App:
// See MMX8.cpp for the implementation of this class
//

class CMMX8App : public CWinApp
{
public:
	CMMX8App();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMMX8App theApp;