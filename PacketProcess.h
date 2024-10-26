#pragma once

#include "Base.h"
#include "ProtoCommon.pb.h"

using namespace rp;
using namespace google::protobuf;

class PacketProcess
{
public:
	/// <summary>
	/// ������Ϣͷ
	/// </summary>
	/// <param name="type">��ϢЭ���</param>
	/// <param name="orgin">��������</param>
	/// <param name="orderID">���кţ�����ͬ����Ϣ</param>
	/// <param name="respondID">�߳�tid������ͬ����Ϣ</param>
	/// <param name="errorCode">�߳�tid������ͬ����Ϣ</param>
	/// <returns></returns>
	static CmnBuf_MsgHead* SetPacketHeader(uint32 type, uint32 orgin = 0, uint32 orderID =0, 
		uint32 respondID =0, uint32 errorCode =0);
	static std::string GetSerializedPacket(CmnBuf_MsgHead* header, const std::string& content);
};
