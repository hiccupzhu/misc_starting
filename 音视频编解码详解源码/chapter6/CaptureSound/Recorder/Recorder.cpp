// Recorder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Recorder.h"
#include "RecorderDlg.h"
#include <process.h> 
#include <Windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecorderApp
HRESULT CreateCaptureBuffer( WAVEFORMATEX* pwfxInput );

UINT ReceiveDataThread(void  *pParam);


BEGIN_MESSAGE_MAP(CRecorderApp, CWinApp)
	//{{AFX_MSG_MAP(CRecorderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecorderApp construction

CRecorderApp::CRecorderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRecorderApp object

CRecorderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CRecorderApp initialization

BOOL CRecorderApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CRecorderDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
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

//-----------------------------------------------------------------------------
// Name: InitDirectSound()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
int CRecorderApp::InitDirectSound()
{
    HRESULT hr;
    m_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hStopThreadEvent   = CreateEvent(NULL,FALSE,FALSE, NULL);

    ZeroMemory( &m_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * (NUM_REC_NOTIFICATIONS + 1) );

    m_dwCaptureBufferSize = 0;
    m_dwNotifySize        = 0;
    m_pWaveFile           = NULL;
    m_bRecording=false;
    m_hThread=NULL;

    // Initialize COM
    if( FAILED( hr = CoInitialize(NULL) ) )
    {
         DXTRACE_ERR_MSGBOX( TEXT("CoInitialize"), hr );
         return -1;
    }
    
    
    // Create IDirectSoundCapture using the preferred capture device
    if( FAILED( hr = DirectSoundCaptureCreate((LPGUID)&m_guidCaptureDevice, &m_pDSCapture, NULL ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("DirectSoundCaptureCreate"), hr );
        MessageBox( NULL, "Error initializing DirectSound.  Sample will now exit.", 
                        "DirectSound Sample", MB_OK | MB_ICONERROR );
        return -2;
    }
    
    return 0;
}

int CRecorderApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
    SAFE_DELETE( m_pWaveFile );
    
    // Release DirectSound interfaces
    SAFE_RELEASE( m_pDSNotify );
    SAFE_RELEASE( m_pDSBCapture );
    SAFE_RELEASE( m_pDSCapture ); 
    
    // Release COM
    CoUninitialize();
    
    CloseHandle( m_hNotificationEvent );

    CloseHandle(m_hStopThreadEvent);

    CloseHandle(m_hThread);

	return CWinApp::ExitInstance();
}



//-----------------------------------------------------------------------------
// Name: StartOrStopRecord()
// Desc: Starts or stops the capture buffer from recording
//-----------------------------------------------------------------------------
int CRecorderApp::StartOrStopRecord(BOOL bStartRecording)
{
    HRESULT hr;
    
    if( bStartRecording )
    {
        //Create capture data thread
        unsigned int ThrdAddr;
        m_hThread  = (HANDLE) _beginthreadex(NULL, 
            0, 
            (unsigned int (__stdcall *)(void *))ReceiveDataThread,
            (void *)(this),//NULL, 
            0, &ThrdAddr);

        // Create a capture buffer, and tell the capture 
        // buffer to start recording   
        if( FAILED( hr = CreateCaptureBuffer( &m_wfxInput ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"), hr );
        
        if( FAILED( hr = m_pDSBCapture->Start( DSCBSTART_LOOPING ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("Start"), hr );
    }
    else
    {
        // Stop the capture and read any data that 
        // was not caught by a notification
        if( NULL == m_pDSBCapture )
            return S_OK;
        
        // Stop the buffer, and read any data that was not 
        // caught by a notification
        if( FAILED( hr = m_pDSBCapture->Stop() ) )
            return DXTRACE_ERR_MSGBOX( TEXT("Stop"), hr );
        
        SetEvent(m_hStopThreadEvent);

        // Close the wav file
        SAFE_DELETE( m_pWaveFile );
    }
    
    return S_OK; 
}





//-----------------------------------------------------------------------------
// Name: RecordCapturedData()
// Desc: Copies data from the capture buffer to the output buffer 
//-----------------------------------------------------------------------------
int CRecorderApp::RecordCapturedData()
{
    HRESULT hr;
    VOID*   pbCaptureData    = NULL;
    DWORD   dwCaptureLength;
    VOID*   pbCaptureData2   = NULL;
    DWORD   dwCaptureLength2;
    UINT    dwDataWrote;
    DWORD   dwReadPos;
    DWORD   dwCapturePos;
    LONG lLockSize;

    if( NULL == m_pDSBCapture )
        return S_FALSE;
    if( NULL == m_pWaveFile )
        return S_FALSE;

    if( FAILED( hr = m_pDSBCapture->GetCurrentPosition( &dwCapturePos, &dwReadPos ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("GetCurrentPosition"), hr );

    lLockSize = dwReadPos - m_dwNextCaptureOffset;
    if( lLockSize < 0 )
        lLockSize += m_dwCaptureBufferSize;

    // Block align lock size so that we are always write on a boundary
    lLockSize -= (lLockSize % m_dwNotifySize);

    if( lLockSize == 0 )
        return S_FALSE;

    // Lock the capture buffer down
    if( FAILED( hr = m_pDSBCapture->Lock( m_dwNextCaptureOffset, lLockSize, 
                                          &pbCaptureData, &dwCaptureLength, 
                                          &pbCaptureData2, &dwCaptureLength2, 0L ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("Lock"), hr );

    // Write the data into the wav file
    if( FAILED( hr = m_pWaveFile->Write( dwCaptureLength, 
                                              (BYTE*)pbCaptureData, 
                                              &dwDataWrote ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("Write"), hr );

    // Move the capture offset along
    m_dwNextCaptureOffset += dwCaptureLength; 
    m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer

    if( pbCaptureData2 != NULL )
    {
        // Write the data into the wav file
        if( FAILED( hr = m_pWaveFile->Write( dwCaptureLength2, 
                                                  (BYTE*)pbCaptureData2, 
                                                  &dwDataWrote ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("Write"), hr );

        // Move the capture offset along
        m_dwNextCaptureOffset += dwCaptureLength2; 
        m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer
    }

    // Unlock the capture buffer
    m_pDSBCapture->Unlock( pbCaptureData,  dwCaptureLength, 
                           pbCaptureData2, dwCaptureLength2 );


    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ReceiveDataThread()
// Desc: This thread waits for data available event to capture audio data 
//-----------------------------------------------------------------------------
UINT ReceiveDataThread(void  *pParam)
{
    CRecorderApp *pApp=(CRecorderApp*)pParam;

    HANDLE hArray[2]={pApp->m_hNotificationEvent,pApp->m_hStopThreadEvent};

    DWORD EventResult;

    while(1)
    {

        EventResult = WaitForMultipleObjects(
            sizeof(hArray)/sizeof(HANDLE),
            hArray,
            FALSE,
		    INFINITE);

        if(WAIT_OBJECT_0+1 == EventResult)
            break;
        
        if(WAIT_OBJECT_0 == EventResult)
		{
            pApp->RecordCapturedData();
            ResetEvent(pApp->m_hNotificationEvent);
        }
    }

    return 0;
}

