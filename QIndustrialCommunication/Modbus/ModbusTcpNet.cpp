#include "ModbusTcpNet.h"

ModbusTcpNet::ModbusTcpNet(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject *parent)
	: EthernetDevice(ipAddr, port, isPersistentConn, enableSendRecvLog, connectTimeOut, receiveTimeOut, parent)
{
	WordsPerAddress = 1;
	BytesOrderPtr.reset(new BytesOrderBase(DataFormat::CDAB, false));
}

ModbusTcpNet::~ModbusTcpNet()
{
	CloseConnect();
}

QICResult<> ModbusTcpNet::InitializationOnConnect(QTcpSocket *socket)
{
	return QICResult<>::CreateSuccessResult();
}

QICResult<> ModbusTcpNet::ReleaseOnDisconnect(QTcpSocket *socket)
{
	return QICResult<>();
}

QICResult<QByteArray> ModbusTcpNet::Read(const QString &address, ushort length)
{
	// 构建请求
	auto buildResult = BuildReadRequest(address, length);
	if (!buildResult.IsSuccess)
		return buildResult;
	// 发送请求
	auto response = ReadFromSocket(buildResult.getContent0());
	if (!response.IsSuccess)
		return response;
	// 解析响应
	return ParseReadResponse(response.getContent0());
}

QICResult<> ModbusTcpNet::Write(const QString &address, const QByteArray &value)
{
	// 构建请求
	auto buildResult = BuildWriteRequest(address, value);
	if (!buildResult.IsSuccess)
		return QICResult<>::CreateFailedResult(buildResult);
	// 发送请求并解析响应
	auto response = ReadFromSocket(buildResult.getContent0());
	if (!response.IsSuccess)
		return QICResult<>::CreateFailedResult(response);
	return ParseWriteResponse(response.getContent0());
}

QICResult<> ModbusTcpNet::Write(const QString &address, const QVector<bool> &values)
{
	// 构建请求
	auto buildResult = BuildWriteBoolRequest(address, values);
	if (!buildResult.IsSuccess)
		return QICResult<>::CreateFailedResult(buildResult);
	// 发送请求并解析响应
	auto response = ReadFromSocket(buildResult.getContent0());
	if (!response.IsSuccess)
		return QICResult<>::CreateFailedResult(response);
	return ParseWriteBoolResponse(response.getContent0());
}

QICResult<QVector<bool>> ModbusTcpNet::ReadBool(const QString &address, ushort length)
{
	// 构建请求
	auto buildResult = BuildReadBoolRequest(address, length);
	if (!buildResult.IsSuccess)
		return QICResult<QVector<bool>>::CreateFailedResult(buildResult);
	// 发送请求
	auto response = ReadFromSocket(buildResult.getContent0());
	if (!response.IsSuccess)
		return QICResult<QVector<bool>>::CreateFailedResult(response);
	// 解析响应
	auto parsedData = ParseReadBoolResponse(response.getContent0());
	if (!parsedData.IsSuccess)
		return QICResult<QVector<bool>>::CreateFailedResult(parsedData);
	QByteArray bytes = parsedData.getContent0();
	auto boolValues = ByteArrayToBoolList(bytes, length);
	return QICResult<QVector<bool>>::CreateSuccessResult(boolValues);
}

