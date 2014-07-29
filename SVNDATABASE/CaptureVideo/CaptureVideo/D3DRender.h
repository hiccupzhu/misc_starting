#pragma once
#include <d3d9.h>
#include <d3dx9.h>


const D3DXCOLOR      WHITE( D3DCOLOR_XRGB(255, 255, 255) );
const D3DXCOLOR      BLACK( D3DCOLOR_XRGB(  0,   0,   0) );
const D3DXCOLOR        RED( D3DCOLOR_XRGB(255,   0,   0) );
const D3DXCOLOR      GREEN( D3DCOLOR_XRGB(  0, 255,   0) );
const D3DXCOLOR       BLUE( D3DCOLOR_XRGB(  0,   0, 255) );
const D3DXCOLOR     YELLOW( D3DCOLOR_XRGB(255, 255,   0) );
const D3DXCOLOR       CYAN( D3DCOLOR_XRGB(  0, 255, 255) );
const D3DXCOLOR    MAGENTA( D3DCOLOR_XRGB(255,   0, 255) );


class CD3DRender
{
public:
	CD3DRender(void);
	~CD3DRender(void);

public:
	bool isInit;
	int m_nWidth;
	int m_nHeight;
	int m_nChannel;
	int m_nImageSize;
	int m_nLineByte;

	HWND m_hWnd;
	IDirect3DDevice9* m_pDevice;
	IDirect3DTexture9* m_pTexture;
	IDirect3DVertexBuffer9* m_pVertexBuffer;
	IDirect3DIndexBuffer9*  m_pIndexBuffer;

public:
	HRESULT InitD3D(IDirect3DDevice9** device,BITMAPINFOHEADER infoheader,HWND hwnd);
	HRESULT DrawD3D(LPBYTE pData);
	HRESULT Setup();
	HRESULT FillTexture(LPBYTE pData);

	D3DLIGHT9 InitDirectionalLight(D3DXVECTOR3* direction, D3DXCOLOR* color);
	static D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);

};
