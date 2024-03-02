#pragma once

#pragma once

#include <Windows.h>

class Event
{
public:
	//event 手动复位方式true，初始状态为false
	Event() { m_phEvent = CreateEvent(NULL, TRUE, FALSE, NULL); }
	~Event() { if (NULL != m_phEvent) { CloseHandle(m_phEvent); } }

public:
	void Wait(DWORD dwMillionSecond);
	void Signal();

private:
	HANDLE m_phEvent;
};
