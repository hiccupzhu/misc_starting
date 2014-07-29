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


// DirectSound.h: interface for the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTSOUND_H__A20FE86F_118F_11D2_9AB3_0060B0CDC13E__INCLUDED_)
#define AFX_DIRECTSOUND_H__A20FE86F_118F_11D2_9AB3_0060B0CDC13E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <mmsystem.h>
#define INITGUID
#include <dsound.h>
//#include "d:\dxsdk\include\dsound.h"



#define  BUFSIZE 400000


enum AUDIO_FORMAT
{
	AUDIO_FORMAT_PCM=1,
	AUDIO_FORMAT_ADPCM_MS=2,
	AUDIO_FORMAT_ADPCM_IMA,
	AUDIO_FORMAT_ALAW,
	AUDIO_FORMAT_ULAW,
};


typedef struct _AUDIO_CONFIG
{ 
  WORD  wFormatTag; 
  WORD  nChannels; 
  DWORD nSamplesPerSec; 
  DWORD nAvgBytesPerSec; 
  WORD  nBlockAlign; 
  WORD  wBitsPerSample; 
  WORD  cbSize; 
} AUDIO_CONFIG; 





class __declspec(dllexport) CDirectSound  
{
public:		// construction/destruction
	CDirectSound();
	~CDirectSound();
	
	BOOL            WriteToBuf();
	int				WriteDataToBuf(BYTE* pBuf,DWORD dwLen);
//	void            SetHWnd(HWND hWnd);
	BOOL            CreateDSound(/*void* pHeader*/AUDIO_CONFIG* WaveHead,DWORD dwHeadLen);
	BOOL            SetPlaySpeed(float fSpeed);
	
public:		// operations

	void            Play(INT nTime = 0, BOOL bLoop = TRUE);
	void			Stop();
	void			Pause();
    void            Open();	
	void			Continue();


protected:	// implementation
	
	
	BOOL			IsValid() const;
	BOOL            GetWaveData(void * pHeader, WAVEFORMATEX * & pWaveHeader,
								  DWORD & dwHeadSize);

	BOOL            GetWaveData(void * pRes, WAVEFORMATEX * & pWaveHeader, 
		void * & pbWaveData, DWORD & cbWaveSize);
	
//	BOOL            ParseBuffer();
	BOOL            SetSoundData(void * pSoundData,DWORD SoundSize);
	BOOL            SetSoundData(DWORD Offset,void * pSoundData, DWORD dwSoundSize);
	BOOL            CreateSoundBuffer(WAVEFORMATEX * pcmwf);
	

private:	// data member
	DSBPOSITIONNOTIFY    DSNotify[16];
	AUDIO_CONFIG         m_config;
	float                m_fRate;
	int                  m_rate;

	DWORD                m_nSamplesPerSec;
	DWORD                m_nAvgBytesPerSec;
	DWORD                m_dwTotalSize;
	DWORD                m_dwHeadLen;
	DWORD                m_dwBufNum;
	DWORD                m_dwBufLen;
	DWORD                m_dwBufSize;
	DWORD                m_dwOffset;
	DWORD                m_dwLen;
	DWORD                m_dwTheSound;
	HANDLE               notify_events[16];
	HANDLE               m_hPlayThread;
	HANDLE               m_sysncObj;
	HANDLE               m_hEvent;
	HWND                 m_hWnd;
	BYTE*                m_pTheSound;

	BOOL                 m_bStop;
	BOOL                 m_bFir;
	BOOL                 m_bEnabled;
	BOOL                 m_bPlay;

             

	static LPDIRECTSOUNDNOTIFY  m_pDsNotify; 
	static LPDIRECTSOUNDBUFFER  m_pDsb;
	static LPDIRECTSOUND m_lpDirectSound;
	static DWORD         m_dwInstances;
};

#endif // !defined(AFX_DIRECTSOUND_H__A20FE86F_118F_11D2_9AB3_0060B0CDC13E__INCLUDED_)
