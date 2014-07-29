//
// CDXGraph.cpp
//

#include "stdafx.h"
#include <streams.h>
#include "CDXGraph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////////
CDXGraph::CDXGraph()
{
	pGraph        = NULL;
	pMediaControl = NULL;
	pMediaEvent   = NULL;
	pBasicVideo   = NULL;
	pBasicAudio   = NULL;
	pVideoWindow  = NULL;
	pMediaSeeking = NULL;

	mObjectTableEntry = 0;
}

CDXGraph::~CDXGraph()
{
	Release();
}

bool CDXGraph::Create(void)
{
	if (!pGraph)
	{
		if (SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void **)&pGraph)))
		{
			AddToObjectTable();

			return QueryInterfaces();
		}
		pGraph = 0;
	}
	return false;
}

bool CDXGraph::QueryInterfaces(void)
{
	if (pGraph)
	{
		HRESULT hr = NOERROR;
		hr |= pGraph->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
		hr |= pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pMediaEvent);
		hr |= pGraph->QueryInterface(IID_IBasicVideo, (void **)&pBasicVideo);
		hr |= pGraph->QueryInterface(IID_IBasicAudio, (void **)&pBasicAudio);
		hr |= pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVideoWindow);
		hr |= pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pMediaSeeking);
		if (pMediaSeeking)
		{
			pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
		}
		return SUCCEEDED(hr);
	}
	return false;
}

void CDXGraph::Release(void)
{
	if (pMediaSeeking)
	{
		pMediaSeeking->Release();
		pMediaSeeking = NULL;
	}
	if (pMediaControl)
	{
		pMediaControl->Release();
		pMediaControl = NULL;
	}
	if (pMediaEvent)
	{
		pMediaEvent->Release();
		pMediaEvent = NULL;
	}
	if (pBasicVideo)
	{
		pBasicVideo->Release();
		pBasicVideo = NULL;
	}
	if (pBasicAudio)
	{
		pBasicAudio->Release();
		pBasicAudio = NULL;
	}
	if (pVideoWindow)
	{
		pVideoWindow->put_Visible(OAFALSE);
		pVideoWindow->put_MessageDrain((OAHWND)NULL);
		pVideoWindow->put_Owner(OAHWND(0));
		pVideoWindow->Release();
		pVideoWindow = NULL;
	}
	RemoveFromObjectTable();
	if (pGraph) 
	{
		pGraph->Release(); 
		pGraph = NULL;
	}
}

bool CDXGraph::Attach(IGraphBuilder * inGraphBuilder)
{
	Release();

	if (inGraphBuilder)
	{
		inGraphBuilder->AddRef();
		pGraph = inGraphBuilder;

		AddToObjectTable();
		return QueryInterfaces();
	}
	return true;
}

//IGraphBuilder * CDXGraph::GetGraph(void)
//{
//	return pGraph;
//}

IMediaEventEx * CDXGraph::GetEventHandle(void)
{
	return pMediaEvent;
}

// Connect filter from the upstream output pin to the downstream input pin
bool CDXGraph::ConnectFilters(IPin * inOutputPin, IPin * inInputPin, 
							  const AM_MEDIA_TYPE * inMediaType)
{
	if (pGraph && inOutputPin && inInputPin)
	{
		HRESULT hr = pGraph->ConnectDirect(inOutputPin, inInputPin, inMediaType);
		return SUCCEEDED(hr) ? true : false;
	}
	return false;
}

void CDXGraph::DisconnectFilters(IPin * inOutputPin)
{
	if (pGraph && inOutputPin)
	{
		HRESULT hr = pGraph->Disconnect(inOutputPin);
	}
}

