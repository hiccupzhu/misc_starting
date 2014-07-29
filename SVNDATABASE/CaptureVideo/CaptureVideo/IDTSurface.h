// IDTSurface.h: interface for the IDTSurface class.

// Code by Karel Donk
// Contact me at karel@miraesoft.com for more information, or visit my homepage at
// http://www.miraesoft.com/karel/ from more codesamples.
// May be modified and used freely

#pragma once

#include "idtpixelformat.h"

class IDTSurface  
{
public:
	HRESULT BltFast(LPDIRECT3DSURFACE9 Dest, BYTE *pData, int x, int y, RECT srcRect);
	HRESULT CreateSurfaceFromFile(LPCTSTR FileName, D3DFORMAT PixelFormat);
	HRESULT FillSurfaceFromBitData(BYTE * pData, int nWidth , int nHeight , D3DFORMAT PixelFormat);
	LPDIRECT3DSURFACE9 GetD3DSurface();
	HRESULT Initialize(LPDIRECT3DDEVICE9 pD3DDevice,int nWidth,int nHeight);
	HRESULT CreateSurface(UINT Width, UINT Height, D3DFORMAT PixelFormat);
	IDTSurface();
	virtual ~IDTSurface();

protected:
	IDTPixelFormat m_PixelFormat;
	LPDIRECT3DDEVICE9 m_D3DDevice;
	LPDIRECT3DSURFACE9 m_D3DSurface;

	UINT m_Width;
	UINT m_Height;
	D3DFORMAT m_Format;
};

typedef IDTSurface DT_SURFACE, *LPDT_SURFACE;
