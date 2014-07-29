// winview.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "winview.h"
#include "Render.h"
#include <Mmsystem.h>
#include <atltypes.h>

#pragma comment(lib,"Winmm.lib")
#ifdef _DEBUG
#pragma comment(lib,"OgreMain_d.lib")
#else
#pragma comment(lib,"OgreMain.lib")
#endif

#define MAX_LOADSTRING 100

static int quit = 0;
static CRender *prender;
HANDLE thandle = NULL;
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名


ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


DWORD ThreadProc(LPVOID lpParameter){
    LONGLONG    cur_time;       // current time
    DWORD       time_count=40;    // ms per frame, default if no performance counter
    LONGLONG    perf_cnt;       // performance timer frequency
    BOOL        perf_flag=FALSE;    // flag determining which timer to use
    LONGLONG    next_time=0;    // time to render next frame

    // is there a performance counter available?

    if (QueryPerformanceFrequency((LARGE_INTEGER *) &perf_cnt)) {
        perf_flag=TRUE;
        time_count=perf_cnt/25;        // calculate time per frame based on frequency
        QueryPerformanceCounter((LARGE_INTEGER *) &next_time);
    } else {
        next_time=timeGetTime();
    }


    while(quit == 0){
        if (perf_flag)  
            QueryPerformanceCounter((LARGE_INTEGER *) &cur_time);  
        else
            cur_time=timeGetTime();

        if (cur_time>next_time) {      
            prender->RenderFrame();               
            next_time += time_count;      
            if (next_time < cur_time)
                next_time = cur_time + time_count;  
        }

    }
    return 0;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 
	MSG msg;
	HACCEL hAccelTable;
	
   

    

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINVIEW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINVIEW));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
        } 

    }
	
	

	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINVIEW));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINVIEW);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; 

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   
   prender = new CRender();
   prender->SetMainHwnd(hWnd);
   
   prender->init();
   
   DWORD threadid = 0;
   thandle = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)ThreadProc, NULL, 0, &threadid);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
		    quit = 1;
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:                 	    
		hdc = BeginPaint(hWnd, &ps);     	    
		
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
	    quit = 1;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