//输入显示窗口的句柄：inWindow
bool CDXGraph::SetDisplayWindow(HWND inWindow)
{	
	if (pVideoWindow)
	{
		// 首先隐藏视频窗口
		pVideoWindow->put_Visible(OAFALSE);
		pVideoWindow->put_Owner((OAHWND)inWindow);

         //获取输入窗口的显示区域
		RECT windowRect;
		::GetClientRect(inWindow, &windowRect);
		pVideoWindow->put_Left(0);
		pVideoWindow->put_Top(0);
		pVideoWindow->put_Width(windowRect.right - windowRect.left);
		pVideoWindow->put_Height(windowRect.bottom - windowRect.top);
		pVideoWindow->put_WindowStyle(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
		pVideoWindow->put_MessageDrain((OAHWND) inWindow);
		// 回复视频窗口
		if (inWindow != NULL)
		{
			pVideoWindow->put_Visible(OATRUE);
		}
		else
		{
			pVideoWindow->put_Visible(OAFALSE);
		}
		return true;
	}
	return false;
}

bool CDXGraph::ResizeVideoWindow(long inLeft, long inTop, long inWidth, long inHeight)
{
	if (pVideoWindow)
	{
		long lVisible = OATRUE;
		pVideoWindow->get_Visible(&lVisible);
		// Hide the video window first
		pVideoWindow->put_Visible(OAFALSE);

		pVideoWindow->put_Left(inLeft);
		pVideoWindow->put_Top(inTop);
		pVideoWindow->put_Width(inWidth);
		pVideoWindow->put_Height(inHeight);
	
		// Restore the video window
		pVideoWindow->put_Visible(lVisible);
		return true;
	}
	return false;
}

bool CDXGraph::SetNotifyWindow(HWND inWindow)
{
	if (pMediaEvent)
	{
		pMediaEvent->SetNotifyWindow((OAHWND)inWindow, WM_GRAPHNOTIFY, 0);
		return true;
	}
	return false;
}

void CDXGraph::HandleEvent(WPARAM inWParam, LPARAM inLParam)
{
	if (pMediaEvent)
	{
		LONG eventCode = 0, eventParam1 = 0, eventParam2 = 0;
		while (SUCCEEDED(pMediaEvent->GetEvent(&eventCode, &eventParam1, &eventParam2, 0)))
		{
			pMediaEvent->FreeEventParams(eventCode, eventParam1, eventParam2);
			switch (eventCode)
			{
			case EC_COMPLETE:
				break;

			case EC_USERABORT:
			case EC_ERRORABORT:
				break;

			default:
				break;
			}
		}
	}
}

bool CDXGraph::Run(void)
{
	if (pGraph && pMediaControl)
	{
		if (!IsRunning())
		{
			if (SUCCEEDED(pMediaControl->Run()))
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	return false;
}

bool CDXGraph::Stop(void)
{
	if (pGraph && pMediaControl)
	{
		if (!IsStopped())
		{	
			if (SUCCEEDED(pMediaControl->Stop()))
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	return false;
}

bool CDXGraph::Pause(void)
{
	if (pGraph && pMediaControl)
	{
		if (!IsPaused())
		{	
			if (SUCCEEDED(pMediaControl->Pause()))
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	return false;
}

bool CDXGraph::IsRunning(void)
{
	if (pGraph && pMediaControl)
	{
		OAFilterState state = State_Stopped;
		if (SUCCEEDED(pMediaControl->GetState(10, &state)))
		{
			return state == State_Running;
		}
	}
	return false;
}

bool CDXGraph::IsStopped(void)
{
	if (pGraph && pMediaControl)
	{
		OAFilterState state = State_Stopped;
		if (SUCCEEDED(pMediaControl->GetState(10, &state)))
		{
			return state == State_Stopped;
		}
	}
	return false;
}

bool CDXGraph::IsPaused(void)
{
	if (pGraph && pMediaControl)
	{
		OAFilterState state = State_Stopped;
		if (SUCCEEDED(pMediaControl->GetState(10, &state)))
		{
			return state == State_Paused;
		}
	}
	return false;
}

bool CDXGraph::SetFullScreen(BOOL inEnabled)
{
	if (pVideoWindow)
	{
		HRESULT hr = pVideoWindow->put_FullScreenMode(inEnabled ? OATRUE : OAFALSE);
		return SUCCEEDED(hr);
	}
	return false;
}

bool CDXGraph::GetFullScreen(void)
{
	if (pVideoWindow)
	{
		long  fullScreenMode = OAFALSE;
		pVideoWindow->get_FullScreenMode(&fullScreenMode);
		return (fullScreenMode == OATRUE);
	}
	return false;
}

// IMediaSeeking features
bool CDXGraph::GetCurrentPosition(double * outPosition)
{
	if (pMediaSeeking)
	{
		__int64 position = 0;
		if (SUCCEEDED(pMediaSeeking->GetCurrentPosition(&position)))
		{
			*outPosition = ((double)position) / 10000000.;
			return true;
		}
	}
	return false;
}

bool CDXGraph::GetStopPosition(double * outPosition)
{
	if (pMediaSeeking)
	{
		__int64 position = 0;
		if (SUCCEEDED(pMediaSeeking->GetStopPosition(&position)))
		{
			*outPosition = ((double)position) / 10000000.;
			return true;
		}
	}
	return false;
}

bool CDXGraph::SetCurrentPosition(double inPosition)
{
	if (pMediaSeeking)
	{
		__int64 one = 10000000;
		__int64 position = (__int64)(one * inPosition);
		HRESULT hr = pMediaSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
			0, AM_SEEKING_NoPositioning);
		return SUCCEEDED(hr);
	}
	return false;
}

bool CDXGraph::SetStartStopPosition(double inStart, double inStop)
{
	if (pMediaSeeking)
	{
		__int64 one = 10000000;
		__int64 startPos = (__int64)(one * inStart);
		__int64 stopPos  = (__int64)(one * inStop);
		HRESULT hr = pMediaSeeking->SetPositions(&startPos, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
			&stopPos, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame);
		return SUCCEEDED(hr);
	}
	return false;
}

bool CDXGraph::GetDuration(double * outDuration)
{
	if (pMediaSeeking)
	{
		__int64 length = 0;
		if (SUCCEEDED(pMediaSeeking->GetDuration(&length)))
		{
			*outDuration = ((double)length) / 10000000.;
			return true;
		}
	}
	return false;
}

bool CDXGraph::SetPlaybackRate(double inRate)
{
	if (pMediaSeeking)
	{
		if (SUCCEEDED(pMediaSeeking->SetRate(inRate)))
		{
			return true;
		}
	}
	return false;
}

// Attention: range from -10000 to 0, and 0 is FULL_VOLUME.
bool CDXGraph::SetAudioVolume(long inVolume)
{
	if (pBasicAudio)
	{
		HRESULT hr = pBasicAudio->put_Volume(inVolume);
		return SUCCEEDED(hr);
	}
	return false;
}

long CDXGraph::GetAudioVolume(void)
{
	long volume = 0;
	if (pBasicAudio)
	{
		pBasicAudio->get_Volume(&volume);
	}
	return volume;
}

void CDXGraph::ChangeAudioVolume(int nVolume)
{
	ASSERT(nVolume >= 0 && nVolume <= 100);

	m_nVolume = nVolume;

	if (!pBasicAudio)
		return;

	long lVolume = (nVolume - 100) * 100;
	pBasicAudio->put_Volume(lVolume);
}

void CDXGraph::Mute()
{
	if (!pBasicAudio)
		return;

	pBasicAudio->put_Volume(-10000);
}

void CDXGraph::UnMute()
{
	if (!pBasicAudio)
		return;

	long lVolume = (m_nVolume - 100) * 100;
	pBasicAudio->put_Volume(lVolume);
}

// Attention: range from -10000(left) to 10000(right), and 0 is both.
bool CDXGraph::SetAudioBalance(long inBalance)
{
	if (pBasicAudio)
	{
		HRESULT hr = pBasicAudio->put_Balance(inBalance);
		return SUCCEEDED(hr);
	}
	return false;
}

long CDXGraph::GetAudioBalance(void)
{
	long balance = 0;
	if (pBasicAudio)
	{
		pBasicAudio->get_Balance(&balance);
	}
	return balance;
}

bool CDXGraph::RenderFile(TCHAR * inFile)
{
	if (pGraph)
	{
		//WCHAR    szFilePath[MAX_PATH];
		//MultiByteToWideChar(CP_ACP, 0, inFile, -1, szFilePath, MAX_PATH);
		//if (SUCCEEDED(pGraph->RenderFile(szFilePath, NULL)))
		if (SUCCEEDED(pGraph->RenderFile(inFile, NULL)))
		{
			return true;
		}
	}
	return false;
}

bool CDXGraph::SnapshotBitmap(TCHAR *outFile)//const char * outFile)
{
	if (pBasicVideo)
	{
		long bitmapSize = 0;
		if (SUCCEEDED(pBasicVideo->GetCurrentImage(&bitmapSize, 0)))
		{
			bool pass = false;
			unsigned char * buffer = new unsigned char[bitmapSize];
			if (SUCCEEDED(pBasicVideo->GetCurrentImage(&bitmapSize, (long *)buffer)))
			{
				BITMAPFILEHEADER	hdr;
				LPBITMAPINFOHEADER	lpbi;

				lpbi = (LPBITMAPINFOHEADER)buffer;
				int nColors = 0;
				if (lpbi->biBitCount < 8)
				{
					nColors = 1 << lpbi->biBitCount;
				}
			

				hdr.bfType		= ((WORD) ('M' << 8) | 'B');	//always is "BM"
				hdr.bfSize		= bitmapSize + sizeof( hdr );
				hdr.bfReserved1 	= 0;
				hdr.bfReserved2 	= 0;
				hdr.bfOffBits		= (DWORD) (sizeof(BITMAPFILEHEADER) + lpbi->biSize +
						nColors * sizeof(RGBQUAD));

				CFile bitmapFile((outFile), CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary);
				bitmapFile.Write(&hdr, sizeof(BITMAPFILEHEADER));
				bitmapFile.Write(buffer, bitmapSize);
				bitmapFile.Close();
				pass = true;
			}
			delete [] buffer;
			return pass;
		}
	}
	return false;
}



//////////////////////// For GraphEdit Dubug purpose /////////////////////////////
void CDXGraph::AddToObjectTable(void)
{
	IMoniker * pMoniker = 0;
    IRunningObjectTable * objectTable = 0;
    if (SUCCEEDED(GetRunningObjectTable(0, &objectTable))) 
	{
		WCHAR wsz[256];
		swprintf(wsz, L"FilterGraph %08p pid %08x", (DWORD_PTR)pGraph, GetCurrentProcessId());
		HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
		if (SUCCEEDED(hr)) 
		{
			hr = objectTable->Register(0, pGraph, pMoniker, &mObjectTableEntry);
			pMoniker->Release();
		}
		objectTable->Release();
	}
}

void CDXGraph::RemoveFromObjectTable(void)
{
	IRunningObjectTable * objectTable = 0;
    if (SUCCEEDED(GetRunningObjectTable(0, &objectTable))) 
	{
        objectTable->Revoke(mObjectTableEntry);
        objectTable->Release();
		mObjectTableEntry = 0;
    }
}
