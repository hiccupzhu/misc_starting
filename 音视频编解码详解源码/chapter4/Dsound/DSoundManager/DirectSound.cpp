/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Jörg König
// All rights reserved
//
// This file is part of the completely free tetris clone "CGTetris".
//
// This is free software.
// You may redistribute it by any means providing it is not sold for profit
// without the authors written consent.
//
// No warrantee of any kind, expressed or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    J.Koenig@adg.de                 (company site)
//    Joerg.Koenig@rhein-neckar.de    (private site)
/////////////////////////////////////////////////////////////////////////////


// DirectSound.cpp: implementation of the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirectSound.h"
#include "INITGUID.h"
#include "TKCore.h"
#include "process.h"

// The following macro is defined since DirectX 5, but will work with
// older versions too.
#ifndef DSBLOCK_ENTIREBUFFER
	#define DSBLOCK_ENTIREBUFFER        0x00000002
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static void DSError( HRESULT hRes ) {
	switch(hRes) {
		case DS_OK: TKL_Trace("NO ERROR\n"); break;
		case DSERR_ALLOCATED: TKL_Trace("ALLOCATED\n"); break;
		case DSERR_INVALIDPARAM: TKL_Trace("INVALIDPARAM\n"); break;
		case DSERR_OUTOFMEMORY: TKL_Trace("OUTOFMEMORY\n"); break;
		case DSERR_UNSUPPORTED: TKL_Trace("UNSUPPORTED\n"); break;
		case DSERR_NOAGGREGATION: TKL_Trace("NOAGGREGATION\n"); break;
		case DSERR_UNINITIALIZED: TKL_Trace("UNINITIALIZED\n"); break;
		case DSERR_BADFORMAT: TKL_Trace("BADFORMAT\n"); break;
		case DSERR_ALREADYINITIALIZED: TKL_Trace("ALREADYINITIALIZED\n"); break;
		case DSERR_BUFFERLOST: TKL_Trace("BUFFERLOST\n"); break;
		case DSERR_CONTROLUNAVAIL: TKL_Trace("CONTROLUNAVAIL\n"); break;
		case DSERR_GENERIC: TKL_Trace("GENERIC\n"); break;
		case DSERR_INVALIDCALL: TKL_Trace("INVALIDCALL\n"); break;
		case DSERR_OTHERAPPHASPRIO: TKL_Trace("OTHERAPPHASPRIO\n"); break;
		case DSERR_PRIOLEVELNEEDED: TKL_Trace("PRIOLEVELNEEDED\n"); break;
		default: TKL_Trace("%lu\n",hRes);break;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPDIRECTSOUND CDirectSound::m_lpDirectSound;
LPDIRECTSOUNDBUFFER  CDirectSound::m_pDsb;
LPDIRECTSOUNDNOTIFY  CDirectSound::m_pDsNotify; 

DWORD CDirectSound::m_dwInstances;


CDirectSound::CDirectSound()
{
	
	::ZeroMemory(&m_config,sizeof(AUDIO_CONFIG));
	m_pDsNotify = 0;
	m_bFir = 1; 
	m_lpDirectSound = 0;
	m_pDsb = 0;
	m_pTheSound = 0;
	m_dwTheSound = 0;
	m_bEnabled = TRUE;

	m_bPlay = 0;
	++m_dwInstances;
	m_hPlayThread = 0;
	m_dwBufNum = 4;
	m_dwBufLen = 8192;
	m_dwOffset = 0;

	m_dwBufSize = 0;
	m_bStop = 0;
	m_pTheSound = (BYTE*)malloc(BUFSIZE);

//	m_pTheSound = new BYTE[BUFSIZE];

	m_sysncObj = CreateMutex(0,0,0);
}

CDirectSound::~CDirectSound()
{
	if(m_hPlayThread)
	{
		WaitForSingleObject(m_hPlayThread,2000);
		CloseHandle(m_hPlayThread);
		m_hPlayThread = NULL;
	}

	free(m_pTheSound);

	if(m_pDsNotify)
	{
		m_pDsNotify->Release();
		m_pDsNotify = NULL;
	}
	if(m_pDsb)
	{
		m_pDsb->Release();
		m_pDsb = NULL;
	}


	if( !--m_dwInstances && m_lpDirectSound ) {
		m_lpDirectSound->Release();
		m_lpDirectSound = 0;
	}

	if(m_sysncObj)
	{
		CloseHandle(m_sysncObj);
		m_sysncObj = NULL;
	}
//		free(m_pTheSound);
//	::delete[] m_pTheSound;
}

unsigned int WINAPI PlaySoundProc(LPVOID pOwner)
{
	CDirectSound* pThis = (CDirectSound*) pOwner;
	pThis->WriteToBuf();
	return 1;
}

int CDirectSound::WriteDataToBuf(BYTE* pBuf,DWORD dwLen)
{

	if(m_dwBufSize + dwLen > BUFSIZE)
		return -1;

//	GENRS_AutoLock lock(m_sysncObj);	
	WaitForSingleObject(m_sysncObj,2000);
	{
		memcpy(m_pTheSound+m_dwBufSize,pBuf,dwLen); 
		m_dwBufSize += dwLen;
		ReleaseMutex(m_sysncObj);
	}

	return 1;

}


BOOL CDirectSound::SetPlaySpeed(float fSpeed)
{
//	AUDIO_CONFIG lc_config;
	m_fRate = fSpeed;
	m_config.nSamplesPerSec = m_nSamplesPerSec*fSpeed;
	m_config.nAvgBytesPerSec = m_nAvgBytesPerSec*fSpeed;
	Stop();
	if(!CreateDSound(&m_config,m_dwBufLen))
		return FALSE;
	Play();
	return TRUE;
	
}

/*
void CDirectSound::SetHWnd(HWND hWnd)
{
	m_hWnd = hWnd;
}*/

BOOL CDirectSound :: CreateDSound(AUDIO_CONFIG* WaveHead,DWORD dwHeadLen)
{

	if(m_config.nSamplesPerSec == NULL)
	{
		m_fRate = 1.0;
		m_config = *WaveHead;
		m_nAvgBytesPerSec = m_config.nAvgBytesPerSec;
		m_nSamplesPerSec  = m_config.nSamplesPerSec;
	}
	
	m_dwBufLen = dwHeadLen ;
	if(m_lpDirectSound)
	{
		return TRUE;
	}
	HRESULT hRes = DS_OK;
	short nRes = 0;
	do {
			if( nRes )
				::Sleep(500);
			hRes = ::DirectSoundCreate(0, &m_lpDirectSound, 0);
			++nRes;
		} 
	while( nRes < 10 && (hRes == DSERR_ALLOCATED || hRes == DSERR_NODRIVER) );

	if( hRes != DS_OK )
		return FALSE;
	
	m_lpDirectSound->SetCooperativeLevel(/*m_hWnd*/::GetDesktopWindow(), DSSCL_NORMAL);

//	WAVEFORMATEX * pcmwf;
//	DWORD dwHeadLen = 1000;

//	if(!GetWaveData(pHeader, pcmwf, m_dwTheSound) )
//		return FALSE;
	m_rate = WaveHead->nAvgBytesPerSec;

	if(!CreateSoundBuffer((WAVEFORMATEX*)(&m_config)))
		return FALSE;
	
	

//	SetSoundData(0,m_pTheSound, m_dwTheSound);
//	m_bPlay = TRUE;

//	dwHeadLen = m_dwTheSound;
	m_bStop = 0;
	m_dwBufSize = 0;

	unsigned int dwID;
//	m_hPlayThread = CreateThread(0,0,PlaySoundProc,this,0,&dwID);
	m_hPlayThread = (HANDLE)_beginthreadex(0,0,PlaySoundProc,this,0,&dwID);
	return TRUE;

}


BOOL CDirectSound :: GetWaveData(void * pRes, WAVEFORMATEX * & pWaveHeader, 
								 void * & pbWaveData, DWORD & cbWaveSize
								 )
{
	pWaveHeader = 0;
	pbWaveData = 0;
	cbWaveSize = 0;

	DWORD * pdw = (DWORD *)pRes;
	DWORD dwRiff = *pdw++;
	DWORD dwLength = *pdw++;
	DWORD dwType = *pdw++;

	if( dwRiff != mmioFOURCC('R', 'I', 'F', 'F') )
		return FALSE;      // not even RIFF

	if( dwType != mmioFOURCC('W', 'A', 'V', 'E') )
		return FALSE;      // not a WAV

	DWORD * pdwEnd = (DWORD *)((BYTE *)pdw + 1000-4);

	while( pdw < pdwEnd ) {
		dwType = *pdw++;
		dwLength = *pdw++;

		switch( dwType ) {
			case mmioFOURCC('f', 'm', 't', ' '):
				if( !pWaveHeader ) {
					if( dwLength < sizeof(WAVEFORMAT) )
						return FALSE;      // not a WAV

					pWaveHeader = (WAVEFORMATEX *)pdw;

					if( pbWaveData && cbWaveSize )
						return TRUE;
				}
				break;

			case mmioFOURCC('d', 'a', 't', 'a'):
				pbWaveData = LPVOID(pdw);
				cbWaveSize = dwLength;

				if( pWaveHeader )
					return TRUE;
				break;
		}
		pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
	}

	return FALSE;
}




BOOL CDirectSound :: GetWaveData(void * pHeader, WAVEFORMATEX * & pWaveHeader,
								  DWORD & dwHeadSize)
{
	dwHeadSize  = 0;
	pWaveHeader = 0;

	DWORD * pdw = (DWORD *)pHeader;
	DWORD dwRiff = *pdw++;
	DWORD dwLength = *pdw++;
	DWORD dwType = *pdw++;

	m_dwTotalSize = dwLength;

	if( dwRiff != mmioFOURCC('R', 'I', 'F', 'F') )
		return FALSE;      // not even RIFF

	if( dwType != mmioFOURCC('W', 'A', 'V', 'E') )
		return FALSE;      // not a WAV

	
	dwType = *pdw++;
	dwLength = *pdw++;

	if(dwType!= mmioFOURCC('f', 'm', 't', ' '))
		return FALSE;

	if( !pWaveHeader )
	{
		if( dwLength < sizeof(WAVEFORMAT) )
			return FALSE;      // not a WAV

		pWaveHeader = (WAVEFORMATEX *)pdw;
	}

	dwHeadSize += 5*sizeof(DWORD);
	return TRUE;

}


BOOL CDirectSound::WriteToBuf()
{

//	m_bPlay = TRUE;

	if( !SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL ))
	{
		return FALSE;
	};

	while (!m_bStop)
	{

	if(m_bFir)
	{
		//ParseBuffer();
		
		//GENRS_AutoLock lock(m_sysncObj);
		while (m_dwBufSize < 3*m_dwBufLen)
		{
				if(m_bStop)
					break;
				Sleep(10);
		}
		WaitForSingleObject(m_sysncObj,20000);
		{
			
			
	
			SetSoundData( m_pTheSound, 3*m_dwBufLen);

			if(m_dwBufSize > 3*m_dwBufLen)
				memcpy(m_pTheSound,m_pTheSound+3*m_dwBufLen,m_dwBufSize - 3*m_dwBufLen);
			m_dwBufSize -= 3*m_dwBufLen;
			
			ReleaseMutex(m_sysncObj);
		}
//		m_pDsb->Play(0,0,1);
		m_bFir = 0;
	}
	else
	{
	

		int id = WaitForMultipleObjects(m_dwBufNum,notify_events,0,INFINITE);
	
		//ParseBuffer();
		
		//GENRS_AutoLock lock(m_sysncObj);
		while (m_dwBufSize < m_dwBufLen)
		{
			if(m_bStop)
				break;
			Sleep(10);
		}

		WaitForSingleObject(m_sysncObj,2000);
		{
		
			TKL_Trace("ID = %d",id);
			if(id == 0)
				m_dwOffset = 3*m_dwBufLen;
			else
				m_dwOffset = (id-1)*m_dwBufLen;
			
			SetSoundData( m_dwOffset,m_pTheSound, m_dwBufLen);
			if(m_dwBufSize > m_dwBufLen)
				memcpy(m_pTheSound,m_pTheSound+m_dwBufLen,m_dwBufSize - m_dwBufLen);
			
			m_dwBufSize -= m_dwBufLen;
			ReleaseMutex(m_sysncObj);
		}

	}
	}
	return TRUE;
//	SetSoundData(m_pSoundBuf,dwSoundSize);


}

BOOL CDirectSound::CreateSoundBuffer(WAVEFORMATEX * pcmwf)
{
	
	DSBUFFERDESC dsbdesc;
	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	// Need no controls (pan, volume, frequency).
	dsbdesc.dwFlags =  dsbdesc.dwFlags = 
   DSBCAPS_GETCURRENTPOSITION2/* Better position accuracy */
   | DSBCAPS_CTRLPOSITIONNOTIFY     /* We need notification */
   | DSBCAPS_GLOBALFOCUS;      /* Allows background playing *///DSBCAPS_STATIC;		// assumes that the sound is played often
	dsbdesc.dwBufferBytes =m_dwBufNum*m_dwBufLen;// m_dwTheSound;
	dsbdesc.lpwfxFormat = pcmwf;    // Create buffer.
	HRESULT hRes;
	if( DS_OK != (hRes = m_lpDirectSound->CreateSoundBuffer(&dsbdesc, &m_pDsb, 0)) ) {
		// Failed.
		DSError(hRes);
		m_pDsb = 0;
		return FALSE;
	}

	for (int i=0;i<m_dwBufNum;i++)
	{
		//create dsound buffer notify points
		//Auto reset event
		m_hEvent=CreateEvent( NULL, FALSE, FALSE, NULL );
		DSNotify[i].hEventNotify=m_hEvent; 
		
		DSNotify[i].dwOffset=i*(m_dwBufLen);//NOTIFYNUM);
	}

	hRes = m_pDsb->QueryInterface(IID_IDirectSoundNotify,(void **)&m_pDsNotify); 
    if (FAILED(hRes)) //SUCCEEDED
	{
          return FALSE;
	}

	//NOTIFYNUM notify point
	hRes=m_pDsNotify->SetNotificationPositions(m_dwBufNum,DSNotify); 
	if (FAILED(hRes)) //SUCCEEDED
	{
     
	 return FALSE;
	}

	for (int n=0;n< m_dwBufNum;n++)
	{
		notify_events[n]=DSNotify[n].hEventNotify;
		ResetEvent(notify_events[n]); 
	}
	return TRUE;
}

BOOL CDirectSound::SetSoundData(void * pSoundData,DWORD SoundSize)
{
	LPVOID lpvPtr1;
	DWORD dwBytes1;

	if (!pSoundData) return FALSE;
//	ASSERT(SoundSize<=DDSOUNDBUFLEN);
	// Obtain write pointer.
	HRESULT hr = m_pDsb->Lock(0, SoundSize, &lpvPtr1, &dwBytes1, 0, 0, DSBLOCK_ENTIREBUFFER);    
    // If DSERR_BUFFERLOST is returned, restore and retry lock.
	if(DSERR_BUFFERLOST == hr) {
		m_pDsb->Restore();
		hr = m_pDsb->Lock(0, SoundSize, &lpvPtr1, &dwBytes1, 0, 0, DSBLOCK_ENTIREBUFFER);
	}
	if(DS_OK == hr) {
		// Write to pointers.
		::CopyMemory(lpvPtr1, pSoundData, SoundSize);
		// Release the data back to DirectSound.
		hr = m_pDsb->Unlock(lpvPtr1, dwBytes1, 0, 0);
		if(DS_OK == hr)
            return TRUE;
	}
	// Lock, Unlock, or Restore failed.
	return FALSE;

}







BOOL CDirectSound::SetSoundData(DWORD Offset,void * pSoundData, DWORD dwSoundSize)
{
	LPVOID lpPart1;
    LPVOID lpPart2;
    DWORD  Part1Len,Part2Len;


	DWORD off_read,off_write;
    //char buf[128];
    m_pDsb->GetCurrentPosition(&off_read,&off_write);
	TKL_Trace("offset = %d,off_read=%d off_write=%d\n",Offset,off_read,off_write);

	// Obtain write pointer.
	HRESULT hr = m_pDsb->Lock(Offset, dwSoundSize, &lpPart1, &Part1Len, &lpPart2, &Part2Len, 0);    
	
    // If DSERR_BUFFERLOST is returned, restore and retry lock.
	if(DSERR_BUFFERLOST == hr) {
		m_pDsb->Restore();
		hr = m_pDsb->Lock(Offset, dwSoundSize, &lpPart1, &Part1Len, &lpPart2, &Part2Len, 0);
	}
	if(DS_OK == hr) {
		// Write to pointers.
		::CopyMemory(lpPart1, pSoundData, Part1Len);
		if (lpPart2)
		{
			::CopyMemory(lpPart2, (BYTE*)pSoundData+Part1Len, Part2Len);
		}
		

		// Release the data back to DirectSound.
		hr = m_pDsb->Unlock(lpPart1, Part1Len, lpPart2, Part2Len);

	}
	// Lock, Unlock, or Restore failed.
	
	return FALSE;
}



void CDirectSound::Play(INT nTime, BOOL bLoop)
{
	if( ! IsValid() /*|| ! IsEnabled()*/ )
		return;		// no chance to play the sound ...


	DWORD dwPlayCursor;
	DWORD dwWriteCursor;
	m_pDsb->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);
	
	int offByte = nTime*m_rate/1000;

	if(dwPlayCursor >0)
	{
		if( dwPlayCursor > m_dwBufLen* m_dwBufNum )
		dwPlayCursor = m_dwBufLen* m_dwBufNum;
	
	}
	if(dwPlayCursor > offByte)
	{
		if(dwPlayCursor  < -offByte)
		{
			dwPlayCursor = 0;
		}
		else
		{
			dwPlayCursor += offByte;
		}
		
	}

	else
		dwPlayCursor = m_dwBufLen* m_dwBufNum;

	m_pDsb->SetCurrentPosition(dwPlayCursor);
	
	if( DSERR_BUFFERLOST == m_pDsb->Play(0, 0, 1) ) 
	{
		// another application had stolen our buffer
		// Note that a "Restore()" is not enough, because
		// the sound data is invalid after Restore().
		SetSoundData(m_pTheSound, m_dwBufLen);
		
		
		// Try playing again
		m_pDsb->Play(0, 0, 1);
	}

}

