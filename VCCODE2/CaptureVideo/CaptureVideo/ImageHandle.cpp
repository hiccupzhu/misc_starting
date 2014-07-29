#include "StdAfx.h"
#include "ImageHandle.h"
#include <brook/brook.h>
#include "brookgenfiles/test.h"
#include "brookgenfiles/test_gpu.h"
#include <emmintrin.h>
// #include <xmmintrin.h>


CImageHandle::CImageHandle(void) :
m_nWidth(0),
m_nHeight(0),
m_nBiCount(0),
m_nChannel(0),
m_nImageSize(0),
m_pCvImage(NULL),
isInit(false)
{
	memset(&m_infoheader,0,sizeof(BITMAPINFOHEADER));
//	memset(&m_cvImage,0,sizeof(IplImage));
	m_pEngine=NULL;

	m_pCvGrayCurrent=NULL;
	m_pCvGrayBack=NULL;

	m_pCvImageHeaderRGB32=NULL;
	m_pCvGrayBackHeader=NULL;
	m_pCvGrayCurrentHeader=NULL;
}

CImageHandle::~CImageHandle(void)
{
	if(m_pCvImage)
	{
		cvReleaseImage(&m_pCvImage);
		m_pCvImage=NULL;
	}
	if(m_pCvGrayCurrent)
	{
		cvReleaseImage(&m_pCvGrayCurrent);
		m_pCvImage=NULL;
	}
	if(m_pCvGrayBack)
	{
		cvReleaseImage(&m_pCvGrayBack);
		m_pCvImage=NULL;
	}
	if(m_pEngine)
	{
		engClose(m_pEngine);
		m_pEngine=NULL;
	}
	if(m_pCvImageHeaderRGB32)
	{
		cvReleaseImageHeader(&m_pCvImageHeaderRGB32);
		m_pCvImageHeaderRGB32=NULL;
	}
	if(m_pCvGrayBackHeader)
	{
		cvReleaseImageHeader(&m_pCvGrayBackHeader);
		m_pCvGrayBackHeader=NULL;
	}
	if(m_pCvGrayCurrentHeader)
	{
		cvReleaseImageHeader(&m_pCvGrayCurrentHeader);
		m_pCvGrayCurrentHeader=NULL;
	}
}

HRESULT CImageHandle::Init(BITMAPINFOHEADER infoheader)
{
	if(!isInit)
	{
		memcpy(&m_infoheader,&infoheader,sizeof(BITMAPINFOHEADER));
		m_nWidth=m_infoheader.biWidth;
		m_nHeight=m_infoheader.biHeight;
		m_nBiCount=m_infoheader.biBitCount;
		m_nChannel=m_infoheader.biBitCount/8;
		m_nImageSize=m_nHeight*m_nWidth*m_nChannel;
		isInit=true;

		m_pCvImage=cvCreateImage(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,4);
		m_pCvGrayCurrent=cvCreateImage(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,1);
		m_pCvGrayBack=cvCreateImage(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,1);

		m_pCvImageHeaderRGB32=cvCreateImageHeader(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,4);
		m_pCvGrayBackHeader=cvCreateImageHeader(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,1);
		m_pCvGrayCurrentHeader=cvCreateImageHeader(cvSize(m_nWidth,m_nHeight),IPL_DEPTH_8U,1);

		m_skeleton.Init(infoheader);

//		m_pEngine=engOpen(NULL);
//		int nStatus=engSetVisible(m_pEngine,false);

	}
	return S_OK;
}
HRESULT CImageHandle::ImageHandle(BYTE *pData)
{
	if(isInit)
	{
		if(pData==NULL)
			return S_FALSE;
//		NegativeImage(pData);
//		NegativeImage2(pData);
//		NegativeImage3(pData);
//		NegativeImage4(pData);
//		RGB32ToGray32(pData);
//		Canny(pData);
//		Matlab(pData);
//		TestHandle(pData);
	}
	return S_FALSE;
}

HRESULT CImageHandle::NegativeImage(BYTE* pData)
{
	if(isInit)
	{
		if(pData==NULL)
			return S_FALSE;
		for(int i=0;i<m_nHeight;i++)
		{
			for(int j=0;j<m_nWidth;j++)
			{
				for(int n=0;n<m_nBiCount/8;n++)
				{
					*pData=255-(*pData++);
				}
			}
		}
	}
	return S_OK;
}

