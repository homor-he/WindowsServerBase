#pragma once

#include "TimerDefine.h"
//#include <vector>
#include <map>
#include "ThreadMutex.h"
#include "ThreadPool.h"

extern ThreadPool gbs_TimerThreadPool;

//禁止在外部delete
//TimerNode* FindTimerNode(vector<TimerNode*>& nodeList, DWORD uniqueID);
//bool InsertTimerNode(vector<TimerNode*>& nodeList, TimerNode* node);
//bool RemoveTimerNode(vector<TimerNode*>& nodeList, TimerNode* node);
TimerNode* FindTimerNode(map<DWORD,TimerNode*>& nodeList, DWORD uniqueID);
bool InsertTimerNode(map<DWORD, TimerNode*>& nodeList, TimerNode* node);
bool RemoveTimerNode(map<DWORD, TimerNode*>& nodeList, TimerNode* node);

// 初始化时间轮，interval为每刻度的时间间隔，currentTime为当前时间
void TimerWheel_init(TimerWheel* tw, uint16_t interval, uint64_t currentTime);

// 初始化时间结点：callback为回调，userData为用户数据
void TimerWheel_node_init(TimerNode* node, TimerCallback callback);

// 增加时间结点，ticks为触发间隔(注意是以interval为单位)
void TimerWheel_add(TimerWheel* tw, TimerNode* node, uint32_t ticks);

// 删除结点
int TimerWheel_del(TimerWheel* tw, TimerNode* node);

//暂停结点
int TimerWheel_pause(TimerWheel* tw, TimerNode* node);

// 更新时间轮
void TimerWheel_update(TimerWheel* tw, uint64_t currentTime, ThreadPool& threadPool);
