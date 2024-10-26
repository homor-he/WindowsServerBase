#pragma once

#include "Base.h"
#include "Log.h"
#include <winsock2.h>
#include <MSWSock.h>
//#include <vector>
#include "ThreadMutex.h"
#include <list>
#include <memory>
#include <atomic>
#include <string>
#include "ServerGlobalDefine.h"
#include "LinkNetObj.h"

using namespace std;

#define MAX_BUFFER_LEN   8 * 1024

typedef enum Operation_Type
{
	ACCEPT_POSTED,    //投递的accept操作，有新socket连上来
	SEND_POSTED,      //投递的send操作， 
	RECV_POSTED,	  //投递的recv操作
	CLOSE_POSTED,
	NULL_POSTED,     //用于初始化，无意义
}OPERATION_TYPE,*POPERATION_TYPE;

//每次套接字操作(如：AcceptEx, WSARecv, WSASend等)对应的数据结构：OVERLAPPED结构(标识本次操作)，关联的套接字，缓冲区，操作类型；
typedef struct PER_IO_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // 每一个重叠网络操作的重叠结构(针对每一个Socket的每一个操作，都要有一个)              
	SOCKET         m_sockAccept;                               // 这个网络操作所使用的Socket
	WSABUF         m_wsaBuf;                                   // WSA类型的缓冲区，用于给重叠操作传参数的
	char           m_szBuffer[MAX_BUFFER_LEN];                 // 这个是WSABUF里具体存字符的缓冲区
	OPERATION_TYPE m_OpType;                                   // 标识网络操作的类型(对应上面的枚举)
	DWORD		   m_nTotalBytes;							   // 数据总的字节数
	DWORD		   m_nSendBytes;							   // 已经发送的字节数，如未发送数据则设置为0

	PER_IO_CONTEXT()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		m_sockAccept = INVALID_SOCKET;
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		m_OpType = OPERATION_TYPE::NULL_POSTED;
		m_nTotalBytes = 0;
		m_nSendBytes = 0;
	}

	~PER_IO_CONTEXT()
	{
		/*if (m_sockAccept != INVALID_SOCKET)
		{
			closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}*/
	}

	//重置缓冲区内容
	void ResetBuffer()
	{
		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
	}
}*PPER_IO_CONTEXT;


