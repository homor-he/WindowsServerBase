
#include "Test_Timer.h"
#include "Test_ClientServer.h"
#include "Test_ThreadPool.h"
#include "TcpThreadServer.h"
#include "Test_ConnectionPool.h"

int main()
{
	//��ʱ������
	//TestTimer();

	//���Կͻ��˺ͷ����
	//TestClientServer();

	//�����̳߳�
	//TestThreadPool();

	//���Է����
	TcpThreadServer server;
	server.SetPara("127.0.0.1", 30301, 16);
	server.Start();

	Sleep(5000);

	//����ʵ��iocp��tcpClient�������ӷ����
	//thread t(ClientIocpProcess);
	//t.detach();

	//����ʵ��iocp��SingleConnection�������ӷ���� �����첽��Ϣ
	thread t(TestSingConnectionAsync);
	t.detach();

	//����ʵ��iocp��SingleConnection�������ӷ���� ����ͬ����Ϣ
	//thread t(TestSingConnectionSync);
	//t.detach();

	while (true)
		Sleep(1000);
}