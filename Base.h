#pragma once

#define WIN32_LEAN_AND_MEAN  //���socket���ض���

#include "windows.h"
#include <windef.h>
#include <string>

using namespace std;

//#define _CRT_SECURE_NO_WARNINGS  //���û��_s������warning

#define _WINSOCK_DEPRECATED_NO_WARNINGS //���wsasocket����

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