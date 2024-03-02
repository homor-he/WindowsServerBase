#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include "Thread.h"

#define MAX_THREADCNT 100


class ThreadPool
{
public:
	ThreadPool();
	ThreadPool(DWORD numThread);
	~ThreadPool();

	void Start();
	void AddTask(function<void()> CallBack);
	void RemoveAllThread();
protected:
	DWORD m_numThread;
	atomic<DWORD> m_threadIndex;
	vector<Thread*> m_threadList;
};

