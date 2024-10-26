#include "PacketProcess.h"

CmnBuf_MsgHead* PacketProcess::SetPacketHeader(uint32 type, uint32 orgin, uint32 orderID,
	uint32 respondID, uint32 errorCode)
{
	CmnBuf_MsgHead* pHeader = new CmnBuf_MsgHead;
	pHeader->set_msgtype(type);
	pHeader->set_origin(orgin);
	pHeader->set_orderid(orderID);
	pHeader->set_respondid(respondID);
	pHeader->set_errorcode(errorCode);
	return pHeader;
}

std::string PacketProcess::GetSerializedPacket(CmnBuf_MsgHead* header, const std::string& content)
{
	CmnBuf cmnBuf;
	cmnBuf.set_allocated_msgheader(header);
	cmnBuf.set_content(content);
	return cmnBuf.SerializeAsString();
}
