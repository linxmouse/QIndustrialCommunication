#include "ModbusTcpNet.h"
#include <QRegExp>

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

QICResult<QByteArray> ModbusTcpNet::BuildReadRequest(const QString& address, ushort length)
{
	// 地址解析
	auto addr = ParseAddress(address);
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
	// 提取有效载荷
	quint8 byteCount = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, byteCount);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteRequest(const QString& address, const QByteArray& value)
{
	// 地址解析
	auto addrInfo = ParseAddress(address);
	if (!addrInfo.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addrInfo);
	// 构建PDU
	QByteArray pdu;
	// 功能码: 写多寄存器
	pdu.append(0x10);
	qToBigEndian(addrInfo.getContent0().address, pdu.data() + 1);
	quint16 regCount = value.size() / WordLenght;
	qToBigEndian(regCount, pdu.data() + 3);
	// 字节数
	pdu.append(static_cast<char>(regCount * WordLenght));
}

QICResult<ModbusAddress> ModbusTcpNet::ParseAddress(const QString& address)
{
	try
	{
		// 首位必须是0-4，且只占1个字符
		QRegExp reg("^([0-4])(\\d+)(?:@(\\d+))?$");
		if (!reg.exactMatch(address)) return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format");
		// 解析地址
		QString typeStr = reg.cap(1);
		quint16 addr = address.toUShort();
		quint8 station = reg.cap(3).isEmpty() ? m_unitId : reg.cap(3).toUShort();
		// 确定功能码
		QMap<QString, quint8> typeMap = {
			{"0", 0x01},// 线圈
			{"1", 0x02},// 离散输入
			{"3", 0x04},// 输入寄存器
			{"4", 0x03},// 保持寄存器
		};
		if (!typeMap.contains(typeStr)) return QICResult<ModbusAddress>::CreateFailedResult("Unspported address type");
		// 地址偏移处理
		if (!m_addressStartWithZero && addr < 1) return QICResult<ModbusAddress>::CreateFailedResult("Address must be greater than 0");
		addr = m_addressStartWithZero ? addr : (addr - 1);
		return QICResult<ModbusAddress>::CreateSuccessResult({ addr, typeMap[typeStr], station });
	}
	catch (...)
	{
		return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format");
	}
}

