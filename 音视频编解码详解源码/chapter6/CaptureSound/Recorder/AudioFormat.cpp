// AudioFormat.cpp : implementation file
//

#include "stdafx.h"
#include "Recorder.h"
#include "AudioFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioFormat dialog

extern CRecorderApp theApp;



//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------


HRESULT ScanAvailableInputFormats();
VOID    GetWaveFormatFromIndex( INT nIndex, WAVEFORMATEX* pwfx );
HRESULT FillFormatListBox( HWND hListBox, BOOL* aFormatSupported );
VOID    ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName );
HRESULT CreateCaptureBuffer( WAVEFORMATEX* pwfxInput );
HRESULT InitNotifications();


//-----------------------------------------------------------------------------
// Name: GetWaveFormatFromIndex()
// Desc: Returns 20 different wave formats based on nIndex
//-----------------------------------------------------------------------------
VOID GetWaveFormatFromIndex( INT nIndex, WAVEFORMATEX* pwfx )
{
    INT iSampleRate = nIndex / 4;
    INT iType = nIndex % 4;
    
    switch( iSampleRate )
    {
    case 0: pwfx->nSamplesPerSec = 48000; break;
    case 1: pwfx->nSamplesPerSec = 44100; break;
    case 2: pwfx->nSamplesPerSec = 22050; break;
    case 3: pwfx->nSamplesPerSec = 11025; break;
    case 4: pwfx->nSamplesPerSec =  8000; break;
    }
    
    switch( iType )
    {
    case 0: pwfx->wBitsPerSample =  8; pwfx->nChannels = 1; break;
    case 1: pwfx->wBitsPerSample = 16; pwfx->nChannels = 1; break;
    case 2: pwfx->wBitsPerSample =  8; pwfx->nChannels = 2; break;
    case 3: pwfx->wBitsPerSample = 16; pwfx->nChannels = 2; break;
    }
    
    pwfx->nBlockAlign = pwfx->nChannels * ( pwfx->wBitsPerSample / 8 );
    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
}



//-----------------------------------------------------------------------------
// Name: ConvertWaveFormatToString()
// Desc: Converts a wave format to a text string
//-----------------------------------------------------------------------------
VOID ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName )
{
    wsprintf( strFormatName, 
        TEXT("%u Hz, %u-bit %s"), 
        pwfx->nSamplesPerSec, 
        pwfx->wBitsPerSample, 
        ( pwfx->nChannels == 1 ) ? TEXT("Mono") : TEXT("Stereo") );
}


