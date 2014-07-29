// WaveFile.h: interface for the CWaveFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEFILE_H__57955E13_E2D7_4AA3_A1AD_414DACE4596D__INCLUDED_)
#define AFX_WAVEFILE_H__57955E13_E2D7_4AA3_A1AD_414DACE4596D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

#include <mmsystem.h>
#include <dsound.h>


#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2


#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)  { if(p) { delete (p);     (p)=NULL; } }

class CWaveFile  
{

public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    DWORD         m_dwFlags;
    BOOL          m_bIsReadingFromMemory;
    BYTE*         m_pbData;
    BYTE*         m_pbDataCur;
    ULONG         m_ulDataSize;
    TCHAR*         m_pResourceBuffer;

protected:
   HRESULT ReadMMIO();
   HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open( LPTSTR strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags );
   // HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT Close();

    HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );
    HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );

    DWORD   GetSize();
    HRESULT ResetFile();
    WAVEFORMATEX* GetFormat() { return m_pwfx; };

};

#endif // !defined(AFX_WAVEFILE_H__57955E13_E2D7_4AA3_A1AD_414DACE4596D__INCLUDED_)
