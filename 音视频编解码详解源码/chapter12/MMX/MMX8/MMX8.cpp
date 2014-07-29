// MMX8.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MMX8.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



/*
Written by: Alex Farber      alexf_1@hotmail.com


Using MMX for 8-bpp image transformations

Sources:
1. Intel Software manuals - http://developer.intel.com/design/archives/processors/mmx/index.htm

Volume 1: Basic Architecture
CHAPTER 8
PROGRAMMING WITH THE INTEL MMX™ TECHNOLOGY

Volume 2: Instruction Set Reference


2. MSDN, MMX Technology
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/vcrefsupportformmxtechnology.asp


3. Microsoft Visual C++ CPUID sample.
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vcsample/html/vcsamcpuiddeterminecpucapabilities.asp


4. Microsoft Visual C++ MMXSwarm sample.
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vcsample/html/vcsamMMXSwarmSampleDemonstratesCImageVisualCsMMXSupport.asp


5. Matt Pietrek. Under The Hood.
February 1998 issue of Microsoft Systems Journal
Overview of Assembly commands and common Intel x86 Registers.
http://www.microsoft.com/msj/0298/hood0298.aspx


Making the simple image processing operations using:
- C++ code
- C++ code with MMX Technology Intrinsics
- Assembly code with MMX intrinsics

Time for each operation is shown in the status bar.


Two image processing operations are executed:
- inverting;
- changing of brightness (adding or subtracting some constant value to each pixel).


MMX code (botn Assembly and C++) performs much better (about 30%) than pure C++ code. 


Using the MMX Intrinsics is quite simple and doesn't require Assembly language
knowledge. In the most cases it allows to get the same execution speed like
Assembly program. However, Assembly code may sometimes perform better because
of intensive using of the registers.


Execution time should be estimated in the Release configuration,
with compiler optimizations.

*/


// CMMX8App

BEGIN_MESSAGE_MAP(CMMX8App, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CMMX8App construction

CMMX8App::CMMX8App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CMMX8App object

CMMX8App theApp;

// CMMX8App initialization

BOOL CMMX8App::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);
	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}


// CMMX8App message handlers



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CMMX8App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CMMX8App message handlers

