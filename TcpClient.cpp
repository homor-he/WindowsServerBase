#include "TcpClient.h"

TcpClientIocpTask::TcpClientIocpTask(TcpClient* pTcpClient) : ThreadTask(),
	m_hIPCompletionPort(nullptr),
	m_pTcpClient(pTcpClient),
	m_cltContext(pTcpClient->m_cltContext)
{
}

TcpClientIocpTask::~TcpClientIocpTask()
{
}

void TcpClientIocpTask::SetIocp(HANDLE iocp)
{
	m_hIPCompletionPort = iocp;
}

bool TcpClientIocpTask::HandleError(PER_SOCKET_CONTEXT* pContext, DWORD dwErr)
{
	if (dwErr == WAIT_TIMEOUT)
	{
		//确认客户端是否断开连接
		if (!IsSocketAlive(pContext->GetSocketFD()))
		{
			WriteLog("%d, client abnormal close, sockid:%d", __LINE__, pContext->GetSocketFD());
			/*if (pContext->Release() == 1)
				RemoveSocketContext();*/
			//客户端尝试重连
			Reconnect();
		}
		else
		{
			WriteLog("%d, net work operate timeout, sockid:%d", __LINE__, pContext->GetSocketFD());
		}
		return true;
	}
	else
	{
		WriteLog("%d, close client socketid:%d, errcode:%d", __LINE__, pContext->GetSocketFD(), dwErr);
		/*if (pContext->Release() == 1)
			RemoveSocketContext();*/
		Reconnect();
		return true;
	}
	return true;
}

bool TcpClientIocpTask::IsSocketAlive(SOCKET sock)
{
	for (int i = 0; i < 2; ++i)
	{
		int sendBytes = send(sock, "1", 1, 0);
		if (sendBytes <= 0)
			return false;
	}
	return true;
}

void TcpClientIocpTask::RemoveSocketContext()
{
	m_pTcpClient->CloseSocket();
	m_pTcpClient->CloseIocpThread();
}

void TcpClientIocpTask::Reconnect()
{
	if( m_pTcpClient && m_pTcpClient->GetCloseStat() == false)
		m_pTcpClient->Reconnect();
}

bool TcpClientIocpTask::PostSend(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext)
{
	DWORD dwSendBytes = 0;
	DWORD dwFlags = 0;

	int ret = WSASend(pIOContext->m_sockAccept, &pIOContext->m_wsaBuf, 1, &dwSendBytes, dwFlags, &pIOContext->m_Overlapped, NULL);
	long lastError = WSAGetLastError();
	if (ret == SOCKET_ERROR)
	{
		if ((lastError != WSA_IO_PENDING) && (lastError != WSAEWOULDBLOCK))
		{
			WriteLog("%s:%d, socket:%d SEND_POSTED fail, lastError:%d", __FILE__, __LINE__, pIOContext->m_sockAccept, WSAGetLastError());
			return false;
		}
	}
	else
	{
		if (dwSendBytes == 0)   //发送0个字节，表示对方关闭连接
		{
			pSocketContext->RemoveIOContext(pIOContext);
			return false;
		}
	}
	return true;
}

bool TcpClientIocpTask::PostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext)
{
	pIOContext->ResetBuffer();
	pIOContext->m_OpType = RECV_POSTED;
	pIOContext->m_sockAccept = pSocketContext->GetSocketFD();
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
			WriteLog("%s:%d, socket:%d RECV_POSTED fail, lastError:%d", __FILE__, __LINE__, pIOContext->m_sockAccept, lastError);
			return false;
		}
	}
	else
	{
		//ret == 0, 但是缓冲区没数据，表明断开了
		if (strcmp(pIOContext->m_wsaBuf.buf, "") == 0)
		{
			return false;
		}
	}
	return true;
}

void TcpClientIocpTask::RePostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext)
{

	if (!PostRecv(pIOContext, pSocketContext))
	{
		if (!IsSocketAlive(pSocketContext->GetSocketFD()))
		{
			WriteLog("%d, server abnormal close, sockid:%d", __LINE__, pSocketContext->GetSocketFD());
			Reconnect();
			return;
		}
		else
		{
			WriteLog("%d, net work operate timeout, sockid:%d", __LINE__, pSocketContext->GetSocketFD());
		}
	}
}


