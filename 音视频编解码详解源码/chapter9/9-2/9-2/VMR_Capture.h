//////////////////////////////////////////////////////////////////////
//
//  This class is designed to provide simple interface for 
//  simultaneous Video Capture & Preview using DirectShow
//
//////////////////////////////////////////////////////////////////////
// VMR_Capture.h: interface for the CVMR_Capture class.
//////////////////////////////////////////////////////////////////////


#pragma once

#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
//#include "Convert.h"

//#define WM_GRAPHNOTIFY  WM_USER+13
enum PLAYER_STATE {INIT,RUNNING,PAUSED,STOPPED};

class CVMR_Capture  
{
public:
	//constructor/deconstructor
	CVMR_Capture();
	virtual ~CVMR_Capture();
	
	int EnumDevices(HWND hList);        //枚举设备
	HRESULT Init(int iDeviceID,HWND hWnd,int iWidth,int iHeight);
	DWORD GetFrame(BYTE ** pFrame);		//获取捕获的图像帧
	BOOL Pause();						//暂停预览、捕获
	DWORD GrabFrame();					//截获图像

	void CloseInterfaces(void);			//关闭接口，释放资源
	void SaveGraph(CString wFileName);	//保存滤波器链表
	//ColorSpaceConversions conv;			//颜色空间转换

protected:
	
	IGraphBuilder			*m_pGB;			//滤波器链表管理器
	IMediaControl			*m_pMC;			//媒体控制接口
	IMediaEventEx			*m_pME;			//媒体事件接口
	IVMRWindowlessControl9	*m_pWC;			//VMR-9接口
	IPin					*m_pCamOutPin;	//视频采集滤波器引脚
	IBaseFilter				*m_pDF;			//视频采集滤波器

	PLAYER_STATE			m_psCurrent;

	int		m_nWidth;						//图像帧宽度
	int		m_nHeight;						//图像帧高度
	BYTE	*m_pFrame;						//捕获的图像帧数据指针
	long	m_nFramelen;					//捕获的图像帧数据大小
	
	bool BindFilter(int deviceId, IBaseFilter **pFilter);
	HRESULT InitializeWindowlessVMR(HWND hWnd);
	HRESULT InitVideoWindow(HWND hWnd,int width, int height);
	void StopCapture();
	
	void DeleteMediaType(AM_MEDIA_TYPE *pmt);
	bool Convert24Image(BYTE *p32Img,BYTE *p24Img,DWORD dwSize32);
	
private:
	
};

