// CaptureDevices.cpp : implementation file
//

#include "stdafx.h"
#include "Recorder.h"
#include "CaptureDevices.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CRecorderApp theApp;

INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName,VOID* pContext );


/////////////////////////////////////////////////////////////////////////////
// CCaptureDevices dialog


//-----------------------------------------------------------------------------
// Name: DSoundEnumCallback()
// Desc: Enumeration callback called by DirectSoundEnumerate
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName,
                                    VOID* pContext )
{
    // Set aside static storage space for 20 audio drivers
    static GUID  AudioDriverGUIDs[20];
    static DWORD dwAudioDriverIndex = 0;
    
    GUID* pTemp  = NULL;
    
    if( pGUID )
    {
        if( dwAudioDriverIndex >= 20 )
            return TRUE;
        
        pTemp = &AudioDriverGUIDs[dwAudioDriverIndex++];
        memcpy( pTemp, pGUID, sizeof(GUID) );
    }
    
    HWND hSoundDeviceCombo = (HWND)pContext;
    
    // Add the string to the combo box
    SendMessage( hSoundDeviceCombo, CB_ADDSTRING, 
        0, (LPARAM) (LPCTSTR) strDesc );
    
    // Get the index of the string in the combo box
    INT nIndex = (INT)SendMessage( hSoundDeviceCombo, CB_FINDSTRING, 
        0, (LPARAM) (LPCTSTR) strDesc );
    
    // Set the item data to a pointer to the static guid stored in AudioDriverGUIDs
    SendMessage( hSoundDeviceCombo, CB_SETITEMDATA, 
        nIndex, (LPARAM) pTemp );
    
    return TRUE;
}



CCaptureDevices::CCaptureDevices(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureDevices::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCaptureDevices)
		// NOTE: the ClassWizard will add member initialization here
    m_pCaptureGUID=NULL;
}


void CCaptureDevices::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCaptureDevices)
	DDX_Control(pDX, IDC_CAPTURE_DEVICE_COMBO, m_ctlDevices);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCaptureDevices, CDialog)
	//{{AFX_MSG_MAP(CCaptureDevices)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCaptureDevices message handlers

void CCaptureDevices::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

//-----------------------------------------------------------------------------
// Name: OnInitDevicesDialog()
// Desc: Initializes the devices dialog
//-----------------------------------------------------------------------------
BOOL CCaptureDevices::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    HWND hCaptureDeviceCombo = GetDlgItem(IDC_CAPTURE_DEVICE_COMBO)->GetSafeHwnd();
    DirectSoundCaptureEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
        (VOID*)hCaptureDeviceCombo );
    
    // Select the first device in the combo box
    ::SendMessage( hCaptureDeviceCombo, CB_SETCURSEL, 0, 0 );
    
    return S_OK;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CCaptureDevices::OnOK() 
{
    m_nDevIdx=m_ctlDevices.GetCurSel();
    m_pCaptureGUID=(GUID*)m_ctlDevices.GetItemData(m_nDevIdx);
    // Remember that guid
    if( m_pCaptureGUID ) 
          theApp.m_guidCaptureDevice = *m_pCaptureGUID;
	CDialog::OnOK();
}


