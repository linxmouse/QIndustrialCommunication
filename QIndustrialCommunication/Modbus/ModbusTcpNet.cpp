#include "ModbusTcpNet.h"

ModbusTcpNet::ModbusTcpNet(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject* parent)
	: EthernetDevice(ipAddr, port, isPersistentConn, enableSendRecvLog, connectTimeOut, receiveTimeOut, parent)
{
	//WordLenght = 2;
	BytesOrderPtr.reset(new BytesOrderBase(DataFormat::ABCD, true));
}

ModbusTcpNet::~ModbusTcpNet()
{
	CloseConnect();
}

QICResult<> ModbusTcpNet::InitializationOnConnect(QTcpSocket* socket)
{
	return QICResult<>::CreateSuccessResult();
}

QICResult<> ModbusTcpNet::ReleaseOnDisconnect(QTcpSocket* socket)
{
	return QICResult<>();
}

QICResult<QByteArray> ModbusTcpNet::Read(const QString& address, ushort length)
{
	// 构建请求
	auto buildResult = BuildReadRequest(address, length);
	if (!buildResult.IsSuccess) return buildResult;
	// 发送请求
	auto response = ReadFromCoreServer(buildResult.getContent0());
	if (!response.IsSuccess) return response;
	// 解析响应
	return ParseReadResponse(response.getContent0());
}

QICResult<> ModbusTcpNet::Write(const QString& address, const QByteArray& value)
{
	// 地址解析
	quint16 startAddr = address.mid(1).toUShort() - 40001;

	// 建构请求头
	QByteArray pdu;
	// 功能码: 写多个寄存器
	pdu.append(0x10);
	// 起始地址
	pdu.append(reinterpret_cast<char*>(&startAddr), 2);
	// 寄存器数量
	pdu.append(static_cast<char>(value.size() / WordLenght));

	// 添加数据
	QByteArray formattedData = BytesOrderPtr->PackByteArray(value, 0, value.size());
	pdu.append(formattedData);

	// 发送请求并验证响应
	QByteArray request = BuildMBAPHeader(pdu.size()) + pdu;
	auto response = ReadFromCoreServer(request);

	// 验证写入结果
	// 成功响应应返回相同地址和数量
	if (response.getContent0().mid(9, 4) != pdu.left(4))
	{
		return QICResult<>::CreateFailedResult("Write verification failed");
	}
	return QICResult<>::CreateSuccessResult();
}

QICResult<QVector<bool>> ModbusTcpNet::ReadBool(const QString& address, ushort length)
{
	return QICResult<QVector<bool>>();
}

QICResult<QByteArray> ModbusTcpNet::BuildReadRequest(const QString& address, ushort length)
{
	// 地址解析
	auto addr = ModbusAddress::ParseAddress(address, 0x03, m_unitId);
	if (!addr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addr);
	// 构建PDU:功能码(1)+地址(2)+长度(2)
	QByteArray pdu;
	QDataStream stream(&pdu, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << static_cast<quint8>(addr.getContent0().functionCode)
		   << addr.getContent0().address
		   << length;
	// 构建完整报文
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<QByteArray> ModbusTcpNet::ParseReadResponse(const QByteArray& response)
{
	// 基础校验
	if (response.size() < 9) return QICResult<QByteArray>::CreateFailedResult("Invalid response length");
	// 校验功能码
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80) return QICResult<QByteArray>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(response[8], 2, 16, QChar('0')));
	// 提取有效载荷: FunctionCode(1) | DataLength(1) | Data (variable)
	quint8 dataLenth = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, dataLenth);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteRequest(const QString& address, const QByteArray& value)
{
	return QICResult<QByteArray>::CreateFailedResult("Not Implement");
}
