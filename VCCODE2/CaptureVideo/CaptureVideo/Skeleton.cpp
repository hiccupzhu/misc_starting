#include "StdAfx.h"
#include "Skeleton.h"


SKELETONPOINT g_skelPoint[17];

CSkeleton::CSkeleton(void):
m_nChannle(0),
m_nHeight(0),
m_nImageSize(0),
m_nLineByte(0),
m_nWidth(0),
m_nSkeletonHeight(0),
m_nSkeletonWidth(0),
isInit(false)
{
	m_nSkePointCount=17;
	m_cvImageHeader=NULL;
	m_pSkePoint=g_skelPoint;

	m_nSkeletonWidth=100;
	m_nSkeletonHeight=200;
}

CSkeleton::~CSkeleton(void)
{
	if(m_cvImageHeader)
	{
		cvReleaseImageHeader(&m_cvImageHeader);
		m_cvImageHeader=NULL;
	}
}

HRESULT CSkeleton::Init(BITMAPINFOHEADER infoheader)
{
	if(!isInit)
	{
		m_nWidth=infoheader.biWidth;
		m_nHeight=infoheader.biHeight;
		m_nChannle=infoheader.biBitCount/8;
		m_nImageSize=m_nWidth*m_nHeight*m_nChannle;
		m_nLineByte=m_nWidth*m_nChannle;

		m_cvImageHeader=cvCreateImageHeader(cvSize(m_nWidth,m_nHeight),8,1);

		for(int i=0;i<m_nSkePointCount-1;i++)
		{
			m_pSkePoint[i].pNext=&m_pSkePoint[i+1];
			m_pSkePoint[i+1].pPrevious=&m_pSkePoint[i];
		}

		m_pSkePoint[2].pNext=&m_pSkePoint[9];
		m_pSkePoint[9].pPrevious=&m_pSkePoint[2];

		m_pSkePoint[2].pRight=&m_pSkePoint[3];
		m_pSkePoint[3].pPrevious=&m_pSkePoint[2];

		m_pSkePoint[2].pLeft=&m_pSkePoint[6];
		m_pSkePoint[4].pPrevious=&m_pSkePoint[2];

		m_pSkePoint[5].pPrevious=NULL;
		m_pSkePoint[5].pNext=NULL;

		m_pSkePoint[8].pPrevious=NULL;
		m_pSkePoint[8].pNext=NULL;

		m_pSkePoint[10].pNext=NULL;
		m_pSkePoint[10].pRight=&m_pSkePoint[11];
		m_pSkePoint[11].pPrevious=&m_pSkePoint[10];

		m_pSkePoint[10].pLeft=&m_pSkePoint[14];
		m_pSkePoint[14].pPrevious=&m_pSkePoint[10];

		m_pSkePoint[13].pNext=NULL;

		m_pSkePoint[0].nx=m_nSkeletonWidth/2;
		m_pSkePoint[0].ny=0;

		m_pSkePoint[1].nx=m_nSkeletonWidth/2;
		m_pSkePoint[1].ny=20;

		m_pSkePoint[2].nx=m_nSkeletonWidth/2;
		m_pSkePoint[2].ny=m_pSkePoint[1].ny+10;

		m_pSkePoint[3].nx=m_nSkeletonWidth/2-20;
		m_pSkePoint[3].ny=m_pSkePoint[2].ny;

		m_pSkePoint[4].nx=m_nSkeletonWidth/2-30;
		m_pSkePoint[4].ny=m_pSkePoint[2].ny+20;

		m_pSkePoint[5].nx=m_nSkeletonWidth/2-30;
		m_pSkePoint[5].ny=m_pSkePoint[2].ny+50;

		m_pSkePoint[6].nx=m_nSkeletonWidth/2+20;
		m_pSkePoint[6].ny=m_pSkePoint[2].ny;

		m_pSkePoint[7].nx=m_nSkeletonWidth/2+30;
		m_pSkePoint[7].ny=m_pSkePoint[4].ny;

		m_pSkePoint[8].nx=m_nSkeletonWidth/2+30;
		m_pSkePoint[8].ny=m_pSkePoint[5].ny;

		m_pSkePoint[9].nx=m_nSkeletonWidth/2;
		m_pSkePoint[9].ny=m_pSkePoint[2].ny+20;

		m_pSkePoint[10].nx=m_nSkeletonWidth/2;
		m_pSkePoint[10].ny=m_pSkePoint[9].ny+20;

		m_pSkePoint[11].nx=m_pSkePoint[10].nx;
		m_pSkePoint[11].ny=m_pSkePoint[10].ny;

		m_pSkePoint[12].nx=m_pSkePoint[10].nx-12;
		m_pSkePoint[12].ny=m_pSkePoint[10].ny+20;

		m_pSkePoint[13].nx=m_pSkePoint[10].nx-12;
		m_pSkePoint[13].ny=m_pSkePoint[12].ny+40;

		m_pSkePoint[14].nx=m_pSkePoint[10].nx;
		m_pSkePoint[14].ny=m_pSkePoint[10].ny;

		m_pSkePoint[15].nx=m_pSkePoint[10].nx+12;
		m_pSkePoint[15].ny=m_pSkePoint[10].ny+20;

		m_pSkePoint[16].nx=m_pSkePoint[10].nx+12;
		m_pSkePoint[16].ny=m_pSkePoint[15].ny+40;


		isInit=true;
	}
	return S_OK;
}

HRESULT CSkeleton::DrawSkeleton(LPBYTE pData)
{
	if(isInit)
	{
		m_cvImageHeader->imageData=(char *)pData;
		CvPoint cvpoint1;
		CvPoint cvpoint2;
		const int nThickness=4;
		
		for(int i=0;i<4;i++)
		{
			cvpoint1=m_pSkePoint[i].GetCvPoint();
			cvpoint2=m_pSkePoint[i+1].GetCvPoint();

			cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);
		}

		for(int i=6;i<8-1;i++)
		{
			cvpoint1=m_pSkePoint[i].GetCvPoint();
			cvpoint2=m_pSkePoint[i+1].GetCvPoint();

			cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);
		}

		cvpoint1=m_pSkePoint[2].GetCvPoint();
		cvpoint2=m_pSkePoint[6].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint2=m_pSkePoint[9].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint1=m_pSkePoint[10].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint2=m_pSkePoint[11].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint1=m_pSkePoint[12].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint2=m_pSkePoint[13].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint1=m_pSkePoint[10].GetCvPoint();
		cvpoint2=m_pSkePoint[14].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint1=m_pSkePoint[15].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);

		cvpoint2=m_pSkePoint[16].GetCvPoint();
		cvLine(m_cvImageHeader,cvpoint1,cvpoint2,cvScalar(255.0,255.0,255.0,255.0),nThickness,8,0);


// 		SKELETONPOINT* pSketon=NULL;
// 		pSketon=m_pSkePoint;
// 
// 		while(pSketon->pNext)
// 		{
// 			cvpoint1=pSketon->GetCvPoint();
// 		}

	}

	return S_OK;
}

HRESULT CSkeleton::TraverTree(SKELETONPOINT *pSkeTree)
{
	if(isInit)
	{
		if(pSkeTree)
		{
			if(pSkeTree)
			{
				
			}
		}
	}

	return S_OK;
}