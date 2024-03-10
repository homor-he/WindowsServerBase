#pragma once

#include "TcpSocketBase.h"
#include "Test_Timer.h"
#include "ServerGlobalDefine.h"
#include "ProtoGobalDefine.h"
#include "ConnectionPool.h"

void ServerProcess()
{
	TcpSocketBase tcpSocket;
	TcpSocketBase acceptSocket;
	string acceptIP = "";
	USHORT acceptPort = 0;
	WriteLog("Server init");
	if (tcpSocket.Socket(isOverLapped::OverLapped_False))
	{
		string szIp = "127.0.0.1";
		short port = 30301;
		if (tcpSocket.Bind(szIp, port))
		{
			if (tcpSocket.Listen(SOMAXCONN, isNonBlock::IsNonBlock_True))
			{
				while (true)
				{
					tcpSocket.Accept(&acceptSocket, &acceptIP, &acceptPort, isOverLapped::OverLapped_False, isNonBlock::IsNonBlock_True);
					string buf;
					if (acceptSocket.Recv(&buf))
					{
						WriteLog("server recv buf cont:%s", buf.c_str());
					}
				}
			}
		}
	}
}

void ClientProcess()
{
	TcpSocketBase tcpSocket;
	string szIP = "127.0.0.1";
	short port = 30301;
	WriteLog("Client init");
	if (tcpSocket.Socket(isOverLapped::OverLapped_False))
	{
		if (tcpSocket.Connect(szIP, port))
		{
#ifdef PROTOBUF
			//初次连接时发送建立连接包
			{
				rp::CmnBuf_MsgHead* pHead = new rp::CmnBuf_MsgHead;
				pHead->set_msgtype(PROTO_BUILD_CONN_REQ);
				pHead->set_origin(SVR_LINK_TYPE_UNKNOWN);
				
				rp::CmnBuf cmnBuf;
				cmnBuf.set_allocated_msgheader(pHead);
				cmnBuf.set_allocated_content(nullptr);
				
				string cont = cmnBuf.SerializeAsString();
				tcpSocket.Send((char*)cont.c_str(), cont.size());
				Sleep(1000);
			}
#endif // PROTOBUF

			string cont;
#ifdef PROTOBUF
			rp::ContentTest sContext;
			sContext.set_testdouble(1.0);
			sContext.set_testfloat(2.0f);
			sContext.set_testint32(3);
			sContext.set_testbool(true);
			
			rp::ContentTest::TestUser* pUser = nullptr;
			for (int i = 0; i < 2; ++i)
			{
				pUser = sContext.add_testuserlist();
				string szAccount = "TestUser" + to_string(i);
				pUser->set_testaccount(szAccount);
				pUser->set_testpwd(to_string(i<<i));
				pUser->set_testtype(rp::ContentTest::TestType::ContentTest_TestType_PC);
			}
			string contentStr = sContext.SerializeAsString();
			//测试协议
			rp::CmnBuf_MsgHead* pHead = new rp::CmnBuf_MsgHead;
			pHead->set_msgtype(PROTO_TEST_REQ);

			rp::CmnBuf cmnBuf;
			cmnBuf.set_allocated_msgheader(pHead);
			cmnBuf.set_content(contentStr);

			//cont.resize(cmnBuf.ByteSizeLong());
			//char* data = (char*)cont.c_str();
			//cmnBuf.SerializeToArray(data,cmnBuf.ByteSizeLong());
			cont = cmnBuf.SerializeAsString();
#endif // PROTOBUF


			int times = 0;
			while (true)
			{
				times++;
				if (times > 1)
				{
					Sleep(5000);
					break;
				}
					
				if (tcpSocket.Send((char*)cont.c_str(), cmnBuf.ByteSizeLong()))
				{
					//WriteLog("client send buf cont:%s", cont.c_str());
					string buf;

					while (true)
					{
						if (tcpSocket.Recv(&buf))
						{
							rp::CmnBuf recvBuf;
							recvBuf.ParseFromArray(buf.c_str(), buf.size());

							rp::CmnBuf_MsgHead recvHead = recvBuf.msgheader();

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
								string account = user.testaccount();
								string pwd = user.testpwd();
								rp::ContentTest::TestType testType = user.testtype();
							}
							break;
						}
					}
				}
				Sleep(20);
			}
			tcpSocket.Close();
		}
	}
}

void TestClientServer()
{
	TimerMgr timerMgr;
	timerMgr.AddTimer(1, ServerProcess, 1000, TimerType::Once);
	timerMgr.AddTimer(2, ClientProcess, 10000, TimerType::Once);

	while (true)
	{
		Sleep(1000);
	}
}

void ClientIocpProcess()
{
	//TcpClient tcpClient("127.0.0.1", 30301);
	SingleConnection tcpClient("127.0.0.1", 30301,"", SVR_LINK_TYPE_UNKNOWN);
	if (!tcpClient.Connect())
		return;

	////初次建立连接发送的包
	//rp::CmnBuf_MsgHead* pHead = new rp::CmnBuf_MsgHead;
	//pHead->set_msgtype(PROTO_BUILD_CONN_REQ);
	//pHead->set_origin(SVR_LINK_TYPE_UNKNOWN);

	//rp::CmnBuf cmnBuf;
	//cmnBuf.set_allocated_msgheader(pHead);
	//cmnBuf.set_allocated_content(nullptr);

	//string cont = cmnBuf.SerializeAsString();
	//tcpClient.SendMsgAsync(cont);
	Sleep(1000);

	while (true)
	{
		string cont;
#ifdef PROTOBUF
		rp::ContentTest sContext;
		sContext.set_testdouble(1.0);
		sContext.set_testfloat(2.0f);
		sContext.set_testint32(3);
		sContext.set_testbool(true);

		rp::ContentTest::TestUser* pUser = nullptr;
		for (int i = 0; i < 2; ++i)
		{
			pUser = sContext.add_testuserlist();
			string szAccount = "TestUser" + to_string(i);
			pUser->set_testaccount(szAccount);
			pUser->set_testpwd(to_string(i << i));
			pUser->set_testtype(rp::ContentTest::TestType::ContentTest_TestType_PC);
		}
		string contentStr = sContext.SerializeAsString();
		//测试协议
		rp::CmnBuf_MsgHead* pHead = new rp::CmnBuf_MsgHead;
		pHead->set_msgtype(PROTO_TEST_REQ);

		rp::CmnBuf cmnBuf;
		cmnBuf.set_allocated_msgheader(pHead);
		cmnBuf.set_content(contentStr);

		//cont.resize(cmnBuf.ByteSizeLong());
		//char* data = (char*)cont.c_str();
		//cmnBuf.SerializeToArray(data,cmnBuf.ByteSizeLong());
		cont = cmnBuf.SerializeAsString();
#endif // PROTOBUF
		tcpClient.SendMsgAsync(cont);

		Sleep(100000);
	}
}

