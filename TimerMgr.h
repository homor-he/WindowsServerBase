#pragma once

#include "TimerWheel.h"
#include <thread>

class TimerMgr
{
public:
	TimerMgr();
	~TimerMgr();
	void Start();
	//timerType = 0 执行一次 1 重复执行 | delayTime_ms：定时器执行间隔 | dueTime_ms：第一次执行延时
	void AddTimer(DWORD uniqueID, TimerCallback callback, DWORD delayTime_ms, DWORD timerType = TimerType::Once, DWORD dueTime_ms = 0);
	void PauseTimer(DWORD uniqueID);
	//add -> pause -> restart  (running状态可以)del
	void ReStartTimer(DWORD uniqueID);
	void DelTimer(DWORD uniqueID);
private:
	CThreadMutex m_timerMutex;
	//vector<TimerNode*> m_timerNodeList;   //管理所有timerNode
	std::map<DWORD, TimerNode*> m_timerNodeMap;
	//timer任务执行线程池
	ThreadPool m_TaskPool;

	TimerWheel m_timerWheel;
	uint16_t m_interval;
	bool m_QuitExecuteThread;
	//timerwheel tick线程
	std::unique_ptr<std::thread> m_TimerMgrExecute;
};