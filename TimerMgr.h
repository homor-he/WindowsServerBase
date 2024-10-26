#pragma once

#include "TimerWheel.h"
#include <thread>

class TimerMgr
{
public:
	TimerMgr();
	~TimerMgr();
	void Start();
	//timerType = 0 ִ��һ�� 1 �ظ�ִ�� | delayTime_ms����ʱ��ִ�м�� | dueTime_ms����һ��ִ����ʱ
	void AddTimer(DWORD uniqueID, TimerCallback callback, DWORD delayTime_ms, DWORD timerType = TimerType::Once, DWORD dueTime_ms = 0);
	void PauseTimer(DWORD uniqueID);
	//add -> pause -> restart  (running״̬����)del
	void ReStartTimer(DWORD uniqueID);
	void DelTimer(DWORD uniqueID);
private:
	CThreadMutex m_timerMutex;
	//vector<TimerNode*> m_timerNodeList;   //��������timerNode
	std::map<DWORD, TimerNode*> m_timerNodeMap;
	//timer����ִ���̳߳�
	ThreadPool m_TaskPool;

	TimerWheel m_timerWheel;
	uint16_t m_interval;
	bool m_QuitExecuteThread;
	//timerwheel tick�߳�
	std::unique_ptr<std::thread> m_TimerMgrExecute;
};