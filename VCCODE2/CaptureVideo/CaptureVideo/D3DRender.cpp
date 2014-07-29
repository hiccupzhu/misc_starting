#include "StdAfx.h"
#include "D3DRender.h"
#include "camera.h"
#include "terrain.h"

template<class T> void Release(T t)
{
	if( t )
	{
		t->Release();
		t = 0;
	}
}

template<class T> void Delete(T t)
{
	if( t )
	{
		delete t;
		t = 0;
	}
}


struct Vertex
{
	Vertex(){}
	Vertex(float x, float y, float z,float u,float v)
	{
		_x = x;  _y = y;  _z = z;
		_u = u;  _v = v;
	}
	float _x, _y, _z;
	float _u, _v;
	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_TEX1;

Terrain* g_pTerrain=NULL;
extern Camera g_Camera;


D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl;
	mtrl.Ambient  = a;
	mtrl.Diffuse  = d;
	mtrl.Specular = s;
	mtrl.Emissive = e;
	mtrl.Power    = p;
	return mtrl;
}
const D3DMATERIAL9 WHITE_MTRL  = InitMtrl(WHITE, WHITE, WHITE, BLACK, 2.0f);
D3DMATERIAL9 FloorMtrl  = WHITE_MTRL;

CD3DRender::CD3DRender(void):
isInit(false),
m_nChannel(0),
m_nHeight(0),
m_nWidth(0),
m_nImageSize(0),
m_nLineByte(0)
{
	m_hWnd=NULL;
	m_pDevice=NULL;
	m_pVertexBuffer=NULL;
	m_pIndexBuffer=NULL;

	g_pTerrain=NULL;
}

CD3DRender::~CD3DRender(void)
{
	Release<IDirect3DVertexBuffer9*>(m_pVertexBuffer);
	Release<IDirect3DIndexBuffer9*>(m_pIndexBuffer);
	if(g_pTerrain)
	{
		delete g_pTerrain;
		g_pTerrain=NULL;
	}
}

HRESULT CD3DRender::InitD3D(IDirect3DDevice9** device,BITMAPINFOHEADER infoheader,HWND hwnd)
{
	if(!isInit)
	{
		m_nWidth=infoheader.biWidth;
		m_nHeight=infoheader.biHeight;
		m_nChannel=infoheader.biBitCount/8;
		m_nImageSize=m_nHeight*m_nWidth*m_nChannel;
		m_nLineByte=m_nWidth*m_nChannel;		
		m_hWnd=hwnd;


		HRESULT hr = NULL;
		// Step 1: Create the IDirect3D9 object.

		IDirect3D9* d3d9 = 0;
		d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

		if( !d3d9 )
		{
			::MessageBox(0, "Direct3DCreate9() - FAILED", 0, 0);
			return false;
		}

		// Step 2: Check for hardware vp.

		D3DCAPS9 caps;
		d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

		int vp = 0;
		if( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
			vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

		// Step 3: Fill out the D3DPRESENT_PARAMETERS structure.

		D3DPRESENT_PARAMETERS d3dpp;
		d3dpp.BackBufferWidth            = m_nWidth;
		d3dpp.BackBufferHeight           = m_nHeight;
		d3dpp.BackBufferFormat           = D3DFMT_A8R8G8B8;
		d3dpp.BackBufferCount            = 1;
		d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
		d3dpp.MultiSampleQuality         = 0;
		d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD; 
		d3dpp.hDeviceWindow              = hwnd;
		d3dpp.Windowed                   = true;
		d3dpp.EnableAutoDepthStencil     = true; 
		d3dpp.AutoDepthStencilFormat     = D3DFMT_D24S8;
		d3dpp.Flags                      = 0;
		d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

		// Step 4: Create the device.

		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT, // primary adapter
			D3DDEVTYPE_HAL,         // device type
			hwnd,               // window associated with device
			vp,                 // vertex processing
			&d3dpp,             // present parameters
			device);            // return created device

		if( FAILED(hr) )
		{
			// try again using a 16-bit depth buffer
			d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

			hr = d3d9->CreateDevice(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				hwnd,
				vp,
				&d3dpp,
				device);

			if( FAILED(hr) )
			{
				d3d9->Release(); // done with d3d9 object
				::MessageBox(0, "CreateDevice() - FAILED", 0, 0);
				return hr;
			}
		}

		d3d9->Release(); // done with d3d9 object
		
		m_pDevice=*device;


		hr = D3DXCreateTexture(*device,
								m_nWidth,m_nHeight,
								0,
								0,
								D3DFMT_X8R8G8B8,
								D3DPOOL_MANAGED,
								&m_pTexture);
		if(FAILED(hr)){ return hr;}

		isInit=true;
	}

	return S_OK;
}

