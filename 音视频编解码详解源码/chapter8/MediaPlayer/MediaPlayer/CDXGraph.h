//
// CDXGraph.h
//

#ifndef __H_CDXGraph__
#define __H_CDXGraph__

#include <streams.h>

// 滤波器链表通知给特定窗口
#define WM_GRAPHNOTIFY  (WM_USER+20)

class CDXGraph
{
//private:
public:
	IGraphBuilder	*pGraph;		//滤波器链表管理器
	IMediaControl	*pMediaControl;	//媒体控制接口，如run、stop、pause
	IMediaEventEx	*pMediaEvent;	//媒体事件接口
	IBasicVideo		*pBasicVideo;	//视频基本接口
	IBasicAudio		*pBasicAudio;	//音频基本接口
	IVideoWindow	*pVideoWindow;	//视频窗口接口
	IMediaSeeking	*pMediaSeeking;	//媒体定位接口

	DWORD				mObjectTableEntry; 

public:
	CDXGraph();
	virtual ~CDXGraph();

public:
	virtual bool Create(void);		//生成滤波器链表管理器
	virtual void Release(void);		//释放所有接口
	virtual bool Attach(IGraphBuilder * inGraphBuilder);

	//IGraphBuilder * GetGraph(void);		//Not outstanding reference count
	IMediaEventEx * GetEventHandle(void);	//返回IMediaEventEx指针
	
	//根据引脚方向连接滤波器
	bool ConnectFilters(IPin * inOutputPin, IPin * inInputPin, const AM_MEDIA_TYPE * inMediaType = 0);
	//断开连接滤波器
	void DisconnectFilters(IPin * inOutputPin);
	//设置显示窗口
	bool SetDisplayWindow(HWND inWindow);
	//设置窗口通知消息
	bool SetNotifyWindow(HWND inWindow);
	//窗口大小改变处理函数
	bool ResizeVideoWindow(long inLeft, long inTop, long inWidth, long inHeight);
	//处理事件
	void HandleEvent(WPARAM inWParam, LPARAM inLParam);
	
	//媒体运行状态
	bool Run(void);        // Control filter graph
	bool Stop(void);
	bool Pause(void);
	bool IsRunning(void);  // Filter graph status
	bool IsStopped(void);
	bool IsPaused(void);
	
	//设置显示窗口全屏显示
	bool SetFullScreen(BOOL inEnabled);
	bool GetFullScreen(void);

	// 媒体定位
	bool GetCurrentPosition(double * outPosition);
	bool GetStopPosition(double * outPosition);
	bool SetCurrentPosition(double inPosition);
	bool SetStartStopPosition(double inStart, double inStop);
	bool GetDuration(double * outDuration);
	bool SetPlaybackRate(double inRate);

	//设置媒体音量: range from -10000 to 0, and 0 is FULL_VOLUME.
	bool SetAudioVolume(long inVolume);
	long GetAudioVolume(void);

	//设置音频平衡: range from -10000(left) to 10000(right), and 0 is both.
	bool SetAudioBalance(long inBalance);
	long GetAudioBalance(void);
	
	//剖析媒体文件
	//bool RenderFile(char * inFile);
	bool RenderFile(TCHAR * inFile);
	
	//抓图
	bool SnapshotBitmap(TCHAR *outFile);//const char * outFile);
	
	int m_nVolume;
	void ChangeAudioVolume(int nVolume);
	
	//静音开关
	void Mute();
	void UnMute();

private:
	//供GraphEdit调试时使用
	void AddToObjectTable(void) ;
	void RemoveFromObjectTable(void);
	//查询有关接口
	bool QueryInterfaces(void);
};

#endif // __H_CDXGraph__