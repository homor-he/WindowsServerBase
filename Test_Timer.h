#pragma once

#include "Base.h"
#include "TimerMgr.h"
#include "Log.h"
#include "CommonFunc.h"

uint64_t time1 = 0;
uint64_t time2 = 0;

void Test1()
{
	time1 = GetTimeStampInMS();
	WriteLog("Test1 %lld", time1);
}

void Test2()
{
	time2 = GetTimeStampInMS();
	WriteLog("Test2 %lld", time2);
}

class Plus
{
public:
	void Debug(void* para)
	{
		char* str = (char*)para;
		cout << str << endl;
	}
};

void TestTimer()
{
	SetLogFileName("WindowServerBase");
	TimerMgr timeMgr;
	//timeMgr.Start();
	time1 = GetTimeStampInMS();
	time2 = GetTimeStampInMS();
	WriteLog("Test1 %lld", time1);
	WriteLog("Test2 %lld", time2);
	timeMgr.AddTimer(1, Test1, 1000, TimerType::Repeat);
	timeMgr.AddTimer(2, Test2, 1000, TimerType::Repeat, 5000);

	//执行对象的成员函数
	/*Plus* p = new Plus;
	const char* str = "hello";
	function<void()> f = bind(&Plus::Debug, p, (void*)str);*/

	Sleep(10000);
	timeMgr.PauseTimer(2);
	WriteLog("pause timer2");

	Sleep(10000);
	timeMgr.ReStartTimer(2);
	WriteLog("restart timer2");

	while (true)
	{
		Sleep(1000);
	}
}