//-----------------------------------------------------------------------------
// Name: FillFormatListBox()
// Desc: Fills the format list box based on the availible formats
//-----------------------------------------------------------------------------
HRESULT FillFormatListBox( HWND hListBox, BOOL* aFormatSupported )
{
    TCHAR        strFormatName[255];
    WAVEFORMATEX wfx;
    DWORD        dwStringIndex;
    
    SendMessage( hListBox, LB_RESETCONTENT, 0, 0 );
    
    for( INT iIndex = 0; iIndex < 20; iIndex++ )
    {
        if( aFormatSupported[ iIndex ] )
        {
            // Turn the index into a WAVEFORMATEX then turn that into a
            // string and put the string in the listbox
            GetWaveFormatFromIndex( iIndex, &wfx );
            ConvertWaveFormatToString( &wfx, strFormatName );
            dwStringIndex = (DWORD)SendMessage( hListBox, LB_ADDSTRING, 0, 
                (LPARAM) (LPCTSTR) strFormatName );
            SendMessage( hListBox, LB_SETITEMDATA, dwStringIndex, iIndex );
        }
    }
    
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CreateCaptureBuffer()
// Desc: Creates a capture buffer and sets the format 
//-----------------------------------------------------------------------------
HRESULT CreateCaptureBuffer( WAVEFORMATEX* pwfxInput )
{
    HRESULT hr;
    DSCBUFFERDESC dscbd;
    
    SAFE_RELEASE(theApp.m_pDSNotify );
    SAFE_RELEASE(theApp.m_pDSBCapture );
    
    // Set the notification size
    theApp.m_dwNotifySize = MAX( 1024, pwfxInput->nAvgBytesPerSec / 8 );
    theApp.m_dwNotifySize -= theApp.m_dwNotifySize % pwfxInput->nBlockAlign;  
    
    // Set the buffer sizes 
    theApp.m_dwCaptureBufferSize = theApp.m_dwNotifySize * NUM_REC_NOTIFICATIONS;
    
    SAFE_RELEASE( theApp.m_pDSNotify );
    SAFE_RELEASE( theApp.m_pDSBCapture );
    
    // Create the capture buffer
    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize        = sizeof(dscbd);
    dscbd.dwBufferBytes = theApp.m_dwCaptureBufferSize;
    dscbd.lpwfxFormat   = pwfxInput; // Set the format during creatation
    
    if( FAILED( hr = theApp.m_pDSCapture->CreateCaptureBuffer( &dscbd, 
        &theApp.m_pDSBCapture, 
        NULL ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"), hr );
    
    theApp.m_dwNextCaptureOffset = 0;
    
    if( FAILED( hr = InitNotifications() ) )
        return DXTRACE_ERR_MSGBOX( TEXT("InitNotifications"), hr );
    
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: InitNotifications()
// Desc: Inits the notifications on the capture buffer which are handled
//       in WinMain()
//-----------------------------------------------------------------------------
HRESULT InitNotifications()
{
    HRESULT hr; 
    
    if( NULL == theApp.m_pDSBCapture )
        return E_FAIL;
    
    // Create a notification event, for when the sound stops playing
    if( FAILED( hr = theApp.m_pDSBCapture->QueryInterface( IID_IDirectSoundNotify, 
        (VOID**)&(theApp.m_pDSNotify )) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("QueryInterface"), hr );
    
    // Setup the notification positions
    for( INT i = 0; i < NUM_REC_NOTIFICATIONS; i++ )
    {
        theApp.m_aPosNotify[i].dwOffset = (theApp.m_dwNotifySize * i) + theApp.m_dwNotifySize - 1;
        theApp.m_aPosNotify[i].hEventNotify = theApp.m_hNotificationEvent;             
    }
    
    // Tell DirectSound when to notify us. the notification will come in the from 
    // of signaled events that are handled in WinMain()
    if( FAILED( hr = theApp.m_pDSNotify->SetNotificationPositions( NUM_REC_NOTIFICATIONS, 
        theApp.m_aPosNotify ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("SetNotificationPositions"), hr );
    
    return S_OK;
}



CAudioFormat::CAudioFormat(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioFormat::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioFormat)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAudioFormat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioFormat)
	DDX_Control(pDX, IDC_FORMATS_INPUT_LISTBOX, m_ctlFormats);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioFormat, CDialog)
	//{{AFX_MSG_MAP(CAudioFormat)
	ON_LBN_SELCHANGE(IDC_FORMATS_INPUT_LISTBOX, OnSelchangeFormatsInputListbox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioFormat message handlers

BOOL CAudioFormat::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    ::EnableWindow(GetDlgItem(IDOK)->GetSafeHwnd(),false);

	// TODO: Add extra initialization here
	ScanAvailableInputFormats();

    HWND hInputList = GetDlgItem(IDC_FORMATS_INPUT_LISTBOX )->GetSafeHwnd();
    FillFormatListBox( hInputList, theApp.m_abInputFormatSupported );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



//-----------------------------------------------------------------------------
// Name: ScanAvailableInputFormats()
// Desc: Tests to see if 20 different standard wave formats are supported by
//       the capture device 
//-----------------------------------------------------------------------------
int CAudioFormat::ScanAvailableInputFormats()
{
    HRESULT       hr;
    WAVEFORMATEX  wfx;
    HCURSOR       hCursor;
    DSCBUFFERDESC dscbd;
    LPDIRECTSOUNDCAPTUREBUFFER pDSCaptureBuffer = NULL;
    
    // This might take a second or two, so throw up the hourglass
    hCursor = GetCursor();
    SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    
    ZeroMemory( &wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    
    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize = sizeof(dscbd);
    
    // Try 20 different standard formats to see if they are supported
    for( INT iIndex = 0; iIndex < 20; iIndex++ )
    {
        GetWaveFormatFromIndex( iIndex, &wfx );
        
        // To test if a capture format is supported, try to create a 
        // new capture buffer using a specific format.  If it works
        // then the format is supported, otherwise not.
        dscbd.dwBufferBytes = wfx.nAvgBytesPerSec;
        dscbd.lpwfxFormat = &wfx;
        
        if( FAILED( hr = theApp.m_pDSCapture->CreateCaptureBuffer( &dscbd, 
            &pDSCaptureBuffer, 
            NULL ) ) )
            theApp.m_abInputFormatSupported[ iIndex ] = FALSE;
        else
            theApp.m_abInputFormatSupported[ iIndex ] = TRUE;
        
        SAFE_RELEASE( pDSCaptureBuffer );
    }
    
    SetCursor( hCursor );
    
    return S_OK;
}

void CAudioFormat::OnOK() 
{
	// TODO: Add extra validation here
    HRESULT       hr;
    DWORD         dwInputSelect;
    DWORD         dwInputWavIndex;
    HWND          hInputList;
    
    hInputList = GetDlgItem(IDC_FORMATS_INPUT_LISTBOX )->GetSafeHwnd();
    
    dwInputSelect   = (DWORD)::SendMessage( hInputList, LB_GETCURSEL, 0, 0 );
    dwInputWavIndex = (DWORD)::SendMessage( hInputList, LB_GETITEMDATA, dwInputSelect, 0 );
    
    ZeroMemory( &(theApp.m_wfxInput), sizeof(WAVEFORMATEX));
    theApp.m_wfxInput.wFormatTag = WAVE_FORMAT_PCM;
    
    GetWaveFormatFromIndex( dwInputWavIndex, &(theApp.m_wfxInput));
    
    if( FAILED( hr = CreateCaptureBuffer( &theApp.m_wfxInput ) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"), hr );
    }

	CDialog::OnOK();
}

void CAudioFormat::OnSelchangeFormatsInputListbox() 
{
	// TODO: Add your control notification handler code here
    HWND hInputList  = GetDlgItem( IDC_FORMATS_INPUT_LISTBOX )->GetSafeHwnd();
    HWND hOK         = GetDlgItem( IDOK )->GetSafeHwnd();
    
    if( ::SendMessage( hInputList,  LB_GETCURSEL, 0, 0 ) != -1 )        
        ::EnableWindow( hOK, TRUE );
    else
        ::EnableWindow( hOK, FALSE );

}
