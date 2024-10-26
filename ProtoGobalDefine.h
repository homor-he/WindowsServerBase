#pragma once

#include "ServerGlobalDefine.h"
#include "ProtoDefine_Auth.h"
#include "ProtoDefine_Game.h"


//#ifdef PROTOBUF
//#define PROTOBUF_USE_DLLS    //����ĿԤ�������м���ú�

#include "ProtoCommon.pb.h"
#include "ProtoAuth.pb.h"

using namespace rp;


#define PROTO_BASE  0x1000     //���ڻ���Э��

#define PROTO_BUILD_CONN_REQ   (PROTO_BASE+0x0001)   //0x1001
#define PROTO_BUILD_CONN_ACK   (PROTO_BASE+0x0002)	 //0x1002

#define PROTO_TEST_REQ	(PROTO_BASE+ 0x0003)		//0x1003
#define PROTO_TEST_ACK  (PROTO_BASE+ 0x0004)        //0x1004

#define PROTO_HEARTBEAT  (PROTO_BASE+ 0x0005)		//0x1005



//#endif // PROTOBUF




 