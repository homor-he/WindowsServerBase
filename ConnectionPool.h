#pragma once

#include "TcpClient.h"
#ifdef PROTOBUF
#include <google\protobuf\message.h>

using namespace google::protobuf;
#endif // PROTOBUF


#define MAX_ORDER_ID 4294967295
#define SYNC_WAIT_TIME 5000
#define DEFALUT_CONN_CNT 2

enum ConnectionType
{
	CntType_Sync,    //处理同步消息，发送消息等待接收
	CntType_Async,	 //处理异步消息
};

struct SyncMsgPairInfo
{
	SyncMsgPairInfo(UINT respondID, Event* pEvent)
	{
		this->respondID = respondID;
		this->pEvent = pEvent;
		pRecvBuff = nullptr;
	}

	~SyncMsgPairInfo()
	{
		delete pEvent;
	}

	void SetShareBuff(share_buff buff)
	{
		this->pRecvBuff = buff;
	}
	UINT respondID;
	Event* pEvent;
	share_buff pRecvBuff;
};

class SingleConnection;
class ConnectionMsgHandler
{
public:
	virtual void OnConnectedCallBack(const SingleConnection& connection) = 0;
	//当且仅当SingleConnection消息为异步时有效
	virtual void OnRecvMsgAsync(const SingleConnection& connection, share_buff buff) = 0;
};

class SingleConnection : public TcpClient
{
public:
	SingleConnection(const std::string& szIP, short wPort, std::string name, UINT svrLinkType,ConnectionType cntType = CntType_Async,
		std::shared_ptr<ConnectionMsgHandler> msgHandler = nullptr);
	virtual ~SingleConnection();
	void CloseConnection();

	virtual bool Connect();
	virtual bool Reconnect();
#ifdef PROTOBUF
	//当且仅当连接用于同步消息时，recvMsg才有用
	UINT SendMsg(Message* msg,rp::CmnBuf::MsgHead* header, share_buff recvMsg = nullptr);
#endif // PROTOBUF

	void OnMsg(char* buf, int len);
protected:
	virtual void OnConnectBuilded();
	virtual void OnRecvMsgAsync(share_buff buff);
	virtual void OnRecvMsgSync(share_buff buff);
private:
	UINT GetOrderID();
#ifdef PROTOBUF
	UINT SendMsgSync(Message* msg, rp::CmnBuf::MsgHead* header, share_buff recvMsg);
#endif // PROTOBUF

private:
	UINT m_svrLinkType;
	ConnectionType m_cntType;
	std::atomic<UINT> m_orderID;
	std::shared_ptr<ConnectionMsgHandler> m_msgHandler;

	CThreadMutex m_mutexSyncEvent;
	std::map<UINT, SyncMsgPairInfo*> m_mapSyncEvent;  //key为orderID

	std::string m_name;
};

class ConnectionPool
{
public:
	ConnectionPool(void);
	virtual ~ConnectionPool(void);

	bool Init(const std::string& szIP, short wPort, uint svrLinktype = SVR_LINK_TYPE_UNKNOWN, uint syncConnCnt = DEFALUT_CONN_CNT,
		uint asyncConnCnt = DEFALUT_CONN_CNT, std::shared_ptr<ConnectionMsgHandler> msgHandler = nullptr);
	void CloseAll();

	//当前仅当初始化时有异步连接时使用
	void SendMsgAsync(char* data, int len);
	void SendMsgAsync(std::string& data);
#ifdef PROTOBUF
	//与上面同理
	uint SendMsgAsync(Message* data);
	uint SendMsgSync(Message* data, rp::CmnBuf::MsgHead* header, share_buff recv = nullptr);
#endif
protected:
	std::atomic<uint> m_syncConnIndex;
	std::vector<SingleConnection*> m_connSyncList;

	std::atomic<uint> m_asyncConnIndex;
	std::vector<SingleConnection*> m_connAsyncList;
private:
	std::string m_szIP;
	short m_wPort;
	bool m_connectedAll;
};