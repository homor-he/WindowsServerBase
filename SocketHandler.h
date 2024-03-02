#pragma once

#include "SocketDefine.h"
#include "ThreadTask.h"
#include <atomic>

class TcpThreadServer;

class SocketHandler 
{
public:
	SocketHandler();
	virtual ~SocketHandler();

	void Execute(void* para);
	void SetIOCP(HANDLE iocp);
	void SetTcpThreadServer(TcpThreadServer* server);
	bool HandleError(PER_SOCKET_CONTEXT* pContext, DWORD dwErr);
	bool IsSocketAlive(SOCKET sock);
	void RemoveSocketContext(PER_SOCKET_CONTEXT* pContext);
	void Quit();
	void RePostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext);

	bool PostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext);
	bool PostSend(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext);
private:
	atomic_bool m_quit;
	HANDLE m_hIPCompletionPort;
	TcpThreadServer* m_server;
};