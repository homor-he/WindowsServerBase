#include "TcpThreadServer.h"
#include "Log.h"


TcpThreadServer::TcpThreadServer(): Thread(),
	m_IOCompletionPort(nullptr),
	m_listenFD((int)INVALID_SOCKET),
	m_szIP(""),
	m_wPort(0),
	m_bStarted(false),
	m_threadNum(0),
	m_sockHandleThreadPool(nullptr),
	m_taskThreadPool(nullptr)
{
}

bool TcpThreadServer::Init()
{
	m_IOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_listenSock.Socket(isOverLapped::OverLapped_True))
	{
		if (m_listenSock.Bind(m_szIP, m_wPort))
		{
			if(!m_listenSock.Listen(SOMAXCONN, isNonBlock::IsNonBlock_True))
				return false;
		}
		else
			return false;
	}

	m_listenFD = (int)m_listenSock.GetFd();
	if (m_listenFD == INVALID_SOCKET)
		return false;
	else
		return true;

	/*if (CreateIoCompletionPort((HANDLE)m_listenSock.GetFd(), m_IOCompletionPort, (ULONG_PTR)m_listenContext.get(), 0))
	{
		WriteLog("%s:%d,bind listen socket to completionport fail", __FILE__,__LINE__);
		return false;
	}*/
}

TcpThreadServer::~TcpThreadServer()
{
	auto itor = m_socketHandlerList.begin();
	for (; itor != m_socketHandlerList.end(); ++itor)
	{
		(*itor)->Quit();
		//让完成端口线程退出
		PostQueuedCompletionStatus(m_IOCompletionPort, 0, (DWORD)nullptr, nullptr);
	}
	m_sockHandleThreadPool->RemoveAllThread();

	CloseHandle(m_IOCompletionPort);
	m_listenSock.Close();
}


bool TcpThreadServer::StartServer()
{
	if (Init())
	{
		m_sockHandleThreadPool = make_shared<ThreadPool>(m_threadNum);
		m_taskThreadPool = make_shared<ThreadPool>(m_threadNum);
		for (int i = 0; i < m_threadNum; ++i)
		{
			shared_ptr<SocketHandler> pHandler = make_shared<SocketHandler>();
			pHandler->SetIOCP(m_IOCompletionPort);
			pHandler->SetTcpThreadServer(this);
			m_sockHandleThreadPool->AddTask(bind(&SocketHandler::Execute, pHandler, nullptr));
			m_socketHandlerList.push_back(move(pHandler));
		}

		m_started = true;
		HandleNewConn();
		return true;
	}
	else
		return false;
}

void TcpThreadServer::SetPara(const string& szIP, short wPort, int threadNum)
{
	m_szIP = szIP;
	m_wPort = wPort;
	m_threadNum = threadNum;
}

bool TcpThreadServer::CloseServer()
{
	m_started = false;
	return m_started;
}

bool TcpThreadServer::HandleNewConn()
{
	SocketArg arg;
	while (m_started)
	{
		if (m_listenSock.Accept(&arg.new_sock, &arg.szIP, &arg.wport))
		{
			WriteLog("new connection from ip:%s port:%d", arg.szIP.c_str(), arg.wport);

			arg.new_sock.SetSocketNonBlocking(arg.new_sock.GetFd());
			SOCKADDR_IN client_addr = { 0 };
			SOCKADDR_IN server_addr = { 0 };
			int addrLen = sizeof(client_addr);

			PER_SOCKET_CONTEXT* pNewSockContext = new PER_SOCKET_CONTEXT;
			getpeername(arg.new_sock.GetFd(), (sockaddr*)&client_addr, &addrLen);
			getsockname(m_listenSock.GetFd(), (sockaddr*)&server_addr, &addrLen);
			
			pNewSockContext->m_Socket = arg.new_sock.GetFd();
			memcpy_s(&pNewSockContext->m_PeerAddr, addrLen, &client_addr, addrLen);
			memcpy_s(&pNewSockContext->m_LocalAddr, addrLen, &server_addr, addrLen);

			HANDLE hNewHandle = CreateIoCompletionPort((HANDLE)pNewSockContext->GetSocketFD(), m_IOCompletionPort, (ULONG_PTR)pNewSockContext, 0);
			if (hNewHandle == NULL)
			{
				WriteLog("%s:%d, create completionport fail,lastError:%d", __FILE__, __LINE__, GetLastError());
				return false;
			}

			shared_ptr<PER_IO_CONTEXT> pNewIOContext = pNewSockContext->GetNewIOContext();
			if (pNewIOContext)
			{
				if (PostRecv(pNewIOContext, pNewSockContext) == false)
				{
					pNewSockContext->RemoveIOContext(pNewIOContext.get());
					continue;
				}

				{
					AutoLock lock(m_sockListMutex);
					//m_socketList.push_back(pNewSockContext);
					m_socketList.insert(pair<SOCKET, PER_SOCKET_CONTEXT*>(pNewSockContext->GetSocketFD(), pNewSockContext));
					pNewSockContext->AddRef();
				}
			}
			else
			{
				WriteLog("%s:%d, pNewIOContext is null", __FILE__, __LINE__);
				//return false;
			}
		}
	}
	return false;
}

