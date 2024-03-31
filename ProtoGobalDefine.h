#pragma once

#include "ServerGlobalDefine.h"



#ifdef PROTOBUF
//#define PROTOBUF_USE_DLLS    //����ĿԤ�������м���ú�

#include "ProtoCommon.pb.h"

using namespace rp;


#define PROTO_BASE  0x1000     //���ڻ���Э��

#define PROTO_BUILD_CONN_REQ   (PROTO_BASE+0x0001)   //0x1001
#define PROTO_BUILD_CONN_ACK   (PROTO_BASE+0x0002)	 //0x1002

#define PROTO_TEST_REQ	(PROTO_BASE+ 0x0003)		//0x1003
#define PROTO_TEST_ACK  (PROTO_BASE+ 0x0004)        //0x1004

#define PROTO_HEARTBEAT  (PROTO_BASE+ 0x0005)		//0x1005


#define PROTO_AUTH  0x2000     //���ڵ�¼���Э��

#define PROTO_CS_LOGIN_REQ  (PROTO_AUTH+0x0001)		//0x2001
#define PROTO_CS_LOGIN_ACK  (PROTO_AUTH+0x0002)		//0x2002


#define PROTO_GAME 0x3000      //������Ϸҵ���Э��

#endif // PROTOBUF




 