void TcpClientIocpTask::Execute(void* para)
{
	//m_quit.store(false);

	DWORD dwByteTransferred = 0;
	OVERLAPPED* pOverlapped = nullptr;
	PER_SOCKET_CONTEXT* pSocketContext = nullptr;
	DWORD dwFlag;
	while (!m_quit.load())
	{
		if (m_hIPCompletionPort == nullptr)
			continue;
		bool ret = GetQueuedCompletionStatus(m_hIPCompletionPort, &dwByteTransferred, (PULONG_PTR)&pSocketContext, &pOverlapped, INFINITE);
		/*if (pSocketContext == nullptr)
		{
			WriteLog("%s:%d, pSocketContext is null",__FILE__,__LINE__);
			break;
		}*/
		//失败时=0 成功时非0
		if (ret == 0)
		{
			//*pOverLapped为空并且函数没有从完成端口取出完成包的情况
			if (pOverlapped == nullptr)
			{
				WriteLog("%s:%d, pOverlapped is null, error:%d", __FILE__, __LINE__, GetLastError());
				continue;
			}

			if (pSocketContext == nullptr)
			{
				WriteLog("%s:%d, pSocketContext is null", __FILE__, __LINE__);
				continue;
			}

			//要得到正确的errorcode，需要先调用WSAGetOverlappedResult
			WSAGetOverlappedResult(pSocketContext->GetSocketFD(), pOverlapped, &dwByteTransferred, false, &dwFlag);

			DWORD dwErr = GetLastError();
			HandleError(pSocketContext, dwErr);
			continue;
		}
		else
		{
			if (pSocketContext == nullptr)
			{
				WriteLog("%s:%d, pSocketContext is null", __FILE__, __LINE__);
				continue;
			}
			//读取传入的参数
			PER_IO_CONTEXT* pIOContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped);
			//判断客户端是否断开了
			if ((dwByteTransferred == 0) && (pIOContext->m_OpType == RECV_POSTED || pIOContext->m_OpType == SEND_POSTED))
			{
				WriteLog("client socketid:%d ip:%s port:%d closed", pSocketContext->GetSocketFD(), pSocketContext->GetPeerIP(),
					pSocketContext->GetPeerPort());
				Reconnect();
				continue;
			}
			else
			{
				switch (pIOContext->m_OpType)
				{
				case RECV_POSTED:
				{ 
					m_pTcpClient->OnMsg(pIOContext->m_wsaBuf.buf, dwByteTransferred);
					RePostRecv(pIOContext, pSocketContext);
					break;
				}
				case SEND_POSTED:
				{
					pIOContext->m_nSendBytes += dwByteTransferred;
					if (pIOContext->m_nSendBytes < pIOContext->m_nTotalBytes)
					{
						//数据没发送玩，继续发送
						pIOContext->m_wsaBuf.buf = pIOContext->m_szBuffer + pIOContext->m_nSendBytes;
						pIOContext->m_wsaBuf.len = pIOContext->m_nTotalBytes - pIOContext->m_nSendBytes;
						PostSend(pIOContext, pSocketContext);
					}
					else
					{
						//移除发送io
						pSocketContext->RemoveIOContext(pIOContext);
					}
					break;
				}
				default:
					WriteLog("%d, unknown opType:%d", __LINE__, pIOContext->m_OpType);
					break;
				}
			}
		}
	}
}

/***************************************************************************/

TcpClient::TcpClient():
	m_cltContext(nullptr),
	m_IOCompletionPort(nullptr),
	m_szIP(""),
	m_wPort(0),
	m_iocpThread("",1,nullptr,false),
	m_pIocpThreadTask(nullptr), 
	m_reconnectTimes(MAX_RECONNECT_TIMES),
	m_bReconnectStarted(false)
{
}

TcpClient::TcpClient(const std::string& szIP, short wPort):
	m_cltContext(nullptr),
	m_IOCompletionPort(nullptr),
	m_szIP(szIP),
	m_wPort(wPort),
	m_iocpThread("", 1, nullptr,false),
	m_pIocpThreadTask(nullptr),
	m_reconnectTimes(MAX_RECONNECT_TIMES),
	m_bReconnectStarted(false),
	m_closed(false)
{
}

TcpClient::~TcpClient()
{
	if(!m_closed)
		CloseAll();
}

bool TcpClient::Connect()
{
	m_IOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	m_clientSock.Socket(isOverLapped::OverLapped_True);
	if (m_clientSock.Connect(m_szIP, m_wPort))
	{
		m_cltContext = new PER_SOCKET_CONTEXT;
		m_cltContext->SetSocketFD(m_clientSock.GetFd());
		if (NULL == CreateIoCompletionPort((HANDLE)m_cltContext->GetSocketFD(), m_IOCompletionPort, (ULONG_PTR)m_cltContext, 0))
		{
			WriteLog("%s:%d, create completionport fail,lastError:%d", __FILE__, __LINE__, GetLastError());
			return false;
		}

		m_pIocpThreadTask = new TcpClientIocpTask(this);
		m_pIocpThreadTask->SetIocp(m_IOCompletionPort);
		m_pIocpThreadTask->SetNotQuit();
		m_iocpThread.SetTask(m_pIocpThreadTask);
		m_iocpThread.Start();

		/*shared_ptr<PER_IO_CONTEXT> pIoContext = m_cltContext->GetNewIOContext();
		if (pIoContext)
		{
			if (m_pIocpThreadTask->PostRecv(pIoContext.get(), m_cltContext) == false)
			{
				m_cltContext->RemoveIOContext(pIoContext.get());
				return false;
			}
		}
		else
		{
			WriteLog("%s:%d, pNewIOContext is null", __FILE__, __LINE__);
			return false;
		}*/
		return SetRecvIO();
	}
	return false;
}

