#pragma once

#include "ServerGlobalDefine.h"



#ifdef PROTOBUF
//#define PROTOBUF_USE_DLLS    //在项目预处理器中加入该宏

#include "ProtoCommon.pb.h"

using namespace rp;



#define PROTO_BASE  0x1000     //用于基础协议

#define PROTO_REQ_BUILD_CONN   (PROTO_BASE+0x0001)
#define PROTO_REQ_BUILD_CONN_ACK (PROTO_BASE+0x0002)

#define PROTO_TEST_REQ	(PROTO_BASE+ 0x0003)
#define PROTO_TEST_ACK  (PROTO_BASE+ 0x0004)

#define PROTO_HEARTBEAT  (PROTO_BASE+ 0x0005)

#endif // PROTOBUF




 