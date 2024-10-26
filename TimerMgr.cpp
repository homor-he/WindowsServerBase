#include "TimerMgr.h"
#include "CommonFunc.h"
#include "Log.h"

TimerMgr::TimerMgr() :
	m_TaskPool(8),
	m_interval(5),
	m_QuitExecuteThread(false)
{
	TimerWheel_init(&m_timerWheel, m_interval, GetTimeStampInMS());
	Start();
}

TimerMgr::~TimerMgr()
{
	m_QuitExecuteThread = true;
}

void TimerMgr::Start()
{
	std::unique_ptr<std::thread> t(new std::thread([&] {
		while (!m_QuitExecuteThread)
		{
			AutoLock autoLock(m_timerMutex);
			TimerWheel_update(&m_timerWheel, GetTimeStampInMS(), m_TaskPool);
		}
	}));
	t->detach();
	m_TimerMgrExecute = move(t);
}

void TimerMgr::AddTimer(DWORD uniqueID, TimerCallback callback, DWORD delayTime_ms, DWORD timerType, DWORD dueTime_ms)
{
	TimerNode* newNode = new TimerNode;
	memset(newNode, 0, sizeof(TimerNode));
	newNode->callback = callback;
	newNode->delayTime = delayTime_ms;
	newNode->timerType = timerType;
	newNode->dueTime = dueTime_ms;
	newNode->uniqueID = uniqueID;
	newNode->timerStat = TimerStat::Running;
	AutoLock autoLock(m_timerMutex);
	if (InsertTimerNode(m_timerNodeMap, newNode))
	{
		TimerWheel_add(&m_timerWheel, newNode, (delayTime_ms + dueTime_ms) / m_interval);
	}
	WriteLog("timerlist size:%d", m_timerNodeMap.size());
}

void TimerMgr::PauseTimer(DWORD uniqueID)
{
	AutoLock autoLock(m_timerMutex);
	TimerNode* pNode = FindTimerNode(m_timerNodeMap, uniqueID);
	if (pNode != nullptr)
	{
		pNode->timerStat = Pauseing;
		TimerWheel_pause(&m_timerWheel, pNode);
	}
}

void TimerMgr::ReStartTimer(DWORD uniqueID)
{
	AutoLock autoLock(m_timerMutex);
	TimerNode* pNode = FindTimerNode(m_timerNodeMap, uniqueID);
	if (pNode != nullptr && pNode->timerStat == Pauseing)
	{
		pNode->timerStat = Running;
		//TimerWheel_add(&m_timerWheel, pNode, pNode->expire);
		CycleLinkList::Linklist_add_back(pNode->lastHead, (LinkNode*)pNode);
	}
}

void TimerMgr::DelTimer(DWORD uniqueID)
{
	AutoLock autoLock(m_timerMutex);
	TimerNode* pNode = FindTimerNode(m_timerNodeMap, uniqueID);
	if (pNode != nullptr)
	{
		TimerWheel_del(&m_timerWheel, pNode);
		RemoveTimerNode(m_timerNodeMap, pNode);
	}
}