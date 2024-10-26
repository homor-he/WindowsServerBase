#pragma once

#include "Log.h"
#include "ThreadPool.h"

void Add(int a, int b)
{
	int res = a + b;
	WriteLog("add res:%d", res);
}

void Del(int a, int b)
{
	int res = a - b;
	WriteLog("del res:%d", res);
}

void TestThreadPool()
{
	ThreadPool pool;
	pool.Start();
	pool.AddTask(std::bind(Add, 3, 4));
	pool.AddTask(std::bind(Del, 4, 3));
	while (true)
	{
		Sleep(1000);
	}
}