QICResult<QByteArray> ModbusTcpNet::BuildReadRequest(const QString &address, ushort length)
{
	// 地址解析
	auto addr = ModbusAddress::ParseAddress(address, ModbusAddress::READ_HOLDING_REGISTER, getUnitId(), getIsOneBaseAddress());
	if (!addr.IsSuccess)
		return QICResult<QByteArray>::CreateFailedResult(addr);
	// 构建PDU:功能码(1)+地址(2)+字节长度(2)
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

QICResult<QByteArray> ModbusTcpNet::ParseReadResponse(const QByteArray &response)
{
	// 基础校验
	if (response.size() < 9)
		return QICResult<QByteArray>::CreateFailedResult("Invalid response length");
	// 校验功能码:检查功能码是否带有异常标志(高位为1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80)
		return QICResult<QByteArray>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(static_cast<quint8>(response[8]), 2, 16, QChar('0')));
	// 提取有效载荷: FunctionCode(1) | DataLength(1) | Data(variable)
	quint8 dataLenth = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, dataLenth);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildReadBoolRequest(const QString &address, ushort length)
{
	// 地址解析
	auto addr = ModbusAddress::ParseAddress(address, ModbusAddress::READ_DISCRETE_INPUT, getUnitId(), getIsOneBaseAddress());
	if (!addr.IsSuccess)
		return QICResult<QByteArray>::CreateFailedResult(addr);
	// 构建PDU:功能码(1)+地址(2)+位长度(2)
	QByteArray pdu;
	QDataStream stream(&pdu, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << addr.getContent0().functionCode
		   << addr.getContent0().address
		   << length;
	// 构建完整报文
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<QByteArray> ModbusTcpNet::ParseReadBoolResponse(const QByteArray &response)
{
	// 基础校验
	if (response.size() < 9)
		return QICResult<QByteArray>::CreateFailedResult("Invalid response length");
	// 校验功能码:检查功能码是否带有异常标志(高位为1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80)
		return QICResult<QByteArray>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(static_cast<quint8>(response[8]), 2, 16, QChar('0')));
	// 提取有效载荷: FunctionCode(1) | DataLength(1) | Data(variable)
	quint8 dataLength = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, dataLength);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteRequest(const QString &address, const QByteArray &value)
{
	// 根据数据长度确定功能码
	quint8 functionCode = (value.size() == 2) ? ModbusAddress::WRITE_SINGLE_REGISTER : ModbusAddress::WRITE_MULTIPLE_REGISTER;
	auto addr = ModbusAddress::ParseAddress(address, functionCode, getUnitId(), getIsOneBaseAddress());
	if (!addr.IsSuccess)
		return QICResult<QByteArray>::CreateFailedResult(addr);
	// 构建PDU
	// WRITE_SINGLE_REGISTER:	功能码(1)+地址(2)+Data(variable)
	// WRITE_MULTIPLE_REGISTER:	功能码(1)+起始地址(2)+寄存器数(2)+字节长度(1)+Data(variable)
	QByteArray pdu;
	QDataStream stream(&pdu, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << functionCode << addr.getContent0().address;
	if (functionCode == ModbusAddress::WRITE_MULTIPLE_REGISTER)
	{
		// 每个寄存器占用2字节
		stream << static_cast<quint16>(value.size() / 2)
			   << static_cast<quint8>(value.size());
	}
	pdu.append(value);
	// 构建完整报文
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<> ModbusTcpNet::ParseWriteResponse(const QByteArray &response)
{
	// MBAP(7)+功能码(1)+地址(2)+值(2)
	if (response.size() < 12)
		return QICResult<>::CreateFailedResult("Invalid response length");
	// 校验功能码:检查功能码是否带有异常标志(高位为1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80)
		return QICResult<>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(static_cast<quint8>(response[8]), 2, 16, QChar('0')));
	// 验证功能码是否匹配
	if (functionCode != ModbusAddress::WRITE_SINGLE_REGISTER && functionCode != ModbusAddress::WRITE_MULTIPLE_REGISTER)
		return QICResult<>::CreateFailedResult("Function code mismath");
	return QICResult<>::CreateSuccessResult();
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteBoolRequest(const QString &address, const QVector<bool> &values)
{
	// 根据线圈数量自动判断功能码
	quint8 functionCode = (values.size() == 1) ? ModbusAddress::WRITE_SINGLE_COIL : ModbusAddress::WRITE_MULTIPLE_COIL;
	auto addr = ModbusAddress::ParseAddress(address, functionCode, getUnitId(), getIsOneBaseAddress());
	if (!addr.IsSuccess)
		return QICResult<QByteArray>::CreateFailedResult(addr);
	// 构建PDU
	// WRITE_SINGLE_COIL:功能码(1)+地址(2)+Data(2)
	// WRITE_MULTIPLE_COIL:功能码(1)+地址(2)+线圈数(2)+字节长度(1)+Data(variable)
	QByteArray pdu;
	QDataStream stream(&pdu, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << functionCode << addr.getContent0().address;
	if (functionCode == ModbusAddress::WRITE_SINGLE_COIL)
	{
		quint16 coilValue = values.at(0) ? 0xff00 : 0x0000;
		stream << coilValue;
	}
	else if (functionCode == ModbusAddress::WRITE_MULTIPLE_COIL)
	{
		quint16 numCoils = values.size();
		quint8 byteCount = qCeil(numCoils / 8.0);
		auto coilBytes = BoolListToByteArray(values);
		stream << numCoils << byteCount;
		pdu.append(coilBytes);
	}
	else
		QICResult<QByteArray>::CreateFailedResult("Invalid functiion code");
	// 构建完整报文
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<> ModbusTcpNet::ParseWriteBoolResponse(const QByteArray &response)
{
	// MBAP(7)+功能码(1)+地址(2)+值(2)
	if (response.size() < 12)
		return QICResult<>::CreateFailedResult("Invalid response length");
	// 校验功能码:检查功能码是否带有异常标志(高位为1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80)
		return QICResult<>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(static_cast<quint8>(response[8]), 2, 16, QChar('0')));
	// 验证功能码是否匹配
	if (functionCode != ModbusAddress::WRITE_SINGLE_COIL && functionCode != ModbusAddress::WRITE_MULTIPLE_COIL)
		return QICResult<>::CreateFailedResult("Function code mismath");
	return QICResult<>::CreateSuccessResult();
}
