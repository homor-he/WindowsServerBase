
#include "Test_Timer.h"
#include "Test_ClientServer.h"
#include "Test_ThreadPool.h"
#include "TcpThreadServer.h"
#include "Test_ConnectionPool.h"

int main()
{
	//定时器测试
	//TestTimer();

	//测试客户端和服务端
	//TestClientServer();

	//测试线程池
	//TestThreadPool();

	//测试服务端
	TcpThreadServer server;
	server.SetPara("127.0.0.1", 30301, 16);
	server.Start();

	Sleep(5000);

	//测试实现iocp的tcpClient对象连接服务端
	//thread t(ClientIocpProcess);
	//t.detach();

	//测试实现iocp的SingleConnection对象连接服务端 发送异步消息
	thread t(TestSingConnectionAsync);
	t.detach();

	//测试实现iocp的SingleConnection对象连接服务端 发送同步消息
	//thread t(TestSingConnectionSync);
	//t.detach();

	while (true)
		Sleep(1000);
}