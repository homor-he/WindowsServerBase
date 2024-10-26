#include "LinkNetObj.h"
#include "ProtoGobalDefine.h"
#include "SocketDefine.h"


LinkNetObj::LinkNetObj():
	m_uniqueID(0),
	m_type(0),
	m_sockOpera(nullptr)
{
}

LinkNetObj::LinkNetObj(PER_SOCKET_CONTEXT* sockOpera, UINT type):
	m_uniqueID((UINT)sockOpera->GetSocketFD()),
	m_type(type),
	m_sockOpera(sockOpera)
{
}

LinkNetObj::~LinkNetObj()
{
	delete m_sockOpera;
	m_sockOpera = nullptr;
}

bool LinkNetObj::OnMsg(PER_SOCKET_CONTEXT* conn, share_buff buff)
{
	WriteLog("LinkNetObj::OnMsg, peer addr:%s, local addr:%s", conn->GetPeerIPPort().c_str(),conn->GetLocalIPPort().c_str());

#ifdef PROTOBUF
	rp::CmnBuf sCmnBuf;
	sCmnBuf.ParseFromArray(buff->data(), buff->size());
	const rp::CmnBuf_MsgHead& sHead = sCmnBuf.msgheader();

	switch (sHead.msgtype())
	{
	case PROTO_TEST_REQ:
		ParseTestReq(sCmnBuf);
		break;
	case PROTO_HEARTBEAT:
		ParseHeartBeat(sCmnBuf);
		break;
	default:
		break;
	}
#endif // PROTOBUF
	return true;
}

bool LinkNetObj::OnDisconnected(PER_SOCKET_CONTEXT* conn)
{
	return false;
}

bool LinkNetObj::SendMsgAsync(string& sendMsg)
{
	m_sockOpera->PostSend((char*)sendMsg.c_str(), sendMsg.size());
	return true;
}

void LinkNetObj::SendBuildConnectAck()
{
	rp::CmnBuf_MsgHead* pHead = new rp::CmnBuf_MsgHead;
	pHead->set_msgtype(PROTO_BUILD_CONN_ACK);

	rp::CmnBuf cmnBuf;
	cmnBuf.set_allocated_msgheader(pHead);
	cmnBuf.set_allocated_content(nullptr);

	string cont = cmnBuf.SerializeAsString();
	SendMsgAsync(cont);
}

UINT LinkNetObj::GetUniqueID(void)
{
	return m_uniqueID;
}

void LinkNetObj::SetCntType(UINT type)
{
	m_type = type;
}

UINT LinkNetObj::GetCntType(void)
{
	return m_type;
}

void LinkNetObj::ParseTestReq(rp::CmnBuf& buf)
{
	rp::ContentTest contextTest;
	contextTest.ParseFromString(buf.content());

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
	string str = contextTest.teststring();

	int size = contextTest.testuserlist_size();
	auto userList = contextTest.testuserlist();
	for (int i = 0; i < size; ++i)
	{
		rp::ContentTest::TestUser user = userList.Get(i);
		string account = user.testaccount();
		string pwd = user.testpwd();
		rp::ContentTest::TestType testType = user.testtype();
	}

	string sReq = buf.SerializeAsString();
	SendMsgAsync(sReq);
}

void LinkNetObj::ParseHeartBeat(rp::CmnBuf& buf)
{
	string sReq = buf.SerializeAsString();
	SendMsgAsync(sReq);
}