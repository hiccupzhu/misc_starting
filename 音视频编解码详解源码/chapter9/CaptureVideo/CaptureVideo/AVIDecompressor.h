// AVIDecompressor.h: interface for the CAVIDecompressor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVIDECOMPRESSOR_H__EC24BB65_2DA1_4A4D_932D_E6E128DE2232__INCLUDED_)
#define AFX_AVIDECOMPRESSOR_H__EC24BB65_2DA1_4A4D_932D_E6E128DE2232__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BaseFilter.h"

class CAVIDecompressor  : public CBaseFilter
{
public:
	CAVIDecompressor(IGraphBuilder *inGraph);
	virtual ~CAVIDecompressor();

public:
	IPin * GetOutputPin();
	IPin * GetInputPin();

};

#endif // !defined(AFX_AVIDECOMPRESSOR_H__EC24BB65_2DA1_4A4D_932D_E6E128DE2232__INCLUDED_)