HRESULT CImageHandle::NegativeImage2(BYTE* pData)
{
	if(isInit)
	{
		if(pData==NULL)
			return S_FALSE;
		int nLoop=m_nHeight*m_nWidth*m_nChannel/8;
		__m64 m1,m2;
		memset(&m1,255,sizeof(__m64));
		
		__m64 * pSrc=(__m64*)pData;
		for(int i=0;i<nLoop;i++)
		{
			m2=_mm_subs_pu8(m1,*pSrc);
			*pSrc++=m2;
		}

	}

	return S_OK;
}

HRESULT CImageHandle::NegativeImage3(BYTE* pData)
{
	if(isInit)
	{
		if(pData==NULL)
			return S_FALSE;
		int nLoop=m_nHeight*m_nWidth*m_nChannel/8;
		BYTE p=(BYTE)255;
		__m64 m1;
		memset(&m1,255,sizeof(__m64));
		__asm
		{
			emms
			mov esi,pData
			mov ecx,nLoop
			movq mm1,m1
			movq mm2,m1

start_loop:
			movq mm0,[esi]
			psubusb mm1,mm0

			movq [esi],mm1
			movq mm1,mm2
			add esi,8
			dec ecx
			jnz start_loop
			emms
		}

	}

	return S_OK;
}

HRESULT CImageHandle::NegativeImage4(BYTE* pData)
{
	if(isInit)
	{
		if(pData==NULL)
			return S_FALSE;
		unsigned int dimension[]={m_nHeight,m_nWidth,m_nChannel};
		::brook::Stream<uchar> inputData(3,dimension);
		::brook::Stream<uchar> outputData(3,dimension);

		streamRead(inputData,pData);

		sub255(inputData,outputData);

		streamWrite(outputData,pData);
	}
	return S_OK;
}

HRESULT CImageHandle::RGB32ToGray32(BYTE *pRGB32,BYTE *pGray32)
{
	if(isInit)
	{
		LPBYTE pSrc=NULL;
		LPBYTE pDes=NULL;
		pSrc=pRGB32;
		if(pGray32==NULL)
		{
			pDes=pRGB32;
		}

		BYTE r=0;
		BYTE g=0;
		BYTE b=0;
		BYTE gray=0;
		int nChannel=m_nBiCount/8;
		int lineBytes=m_nWidth*nChannel;

		for(int i=0;i<m_nHeight;i++)
		{
			for(int j=0;j<m_nWidth;j++)
			{
				r=pSrc[lineBytes*i + j * nChannel + 0];
				g=pSrc[lineBytes*i + j * nChannel + 1];
				b=pSrc[lineBytes*i + j * nChannel + 2];

				gray=0.212671*r + 0.715160*g + 0.072169*b;

				pDes[lineBytes*i + j * nChannel + 0]=gray;
				pDes[lineBytes*i + j * nChannel + 1]=gray;
				pDes[lineBytes*i + j * nChannel + 2]=gray;
			}
		}
	}
	return S_OK;
}

HRESULT CImageHandle::Canny(BYTE *pData)
{
	if(isInit)
	{
		memcpy(m_pCvImage->imageData,pData,m_nImageSize);

		IplImage* pGrayImg=NULL;
		IplImage* pCannyImg=NULL;

		pGrayImg=cvCreateImage(cvGetSize(m_pCvImage),IPL_DEPTH_8U,1);
		pCannyImg=cvCreateImage(cvGetSize(m_pCvImage),IPL_DEPTH_8U,1);

		cvCvtColor(m_pCvImage,pGrayImg,CV_RGBA2GRAY);
		cvCanny(pGrayImg,pCannyImg,50,150,3);

		FillGray32WithGray8(pData,(BYTE*)pCannyImg->imageData);

		cvReleaseImage(&pCannyImg);
		cvReleaseImage(&pGrayImg);
	}
	return S_OK;
}

