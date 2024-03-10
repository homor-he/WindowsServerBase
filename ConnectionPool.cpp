#include "ConnectionPool.h"
#include "Event.h"

SingleConnection::SingleConnection(const string& szIP, short wPort, string name, UINT svrLinkType, ConnectionType cntType,
	shared_ptr<ConnectionMsgHandler> msgHandler):
	TcpClient(szIP,wPort),
	m_svrLinkType(svrLinkType),
	m_cntType(cntType),
	m_orderID(0),
	m_msgHandler(msgHandler),
	m_name(name)
{
}

SingleConnection::~SingleConnection()
{
}

void SingleConnection::CloseConnection()
{
	{
		AutoLock lock(m_mutexSyncEvent);
		for (auto itor = m_mapSyncEvent.begin(); itor != m_mapSyncEvent.end(); ++itor)
		{
			delete itor->second;
			m_mapSyncEvent.erase(itor);
		}
	}
	TcpClient::CloseAll();
}

bool SingleConnection::Connect()
{
	if (TcpClient::Connect())
	{
		OnConnectBuilded();
		return true;
	}
	return false;
}

bool SingleConnection::Reconnect()
{
	if (TcpClient::Reconnect())
	{
		OnConnectBuilded();
		return true;
	}
	return false;
}

#ifdef PROTOBUF
UINT SingleConnection::SendMsg(Message* msg, rp::CmnBuf::MsgHead* header, share_buff recvMsg)
{
	switch (m_cntType)
	{
	case CntType_Sync:
		return SendMsgSync(msg, header, recvMsg);
	case CntType_Async:
	{
		string sendMsg = msg->SerializeAsString();
		SendMsgAsync(sendMsg);
		return SEND_MSG_SUCCESS;
	}
	default:
		break;
	}
	return SEND_MSG_FAIL;
}

UINT SingleConnection::SendMsgSync(Message* msg, rp::CmnBuf::MsgHead* header, share_buff recvMsg)
{
	if (!header)
		return SEND_MSG_FAIL;
	if (header->msgtype() == PROTO_BUILD_CONN_REQ)
	{
		string sendMsg = msg->SerializeAsString();
		SendMsgAsync(sendMsg);
		return SEND_MSG_SUCCESS;
	}
	UINT orderID = GetOrderID();
	std::thread::id threadID = std::this_thread::get_id();
	UINT respondID = *(unsigned int*)&threadID;

	header->set_orderid(orderID);
	header->set_respondid(respondID);

	Event* pEvent = new Event();
	{
		AutoLock lock(m_mutexSyncEvent);
		m_mapSyncEvent[orderID] = new SyncMsgPairInfo(respondID, pEvent);
	}
	string sendMsg = msg->SerializeAsString();
	SendMsgAsync(sendMsg);
	pEvent->Wait(SYNC_WAIT_TIME);
	{
		AutoLock lock(m_mutexSyncEvent);
		SyncMsgPairInfo* info = m_mapSyncEvent[orderID];
		if (!info->pRecvBuff)
			return SEND_MSG_TIMEOUT;
		if (m_cntType == CntType_Sync && recvMsg)
			recvMsg->swap(*info->pRecvBuff);
		m_mapSyncEvent.erase(orderID);
		delete info;
		info = nullptr;
	}
	return SEND_MSG_SUCCESS;
}
#endif // PROTOBUF




void SingleConnection::OnMsg(char* buf, int len)
{
	share_buff buff = make_shared<vector<char>>(buf, buf + len);
	switch (m_cntType)
	{
	case CntType_Sync:
		OnRecvMsgSync(buff);
		break;
	case CntType_Async:
		OnRecvMsgAsync(buff);
		break;
	default:
		break;
	}
}

void SingleConnection::OnConnectBuilded()
{
#ifdef PROTOBUF
	//初次建立连接发送的包
	rp::CmnBuf_MsgHead* pHead = new rp::CmnBuf_MsgHead;
	pHead->set_msgtype(PROTO_BUILD_CONN_REQ);
	pHead->set_origin(m_svrLinkType);

	rp::CmnBuf cmnBuf;
	cmnBuf.set_allocated_msgheader(pHead);
	cmnBuf.set_allocated_content(nullptr);

	//string cont = cmnBuf.SerializeAsString();
	//SendMsgAsync(cont);
	SendMsg((Message*)&cmnBuf, pHead);
#endif // PROTOBUF
}

