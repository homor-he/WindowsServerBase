#pragma once

#include "ConnectionPool.h"

class RecvMsgHandler : public ConnectionMsgHandler
{
public:
	RecvMsgHandler();
	virtual ~RecvMsgHandler();

	void OnConnectedCallBack(const SingleConnection& connection);
	void OnRecvMsgAsync(const SingleConnection& connection, share_buff buff);
};

inline RecvMsgHandler::RecvMsgHandler() {}
inline RecvMsgHandler::~RecvMsgHandler() {}

inline void RecvMsgHandler::OnConnectedCallBack(const SingleConnection& connection)
{
	WriteLog("recvMsgHandler::OnConnectedCallBack");
}

inline void RecvMsgHandler::OnRecvMsgAsync(const SingleConnection& connection, share_buff buff)
{
#ifdef PROTOBUF
	rp::CmnBuf recvBuf;
	recvBuf.ParseFromArray(buff->data(), buff->size());

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
			string account = user.testaccount();
			string pwd = user.testpwd();
			rp::ContentTest::TestType testType = user.testtype();
		}
	}
		break;
	case PROTO_HEARTBEAT:
		WriteLog("ack hearbeat");
		break;
	default:
		break;
	}
#endif
}


void TestSingConnectionAsync()
{
	shared_ptr<RecvMsgHandler> recvMsgHandler = make_shared<RecvMsgHandler>();
	SingleConnection singleConnect("127.0.0.1", 30301, "", SVR_LINK_TYPE_UNKNOWN, CntType_Async, recvMsgHandler);
	if (!singleConnect.Connect())
		return;

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
		singleConnect.SendMsg(&cmnBuf,pHead);

		Sleep(100000);
	}
}


void TestSingConnectionSync()
{
	shared_ptr<RecvMsgHandler> recvMsgHandler = make_shared<RecvMsgHandler>();
	SingleConnection singleConnect("127.0.0.1", 30301, "", SVR_LINK_TYPE_UNKNOWN, CntType_Sync, recvMsgHandler);
	if (!singleConnect.Connect())
		return;

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
		share_buff recvMsg = NEW_SHAREDBUFF;
		int ret = singleConnect.SendMsg(&cmnBuf, pHead, recvMsg);
		if (ret == SEND_MSG_TIMEOUT)
		{
			WriteLog("sync msg send timeout");
			return;
		}
		else if (ret == SEND_MSG_FAIL)
		{
			WriteLog("sync msg send fail");
			return;
		}

		rp::CmnBuf recvBuf;
		recvBuf.ParseFromArray(recvMsg->data(), recvMsg->size());

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
				string account = user.testaccount();
				string pwd = user.testpwd();
				rp::ContentTest::TestType testType = user.testtype();
			}
		}
		break;
		default:
			break;
		}
		Sleep(100000);
	}
}

ConnectionPool gbs_Pool;
void TestConnectionPoolAsync()
{
	shared_ptr<RecvMsgHandler> recvMsgHandler = make_shared<RecvMsgHandler>();
	if (gbs_Pool.Init("192.168.1.3", 30301, SVR_LINK_TYPE_CLIENT, 0, 1, recvMsgHandler))
	{
		WriteLog("connect 127.0.0.1 success");
	}

	while (true)
	{
		Sleep(5000);

		rp::CmnBuf_MsgHead* pHead = new rp::CmnBuf_MsgHead;
		pHead->set_msgtype(PROTO_HEARTBEAT);
		rp::CmnBuf buf;
		buf.set_allocated_msgheader(pHead);
		buf.set_content("");
		gbs_Pool.SendMsgAsync(&buf);

		
	}
}