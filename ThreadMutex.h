#pragma once

#include <windows.h>
#include <shared_mutex>

typedef std::unique_lock<std::shared_mutex> WAutoLock;
typedef std::shared_lock<std::shared_mutex> RAutoLock;

class CThreadMutex
{
public:
	CThreadMutex() { ::InitializeCriticalSection(&m_critSec); };
	~CThreadMutex()
	{
		//::EnterCriticalSection(&m_critSec);
		::DeleteCriticalSection(&m_critSec);
	}

public:
	void Lock() { ::EnterCriticalSection(&m_critSec); }
	void UnLock() { ::LeaveCriticalSection(&m_critSec); }
	void TryLock() { TryEnterCriticalSection(&m_critSec); }

private:
	CRITICAL_SECTION m_critSec;
};


class AutoLock
{
public:
	explicit AutoLock(CThreadMutex& mutex) :m_mutex(mutex) { m_mutex.Lock(); }
	~AutoLock() { m_mutex.UnLock(); }

private:
	CThreadMutex& m_mutex;
};



