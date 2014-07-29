; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CRecorderDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Recorder.h"

ClassCount=5
Class1=CRecorderApp
Class2=CRecorderDlg
Class3=CAboutDlg

ResourceCount=5
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_DEVICE
Resource4=IDD_RECORDER_DIALOG
Class4=CAudioFormat
Class5=CCaptureDevices
Resource5=IDD_FORMATS

[CLS:CRecorderApp]
Type=0
HeaderFile=Recorder.h
ImplementationFile=Recorder.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC
LastObject=CRecorderApp

[CLS:CRecorderDlg]
Type=0
HeaderFile=RecorderDlg.h
ImplementationFile=RecorderDlg.cpp
Filter=D
LastObject=IDC_FORMAT
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=RecorderDlg.h
ImplementationFile=RecorderDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_RECORDER_DIALOG]
Type=1
Class=CRecorderDlg
ControlCount=6
Control1=IDC_FORMAT,static,1342177280
Control2=IDC_BUTTON_SELFILE,button,1342242816
Control3=IDC_FILENAME,edit,1350631552
Control4=IDC_BUTTON_HELP,button,1342242816
Control5=IDC_RECORD,button,1342259200
Control6=IDC_MAIN_INPUTFORMAT_TEXT,static,1342308352

[DLG:IDD_DEVICE]
Type=1
Class=CCaptureDevices
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_CAPTURE_DEVICE_COMBO,combobox,1344340226
Control4=IDC_STATIC,static,1342308352

[DLG:IDD_FORMATS]
Type=1
Class=CAudioFormat
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_FORMATS_INPUT_LISTBOX,listbox,1352728835

[CLS:CAudioFormat]
Type=0
HeaderFile=AudioFormat.h
ImplementationFile=AudioFormat.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CAudioFormat

[CLS:CCaptureDevices]
Type=0
HeaderFile=CaptureDevices.h
ImplementationFile=CaptureDevices.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CCaptureDevices

