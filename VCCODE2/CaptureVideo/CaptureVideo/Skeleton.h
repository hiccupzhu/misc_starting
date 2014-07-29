#pragma once

#include <cv.h>
#include <cxcore.h>


typedef struct _SKELETONPOINT
{
	int nx;
	int ny;
	int nIndex;

	_SKELETONPOINT()
	{
		nx=0;
		ny=0;
		nIndex=0;

		pPrevious=NULL;
		pNext=NULL;
		pRight=NULL;
		pLeft=NULL;
	}

	CvPoint GetCvPoint()
	{
		return cvPoint(nx,ny);
	}

	_SKELETONPOINT* pPrevious;
	_SKELETONPOINT* pNext;
	_SKELETONPOINT* pRight;
	_SKELETONPOINT* pLeft;
}SKELETONPOINT;

class CSkeleton
{
public:
	CSkeleton(void);
	~CSkeleton(void);

public:
	int m_nWidth;
	int m_nHeight;
	int m_nChannle;
	int m_nLineByte;
	int m_nImageSize;

	int m_nSkeletonWidth;
	int m_nSkeletonHeight;

	IplImage* m_cvImageHeader;
	int m_nSkePointCount;
	SKELETONPOINT* m_pSkePoint;

private:
	bool isInit;

public:
	HRESULT Init(BITMAPINFOHEADER infoheader);
	HRESULT DrawSkeleton(LPBYTE pData);
	HRESULT TraverTree(SKELETONPOINT *pSkeTree);
};
