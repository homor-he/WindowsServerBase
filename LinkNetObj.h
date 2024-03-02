#pragma once

//#include "SocketDefine.h"
#include "Base.h"
#include "ServerGlobalDefine.h"
#include "ProtoGobalDefine.h"

#pragma warning(disable: 4267)

struct PER_SOCKET_CONTEXT;
class LinkNetObj
{
public:
	LinkNetObj();
	LinkNetObj(PER_SOCKET_CONTEXT* sockOpera, UINT type);
	virtual ~LinkNetObj();

	virtual bool OnMsg(PER_SOCKET_CONTEXT* conn, share_buff buff);
	virtual bool OnDisconnected(PER_SOCKET_CONTEXT* conn);
	virtual bool SendMsgAsync(string &sendMsg);

	void SendBuildConnectAck();
	UINT GetUniqueID(void);
	void SetCntType(UINT type);
	UINT GetCntType(void);
public:
#ifdef PROTOBUF
	void ParseTestReq(rp::CmnBuf& buf);
	void ParseHeartBeat(rp::CmnBuf& buf);
#endif // PROTOBUF
protected:
	UINT m_uniqueID;
	UINT m_type;
	PER_SOCKET_CONTEXT* m_sockOpera;
};