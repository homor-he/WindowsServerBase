#pragma once

#include "Base.h"
#include "ProtoCommon.pb.h"

using namespace rp;
using namespace google::protobuf;

class PacketProcess
{
public:
	/// <summary>
	/// 设置消息头
	/// </summary>
	/// <param name="type">消息协议号</param>
	/// <param name="orgin">连接类型</param>
	/// <param name="orderID">序列号，用于同步消息</param>
	/// <param name="respondID">线程tid，用于同步消息</param>
	/// <param name="errorCode">线程tid，用于同步消息</param>
	/// <returns></returns>
	static CmnBuf_MsgHead* SetPacketHeader(uint32 type, uint32 orgin = 0, uint32 orderID =0, 
		uint32 respondID =0, uint32 errorCode =0);
	static std::string GetSerializedPacket(CmnBuf_MsgHead* header, const std::string& content);
};