void SingleConnection::OnRecvMsgAsync(share_buff buff)
{
#ifdef PROTOBUF
	rp::CmnBuf sCmnBuf;
	sCmnBuf.ParseFromArray(buff->data(), buff->size());
	const rp::CmnBuf_MsgHead& sHead = sCmnBuf.msgheader();

	//收到建立连接回包后，调用建立完成的回调
	if (sHead.msgtype() == PROTO_BUILD_CONN_ACK)
	{
		if (m_msgHandler)
			m_msgHandler->OnConnectedCallBack(*this);
	}
#endif // PROTOBUF
	if(m_cntType == CntType_Async && m_msgHandler)
		m_msgHandler->OnRecvMsgAsync(*this, buff);
}

void SingleConnection::OnRecvMsgSync(share_buff buff)
{
#ifdef PROTOBUF
	rp::CmnBuf sCmnBuf;
	sCmnBuf.ParseFromArray(buff->data(), buff->size());
	const rp::CmnBuf_MsgHead& sHead = sCmnBuf.msgheader();
	//收到建立连接回包后，调用建立完成的回调
	if (sHead.msgtype() == PROTO_BUILD_CONN_ACK)
	{
		if (m_msgHandler)
			m_msgHandler->OnConnectedCallBack(*this);
		return;
	}

	{
		AutoLock lock(m_mutexSyncEvent);
		//orderid与第几个同步包有关
		map<UINT,SyncMsgPairInfo*>::iterator it = m_mapSyncEvent.find(sHead.orderid());
		if (it != m_mapSyncEvent.end())
		{
			//respondid和线程有关
			SyncMsgPairInfo* info = it->second;
			if (info->respondID != sHead.respondid())
			{
				WriteLog("***client::%s:%d,responid not suit,req oid:%d, ack oid:%d", __FILE__, __LINE__,
					info->respondID, sHead.respondid());
				return;
			}
			info->pRecvBuff = buff;
			info->pEvent->Signal();
		}
	}
#endif // PROTOBUF
}

UINT SingleConnection::GetOrderID()
{
	uint expect = MAX_ORDER_ID;
	uint desired = 0;
	//如果m_orderID==expect则为true， m_orderID = desired
	//如果m_orderI!=expect为false, expect = m_orderID
	if (!atomic_compare_exchange_strong(&m_orderID, &expect, desired))
	{
		m_orderID++;
		desired = expect;
	}
	return desired;
}

//-------------------------------------------------------------------------

ConnectionPool::ConnectionPool(void) : m_szIP(""),m_wPort(0), m_connectedAll(true)
{
}

ConnectionPool::~ConnectionPool(void)
{
	CloseAll();
}

bool ConnectionPool::Init(const string& szIP, short wPort, uint svrLinktype, uint syncConnCnt, uint asyncConnCnt, shared_ptr<ConnectionMsgHandler> msgHandler)
{
	m_szIP = szIP;
	m_wPort = wPort;
	for (uint i = 0; i < syncConnCnt; ++i)
	{
		string name = "sync-remote:" + m_szIP + ":" + to_string(wPort) + "#" + to_string(i);
		m_connSyncList.push_back(new SingleConnection(m_szIP, m_wPort, name, svrLinktype, ConnectionType::CntType_Sync, msgHandler));
		if (!m_connSyncList[i]->Connect())
			m_connectedAll = false;
	}

	for (uint i = 0; i < asyncConnCnt; ++i)
	{
		string name = "async-remote:" + m_szIP + ":" + to_string(wPort) + "#" + to_string(i);
		m_connAsyncList.push_back(new SingleConnection(m_szIP, m_wPort, name, svrLinktype, ConnectionType::CntType_Async, msgHandler));
		if (!m_connAsyncList[i]->Connect())
			m_connectedAll = false;
	}
	return m_connectedAll;
}

void ConnectionPool::CloseAll()
{
	for (SingleConnection* pObj : m_connSyncList)
	{
		pObj->CloseConnection();
		delete pObj;
		pObj = nullptr;
	}
	m_connSyncList.clear();

	for (SingleConnection* pObj : m_connAsyncList)
	{
		pObj->CloseConnection();
		delete pObj;
		pObj = nullptr;
	}
	m_connAsyncList.clear();
}

uint ConnectionPool::SendMsgAsync(const void* data, int len)
{
	return uint();
}

uint ConnectionPool::SendMsgSync(const void* data, int len, vector<char>* recv)
{
	return uint();
}

#ifdef PROTOBUF
uint ConnectionPool::SendMsgAsync(Message* data)
{
	return m_connAsyncList[(rand() % m_connAsyncList.size())]->SendMsg(data, nullptr);
}

uint ConnectionPool::SendMsgSync(Message* data, rp::CmnBuf::MsgHead* header, share_buff recv)
{
	return m_connSyncList[(rand() % m_connSyncList.size())]->SendMsg(data, header, recv);
}
#endif // PROTOBUF