bool TcpThreadServer::PostRecv(shared_ptr<PER_IO_CONTEXT> pIOContext, PER_SOCKET_CONTEXT* pSocketContext)
{
	pIOContext->ResetBuffer();
	pIOContext->m_OpType = RECV_POSTED;
	pIOContext->m_sockAccept = pSocketContext->m_Socket;
	pIOContext->m_nSendBytes = 0;
	pIOContext->m_nTotalBytes = 0;

	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;

	int ret = WSARecv(pIOContext->m_sockAccept, &pIOContext->m_wsaBuf, 1, &dwRecvBytes, &dwFlags, &pIOContext->m_Overlapped, NULL);
	long lastError = WSAGetLastError();
	if (ret == SOCKET_ERROR)
	{
		if ((lastError != WSA_IO_PENDING) && (lastError != WSAEWOULDBLOCK))
		{
			WriteLog("%s:%d, socket:%d RECV_POSTED fail, lastError:%d", __FILE__, __LINE__, pIOContext->m_sockAccept, WSAGetLastError());
			return false;
		}
		return true;
	}
	else
	{
		//ret == 0, 但是缓冲区没数据，表明客户端断开了
		if (strcmp(pIOContext->m_wsaBuf.buf, "") == 0)
		{
			return false;
		}
		return true;
	}
}

void TcpThreadServer::RemoveSocketContext(SOCKET sock)
{
	AutoLock lock(m_sockListMutex);
	map<SOCKET, PER_SOCKET_CONTEXT*>::iterator it = m_socketList.find(sock);
	if (it != m_socketList.end())
	{
		m_socketList.erase(it);
	}
}

void TcpThreadServer::DeleteSocketContext(SOCKET sock)
{
	AutoLock lock(m_sockListMutex);
	map<SOCKET, PER_SOCKET_CONTEXT*>::iterator it = m_socketList.find(sock);
	if (it != m_socketList.end())
	{
		delete it->second;
		it->second = nullptr;
		m_socketList.erase(it);
	}
}

void TcpThreadServer::OnMsg(PER_SOCKET_CONTEXT* socket, char* buf, int len)
{
	//string recvMsg;
	//recvMsg.append(buf, len);
	//WriteLog("RECV MSG :%s",recvMsg.c_str());
	
	share_buff buff = make_shared<vector<char>>(buf, buf + len);
	m_taskThreadPool->AddTask(bind(&TcpThreadServer::OnAsyncMsg, this, socket, buff));
}

