#include "SocketHandler.h"
#include "Log.h"
#include "TcpThreadServer.h"

SocketHandler::SocketHandler():
	m_quit(false),
	m_hIPCompletionPort(nullptr),
	m_server(nullptr)
{
}

SocketHandler::~SocketHandler()
{
}

void SocketHandler::Execute(void* para)
{
	DWORD dwByteTransferred = 0;
	OVERLAPPED* pOverlapped = nullptr;
	PER_SOCKET_CONTEXT* pSocketContext = nullptr;
	DWORD dwFlag;
	while (!m_quit)
	{
		if (m_hIPCompletionPort == nullptr)
			continue;
		bool ret = GetQueuedCompletionStatus(m_hIPCompletionPort, &dwByteTransferred, (PULONG_PTR)&pSocketContext, &pOverlapped, INFINITE);
	
		//ʧ��ʱ=0 �ɹ�ʱ��0
		if (ret == 0)
		{
			//*pOverLappedΪ�ղ��Һ���û�д���ɶ˿�ȡ����ɰ������
			if (pOverlapped == nullptr)
			{
				WriteLog("%s:%d, pOverlapped is null, error:%d", __FILE__,__LINE__,GetLastError());
				continue;
			}

			if (pSocketContext == nullptr)
			{
				WriteLog("%s:%d, pSocketContext is null",__FILE__,__LINE__);
				continue;
			}
			//Ҫ�õ���ȷ��errorcode����Ҫ�ȵ���WSAGetOverlappedResult
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
			pSocketContext->AddRef();
			//��ȡ����Ĳ���
			PER_IO_CONTEXT* pIOContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped);
			//�жϿͻ����Ƿ�Ͽ���
			if ((dwByteTransferred == 0) && (pIOContext->m_OpType == RECV_POSTED || pIOContext->m_OpType == SEND_POSTED))
			{
				if (pSocketContext->Release() == 1)
				{
					RemoveSocketContext(pSocketContext);
					WriteLog("client socketid:%d ip:%s port:%d closed", pSocketContext->GetSocketFD(), pSocketContext->GetPeerIP(),
						pSocketContext->GetPeerPort());
					continue;
				}
			}
			else
			{
				switch (pIOContext->m_OpType)
				{
				case RECV_POSTED:
				{
					m_server->OnMsg(pSocketContext, pIOContext->m_wsaBuf.buf, dwByteTransferred);
					RePostRecv(pIOContext, pSocketContext);
					break;
				}
				case SEND_POSTED:
				{
					pIOContext->m_nSendBytes += dwByteTransferred;
					if (pIOContext->m_nSendBytes < pIOContext->m_nTotalBytes)
					{
						//����û�����棬��������
						pIOContext->m_wsaBuf.buf = pIOContext->m_szBuffer + pIOContext->m_nSendBytes;
						pIOContext->m_wsaBuf.len = pIOContext->m_nTotalBytes - pIOContext->m_nSendBytes;
						PostSend(pIOContext, pSocketContext);
					}
					else
					{
						//�Ƴ�����io
						pSocketContext->RemoveIOContext(pIOContext);
					}
					break;
				}
				default:
					WriteLog("%d, unknown opType:%d", __LINE__, pIOContext->m_OpType);
					break;
				}
			}
			pSocketContext->Release();
		}
	}
}

void SocketHandler::SetIOCP(HANDLE iocp)
{
	m_hIPCompletionPort = iocp;
}

void SocketHandler::SetTcpThreadServer(TcpThreadServer* server)
{
	m_server = server;
}

bool SocketHandler::HandleError(PER_SOCKET_CONTEXT* pContext, DWORD dwErr)
{
	pContext->AddRef();
	if (dwErr == WAIT_TIMEOUT)
	{
		//ȷ�Ͽͻ����Ƿ���
		if (!IsSocketAlive(pContext->GetSocketFD()))
		{
			WriteLog("%d, client abnormal close, sockid:%d", __LINE__, pContext->GetSocketFD());
			if(pContext->Release() == 1)
				RemoveSocketContext(pContext);
			return true;
		}
		else
		{
			WriteLog("%d, net work operate timeout, sockid:%d", __LINE__, pContext->GetSocketFD());
			pContext->Release();
			return true;
		}
	}
	else
	{
		WriteLog("%d, close client socketid:%d, errcode:%d", __LINE__, pContext->GetSocketFD(), dwErr);
		if (pContext->Release() == 1)
			RemoveSocketContext(pContext);
		return true;
	}
}

bool SocketHandler::IsSocketAlive(SOCKET sock)
{
	for (int i = 0; i < 2; ++i)
	{
		int sendBytes = send(sock, "1", 1, 0);
		if (sendBytes <= 0)
			return false;
	}
	return true;
}

void SocketHandler::RemoveSocketContext(PER_SOCKET_CONTEXT* pContext)
{
	if (!m_server->DisConnected(pContext))
		m_server->DeleteSocketContext(pContext->GetSocketFD());
}

void SocketHandler::Quit()
{
	m_quit = true;
}

void SocketHandler::RePostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext)
{
	pSocketContext->AddRef();
	if (!PostRecv(pIOContext, pSocketContext))
	{
		if (!IsSocketAlive(pSocketContext->GetSocketFD()))
		{
			if (pSocketContext->Release() == 1)
			{
				WriteLog("%d, client abnormal close, sockid:%d", __LINE__, pSocketContext->GetSocketFD());
				RemoveSocketContext(pSocketContext);
				return;
			}
		}
		else
		{
			WriteLog("%d, net work operate timeout, sockid:%d", __LINE__, pSocketContext->GetSocketFD());
		}
	}
	pSocketContext->Release();
}

bool SocketHandler::PostRecv(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext)
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
		//ret == 0, ���ǻ�����û���ݣ������ͻ��˶Ͽ���
		if (strcmp(pIOContext->m_wsaBuf.buf,"")==0)
		{
			return false;
		}
	}
	return true;
}

bool SocketHandler::PostSend(PER_IO_CONTEXT* pIOContext, PER_SOCKET_CONTEXT* pSocketContext)
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
		if (dwSendBytes == 0)   //����0���ֽڣ���ʾ�Է��ر�����
		{
			pSocketContext->RemoveIOContext(pIOContext);
			return false;
		}
	}
	return true;
}