void CDirectSound::Stop()
{
	m_bStop = 1;
	if(m_hPlayThread)
	{
	
		WaitForSingleObject(m_hPlayThread,2000);
		CloseHandle(m_hPlayThread);
		m_hPlayThread = NULL;
	}
	if( IsValid() )
		m_pDsb->Stop();

	for (int i=0;i<m_dwBufNum;i++)
	{
		//create dsound buffer notify points
		//Auto reset event
		ResetEvent(notify_events[i]);
		CloseHandle(notify_events[i]);//=m_hEvent; 
		
	}
	if(m_pDsNotify)
	{
		m_pDsNotify->Release();
		m_pDsNotify = NULL;
	}
	if(m_pDsb)
	{
		m_pDsb->Release();
		m_pDsb = NULL;
	}
	if(m_lpDirectSound)
	{
		m_lpDirectSound->Release();
		m_lpDirectSound = NULL;
	}
	m_bFir = 1;
	m_dwBufSize = 0;
}

void CDirectSound::Pause()
{
	if( IsValid() )
		m_pDsb->Stop();
}

void CDirectSound::Continue()
{
	if( IsValid() ) {
		DWORD dwPlayCursor, dwWriteCursor;
		m_pDsb->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);
		Play(dwPlayCursor);
	}
}

BOOL CDirectSound::IsValid() const
{
	return (m_lpDirectSound && m_pDsb ) ? TRUE : FALSE;
}
