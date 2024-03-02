#pragma once

#include "Base.h"
//#include <string>
#include "TcpSocketBase.h"
#include "SocketDefine.h"
#include <atomic>
#include "ThreadTask.h"
#include "Thread.h"

#define  MAX_RECONNECT_TIMES 30

class TcpClient;
class TcpClientIocpTask : public ThreadTask
{
public:
	TcpClientIocpTask(TcpClient* pTcpClient);
	virtual ~TcpClientIocpTask();

	void Execute(void* para);

	void SetIocp(HANDLE iocp);
	bool HandleError(PER_SOCKET_CONTEXT* pContext, DWORD dwErr);
	bool IsSocketAlive(SOCKET sock);
	void RemoveSocketContext();
	void Reconnect();
	
	bool PostSend(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext);
	bool PostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext);
	void RePostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext);
private:
	HANDLE m_hIPCompletionPort;
	TcpClient* m_pTcpClient;
	PER_SOCKET_CONTEXT* m_cltContext;
};

class TcpClient
{
	friend class TcpClientIocpTask;
public:
	TcpClient();
	TcpClient(const string & szIP, short wPort);
	virtual ~TcpClient();

	virtual bool Connect();
	virtual bool Reconnect();
	void CloseIocpThread();
	void CloseSocket();
	
	void SendMsgAsync(string & sendMsg);
	virtual void OnMsg(char* buf, int len);
private:
	bool SetRecvIO();
private:
	TcpSocketBase m_clientSock;
	PER_SOCKET_CONTEXT* m_cltContext;

	HANDLE m_IOCompletionPort;
	string m_szIP;
	short m_wPort;

	Thread m_iocpThread;
	TcpClientIocpTask* m_pIocpThreadTask;

	int m_reconnectTimes;
	atomic<bool> m_bReconnectStarted;
};
