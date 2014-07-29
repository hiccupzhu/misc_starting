// IDTSurface.cpp: implementation of the IDTSurface class.

// Code by Karel Donk
// Contact me at karel@miraesoft.com for more information, or visit my homepage at
// http://www.miraesoft.com/karel/ from more codesamples.
// May be modified and used freely

#include "IDTSurface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IDTSurface::IDTSurface()
{
	m_D3DDevice = NULL;
	m_D3DSurface = NULL;

	m_Width = 0;
	m_Height = 0;
	m_Format = D3DFMT_UNKNOWN;
}

IDTSurface::~IDTSurface()
{
	if (m_D3DSurface != NULL) 
	{
		m_D3DSurface->Release();
		m_D3DSurface = NULL;
	}

	m_Width = 0;
	m_Height = 0;
	m_Format = D3DFMT_UNKNOWN;
}

HRESULT IDTSurface::CreateSurface(UINT Width, UINT Height, D3DFORMAT PixelFormat)
{
	if (m_D3DDevice == NULL) return DT_NOT_INITIALIZED;

	if (m_D3DDevice->CreateOffscreenPlainSurface(Width, Height, PixelFormat, D3DPOOL_DEFAULT, &m_D3DSurface, NULL) != D3D_OK) return DT_ERROR;

	m_Width = Width;
	m_Height = Height;
	m_Format = PixelFormat;

	return DT_OK;
}

// Should be called to set the D3DDevice before doing anything with the surface
HRESULT IDTSurface::Initialize(LPDIRECT3DDEVICE9 pD3DDevice,int nWidth,int nHeight)
{
	if (pD3DDevice == NULL) return DT_NOT_INITIALIZED;

	m_D3DDevice = pD3DDevice;
	m_Width = nWidth;
	m_Height = nHeight;

	return DT_OK;
}

LPDIRECT3DSURFACE9 IDTSurface::GetD3DSurface()
{
	return m_D3DSurface;
}

// Crappy function which quickly loads an image into the surface, uses D3DX functions,
// and defaults to 255x255 image size.
// TODO: Write your own code to create a surface and load an image into it regardless of surface format.
// NOTE: This function currently creates the surface m_D3DSurface using the D3DPOOL_SCRATCH flag. For
// best performance, use the D3DPOOL_DEFAULT flag with a supported pixelformat for the creation of m_D3DSurface.
//
// For surfaces compatible with GDI (for use in windowed mode) use pixelformats D3DFMT_X1R5G5B5, D3DFMT_R5G6B5, 
// or D3DFMT_X8R8G8B8 (check the format of the backbuffer surface and use that for best performance).
// For surfaces compatible with the D3DPOOL_DEFAULT flag use pixelformats D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, 
// D3DFMT_A2B10G10R10, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5, and D3DFMT_R5G6B5
HRESULT IDTSurface::CreateSurfaceFromFile(LPCTSTR FileName, D3DFORMAT PixelFormat)
{
	if (m_D3DDevice == NULL) return DT_NOT_INITIALIZED;

	LPDIRECT3DSURFACE9 tempSurf;
	if (m_D3DDevice->CreateOffscreenPlainSurface(256, 256, PixelFormat, D3DPOOL_SCRATCH, &tempSurf, NULL) != D3D_OK) return DT_ERROR;

	D3DXIMAGE_INFO ImageInfo;
	if (D3DXLoadSurfaceFromFileA(tempSurf, NULL, NULL, FileName, NULL, D3DX_FILTER_LINEAR, D3DCOLOR_ARGB(255,0,0,0), &ImageInfo) != D3D_OK) 
	{
		tempSurf->Release();
		return DT_ERROR;
	}

	if (m_D3DDevice->CreateOffscreenPlainSurface(256, 256, PixelFormat, D3DPOOL_SCRATCH, &m_D3DSurface, NULL) != D3D_OK)
	{
		tempSurf->Release();
		return DT_ERROR;
	}

	if (D3DXLoadSurfaceFromSurface(m_D3DSurface, NULL, NULL, tempSurf, NULL, NULL, D3DX_FILTER_LINEAR, 0) != D3D_OK)
	{
		m_D3DSurface->Release();
		tempSurf->Release();
		return DT_ERROR;
	}

	tempSurf->Release();

	m_Width = 256;
	m_Height = 256;
	m_Format = PixelFormat;

	return DT_OK;
}

HRESULT IDTSurface::FillSurfaceFromBitData(BYTE * pSrc, int nWidth , int nHeight , D3DFORMAT PixelFormat)
{
	if (m_D3DDevice == NULL) return DT_NOT_INITIALIZED;

	int nBiCount=0;
	if(PixelFormat==D3DFMT_A8R8G8B8 || PixelFormat==D3DFMT_X8R8G8B8)
	{
		nBiCount=4;
	}
	else
	{
		return S_FALSE;
	}
	D3DLOCKED_RECT d3Rect;
	if (m_D3DSurface->LockRect(&d3Rect, 0, 0) == D3D_OK)
	{
		memcpy(d3Rect.pBits,pSrc,nWidth*m_Height*nBiCount);
		m_D3DSurface->UnlockRect();
	}

	m_Width = nWidth;
	m_Height = nHeight;
	m_Format = PixelFormat;

	return DT_OK;
}

// And now for the beaf!
HRESULT IDTSurface::BltFast(LPDIRECT3DSURFACE9 Dest,BYTE *pData, int x, int y, RECT srcRect)
{
	if (m_D3DDevice == NULL) return DT_NOT_INITIALIZED;
//	if (m_D3DSurface == NULL) return DT_NOT_INITIALIZED;
	if (Dest == NULL) return DT_ERROR;

	D3DLOCKED_RECT dlRect;
	
	if (Dest->LockRect(&dlRect, &srcRect, 0) == D3D_OK)
	{
		BYTE* pDest = (BYTE*)dlRect.pBits;
		
		int nSrcLineLenght=m_Width*4;
		for(int n=m_Height-1;n>=0;n--)
		{
			memcpy(pDest + dlRect.Pitch * (m_Height-(n+1)),pData + n * nSrcLineLenght,nSrcLineLenght);
		}	
	}
	else
	{
//		m_D3DSurface->UnlockRect();
		return DT_ERROR;
	}

	Dest->UnlockRect();
	

	return DT_OK;
}
