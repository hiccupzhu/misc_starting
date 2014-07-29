#pragma once

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <engine.h>
#include "Skeleton.h"

class CImageHandle
{
public:
	CImageHandle(void);
	~CImageHandle(void);

private:
	BITMAPINFOHEADER m_infoheader;
	int m_nWidth;
	int m_nHeight;
	int m_nBiCount;
	int m_nChannel;
	int m_nImageSize;
	bool isInit;

public:
	HRESULT Init(BITMAPINFOHEADER infoheader);
	HRESULT ImageHandle(BYTE *pData);
	HRESULT NegativeImage(BYTE* pData);
	HRESULT NegativeImage2(BYTE* pData);
	HRESULT NegativeImage3(BYTE* pData);
	HRESULT NegativeImage4(BYTE* pData);
	HRESULT RGB32ToGray32(BYTE *pRGB32,BYTE *pGray32=NULL);
	HRESULT Canny(BYTE *pData);
	HRESULT FillGray32WithGray8(LPBYTE pGray32,LPBYTE pGray8);
	HRESULT Canny2(BYTE *pData);
	HRESULT Matlab(LPBYTE pData);
	HRESULT TestHandle(LPBYTE pData);
	HRESULT DrawHistogram(LPBYTE pData);
	HRESULT MakeThreshold(int hist[],int &nThreshold,int &nSum1,int &nSum2);
	HRESULT MakeThreshold(LPBYTE pData,int &nThreshold);

	IplImage* m_pCvImage;
	IplImage* m_pCvGrayCurrent;
	IplImage* m_pCvGrayBack;
	Engine* m_pEngine;

	IplImage* m_pCvImageHeaderRGB32;
	IplImage* m_pCvGrayCurrentHeader;
	IplImage* m_pCvGrayBackHeader;

	CSkeleton m_skeleton;
};