void TcpClient::CloseIocpThread()
{
	PostQueuedCompletionStatus(m_IOCompletionPort, 0, (DWORD)nullptr, nullptr);
	CloseHandle(m_IOCompletionPort);

	m_iocpThread.Quit();
}

void TcpClient::CloseSocket()
{
	m_clientSock.Close();
}

void TcpClient::SetCloseStat()
{
	m_closed = true;
}

void TcpClient::CloseAll()
{
	if (m_closed)
		return;
	SetCloseStat();

	CloseIocpThread();
	CloseSocket();

	if (m_cltContext)
	{
		delete m_cltContext;
		m_cltContext = nullptr;
	}
	
	if (m_pIocpThreadTask)
	{
		delete m_pIocpThreadTask;
		m_pIocpThreadTask = nullptr;
	}
}

bool TcpClient::GetCloseStat()
{
	return m_closed;
}

bool TcpClient::Reconnect()
{
	if (m_bReconnectStarted.load())
		return false;
	m_bReconnectStarted.store(true);

	CloseSocket();
	m_clientSock.Socket(isOverLapped::OverLapped_True);
	do
	{
		if (m_clientSock.Connect(m_szIP, m_wPort))
		{
			m_cltContext->SetSocketFD(m_clientSock.GetFd());
			if (NULL == CreateIoCompletionPort((HANDLE)m_cltContext->GetSocketFD(), m_IOCompletionPort, (ULONG_PTR)m_cltContext, 0))
			{
				WriteLog("%s:%d, create completionport fail,lastError:%d", __FILE__, __LINE__, GetLastError());
				return false;
			}

			WriteLog("reconnect to ip:%s port:%d success", m_szIP.c_str(), m_wPort);
			m_reconnectTimes = MAX_RECONNECT_TIMES;
			m_bReconnectStarted.store(false);
			return SetRecvIO();
		}
		else
			m_reconnectTimes--;

		Sleep(1000);
	} while (m_reconnectTimes > 0);

	WriteLog("reconnect to ip:%s port:%d fail", m_szIP.c_str(), m_wPort);
	CloseSocket();
	CloseIocpThread();
	m_bReconnectStarted.store(false);
	return false;
}

void TcpClient::SendMsgAsync(std::string& sendMsg)
{
	if (m_pIocpThreadTask == nullptr)
	{
		WriteLog("<%s %d>, iocp thread task is null", __FUNCTION__, __LINE__);
		return;
	}
		

	if (!m_pIocpThreadTask->GetQuitStat() && m_cltContext)
		m_cltContext->PostSend((char*)sendMsg.c_str(), sendMsg.size());
}

void TcpClient::OnMsg(char* buf, int len)
{
#ifdef PROTOBUF
	rp::CmnBuf recvBuf;
	recvBuf.ParseFromArray(buf, len);
	const rp::CmnBuf_MsgHead& recvHead = recvBuf.msgheader();
	switch (recvHead.msgtype())
	{
	case PROTO_TEST_REQ:
	{
		rp::ContentTest contextTest;
		contextTest.ParseFromString(recvBuf.content());

		double d = contextTest.testdouble();
		float f = contextTest.testfloat();
		int int32 = contextTest.testint32();
		/*int sint32 = contextTest.testsint32();
		int uint32 = contextTest.testuint32();
		INT64 int64 = contextTest.testint64();
		INT64 sint64 = contextTest.testsin64();
		UINT64 uint64 = contextTest.testuint64();
		int fixed32 = contextTest.testfixed32();
		INT64 fixed64 = contextTest.testfixed64();
		int sfixed32 = contextTest.testsfixed32();
		INT64 sfixed64 = contextTest.testsfixed64();*/
		bool b = contextTest.testbool();
		/*string str = contextTest.teststring();
		string bytes = contextTest.testbytes();*/

		int size = contextTest.testuserlist_size();
		auto userList = contextTest.testuserlist();
		for (int i = 0; i < size; ++i)
		{
			rp::ContentTest::TestUser user = userList.Get(i);
			std::string account = user.testaccount();
			std::string pwd = user.testpwd();
			rp::ContentTest::TestType testType = user.testtype();
		}
	}
		break;
	default:
		break;
	}
#endif
}

bool TcpClient::SetRecvIO()
{
	std::shared_ptr<PER_IO_CONTEXT> pIoContext = m_cltContext->GetNewIOContext();
	if (pIoContext)
	{
		if (m_pIocpThreadTask->PostRecv(pIoContext.get(), m_cltContext) == false)
		{
			m_cltContext->RemoveIOContext(pIoContext.get());
			return false;
		}
	}
	else
	{
		WriteLog("%s:%d, pNewIOContext is null", __FILE__, __LINE__);
		return false;
	}
	return true;
}


