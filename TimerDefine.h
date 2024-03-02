#pragma once

#include "CycleLinkList.h"
#include <functional>

using namespace std;


// 第1个轮占的位数
#define TWF_BITS 8
// 第1个轮的长度 256刻度
#define TWF_SIZE (1 << TWF_BITS) 
// 第n个轮占的位数 
#define TWN_BITS 6
// 第n个轮的长度 64刻度
#define TWN_SIZE (1 << TWN_BITS)
// 掩码：取模或整除用
#define TWF_MASK (TWF_SIZE - 1)
#define TWN_MASK (TWN_SIZE - 1)

enum TimerType
{
	Once,
	Repeat,
};

enum TimerStat
{
	Running,
	Pauseing,
};

typedef function<void()> TimerCallback;

// 第1个轮 
typedef struct TWFirst {
	LinkNode list[TWF_SIZE];
} TWFIRST, * PTWFIRST;

// 后面几个轮
typedef struct TWOther {
	LinkNode list[TWN_SIZE];
} TWOTHER, * PTWOTHER;

// 时间轮定时器
typedef struct TimerWheel {
	TWFirst twFirst;				// 第1个轮
	TWOther twOther[4];				// 后面4个轮
	uint64_t lastTimeStamp;         // 上一次调用时的时间 单位：毫秒
	uint32_t currentTick;           // 当前的tick
	uint16_t interval;              // 每个时间点的毫秒间隔(每个刻度的毫秒间隔)
	uint16_t remainder;             // 剩余的毫秒
}TIMERWHEEL, * PTIMEWHEEL;

// 定时器结点
typedef struct TimerNode {
	LinkNode* prev;					// 上一个结点
	LinkNode* next;					// 下一个结点
	TimerCallback callback;			// 回调函数
	uint32_t expire;                // 到期时间
	uint32_t uniqueID;				// 定时器节点标识
	uint32_t timerType;				// 定时器类型 （区分是延时执行一次的定时器还是循环触发的定时器）
	uint32_t timerStat;
	uint32_t delayTime;				// 用户传入的延时时间
	uint32_t dueTime;				// 第一次调用的延时时间
	LinkNode* lastHead;				// 定时器暂停前的循环链表头
} TIMERNODE, * PTIMERNODE;
