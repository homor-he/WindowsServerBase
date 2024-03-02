#pragma once

#include "TimerDefine.h"
//#include <vector>
#include <map>
#include "ThreadMutex.h"
#include "ThreadPool.h"

extern ThreadPool gbs_TimerThreadPool;

//��ֹ���ⲿdelete
//TimerNode* FindTimerNode(vector<TimerNode*>& nodeList, DWORD uniqueID);
//bool InsertTimerNode(vector<TimerNode*>& nodeList, TimerNode* node);
//bool RemoveTimerNode(vector<TimerNode*>& nodeList, TimerNode* node);
TimerNode* FindTimerNode(map<DWORD,TimerNode*>& nodeList, DWORD uniqueID);
bool InsertTimerNode(map<DWORD, TimerNode*>& nodeList, TimerNode* node);
bool RemoveTimerNode(map<DWORD, TimerNode*>& nodeList, TimerNode* node);

// ��ʼ��ʱ���֣�intervalΪÿ�̶ȵ�ʱ������currentTimeΪ��ǰʱ��
void TimerWheel_init(TimerWheel* tw, uint16_t interval, uint64_t currentTime);

// ��ʼ��ʱ���㣺callbackΪ�ص���userDataΪ�û�����
void TimerWheel_node_init(TimerNode* node, TimerCallback callback);

// ����ʱ���㣬ticksΪ�������(ע������intervalΪ��λ)
void TimerWheel_add(TimerWheel* tw, TimerNode* node, uint32_t ticks);

// ɾ�����
int TimerWheel_del(TimerWheel* tw, TimerNode* node);

//��ͣ���
int TimerWheel_pause(TimerWheel* tw, TimerNode* node);

// ����ʱ����
void TimerWheel_update(TimerWheel* tw, uint64_t currentTime, ThreadPool& threadPool);
