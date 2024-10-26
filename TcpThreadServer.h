#pragma once

#include "Base.h"
#include <string>
#include "Thread.h"
#include "TcpSocketBase.h"
#include "SocketDefine.h"
#include <atomic>
#include "ThreadMutex.h"
#include "ThreadPool.h"
#include "SocketHandler.h"
#include <map>
#include "ProtoGobalDefine.h"

#pragma warning(disable: 4244)

struct SocketArg
{
	TcpSocketBase new_sock;
	std::string szIP;
	USHORT wport;
	//Handler handler;
	SocketArg()
	{
		szIP = "";
		wport = 0;
	}
};

class TcpThreadServer : public Thread
{
public:
	//TcpThreadServer(const string& szIP, short wPort);
	TcpThreadServer();
	virtual ~TcpThreadServer();

	static TcpThreadServer& Instance()
	{
		static TcpThreadServer tcpServer;
		return tcpServer;
	}

	//override
	virtual void Run();

	//构造后立刻调用
	void SetPara(const std::string& szIP, short wPort, int threadNum);

	bool StartServer();
	bool CloseServer();
	bool HandleNewConn();
	bool PostRecv(std::shared_ptr<PER_IO_CONTEXT> pIOContext, PER_SOCKET_CONTEXT* pSocketContext);
	void RemoveSocketContext(SOCKET sock);
	void DeleteSocketContext(SOCKET sock);
	
	virtual void OnMsg(PER_SOCKET_CONTEXT* socket, char* buf, int len);
	void OnAsyncMsg(PER_SOCKET_CONTEXT* socket, share_buff buff);
	bool DisConnected(PER_SOCKET_CONTEXT* socket);
public:
	std::shared_ptr<LinkNetObj> AddLinkNetObj(PER_SOCKET_CONTEXT* socket, UINT type);
	const std::shared_ptr<LinkNetObj> GetLinkNetObj(const PER_SOCKET_CONTEXT* socket);
	virtual std::shared_ptr<LinkNetObj> CreateLinkNetObjBase(PER_SOCKET_CONTEXT* socket, UINT type);
	bool RemoveLinkNetObj(SOCKET sock);
private:
	bool Init();
protected:
	TcpSocketBase m_listenSock;
	std::shared_ptr<PER_SOCKET_CONTEXT> m_listenContext;
	int m_listenFD;

	HANDLE m_IOCompletionPort;
	std::string m_szIP;
	short m_wPort;

	std::atomic_bool m_bStarted;
	int m_threadNum;

	CThreadMutex m_sockListMutex;
	//list<shared_ptr<Per_Socket_Context>> m_socketList;
	std::map<SOCKET, PER_SOCKET_CONTEXT*> m_socketList;

	std::shared_ptr<ThreadPool> m_sockHandleThreadPool;    //socket底层消息处理线程池
	std::list<std::shared_ptr<SocketHandler>> m_socketHandlerList;

	CThreadMutex m_linkNetMapLock;
	std::map<UINT, std::shared_ptr<LinkNetObj>> m_linkNetMap;

	std::shared_ptr<ThreadPool> m_taskThreadPool;
};