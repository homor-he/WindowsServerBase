#include "TcpSocketBase.h"
#include "Log.h"

TcpSocketBase::TcpSocketBase() : m_sock(INVALID_SOCKET), m_szIP("0.0.0.0"),m_port(0)
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
	{
		return;
	}
}

TcpSocketBase::~TcpSocketBase()
{
	WSACleanup();
}

bool TcpSocketBase::Socket(isOverLapped check)
{
	if (check == isOverLapped::OverLapped_True)
	{
		m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	else
	{
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
	}

	int alive = 1;
	if (setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&alive, sizeof(alive)) != 0)
	{
		WriteLog("Set keep alive error:%s", strerror(errno));
		return false;
	}

	struct linger so_linger;
	so_linger.l_onoff = true;
	so_linger.l_linger = 30;

	if (setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger)) != 0)
	{
		WriteLog("Set linger error:%s", strerror(errno));
		return false;
	}

	if (m_sock < 0)
	{
		WriteLog("<%s %d>,create socket：%d fail", __FUNCTION__, __LINE__, GetFd());
		return false;
	}
	return true;
}

bool TcpSocketBase::Close()
{
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
	return true;
}

bool TcpSocketBase::Bind(const std::string& ip, short port)
{
	SetIP(ip);
	SetPort(port);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	int ret = bind(m_sock, (sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		WriteLog("<%s %d>, ip:%s port:%d, bind fail",__FUNCTION__,__LINE__,ip.c_str(),port);
		return false;
	}
	return true;
}

//num：缓冲区的长度，能放多少个客户端请求
bool TcpSocketBase::Listen(int num, isNonBlock isblock)
{
	int ret = listen(m_sock, num);
	if (ret < 0)
	{
		WriteLog("<%s %d>, ip:%s port:%d, listen fail", __FUNCTION__, __LINE__, m_szIP.c_str(), m_port);
		return false;
	}

	if (isblock == isNonBlock::IsNonBlock_True)
	{
		if (SetSocketNonBlocking(m_sock) == SOCKET_ERROR)
		{
			WriteLog("<%s %d>,Listen SetSocketNonBlocking fail", __FUNCTION__, __LINE__);
			return false;
		}
	}
	return true;
}

bool TcpSocketBase::Accept(TcpSocketBase* peer, std::string* ip, USHORT* port, isOverLapped check, isNonBlock isblock) const
{
	sockaddr_in client_addr;
	int addrLen = sizeof(client_addr);
	SOCKET clientSocket = INVALID_SOCKET;
	if(check == isOverLapped::OverLapped_True)
		clientSocket = WSAAccept(m_sock, (sockaddr*)&client_addr, &addrLen, NULL, NULL);
	else
		clientSocket = accept(m_sock, (sockaddr*)&client_addr, &addrLen);
	if (INVALID_SOCKET == clientSocket)
	{
		//WriteLog("%s:%d, accept fail, listen ip:%s port:%d", __FILE__, __LINE__, m_szIP.c_str(), m_port);
		return false;
	}
	peer->m_sock = clientSocket;
	if (NULL != ip)
	{
		*ip = inet_ntoa(client_addr.sin_addr);
		peer->SetIP(*ip);
	}
	if (NULL != port)
	{
		//不同的机器主机字节序不同，linux和windows上一般要用以下函数进行转换
		//ntohs将网络字节序（网络统一字节顺序，避免兼容性问题）转换为主机字节序 htons主机字节序转换为网络字节序
		*port = ntohs(client_addr.sin_port);
		peer->SetPort(*port);
	}
	if(NULL != ip && NULL != port)
		WriteLog("accept success, listen ip:%s port:%d clientIP:%s port:%d", m_szIP.c_str(), m_port,ip->c_str(),*port);
	return true;
}

bool TcpSocketBase::Recv(std::string* buf) const
{
	buf->clear();

	int read_bytes = 0;
	char tmp[BUF_SOCKDATA_LEN+1] = { 0 };
	while (true)
	{
		int ret = recv(m_sock, tmp, BUF_SOCKDATA_LEN, 0);
		
		if (ret < 0)
		{
			//WriteLog("%s:%d, recv fail, ip:%s port:%d errno = %d", __FILE__, __LINE__, m_szIP.c_str(), m_port, errno);
			WriteLog("errno:%d", errno);
			return false;
		}

		if (ret == 0)
		{
			//WriteLog("%s:%d, recv fail, ip:%s port:%d socket is closed", __FILE__, __LINE__, m_szIP.c_str(), m_port);
			return false;
		}

		if (ret == BUF_SOCKDATA_LEN)
		{
			if (tmp[BUF_SOCKDATA_LEN] != '\0')
			{
				if (buf->size() > BUF_SOCKDATA_MAXLEN)
				{
					WriteLog("<%s %d>, buf is too long", __FUNCTION__, __LINE__);
					return false;
				}
				buf->append(tmp, BUF_SOCKDATA_LEN);
				continue;
			}
			else
			{
				buf->append(tmp, ret);
				break;
			}
		}
		else if(ret > BUF_SOCKDATA_LEN)
		{
			WriteLog("<%s %d>, recv ret len error, ret:%d", __FUNCTION__, __LINE__, ret);
			return false;
		}
		else
		{
			if (tmp[ret] == '\0')
			{
				buf->append(tmp, ret);
				break;
			}
		}
	}
	return true;
}

bool TcpSocketBase::Send(char* buf, int bufLen) const
{
	int send_bytes = 0;
	int ret = 0;
	while (true)
	{
		ret = send(m_sock, buf+send_bytes, bufLen- send_bytes, 0);
		if (ret < 0)
		{
			if (errno == WSAEWOULDBLOCK)
			{
				WriteLog("<%s %d>, Send fail, ip:%s port:%d errno = WSAEWOULDBLOCK", __FUNCTION__, __LINE__, m_szIP.c_str(), m_port);
				break;
			}
			else if (errno == EINTR)
			{
				continue;
			}
			else
			{
				WriteLog("<%s %d>, Send fail, ip:%s port:%d errno = %d", __FUNCTION__, __LINE__, m_szIP.c_str(), m_port, errno);
				return false;
			}
		}
		else if (ret == 0)
		{
			WriteLog("<%s %d>, Send fail, ip:%s port:%d socket is closed", __FUNCTION__, __LINE__, m_szIP.c_str(), m_port);
			return false;
		}
		send_bytes += ret;
		if (send_bytes == bufLen)
			break;
	}
	return true;
}

bool TcpSocketBase::Connect(std::string& ip, short port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);
	int ret = connect(m_sock, (sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		WriteLog("<%s %d>, socket connect to ip:%s port:%d fail", __FUNCTION__, __LINE__, ip.c_str(), port);
		return false;
	}
	WriteLog("socket connect to ip:%s port:%d success", ip.c_str(), port);
	SetIP(ip);
	SetPort(port);
	return true;
}

SOCKET TcpSocketBase::GetFd() const
{
	return m_sock;
}

int TcpSocketBase::SetSocketNonBlocking(SOCKET fd)
{
	int ul = 1;
	return ioctlsocket(m_sock, FIONBIO, (unsigned long*)&ul);
}

void TcpSocketBase::SetIP(std::string szIP)
{
	this->m_szIP = szIP;
}

void TcpSocketBase::SetPort(short port)
{
	this->m_port = port;
}

std::string TcpSocketBase::GetIP()
{
	return m_szIP;
}

short TcpSocketBase::GetPort()
{
	return m_port;
}
