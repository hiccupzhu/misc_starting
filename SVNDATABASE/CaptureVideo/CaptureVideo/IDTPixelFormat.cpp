// IDTPixelFormat.cpp: implementation of the IDTPixelFormat class.

// Code by Karel Donk
// Contact me at karel@miraesoft.com for more information, or visit my homepage at
// http://www.miraesoft.com/karel/ from more codesamples.
// May be modified and used freely

#include "IDTPixelFormat.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IDTPixelFormat::IDTPixelFormat()
{
}

IDTPixelFormat::~IDTPixelFormat()
{
}

DWORD IDTPixelFormat::GetRBitLoc(D3DFORMAT format)
{
	return GetShiftValue(GetRBitMask(format));
}

DWORD IDTPixelFormat::GetGBitLoc(D3DFORMAT format)
{
	return GetShiftValue(GetGBitMask(format));
}

DWORD IDTPixelFormat::GetBBitLoc(D3DFORMAT format)
{
	return GetShiftValue(GetBBitMask(format));
}

DWORD IDTPixelFormat::GetABitLoc(D3DFORMAT format)
{
	return GetShiftValue(GetABitMask(format));
}

DWORD IDTPixelFormat::GetBitDept(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 16;
	if (format == D3DFMT_A8R8G8B8) return 32;
	if (format == D3DFMT_R8G8B8) return 24;
	if (format == D3DFMT_A4R4G4B4) return 16;
	if (format == D3DFMT_A1R5G5B5) return 16;
	if (format == D3DFMT_X8R8G8B8) return 32;
	if (format == D3DFMT_X1R5G5B5) return 16;
	if (format == D3DFMT_A2B10G10R10) return 32;

	return 0;
}

HRESULT IDTPixelFormat::GetDTPixelFormat(D3DFORMAT format, DT_PIXELFMT *dtpixelfmt)
{
	if (dtpixelfmt == NULL) return DT_ERROR;

	// Check if pixelformat is supported
	if ((format != D3DFMT_R5G6B5) &&
		(format != D3DFMT_A8R8G8B8) &&
		(format != D3DFMT_R8G8B8) &&
		(format != D3DFMT_A4R4G4B4) &&
		(format != D3DFMT_A1R5G5B5) &&
		(format != D3DFMT_X1R5G5B5) &&
		(format != D3DFMT_A2B10G10R10) &&
		(format != D3DFMT_X8R8G8B8)) return DT_INVALID_FORMAT;

	dtpixelfmt->Format = format;
	dtpixelfmt->RShift = GetRBitLoc(format);
	dtpixelfmt->GShift = GetGBitLoc(format);
	dtpixelfmt->BShift = GetBBitLoc(format);
	dtpixelfmt->AShift = GetABitLoc(format);
	dtpixelfmt->BitsPerPixel = GetBitDept(format);
	dtpixelfmt->ABMask = GetABitMask(format);
	dtpixelfmt->RBMask = GetRBitMask(format);
	dtpixelfmt->GBMask = GetGBitMask(format);
	dtpixelfmt->BBMask = GetBBitMask(format);
	dtpixelfmt->AMaxVal = GetAMaxVal(format);
	dtpixelfmt->RMaxVal = GetRMaxVal(format);
	dtpixelfmt->GMaxVal = GetGMaxVal(format);
	dtpixelfmt->BMaxVal = GetBMaxVal(format);

	return DT_OK;
}

DWORD IDTPixelFormat::GetABitMask(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 0x00000000;
	if (format == D3DFMT_A8R8G8B8) return 0xff000000;
	if (format == D3DFMT_R8G8B8) return 0x00000000;
	if (format == D3DFMT_A4R4G4B4) return 0x0000f000;
	if (format == D3DFMT_A1R5G5B5) return 0x00008000;
	if (format == D3DFMT_X8R8G8B8) return 0x00000000;
	if (format == D3DFMT_X1R5G5B5) return 0x00008000;
	if (format == D3DFMT_A2B10G10R10) return 0xc0000000;

	return 0xff;
}

