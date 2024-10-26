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
	//�����ڼ̳�
	Thread();

	//defaulttask = true �߳��Լ�����Ĭ��task��task����������thread����
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
	HANDLE  m_handle;		//�߳̾��
	DWORD m_threadID;		//�߳�ID
	DWORD m_threadNo;		//�ڼ��������������߳�
	Event m_event;
	ThreadTask* m_task;
	bool m_defaultTask;
};


