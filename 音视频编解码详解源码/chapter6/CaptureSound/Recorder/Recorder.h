// Recorder.h : main header file for the RECORDER application
//

#if !defined(AFX_RECORDER_H__CD066190_2DA2_4830_926A_35670557CA1C__INCLUDED_)
#define AFX_RECORDER_H__CD066190_2DA2_4830_926A_35670557CA1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include <basetsd.h>
#include <commdlg.h>
#include <dxerr9.h>
#include "resource.h"
#include "DSUtil.h"
#include "DXUtil.h"

/////////////////////////////////////////////////////////////////////////////
// CRecorderApp:
// See Recorder.cpp for the implementation of this class
//
//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define NUM_REC_NOTIFICATIONS  16
#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

class CRecorderApp : public CWinApp
{
public:
	CRecorderApp();

public:
	int RecordCapturedData();
	int StartOrStopRecord( BOOL bStartRecording );
	int InitDirectSound();
    LPDIRECTSOUNDCAPTURE       m_pDSCapture;
    LPDIRECTSOUNDCAPTUREBUFFER m_pDSBCapture;
    LPDIRECTSOUNDNOTIFY        m_pDSNotify;
    HINSTANCE                  m_hInst;
    GUID                       m_guidCaptureDevice;
    BOOL                       m_bRecording;
    WAVEFORMATEX               m_wfxInput;
    DSBPOSITIONNOTIFY          m_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ];  
    HANDLE                     m_hNotificationEvent; 
    HANDLE                     m_hStopThreadEvent;
    BOOL                       m_abInputFormatSupported[20];
    DWORD                      m_dwCaptureBufferSize;
    DWORD                      m_dwNextCaptureOffset;
    DWORD                      m_dwNotifySize;
    CWaveFile*                 m_pWaveFile;

    HANDLE                     m_hThread;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecorderApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRecorderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDER_H__CD066190_2DA2_4830_926A_35670557CA1C__INCLUDED_)
