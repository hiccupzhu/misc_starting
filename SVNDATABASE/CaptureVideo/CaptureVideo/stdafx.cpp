// stdafx.cpp : 只包括标准包含文件的源文件
// CaptureVideo.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")


#ifdef DEBUG
#pragma comment(lib,"cv210d.lib")
#pragma comment(lib,"cxcore210d.lib")
#pragma comment(lib,"highgui210d.lib")
#else
#pragma comment(lib,"cv210.lib")
#pragma comment(lib,"cxcore210.lib")
#pragma comment(lib,"highgui210.lib")
#endif

#pragma comment(lib,"libmx")
#pragma comment(lib,"libmat")
#pragma comment(lib,"libeng")

#pragma comment(lib,"brook")