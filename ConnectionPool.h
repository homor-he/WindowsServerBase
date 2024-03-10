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
	CntType_Sync,    //����ͬ����Ϣ��������Ϣ�ȴ�����
	CntType_Async,	 //�����첽��Ϣ
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
	//���ҽ���SingleConnection��ϢΪ�첽ʱ��Ч
	virtual void OnRecvMsgAsync(const SingleConnection& connection, share_buff buff) = 0;
};

class SingleConnection : public TcpClient
{
public:
	SingleConnection(const string& szIP, short wPort, string name, UINT svrLinkType,ConnectionType cntType = CntType_Async,
		shared_ptr<ConnectionMsgHandler> msgHandler = nullptr);
	virtual ~SingleConnection();
	void CloseConnection();

	virtual bool Connect();
	virtual bool Reconnect();
#ifdef PROTOBUF
	//���ҽ�����������ͬ����Ϣʱ��recvMsg������
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
	atomic<UINT> m_orderID;
	shared_ptr<ConnectionMsgHandler> m_msgHandler;

	CThreadMutex m_mutexSyncEvent;
	std::map<UINT, SyncMsgPairInfo*> m_mapSyncEvent;  //keyΪorderID

	string m_name;
};

class ConnectionPool
{
public:
	ConnectionPool(void);
	virtual ~ConnectionPool(void);

	bool Init(const string& szIP, short wPort, uint svrLinktype = SVR_LINK_TYPE_UNKNOWN, uint syncConnCnt = DEFALUT_CONN_CNT,
		uint asyncConnCnt = DEFALUT_CONN_CNT, shared_ptr<ConnectionMsgHandler> msgHandler = nullptr);
	void CloseAll();

	//��ǰ������ʼ��ʱ���첽����ʱʹ��
	uint SendMsgAsync(const void* data, int len);
	//��ǰ������ʼ��ʱ��ͬ������ʹ��
	uint SendMsgSync(const void* data, int len, vector<char>* recv = nullptr);



#ifdef PROTOBUF
	//������ͬ��
	uint SendMsgAsync(Message* data);
	uint SendMsgSync(Message* data, rp::CmnBuf::MsgHead* header, share_buff recv = nullptr);
#endif
protected:
	atomic<uint> m_syncConnIndex;
	vector<SingleConnection*> m_connSyncList;

	atomic<uint> m_asyncConnIndex;
	vector<SingleConnection*> m_connAsyncList;
private:
	string m_szIP;
	short m_wPort;
	bool m_connectedAll;
};