#pragma once

#pragma once

#include <functional>
#include <queue>
#include <memory>
#include <atomic>
#include "ThreadMutex.h"

using namespace std;

class ThreadTask
{
public:
	ThreadTask();
	virtual ~ThreadTask() {};
	virtual void Execute(void* para);

	bool AddTask(function<void()> callback);
	bool GetQuitStat();
	void SetQuit();
	void SetNotQuit();
protected:
	atomic_bool m_quit;
	CThreadMutex m_mutex;
	queue<function<void()>> m_callbackList;
	//void(*funcCallBack)(void*);
};

