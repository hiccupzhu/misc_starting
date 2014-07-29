// SSESample.cpp : Defines the class behaviors for the application.
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


3. Microsoft Visual C++ CPUID sample.
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vcsample/html/vcsamcpuiddeterminecpucapabilities.asp


4. Matt Pietrek. Under The Hood.
February 1998 issue of Microsoft Systems Journal
Overview of Assembly commands and common Intel x86 Registers.
http://www.microsoft.com/msj/0298/hood0298.aspx


On the start program checks support for SSE and disables appropriate
buttons id SSE is not supported - see CSSESampleDlg::OnInitDialog.

Class CSSESampleDlg has two float arrays. First of them (initial) is
filled on the program start and shown in the listbox.

Second array is filled using first array. Each it's element is calculated
as some function of appropriate initial array member - see CSSESampleDlg::InitArray. 
The conversion result is shown in the listbox. Program also computes
minimum and maximum values in the result array.

Conversion may be done by one of the following ways:
1) C++ code
2) C++ code using inline Assembly with SSE instructions
3) C++ code using SSE Intrinsics

Execution time of each conversion way is shown in the dialog.

Conclusion:
Using SSE may significally reduce calculation time.
Using SSE Intrinsics gives almost same results as  inline assembly
(assembly program may be faster because of intensive using of
registers). But programming with SSE Intrinsics is much simpler.

Execution time on my computer:

C++ - 13 ms
C++ code using inline Assembly - 3 ms
C++ code using SSE Intrinsics - 4 ms

Execution time should be estimated in the Release configuration,
with compiler optimizations.

*/


#include "stdafx.h"
#include "SSESample.h"
#include "SSESampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSSESampleApp

BEGIN_MESSAGE_MAP(CSSESampleApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSSESampleApp construction

CSSESampleApp::CSSESampleApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CSSESampleApp object

CSSESampleApp theApp;



// CSSESampleApp initialization

BOOL CSSESampleApp::InitInstance()
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

	CSSESampleDlg dlg;
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
