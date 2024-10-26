#pragma once

#include "Base.h"
#include "windows.h"
#include <iostream>
#include <string>
#include <sys/types.h>
//#include <sys/socket.h>
#include <winsock2.h>
#include <MSWSock.h>

//using namespace std;

#define BUF_SOCKDATA_LEN 1024*8
#define BUF_SOCKDATA_MAXLEN 1024*64

enum isOverLapped
{
	OverLapped_True,     //socket�Ƿ�����ɶ˿�
	OverLapped_False,
};

enum isNonBlock
{
	IsNonBlock_True,
	IsNonBlock_False,
};

class  TcpSocketBase
{
public:
	TcpSocketBase();
	~TcpSocketBase();
	//int getListenFd() { return _sock; }
public:
	//����� bind���׽���--> listen��ʼ����
	bool Socket(isOverLapped check = isOverLapped::OverLapped_False);
	bool Close();
	bool Bind(const std::string& ip, short port);
	bool Listen(int num, isNonBlock isblock = IsNonBlock_False);
	//�����׽����յ��µ����������󣬴����µ��׽���
	bool Accept(TcpSocketBase* peer, std::string* ip = nullptr, USHORT* port = nullptr, 
		isOverLapped check = isOverLapped::OverLapped_False, isNonBlock isblock = IsNonBlock_False) const;
	bool Recv(std::string* buf) const;
	bool Send(char* buf, int bufLen) const;
	//�ͻ��� connect
	bool Connect(std::string& ip, short port);
	SOCKET GetFd() const;
	int  SetSocketNonBlocking(SOCKET fd);
	void SetIP(std::string szIP);
	void SetPort(short port);
	std::string GetIP();
	short GetPort();
private:
	SOCKET m_sock;
	WSADATA m_wsa;
	std::string m_szIP;
	short m_port;
};