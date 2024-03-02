#include "TimerWheel.h"
#include <string.h>
#include "Log.h"
#include <algorithm>

using namespace std;

#define FIRST_INDEX(v) ((v) & TWF_MASK)
#define NTH_INDEX(v,n) (((v)>>(TWF_BITS +(n)* TWN_BITS )) & TWN_MASK)

ThreadPool gbs_TimerThreadPool;

//static bool cmp(TimerNode* node1, TimerNode* node2)
//{
//	return node1->uniqueID < node2->uniqueID;
//}

//TimerNode* FindTimerNode(vector<TimerNode*>& nodeList, DWORD uniqueID)
//{
//	TimerNode node;
//	node.uniqueID = uniqueID;
//	vector<TimerNode*>::iterator itor = lower_bound(nodeList.begin(), nodeList.end(), &node, cmp);
//	if (itor == nodeList.end() || (*itor)->uniqueID > uniqueID)
//		return nullptr;
//	else
//		return *itor;
//}
//
//bool InsertTimerNode(vector<TimerNode*>& nodeList, TimerNode* node)
//{
//	vector<TimerNode*>::iterator itor = lower_bound(nodeList.begin(), nodeList.end(), node, cmp);
//	if (itor != nodeList.end() && (*itor)->uniqueID == node->uniqueID)
//	{
//		WriteLog("%s:%d, uniqueID:%u is same, insertTimerNode fail", __FILE__, __LINE__, node->uniqueID);
//		return false;
//	}
//	else
//	{
//		nodeList.insert(itor, node);
//		return true;
//	}
//}
//
//bool RemoveTimerNode(vector<TimerNode*>& nodeList, TimerNode* node)
//{
//	vector<TimerNode*>::iterator itor = lower_bound(nodeList.begin(), nodeList.end(), node, cmp);
//	if (itor != nodeList.end() && (*itor)->uniqueID == node->uniqueID)
//	{
//		nodeList.erase(itor);
//	}
//	return true;
//}

TimerNode* FindTimerNode(map<DWORD, TimerNode*>& nodeList, DWORD uniqueID)
{
	map<DWORD, TimerNode*>::iterator itor = nodeList.find(uniqueID);
	if (itor != nodeList.end())
		return itor->second;
	return nullptr;
}

bool InsertTimerNode(map<DWORD, TimerNode*>& nodeList, TimerNode* node)
{
	if (nodeList.count(node->uniqueID) == 1)
	{
		WriteLog("%s:%d, uniqueID:%u is same, insertTimerNode fail", __FILE__, __LINE__, node->uniqueID);
		return false;
	}
	else
	{
		nodeList.insert(pair<DWORD, TimerNode*>(node->uniqueID, node));
		return true;
	}
}

bool RemoveTimerNode(map<DWORD, TimerNode*>& nodeList, TimerNode* node)
{
	map<DWORD, TimerNode*>::iterator itor = nodeList.find(node->uniqueID);
	if (itor != nodeList.end())
		nodeList.erase(itor);
	return true;
}

void TimerWheel_init(TimerWheel* tw, uint16_t interval, uint64_t currentTime)
{
	if (tw == nullptr)
	{
		WriteLog("%s:%d, timerwheel is null", __FILE__, __LINE__);
		return;
	}

	memset(tw, 0, sizeof(TimerWheel));
	tw->interval = interval;
	tw->lastTimeStamp = currentTime;
	int i, j;
	for (i = 0; i < TWF_SIZE; ++i)
	{
		CycleLinkList::Linklist_init(&tw->twFirst.list[i]);
	}

	for (i = 0; i < 4; ++i)
	{
		for (j = 0; j < TWN_SIZE; ++j)
		{
			CycleLinkList::Linklist_init(&tw->twOther[i].list[j]);
		}
	}
}

void TimerWheel_node_init(TimerNode* node, TimerCallback callback)
{
	node->prev = nullptr;
	node->next = nullptr;
	node->callback = callback;
	node->expire = 0;
}

void _TimerWheel_add(TimerWheel* tw, TimerNode* node)
{
	uint32_t expire = node->expire;
	if (expire < tw->currentTick)
	{
		WriteLog("%s:%d, timernode expire < timerwheel currentTick", __FILE__, __LINE__);
		return;
	}
	uint32_t idx = expire - tw->currentTick;
	LinkNode* head = nullptr;
	if (idx < TWF_SIZE)
		head = &tw->twFirst.list[FIRST_INDEX(expire)];
	else
	{
		uint32_t compareVal = 0;
		for (int i = 0; i < 4; ++i)
		{
			compareVal = 1 << (TWF_BITS + (i + 1) * TWN_BITS);
			if (compareVal > idx)
			{
				//获取时间轮高刻度位置的链表头
				idx = NTH_INDEX(expire, i);
				head = &tw->twOther[i].list[idx];
				break;
			}
		}
	}
	CycleLinkList::Linklist_add_back(head, (LinkNode*)node);
}

