#include "Thread.h"
#include "Log.h"

Thread::Thread() : 
	m_quit(false),
	m_started(false),
	m_threadID(0),
	m_threadNo(0),
	m_task(nullptr),
	m_handle(nullptr)
{
}

Thread::Thread(const std::string& name, int threadNo, ThreadTask* task, bool defaultTask) :
	m_quit(false),
	m_started(false),
	m_threadID(0),
	m_threadNo(threadNo),
	m_handle(nullptr),
	m_task(task),
	m_defaultTask(defaultTask)
{
	if(defaultTask)
		m_task = new ThreadTask;
}

Thread::~Thread()
{
	Quit();
}

void Thread::Run()
{
	while (!m_quit.load())
	{
		if (m_task == nullptr || m_task->GetQuitStat())
		{
			Join(INFINITE);
		}
		m_task->Execute(nullptr);
		//WriteLog("threadNo:%d threadID:%u is run",m_threadNo, m_threadID);
	}
}

void Thread::Start()
{   
	//linux方式
	//int res = pthread_create(&m_threadID, nullptr, ThreadFunction, this);
	//if (res != 0)
	//	WriteLog("%s:%d, create thread fail", __FILE__, __LINE__);
	//else
	//{
	//	//WriteLog("create thread %u success", m_threadID);
	//}
	unsigned int threadID =0;
	m_handle = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, this, 0, &threadID);
	this->m_threadID = threadID;
}

void Thread::Join(int t_ms)
{
	m_event.Wait(t_ms);
}

void Thread::Signal()
{
	m_event.Signal();
}

void Thread::SetStartStat(bool val)
{
	m_started.store(val);
}

bool Thread::GetStartedStatus()
{
	return m_started.load();
}

DWORD Thread::GetThreadID()
{
	return m_threadID;
}

void Thread::Quit()
{
	if (m_task != NULL)
	{
		m_quit.store(true);
		m_task->SetQuit();
		this->Join(2000);    //主线程等待子线程执行完
		Signal();
		if (m_defaultTask)
		{
			delete m_task;
			m_task = NULL;
		}
	}
}

ThreadTask* Thread::GetThreadTask()
{
	return m_task;
}

void Thread::SetTask(ThreadTask* task)
{
	m_task = task;
}

unsigned int __stdcall Thread::ThreadFunction(void* pArg)
{
	Thread* pThread = (Thread*)pArg;
	if (pThread != nullptr)
		pThread->Run();
	else
		WriteLog("%s:%d,thread is null", __FILE__, __LINE__);
	return 0;
}