DWORD IDTPixelFormat::GetRBitMask(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 0x0000f800;
	if (format == D3DFMT_A8R8G8B8) return 0x00ff0000;
	if (format == D3DFMT_R8G8B8) return 0x00ff0000;
	if (format == D3DFMT_A4R4G4B4) return 0x00000f00;
	if (format == D3DFMT_A1R5G5B5) return 0x00007c00;
	if (format == D3DFMT_X8R8G8B8) return 0x00ff0000;
	if (format == D3DFMT_X1R5G5B5) return 0x00007c00;
	if (format == D3DFMT_A2B10G10R10) return 0x000003ff;

	return 0xff;
}

DWORD IDTPixelFormat::GetGBitMask(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 0x000007e0;
	if (format == D3DFMT_A8R8G8B8) return 0x0000ff00;
	if (format == D3DFMT_R8G8B8) return 0x0000ff00;
	if (format == D3DFMT_A4R4G4B4) return 0x000000f0;
	if (format == D3DFMT_A1R5G5B5) return 0x000003e0;
	if (format == D3DFMT_X8R8G8B8) return 0x0000ff00;
	if (format == D3DFMT_X1R5G5B5) return 0x000003e0;
	if (format == D3DFMT_A2B10G10R10) return 0x000ffc00;

	return 0xff;
}

DWORD IDTPixelFormat::GetBBitMask(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 0x0000001f;
	if (format == D3DFMT_A8R8G8B8) return 0x000000ff;
	if (format == D3DFMT_R8G8B8) return 0x000000ff;
	if (format == D3DFMT_A4R4G4B4) return 0x0000000f;
	if (format == D3DFMT_A1R5G5B5) return 0x0000001f;
	if (format == D3DFMT_X8R8G8B8) return 0x000000ff;
	if (format == D3DFMT_X1R5G5B5) return 0x0000001f;
	if (format == D3DFMT_A2B10G10R10) return 0x3ff00000;

	return 0xff;
}

DWORD IDTPixelFormat::GetShiftValue(DWORD dmask)
{
	DWORD dwShift = 0;

	if (dmask)
	{
		while ((dmask & 1) == 0 )
		{
			dwShift++;
			dmask >>= 1;
		}
	}
    
	return dwShift;
}

DWORD IDTPixelFormat::GetAMaxVal(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 0;
	if (format == D3DFMT_A8R8G8B8) return 255;
	if (format == D3DFMT_R8G8B8) return 0;
	if (format == D3DFMT_A4R4G4B4) return 15;
	if (format == D3DFMT_A1R5G5B5) return 1;
	if (format == D3DFMT_X8R8G8B8) return 0;
	if (format == D3DFMT_X1R5G5B5) return 0;
	if (format == D3DFMT_A2B10G10R10) return 3;

	return 0;
}

DWORD IDTPixelFormat::GetRMaxVal(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 31;
	if (format == D3DFMT_A8R8G8B8) return 255;
	if (format == D3DFMT_R8G8B8) return 255;
	if (format == D3DFMT_A4R4G4B4) return 15;
	if (format == D3DFMT_A1R5G5B5) return 31;
	if (format == D3DFMT_X8R8G8B8) return 255;
	if (format == D3DFMT_X1R5G5B5) return 31;
	if (format == D3DFMT_A2B10G10R10) return 1023;

	return 0;
}

DWORD IDTPixelFormat::GetGMaxVal(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 63;
	if (format == D3DFMT_A8R8G8B8) return 255;
	if (format == D3DFMT_R8G8B8) return 255;
	if (format == D3DFMT_A4R4G4B4) return 15;
	if (format == D3DFMT_A1R5G5B5) return 31;
	if (format == D3DFMT_X8R8G8B8) return 255;
	if (format == D3DFMT_X1R5G5B5) return 31;
	if (format == D3DFMT_A2B10G10R10) return 1023;

	return 0;
}

DWORD IDTPixelFormat::GetBMaxVal(D3DFORMAT format)
{
	if (format == D3DFMT_R5G6B5) return 31;
	if (format == D3DFMT_A8R8G8B8) return 255;
	if (format == D3DFMT_R8G8B8) return 255;
	if (format == D3DFMT_A4R4G4B4) return 15;
	if (format == D3DFMT_A1R5G5B5) return 31;
	if (format == D3DFMT_X8R8G8B8) return 255;
	if (format == D3DFMT_X1R5G5B5) return 31;
	if (format == D3DFMT_A2B10G10R10) return 1023;

	return 0;
}