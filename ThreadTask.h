#pragma once

#pragma once

#include <functional>
#include <queue>
#include <memory>
#include <atomic>
#include "ThreadMutex.h"

//using namespace std;

class ThreadTask
{
public:
	ThreadTask();
	virtual ~ThreadTask() {};
	virtual void Execute(void* para);

	bool AddTask(std::function<void()> callback);
	bool GetQuitStat();
	void SetQuit();
	void SetNotQuit();
protected:
	std::atomic_bool m_quit;
	CThreadMutex m_mutex;
	std::queue<std::function<void()>> m_callbackList;
	//void(*funcCallBack)(void*);
};

