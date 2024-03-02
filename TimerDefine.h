#pragma once

#include "CycleLinkList.h"
#include <functional>

using namespace std;


// ��1����ռ��λ��
#define TWF_BITS 8
// ��1���ֵĳ��� 256�̶�
#define TWF_SIZE (1 << TWF_BITS) 
// ��n����ռ��λ�� 
#define TWN_BITS 6
// ��n���ֵĳ��� 64�̶�
#define TWN_SIZE (1 << TWN_BITS)
// ���룺ȡģ��������
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

// ��1���� 
typedef struct TWFirst {
	LinkNode list[TWF_SIZE];
} TWFIRST, * PTWFIRST;

// ���漸����
typedef struct TWOther {
	LinkNode list[TWN_SIZE];
} TWOTHER, * PTWOTHER;

// ʱ���ֶ�ʱ��
typedef struct TimerWheel {
	TWFirst twFirst;				// ��1����
	TWOther twOther[4];				// ����4����
	uint64_t lastTimeStamp;         // ��һ�ε���ʱ��ʱ�� ��λ������
	uint32_t currentTick;           // ��ǰ��tick
	uint16_t interval;              // ÿ��ʱ���ĺ�����(ÿ���̶ȵĺ�����)
	uint16_t remainder;             // ʣ��ĺ���
}TIMERWHEEL, * PTIMEWHEEL;

// ��ʱ�����
typedef struct TimerNode {
	LinkNode* prev;					// ��һ�����
	LinkNode* next;					// ��һ�����
	TimerCallback callback;			// �ص�����
	uint32_t expire;                // ����ʱ��
	uint32_t uniqueID;				// ��ʱ���ڵ��ʶ
	uint32_t timerType;				// ��ʱ������ ����������ʱִ��һ�εĶ�ʱ������ѭ�������Ķ�ʱ����
	uint32_t timerStat;
	uint32_t delayTime;				// �û��������ʱʱ��
	uint32_t dueTime;				// ��һ�ε��õ���ʱʱ��
	LinkNode* lastHead;				// ��ʱ����ͣǰ��ѭ������ͷ
} TIMERNODE, * PTIMERNODE;