HRESULT CImageHandle::FillGray32WithGray8(LPBYTE pGray32,LPBYTE pGray8)
{
	if(isInit)
	{
		LPBYTE pTemp=pGray32;
		for(int i=0;i<m_nHeight*m_nWidth;i++)
		{
			*pTemp++=*pGray8;
			*pTemp++=*pGray8;
			*pTemp++=*pGray8;
			*pTemp++=*pGray8++;
		}
	}
	return S_OK;
}

HRESULT CImageHandle::Canny2(BYTE *pData)
{
	if(isInit)
	{
		
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//????
//matlab call failled:open matlab Engine is successful , but call engPutVariable failed ,
//the reason is not found , now is waiting to find ......
//////////////////////////////////////////////////////////////////////////
HRESULT CImageHandle::Matlab(BYTE *pData)
{
	if(isInit)
	{
		mxArray *A=NULL;
		mxArray *B=NULL;
		A=mxCreateDoubleMatrix(m_nHeight,m_nWidth*m_nChannel,mxREAL);
		B=mxCreateDoubleMatrix(m_nHeight,m_nWidth*m_nChannel,mxREAL);
		double *dTemp=(double *)mxGetPr(A);
		LPBYTE pdata=pData;
		for(int n=0;n<m_nWidth*m_nHeight*m_nChannel;n++)
		{			
			*dTemp++=*pdata++;
		}
		int nStatus=engPutVariable(m_pEngine,"A",A);
		if(nStatus!=0)
		{
			return S_FALSE;
		}
		nStatus=engEvalString(m_pEngine,"B=255-A");
		if(nStatus!=0)
		{
			return S_FALSE;
		}
		B = engGetVariable(m_pEngine,"B");
		dTemp=(double *)mxGetPr(B);
		for(int n=0;n<m_nWidth*m_nHeight*m_nChannel;n++)
		{			
			*pData++=*dTemp++;
		}
		mxDestroyArray(A);
		mxDestroyArray(B);
	}
	return S_OK;
}

HRESULT CImageHandle::TestHandle(LPBYTE pData)
{
	if(isInit)
	{
//		NegativeImage3(pData);
//		memcpy(m_pCvImage->imageData,pData,m_nImageSize);
		m_pCvImageHeaderRGB32->imageData=(char *)pData;

		cvCvtColor(m_pCvImageHeaderRGB32,m_pCvGrayCurrent,CV_RGBA2GRAY);
		static int nFlag=0;
		if(!nFlag)
		{ 		
			cvCopy(m_pCvGrayCurrent,m_pCvGrayBack);// 		
			nFlag=1;
		}
//		cvSmooth(m_pCvGrayCurrent,m_pCvGrayCurrent);
//		cvEqualizeHist(m_pCvGrayCurrent,m_pCvGrayCurrent);		
		cvCanny(m_pCvGrayCurrent,m_pCvGrayCurrent,50,150,3);		
//		cvSub(m_pCvGrayCurrent,m_pCvGrayBack,m_pCvGrayCurrent);

// 		int nThreshold=10;
// 		MakeThreshold((LPBYTE)m_pCvGrayCurrent->imageData,nThreshold);
 		cvThreshold( m_pCvGrayCurrent, m_pCvGrayCurrent ,140, 255, CV_THRESH_BINARY ); 


//		cvCanny(m_pCvGrayCurrent,m_pCvGrayCurrent,50,150,3);
//		cvExp(m_pCvGrayBack,m_pCvGrayCurrent);
//		DrawHistogram((LPBYTE)m_pCvGrayCurrent->imageData);


		
// 		int nRow=m_nHeight/2*m_nWidth;
// 		int nCol=m_nWidth/2;
// 
// 		LPBYTE pgray=(LPBYTE)m_pCvGrayCurrent->imageData+nRow+nCol;
// 
// 		while(true)
// 		{
// 			if(!*(pgray))
// 			{
// 				
// 			}
// 
// 		}

		m_skeleton.DrawSkeleton((LPBYTE)m_pCvGrayCurrent->imageData);

		FillGray32WithGray8(pData,(LPBYTE)m_pCvGrayCurrent->imageData);

//		cvAnd(m_pCvImageHeaderRGB32,m_pCvImage,m_pCvImageHeaderRGB32);


// 		LPBYTE pgray=(LPBYTE)m_pCvGrayCurrent->imageData;
// 
// 		for(int i=0;i<m_nHeight;i++)
// 		{
// 			for(int j=0;j<m_nWidth;j++)
// 			{				
// 				if(*(pgray+m_nWidth*i+j))
// 				{
// 					memset(pData+(m_nWidth*i+j)*m_nChannel,0,m_nChannel);
// 				}
// 			}
// 		}



	}
	return S_OK;
}

HRESULT CImageHandle::DrawHistogram(LPBYTE pData)
{
	if(isInit)
	{
		m_pCvGrayCurrentHeader->imageData=(char *)pData;
		IplImage* hist_image=cvCreateImage(cvSize(m_nWidth,m_nHeight),8,1);
		cvZero(hist_image);
		
		LPBYTE pTemp=pData;
		int hist[256]={0};
		for(int i=0;i<m_nWidth*m_nHeight;i++)
		{
			hist[*pTemp++]++;
		}

		int nThreshold=10;
		int nSum1=0;
		int nSum2=0;
		MakeThreshold(hist,nThreshold,nSum1,nSum2);
		
		
		CvFont font;
		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX ,0.5f,0.5f);

		char ch[40]={0};
		sprintf(ch,"<=%d is:%d",nThreshold,nSum1);
		cvPutText(hist_image,ch,cvPoint(0,20),&font,cvScalar(128.0,128.0,128.0,0));
		sprintf(ch," >%d is:%d",nThreshold,nSum2);
		cvPutText(hist_image,ch,cvPoint(0,40),&font,cvScalar(128.0,128.0,128.0,0));
		sprintf(ch,"sum1/sum2 is:%f",(float)nSum1/(float)nSum2);
		cvPutText(hist_image,ch,cvPoint(0,60),&font,cvScalar(128.0,128.0,128.0,0));
		sprintf(ch,"nThreshold is:%d",nThreshold);
		cvPutText(hist_image,ch,cvPoint(0,80),&font,cvScalar(128.0,128.0,128.0,0));


		const float nnwidth=256.0f;
		float h=1.0f;
		float w=m_nWidth/nnwidth;
		float fValue=0.0f;
		for(int i=0;i<nnwidth;i++)
		{			
			fValue=(float)hist[i]/(float)(m_nWidth*m_nHeight);
			if(i==nThreshold)
			{
				cvPutText(hist_image,"-|-",cvPoint(i*w,m_nHeight-fValue*m_nHeight*20-100),&font,cvScalar(128.0,128.0,128.0,0));
			}
			cvRectangle(hist_image,cvPoint(i*w,m_nHeight),cvPoint((i+1)*(w),m_nHeight-fValue*m_nHeight*10),cvScalar(128.0,128.0,128.0,0),-1,8,0);
		}

		for(int i=0;i<m_nHeight;i++)
		{
			memcpy(pData+(m_nHeight-i-1)*m_nWidth,hist_image->imageData+i*m_nWidth,m_nWidth);
		}

		if(hist_image)
		{
			cvReleaseImage(&hist_image);
		}
	}
	return S_OK;
}

HRESULT CImageHandle::MakeThreshold(int hist[],int &nThreshold,int &nSum1,int &nSum2)
{
	if(isInit)
	{
		while(true)
		{			
			nSum1=0;
			nSum2=0;
			for(int i=0;i<256;i++)
			{
				if(i<=nThreshold)
				{
					nSum1+=hist[i];
				}
				else
				{
					nSum2+=hist[i];
				}
			}
			float ftemp=(float)nSum1/(float)nSum2;
			if(ftemp>=0.68f && ftemp<=0.72f || nThreshold>255)
				break;
			nThreshold++;
		}

		if(nThreshold>255)
			nThreshold=255;
	}
	return S_OK;
}

HRESULT CImageHandle::MakeThreshold(LPBYTE pData,int &nThreshold)
{
	if(isInit)
	{
		LPBYTE pTemp=pData;
		int hist[256]={0};
		for(int i=0;i<m_nWidth*m_nHeight;i++)
		{
			hist[*pTemp++]++;
		}
		int nSum1=0;
		int nSum2=0;
		MakeThreshold(hist,nThreshold,nSum1,nSum2);
	}
	return S_OK;
}