void TimerWheel_add(TimerWheel* tw, TimerNode* node, uint32_t ticks)
{
	if (tw == nullptr || node == nullptr)
	{
		WriteLog("%s:%d, node or timerwheel is null", __FILE__, __LINE__);
		return;
	}
	node->expire = tw->currentTick + ((ticks > 0) ? ticks : 1);
	_TimerWheel_add(tw, node);
}

int TimerWheel_del(TimerWheel* tw, TimerNode* node)
{
	if (!CycleLinkList::Linklist_is_empty((LinkNode*)node))
	{
		CycleLinkList::Linklist_remote((LinkNode*)node);
		return 1;
	}
	return 0;
}

int TimerWheel_pause(TimerWheel* tw, TimerNode* node)
{
	TimerWheel_del(tw, node);
	uint32_t expire = node->expire;
	if (expire < tw->currentTick)
	{
		WriteLog("%s:%d, timernode expire < timerwheel currentTick", __FILE__, __LINE__);
		return false;
	}
	uint32_t idx = expire - tw->currentTick;
	LinkNode* head = nullptr;
	if (idx < TWF_SIZE)
		head = &tw->twFirst.list[FIRST_INDEX(expire)];
	else
	{
		uint32_t compareVal = 0;
		for (int i = 0; i < 4; ++i)
		{
			compareVal = 1 << (TWF_BITS + (i + 1) * TWN_BITS);
			if (compareVal > idx)
			{
				idx = NTH_INDEX(expire, i);
				head = &tw->twOther[i].list[idx];
				break;
			}
		}
	}
	node->lastHead = head;
	return 0;
}

//向低刻度传达
void _TimerWheel_cascade(TimerWheel* tw, TWOther* twOther, DWORD index)
{
	LinkNode head;
	CycleLinkList::Linklist_init(&head);
	CycleLinkList::Linklist_splice(&twOther->list[index], &head);
	while (!CycleLinkList::Linklist_is_empty(&head))
	{
		TimerNode* next = (TimerNode*)head.next;			//依次取得头节点后的第一个节点
		CycleLinkList::Linklist_remote(head.next);			//从链表中移除该节点
		_TimerWheel_add(tw, next);							//将满足触发要求的节点放在低位轮,以便触发
	}
}

void _TimerWheel_tick(TimerWheel* tw, ThreadPool& threadPool)
{
	++tw->currentTick;     //刻度+1
	uint32_t currentick = tw->currentTick;
	int index = currentick & TWF_MASK;
	//如果index==0,则检查高刻度的轮子
	if (index == 0)
	{
		int i = 0;
		DWORD idx = 0;
		do {
			//获取高刻度位置的链表头
			idx = NTH_INDEX(currentick, i);
			//把高刻度上的任务放到低刻度执行
			_TimerWheel_cascade(tw, &tw->twOther[i], idx);
		} while (idx == 0 && (++i < 4));
	}

	LinkNode head;
	CycleLinkList::Linklist_init(&head);
	CycleLinkList::Linklist_splice(tw->twFirst.list + index, &head);
	while (!CycleLinkList::Linklist_is_empty(&head))
	{
		TimerNode* next = (TimerNode*)head.next;
		CycleLinkList::Linklist_remote(head.next);
		if (next != nullptr)
		{
			if (next->callback)
			{
				//加到多线程中
				threadPool.AddTask(next->callback);
			}

			//区分1、执行一次的定时器 2、需要循环间隔执行的定时器
			if (next->timerType == TimerType::Repeat)
			{
				next->expire = currentick + next->delayTime / tw->interval;
				_TimerWheel_add(tw, next);
			}
		}
	}
}

void TimerWheel_update(TimerWheel* tw, uint64_t currentTime, ThreadPool& threadPool)
{
	if (currentTime > tw->lastTimeStamp)
	{
		uint64_t diff = currentTime - tw->lastTimeStamp + tw->remainder;
		tw->lastTimeStamp = currentTime;
		while (diff > tw->interval)
		{
			diff -= tw->interval;
			//执行tick
			_TimerWheel_tick(tw, threadPool);
		}
		tw->remainder = (uint16_t)diff;
	}
}