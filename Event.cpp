#include "Event.h"

void Event::Wait(DWORD dwMillionSecond)
{
	WaitForSingleObject(m_phEvent, dwMillionSecond);

	ResetEvent(m_phEvent);
}

void Event::Signal()
{
	SetEvent(m_phEvent);
}
