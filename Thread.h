#pragma once

#include <string>
//#include <pthread.h>
#include <thread>
#include "Event.h"
#include "ThreadTask.h"
#include <atomic>

//using namespace std;


class Thread
{
public:
	//仅用于继承
	Thread();

	//defaulttask = true 线程自己创建默认task，task生命周期由thread控制
	Thread(const std::string& name, int threadNo, ThreadTask * task = nullptr, bool defaultTask = true);
	virtual ~Thread();

	virtual void Run();
	virtual void Start();
	void Join(int t_ms);
	void Signal();
	void SetStartStat(bool val);
	bool GetStartedStatus();
	DWORD GetThreadID();
	void Quit();
	ThreadTask* GetThreadTask();
	void SetTask(ThreadTask* task);
protected:
	static unsigned int __stdcall ThreadFunction(void*);

protected:
	std::atomic_bool m_quit;
	std::atomic_bool m_started;
	HANDLE  m_handle;		//线程句柄
	DWORD m_threadID;		//线程ID
	DWORD m_threadNo;		//第几个额外启动的线程
	Event m_event;
	ThreadTask* m_task;
	bool m_defaultTask;
};


