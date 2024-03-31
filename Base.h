#pragma once

#define WIN32_LEAN_AND_MEAN  //解决socket库重定义

#include "windows.h"
#include <windef.h>
#include <string>

using namespace std;

//#define _CRT_SECURE_NO_WARNINGS  //解决没有_s函数的warning

#define _WINSOCK_DEPRECATED_NO_WARNINGS //解决wsasocket报错

#pragma comment(lib,"ws2_32.lib")

#ifdef NDEBUG
#pragma comment(lib,"libprotobuf-lite.lib")
#pragma comment(lib,"libprotobuf.lib")
#pragma comment(lib,"libprotoc.lib")
#elif DEBUG
#pragma comment(lib,"libprotobuf-lited.lib")
#pragma comment(lib,"libprotobufd.lib")
#pragma comment(lib,"libprotocd.lib")
#endif