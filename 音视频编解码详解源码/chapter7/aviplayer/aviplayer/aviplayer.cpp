// aviplayer.cpp : 定义控制台应用程序的入口点。
//

//


#include "stdafx.h"
#include <dshow.h>

// 用到的DirectShow SDK链接库
#pragma comment(lib,"strmiids.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaEvent   *pEvent = NULL; 
    // 初始化COM库.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        printf("ERROR - Could not initialize COM library");
        return -1;
    }
// 创建滤波器图表管理器
   hr=CoCreateInstance(CLSID_FilterGraph, NULL,
CLSCTX_INPROC_SERVER, 
                               IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr))
    {
        printf("ERROR - Could not create the Filter Graph Manager.");
        return -1;
    }
   // 查询媒体控制和媒体事件接口
   hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
// 建立图表，在这里你可以更改待播放的文件名称
    hr = pGraph->RenderFile(L"D:\\DXSDK\\Samples\\Media\\ruby.avi", NULL);
    if (SUCCEEDED(hr))
    {
        // 运行图表.
        hr = pControl->Run();
        if (SUCCEEDED(hr))
        {
            //等待回放结束事件.
            long evCode;
            pEvent->WaitForCompletion(INFINITE, &evCode);
           // 切记: 在实际应用当中,不能使用INFINITE标识, 因为它会不确定的阻塞程序
        }
    }
// 释放所有资源和关闭COM库
    pControl->Release();
    pEvent->Release();
    pGraph->Release();
    CoUninitialize();

	printf("this is a test for vc2005!\n");
	return 0;
}