HRESULT CD3DRender::Setup()
{
	if(isInit)
	{
		m_pDevice->CreateVertexBuffer(
			6 * sizeof(Vertex), 
			D3DUSAGE_WRITEONLY,
			Vertex::FVF,
			D3DPOOL_MANAGED,
			&m_pVertexBuffer,
			0);

		//
		// Fill the buffers with the cube data.
		//

		// define unique vertices:
		Vertex* vertices;
		m_pVertexBuffer->Lock(0, 0, (void**)&vertices, 0);

		// vertices of a unit cube
		vertices[0] = Vertex(-20.0f, 50.0f,-50.0f, 1.0f, 1.0f);
		vertices[1] = Vertex(-20.0f, 10.0f,-50.0f, 1.0f, 0.0f);
		vertices[2] = Vertex( 20.0f, 50.0f,-50.0f, 0.0f, 1.0f);

		vertices[3] = Vertex(-20.0f, 10.0f,-50.0f, 1.0f, 0.0f);
		vertices[4] = Vertex( 20.0f, 10.0f,-50.0f, 0.0f, 0.0f);
		vertices[5] = Vertex( 20.0f, 50.0f,-50.0f, 0.0f, 1.0f);

		m_pVertexBuffer->Unlock();

		//
		// Position and aim the camera.
		//

		D3DXMATRIX V;
		g_Camera.getViewMatrix(&V);
		m_pDevice->SetTransform(D3DTS_VIEW, &V);

		D3DLIGHT9 light;
		::ZeroMemory(&light, sizeof(light));
		light.Type      = D3DLIGHT_DIRECTIONAL;
		light.Ambient   = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
		light.Diffuse   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		light.Specular  = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
		light.Direction = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		m_pDevice->SetLight(0, &light);
		m_pDevice->LightEnable(0, true);

		m_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, true);
		m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, true);

		g_pTerrain=new Terrain(m_pDevice,"e:\\background\\coastMountain64.raw",64,64,10,0.5f);
		g_pTerrain->genTexture(&D3DXVECTOR3(0.0f, 1.0f, 0.0f));

		D3DXMATRIX proj;
		D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI * 0.25f, // 90 - degree
			(float)m_nWidth / (float)m_nHeight,
			0.0f,
			1000.0f);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &proj);

//		m_pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);

//		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	}

	return S_OK;
}

HRESULT CD3DRender::SetViewPort(D3DXVECTOR3 position,D3DXVECTOR3 target,D3DXVECTOR3 up)
{
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	
	m_pDevice->SetTransform(D3DTS_VIEW, &V);

	return S_OK;
}

D3DLIGHT9 CD3DRender::InitDirectionalLight(D3DXVECTOR3* direction, D3DXCOLOR* color)
{
	D3DLIGHT9 light;
	::ZeroMemory(&light, sizeof(light));

	light.Type      = D3DLIGHT_DIRECTIONAL;
	light.Ambient   = *color * 0.4f;
	light.Diffuse   = *color;
	light.Specular  = *color * 0.6f;
	light.Direction = *direction;

	return light;
}


HRESULT CD3DRender::FillTexture(LPBYTE pData)
{
	if(isInit && pData)
	{
		D3DSURFACE_DESC textureDesc; 
		m_pTexture->GetLevelDesc(0 /*level*/, &textureDesc);

		// make sure we got the requested format because our code 
		// that fills the texture is hard coded to a 32 bit pixel depth.
		if( textureDesc.Format != D3DFMT_X8R8G8B8 )
			return S_FALSE;

		D3DLOCKED_RECT lockedRect;
		m_pTexture->LockRect(0/*lock top surface*/, &lockedRect, 
			0 /* lock entire tex*/, 0/*flags*/);         

		BYTE* imageData = (BYTE*)lockedRect.pBits;

		memcpy(imageData,pData,m_nImageSize);

		m_pTexture->UnlockRect(0);

// 		HRESULT hr = D3DXFilterTexture(	m_pTexture,
// 			0, // default palette
// 			0, // use top level as source level
// 			D3DX_DEFAULT); // default filter
// 
// 		if(FAILED(hr))
// 		{
// 			::MessageBox(0, "D3DXFilterTexture() - FAILED", 0, 0);
// 			return hr;
// 		}
	}

	return S_OK;
}
HRESULT CD3DRender::DrawD3D(LPBYTE pData)
{
	if(isInit)
	{
		if(pData)
		{
			FillTexture(pData);
		}

		m_pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);		
		m_pDevice->BeginScene();		

		m_pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(Vertex));
		m_pDevice->SetFVF(Vertex::FVF);
		m_pDevice->SetMaterial(&FloorMtrl);
		m_pDevice->SetTexture(0, m_pTexture);
		m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST,0,2);

// 		D3DXMATRIX I;
// 		D3DXMatrixIdentity(&I);
// 		if( g_pTerrain )
// 			g_pTerrain->draw(&I, false);

		m_pDevice->EndScene();

		m_pDevice->Present(0,0,0,0);
	}

	return S_OK;
}

HRESULT CD3DRender::SetWorld(float anglex,float angley)
{
	if(isInit)
	{
		D3DXMATRIX Rx,Ry;
		D3DXMatrixRotationX(&Rx,anglex);
		D3DXMatrixRotationY(&Ry,angley);

		D3DXMATRIX p=Rx*Ry;
		m_pDevice->SetTransform(D3DTS_WORLD,&p);
	}
	return S_OK;
}

HRESULT CD3DRender::SetWorldX(float angle)
{
	if(isInit)
	{
		D3DXMATRIX Rx;
		D3DXMatrixRotationX(&Rx,angle);

		m_pDevice->SetTransform(D3DTS_WORLD,&Rx);
	}

	return S_OK;
}

HRESULT CD3DRender::SetWorldY(float angle)
{
	if(isInit)
	{
		D3DXMATRIX Ry;
		D3DXMatrixRotationY(&Ry,angle);
		
		m_pDevice->SetTransform(D3DTS_WORLD,&Ry);
	}
	
	return S_OK;
}

HRESULT CD3DRender::SetWorldZ(float angle)
{
	if(isInit)
	{
		D3DXMATRIX Rz;
		D3DXMatrixRotationZ(&Rz,angle);
		
		m_pDevice->SetTransform(D3DTS_WORLD,&Rz);
	}
	
	return S_OK;
}

HRESULT CD3DRender::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX *pMatrix)
{
	if(isInit)
	{
		m_pDevice->SetTransform(State,pMatrix);
	}
	return S_OK;
}