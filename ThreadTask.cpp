#include "ThreadTask.h"

ThreadTask::ThreadTask() :
	m_quit(true)
{
}

void ThreadTask::Execute(void* para)
{
	m_quit.store(false);
	std::function<void()> callBack;
	std::queue<std::function<void()>> tmpQueue;
	{
		AutoLock autoLock(m_mutex);
		tmpQueue.swap(m_callbackList);
	}
	
	while (!tmpQueue.empty())
	{
		callBack = move(tmpQueue.front());
		tmpQueue.pop();
		callBack();
	}

	m_quit.store(true);
}

bool ThreadTask::AddTask(std::function<void()> callback)
{
	AutoLock autoLock(m_mutex);
	m_callbackList.push(callback);
	return true;
}

bool ThreadTask::GetQuitStat()
{
	return m_quit.load();
}

void ThreadTask::SetQuit()
{
	m_quit.store(true);
}

void ThreadTask::SetNotQuit()
{
	m_quit.store(false);
}
