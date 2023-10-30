#include "KeyenceNanoSerialOverTcp.h"

KeyenceNanoSerialOverTcp::KeyenceNanoSerialOverTcp(QString ipAddress, int port, bool isPersistentConn, 
	bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject* parent)
	: NetworkDevice(ipAddress, port, isPersistentConn, enableSendRecvLog, connectTimeOut, receiveTimeOut, parent)
{
	byteConverter = new ByteConverterBase(DataFormat::DCBA, true);
}

KeyenceNanoSerialOverTcp::~KeyenceNanoSerialOverTcp()
{
	CloseConnect();
}
