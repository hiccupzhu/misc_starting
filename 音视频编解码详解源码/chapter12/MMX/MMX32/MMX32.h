// MMX32.h : main header file for the MMX32 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CMMX32App:
// See MMX32.cpp for the implementation of this class
//

class CMMX32App : public CWinApp
{
public:
	CMMX32App();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMMX32App theApp;