void TcpThreadServer::OnAsyncMsg(PER_SOCKET_CONTEXT* socket, share_buff buff)
{
	if (!socket)
	{
		WriteLog("%s:%d, socket is null", __FILE__, __LINE__);
		return;
	}

	if (!buff)
	{
		WriteLog("%s:%d, socketid:%d peerAddr:%s, buff is null", __FILE__, __LINE__,
			socket->GetSocketFD(), socket->GetPeerIPPort().c_str());
		return;
	}

#ifdef PROTOBUF
	rp::CmnBuf cmnBuf;
	cmnBuf.ParseFromArray(buff->data(), (int)buff->size());
	const rp::CmnBuf_MsgHead header = cmnBuf.msgheader();

	if (header.msgtype() == PROTO_BUILD_CONN_REQ)
	{
		UINT linkType = header.origin();
		shared_ptr<LinkNetObj> pLinkNetObj = AddLinkNetObj(socket, linkType);
		if (pLinkNetObj)
		{
			//只让LinkNetObj拥有PER_SOCKET_CONTEXT的指针对象
			RemoveSocketContext(socket->GetSocketFD());
			WriteLog("***sever::addLinkNetUser success, linkType:%d, fd:%d, remote:%s->local:%s", linkType, socket->GetSocketFD(),
				socket->GetPeerIPPort().c_str(), socket->GetLocalIPPort().c_str());
			//连接建立成功后给一个回包
			pLinkNetObj->SendBuildConnectAck();
		}
		else
		{
			WriteLog("***sever::%s:%d,err, fd repeat:%d, remote:%s->local:%s", __FILE__, __LINE__, socket->GetSocketFD(),
				socket->GetPeerIPPort().c_str(), socket->GetLocalIPPort().c_str());
		}
	}
	else
	{
		if (!socket->OnMsg(buff))
		{
			WriteLog("%s:%d, socketid:%d peerAddr:%s no linknetobj", __FILE__, __LINE__,
				socket->GetSocketFD(), socket->GetPeerIPPort().c_str());
		}
	}
#endif // 
}

bool TcpThreadServer::DisConnected(PER_SOCKET_CONTEXT* socket)
{
	shared_ptr<LinkNetObj> pLinkNetObj = GetLinkNetObj(socket);
	if (pLinkNetObj)
	{
		pLinkNetObj->OnDisconnected(socket);
		RemoveLinkNetObj(socket->GetSocketFD());
		//socket不再拥有pLinkNet对象
		socket->SetLinkNetObj(nullptr);
		return true;
	}
	return false;
}

shared_ptr<LinkNetObj> TcpThreadServer::AddLinkNetObj(PER_SOCKET_CONTEXT* socket, UINT type)
{
	shared_ptr<LinkNetObj> pLinkObj = CreateLinkNetObjBase(socket, type);
	if (pLinkObj)
	{
		AutoLock lock(m_linkNetMapLock);
		if (m_linkNetMap.count((UINT)socket->GetSocketFD()) == 0)
		{
			//使socket能获取linkObj指针对象
			socket->SetLinkNetObj(pLinkObj);

			m_linkNetMap.insert(pair<UINT, shared_ptr<LinkNetObj>>(socket->GetSocketFD(), pLinkObj));
			return pLinkObj;
		}
	}
	return nullptr;
}

const shared_ptr<LinkNetObj> TcpThreadServer::GetLinkNetObj(const PER_SOCKET_CONTEXT* socket)
{
	AutoLock lock(m_linkNetMapLock);
	map<UINT, shared_ptr<LinkNetObj>>::iterator it = m_linkNetMap.find((UINT)socket->GetSocketFD());
	if (it != m_linkNetMap.end())
		return it->second;
	return nullptr;
}

shared_ptr<LinkNetObj> TcpThreadServer::CreateLinkNetObjBase(PER_SOCKET_CONTEXT* socket, UINT type)
{
	switch (type)
	{
	case SVR_LINK_TYPE_UNKNOWN:
		return make_shared<LinkNetObj>(socket, type);
	case SVR_LINK_TYPE_CLIENT:
		break;
	default:
		break;
	}
	return nullptr;
}

bool TcpThreadServer::RemoveLinkNetObj(SOCKET sock)
{
	AutoLock lock(m_linkNetMapLock);
	map<UINT, shared_ptr<LinkNetObj>>::iterator it = m_linkNetMap.find((UINT)sock);
	if (it != m_linkNetMap.end())
	{
		m_linkNetMap.erase(it);
		return true;
	}
	return false;
}

void TcpThreadServer::Run()
{
	StartServer();
}


