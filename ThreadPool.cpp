#include "ThreadPool.h"
#include "Log.h"

ThreadPool::ThreadPool() :
	m_numThread(2),
	m_threadIndex(0)
{
}

ThreadPool::ThreadPool(DWORD numThread) :
	m_numThread(numThread > MAX_THREADCNT ? MAX_THREADCNT : numThread),
	m_threadIndex(0)
{
	Start();
}

ThreadPool::~ThreadPool()
{
	RemoveAllThread();
}

void ThreadPool::Start()
{
	for (DWORD i = 0; i < m_numThread; ++i)
	{
		string name = "Thread" + to_string(i);
		Thread* pThread = new Thread(name, i);
		m_threadList.push_back(move(pThread));
		pThread->Start();
	}
}

void ThreadPool::AddTask(function<void()> CallBack)
{
	DWORD expect = m_numThread;
	DWORD desired = 0;
	//如果为true， m_threadIndex = desired
	//如果为false, expect = m_threadIndex
	if (!atomic_compare_exchange_strong(&m_threadIndex, &expect, desired))
	{
		desired = expect;
		m_threadIndex++;
	}
	Thread* pThread = m_threadList[desired];
	if (pThread != nullptr)
	{
		ThreadTask* pTask = pThread->GetThreadTask();
		if (pTask != nullptr)
		{
			pTask->AddTask(CallBack);
			pThread->Signal();
		}
		else
			WriteLog("%s:%d, task is null", __FILE__, __LINE__);
	}
	else
		WriteLog("%s:%d, thread is null", __FILE__, __LINE__);
}

void ThreadPool::RemoveAllThread()
{
	for (size_t i = 0; i < m_threadList.size(); ++i)
	{
		delete m_threadList[i];
		m_threadList[i] = nullptr;
	}
}
