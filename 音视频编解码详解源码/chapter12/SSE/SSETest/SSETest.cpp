// SSETest.cpp : Defines the class behaviors for the application.
//


/*
Written by: Alex Farber      alexf_1@hotmail.com

Using SSE for floating-point operations.

Sources:
1. Intel Software manuals - http://developer.intel.com/design/archives/processors/mmx/index.htm

Volume 1: Basic Architecture
CHAPTER 9
PROGRAMMING WITH THE STREAMING SIMD EXTENSIONS

Volume 2: Instruction Set Reference


2. MSDN, Streaming SIMD Extensions (SSE)
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/vcrefstreamingsimdextensions.asp


3. Waterfall chart control written by Kris Jearakul
http://www.codeguru.com/controls/Waterfall.shtml


4. Microsoft Visual C++ CPUID sample.
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vcsample/html/vcsamcpuiddeterminecpucapabilities.asp


5. Matt Pietrek. Under The Hood.
February 1998 issue of Microsoft Systems Journal
Overview of Assembly commands and common Intel x86 Registers.
http://www.microsoft.com/msj/0298/hood0298.aspx


Program makes arithmetic operations with float arrays using C++ code
and SSE instructions. Execution time of each computation way is shown in the dialog.

Graph control written by Kris Jearakul is used to show computation results.



Conclusion:
Using SSE may significally reduce calculation time.
Using SSE Intrinsics gives the same results as  inline assembly
But programming with SSE Intrinsics is much simpler.
(In the second program, SSESample, Assembly code is faster
than C++ SSE code, because of intensive using of SSE registers.)

Execution time on my computer:

C++ - 26 ms
C++ code using inline Assembly - 9 ms
C++ code using SSE Intrinsics - 9 ms

Execution time should be estimated in the Release configuration,
with compiler optimizations.

*/



#include "stdafx.h"
#include "SSETest.h"
#include "SSETestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSSETestApp

BEGIN_MESSAGE_MAP(CSSETestApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSSETestApp construction

CSSETestApp::CSSETestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}



// The one and only CSSETestApp object

CSSETestApp theApp;


// CSSETestApp initialization

BOOL CSSETestApp::InitInstance()
{

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CSSETestDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
