// IDTPixelFormat.h: interface for the IDTPixelFormat class.

// Code by Karel Donk
// Contact me at karel@miraesoft.com for more information, or visit my homepage at
// http://www.miraesoft.com/karel/ from more codesamples.
// May be modified and used freely

#pragma once

#include "dtdefs.h"
#include "d3dutil.h"

// DT color information
typedef struct _DT_COLOR {
	int Red;
	int Green;
	int Blue;
	int Alpha;
} DT_COLOR;

// Surface pixelformat information structure
typedef struct _DT_PIXELFMT {
	D3DFORMAT Format;
	DWORD RShift;
	DWORD GShift;
	DWORD BShift;
	DWORD AShift;
	DWORD ABMask;
	DWORD RBMask;
	DWORD GBMask;
	DWORD BBMask;

	// Maximum color component values for the pixelformat
	// Some surfaces have a max of 255, others 63, others 15 etc.
	DWORD AMaxVal;
	DWORD RMaxVal;
	DWORD GMaxVal;
	DWORD BMaxVal;
	int BitsPerPixel;
} DT_PIXELFMT;

class IDTPixelFormat  
{
public:
	DWORD GetAMaxVal(D3DFORMAT format);
	DWORD GetRMaxVal(D3DFORMAT format);
	DWORD GetGMaxVal(D3DFORMAT format);
	DWORD GetBMaxVal(D3DFORMAT format);
	DWORD GetShiftValue(DWORD dmask);
	DWORD GetABitMask(D3DFORMAT format);
	DWORD GetRBitMask(D3DFORMAT format);
	DWORD GetGBitMask(D3DFORMAT format);
	DWORD GetBBitMask(D3DFORMAT format);
	HRESULT GetDTPixelFormat(D3DFORMAT format, DT_PIXELFMT * dtpixelfmt);
	DWORD GetBitDept(D3DFORMAT format);
	DWORD GetRBitLoc(D3DFORMAT format);
	DWORD GetGBitLoc(D3DFORMAT format);
	DWORD GetBBitLoc(D3DFORMAT format);
	DWORD GetABitLoc(D3DFORMAT format);
	IDTPixelFormat();
	virtual ~IDTPixelFormat();

};