//每个SOCKET对应的数据结构(调用GetQueuedCompletionStatus传入)：-
//SOCKET，该SOCKET对应的客户端地址，作用在该SOCKET操作集合(对应结构PER_IO_CONTEXT)；
class TcpThreadServer;
class SocketDefine;
class LinkNetObj;
typedef struct PER_SOCKET_CONTEXT
{
	friend SocketDefine;
	friend TcpThreadServer;
	
private:
	SOCKET m_Socket;						
	SOCKADDR_IN m_PeerAddr;
	SOCKADDR_IN m_LocalAddr;
	atomic_uint16_t m_Ref;
	list<shared_ptr<PER_IO_CONTEXT>> m_IOContextList; //套接字操作，本例是WSARecv和WSASend共用一个PER_IO_CONTEXT
	CThreadMutex m_IOContextListMutex;
	shared_ptr<LinkNetObj> m_pLinkNetObj;
public:
	PER_SOCKET_CONTEXT()
	{
		m_Socket = INVALID_SOCKET;
		ZeroMemory(&m_PeerAddr, sizeof(m_PeerAddr));
		ZeroMemory(&m_LocalAddr, sizeof(m_LocalAddr));
		m_Ref = 0;
		m_pLinkNetObj = nullptr;
	}

	~PER_SOCKET_CONTEXT()
	{
		if (m_Socket != INVALID_SOCKET)
		{
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
		}

		RemoveAllIOContext();
	}

	shared_ptr<PER_IO_CONTEXT> GetNewIOContext()
	{
		shared_ptr<PER_IO_CONTEXT> newIOContext = make_shared<PER_IO_CONTEXT>();
		AutoLock lock(m_IOContextListMutex);
		m_IOContextList.push_back(newIOContext);
		return newIOContext;
	}

	void RemoveIOContext(PER_IO_CONTEXT* pContext)
	{
		if (pContext == nullptr)
			return;
		AutoLock lock(m_IOContextListMutex);
		list<shared_ptr<PER_IO_CONTEXT>>::iterator it = m_IOContextList.begin();
		for (; it != m_IOContextList.end(); ++it)
		{
			if (pContext == (*it).get())
			{
				m_IOContextList.erase(it);
				break;
			}
		}
	}

	DWORD RemoveAllIOContext()
	{
		AutoLock lock(m_IOContextListMutex);
		DWORD resetCount = (DWORD)m_IOContextList.size();
		list<shared_ptr<PER_IO_CONTEXT>>().swap(m_IOContextList);
		return resetCount;
	}

	void AddRef()
	{
		m_Ref++;
	}

	atomic_uint16_t & Release()
	{
		m_Ref--;
		return m_Ref;
	}


	void PostSend(char* buffer, int len)
	{
		shared_ptr<PER_IO_CONTEXT> pIOContext = GetNewIOContext();
		pIOContext->ResetBuffer();
		pIOContext->m_OpType = SEND_POSTED;
		pIOContext->m_sockAccept = this->m_Socket;
		pIOContext->m_nSendBytes = 0;
		pIOContext->m_nTotalBytes = len;
		memcpy_s(pIOContext->m_szBuffer, len, buffer, len);

		DWORD dwSendBytes = 0;
		DWORD dwFlags = 0;

		int ret = WSASend(pIOContext->m_sockAccept, &pIOContext->m_wsaBuf, 1, &dwSendBytes, dwFlags, &pIOContext->m_Overlapped, NULL);
		long lastError = WSAGetLastError();
		if (ret == SOCKET_ERROR)
		{
			if ((lastError != WSA_IO_PENDING) && (lastError != WSAEWOULDBLOCK))
			{
				WriteLog("%s:%d, socket:%d SEND_POSTED fail, lastError:%d", __FILE__, __LINE__, pIOContext->m_sockAccept, lastError);
				return;
			}
			return;
		}
		else
		{
			if (dwSendBytes == 0)   //发送0个字节，表示对方关闭连接
			{
				RemoveIOContext(pIOContext.get());
			}
		}
	}

	void SetSocketFD(SOCKET socketFD)
	{
		m_Socket = socketFD;
	}

	SOCKET GetSocketFD() const
	{
		return m_Socket;
	}

	char* GetPeerIP() const
	{
		return inet_ntoa(m_PeerAddr.sin_addr);
	}

	USHORT GetPeerPort() const
	{
		return ntohs(m_PeerAddr.sin_port);
	}

	string GetPeerIPPort() const
	{
		char hostAddr[LEN_HOSTADDR] = { 0 };
		sprintf_s(hostAddr, "%s:%d", GetPeerIP(), GetPeerPort());
		return move(string(hostAddr));
	}

	char* GetLocalIP() const
	{
		return inet_ntoa(m_LocalAddr.sin_addr);
	}

	USHORT GetLocalPort() const
	{
		return ntohs(m_LocalAddr.sin_port);
	}

	string GetLocalIPPort() const
	{
		char hostAddr[LEN_HOSTADDR] = { 0 };
		sprintf_s(hostAddr, "%s:%d", GetLocalIP(), GetLocalPort());
		return move(string(hostAddr));
	}

	void SetLinkNetObj(shared_ptr<LinkNetObj> pObj)
	{
		m_pLinkNetObj = pObj;
	}

	bool OnMsg(share_buff buff)
	{
		if (m_pLinkNetObj)
		{
			m_pLinkNetObj->OnMsg(this, buff);
			return true;
		}
		else
			return false;
	}
}*PPER_SOCKET_CONTEXT;
