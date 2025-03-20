#include "ModbusTcpNet.h"

ModbusTcpNet::ModbusTcpNet(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject* parent)
	: EthernetDevice(ipAddr, port, isPersistentConn, enableSendRecvLog, connectTimeOut, receiveTimeOut, parent)
{
	WordLenght = 1;
	BytesOrderPtr.reset(new BytesOrderBase(DataFormat::DCBA, false));
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
	// ��������
	auto buildResult = BuildReadRequest(address, length);
	if (!buildResult.IsSuccess) return buildResult;
	// ��������
	auto response = ReadFromCoreServer(buildResult.getContent0());
	if (!response.IsSuccess) return response;
	// ������Ӧ
	return ParseReadResponse(response.getContent0());
}

QICResult<> ModbusTcpNet::Write(const QString& address, const QByteArray& value)
{
	// ��������
	auto buildResult = BuildWriteRequest(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �������󲢽�����Ӧ
	auto response = ReadFromCoreServer(buildResult.getContent0());
	if (!response.IsSuccess) return QICResult<>::CreateFailedResult(response);
	return ParseWriteResponse(response.getContent0());
}

QICResult<> ModbusTcpNet::Write(const QString& address, const QVector<bool>& values)
{
	// ��������
	auto buildResult = BuildWriteBoolRequest(address, values);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �������󲢽�����Ӧ
	auto response = ReadFromCoreServer(buildResult.getContent0());
	if (!response.IsSuccess) return QICResult<>::CreateFailedResult(response);
	return ParseWriteBoolResponse(response.getContent0());
}

QICResult<QVector<bool>> ModbusTcpNet::ReadBool(const QString& address, ushort length)
{
	// ��������
	auto buildResult = BuildReadBoolRequest(address, length);
	if (!buildResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(buildResult);
	// ��������
	auto response = ReadFromCoreServer(buildResult.getContent0());
	if (!response.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(response);
	// ������Ӧ
	auto parsedData = ParseReadBoolResponse(response.getContent0());
	if (!parsedData.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(parsedData);
	QByteArray bytes = parsedData.getContent0();
	auto boolValues = ByteArrayToBoolList(bytes);
	return QICResult<QVector<bool>>::CreateSuccessResult(boolValues);
}

QICResult<QByteArray> ModbusTcpNet::BuildReadRequest(const QString& address, ushort length)
{
	// ��ַ����
	auto addr = ModbusAddress::ParseAddress(address, ModbusAddress::READ_HOLDING_REGISTER, getUnitId());
	if (!addr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addr);
	// ����PDU:������(1)+��ַ(2)+�ֽڳ���(2)
	QByteArray pdu;
	QDataStream stream(&pdu, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << static_cast<quint8>(addr.getContent0().functionCode)
		   << addr.getContent0().address
		   << length;
	// ������������
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<QByteArray> ModbusTcpNet::ParseReadResponse(const QByteArray& response)
{
	// ����У��
	if (response.size() < 9) return QICResult<QByteArray>::CreateFailedResult("Invalid response length");
	// У�鹦����:��鹦�����Ƿ�����쳣��־(��λΪ1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80) return QICResult<QByteArray>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(response[8], 2, 16, QChar('0')));
	// ��ȡ��Ч�غ�: FunctionCode(1) | DataLength(1) | Data(variable)
	quint8 dataLenth = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, dataLenth);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildReadBoolRequest(const QString& address, ushort length)
{
	// ��ַ����
	auto addr = ModbusAddress::ParseAddress(address, ModbusAddress::READ_DISCRETE_INPUT, getUnitId());
	if (!addr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addr);
	// ����PDU:������(1)+��ַ(2)+λ����(2)
	QByteArray pdu;
	QDataStream stream(&pdu, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << addr.getContent0().functionCode
		<< addr.getContent0().address
		<< length;
	// ������������
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<QByteArray> ModbusTcpNet::ParseReadBoolResponse(const QByteArray& response)
{
	// ����У��
	if (response.size() < 9) return QICResult<QByteArray>::CreateFailedResult("Invalid response length");
	// У�鹦����:��鹦�����Ƿ�����쳣��־(��λΪ1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80) return QICResult<QByteArray>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(response[8], 2, 16, QChar('0')));
	// ��ȡ��Ч�غ�: FunctionCode(1) | DataLength(1) | Data(variable)
	quint8 dataLength = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, dataLength);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteRequest(const QString& address, const QByteArray& value)
{
	// �������ݳ���ȷ��������
	quint8 functionCode = (value.size() == 2) ? ModbusAddress::WRITE_SINGLE_REGISTER : ModbusAddress::WRITE_MULTIPLE_REGISTER;
	auto addr = ModbusAddress::ParseAddress(address, functionCode, getUnitId());
	if (!addr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addr);
	// ����PDU
	// WRITE_SINGLE_REGISTER:������(1)+��ַ(2)+Data(variable)
	// WRITE_MULTIPLE_REGISTER:������(1)+��ַ(2)+�Ĵ�����(2)+�ֽڳ���(1)+Data(variable)
	QByteArray pdu;
	QDataStream stream(&pdu, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << functionCode << addr.getContent0().address;
	if (functionCode == ModbusAddress::WRITE_MULTIPLE_REGISTER)
	{
		// ÿ���Ĵ���ռ��2�ֽ�
		stream << static_cast<quint16>(value.size() / 2)
			<< static_cast<quint8>(value.size());
	}
	pdu.append(value);
	// ������������
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<> ModbusTcpNet::ParseWriteResponse(const QByteArray& response)
{
	// MBAP(7)+������(1)+��ַ(2)+ֵ(2)
	if (response.size() < 12) return QICResult<>::CreateFailedResult("Invalid response length");
	// У�鹦����:��鹦�����Ƿ�����쳣��־(��λΪ1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80) return QICResult<>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(response[8], 2, 16, QChar('0')));
	// ��֤�������Ƿ�ƥ��
	if (functionCode != ModbusAddress::WRITE_SINGLE_REGISTER && functionCode != ModbusAddress::WRITE_MULTIPLE_REGISTER) return QICResult<>::CreateFailedResult("Function code mismath");
	return QICResult<>::CreateSuccessResult();
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteBoolRequest(const QString& address, const QVector<bool>& values)
{
	// ������Ȧ�����Զ��жϹ�����
	quint8 functionCode = (values.size() == 1) ? ModbusAddress::WRITE_SINGLE_COIL : ModbusAddress::WRITE_MULTIPLE_COIL;
	auto addr = ModbusAddress::ParseAddress(address, functionCode, getUnitId());
	if (!addr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addr);
	// ����PDU
	// WRITE_SINGLE_COIL:������(1)+��ַ(2)+Data(2)
	// WRITE_MULTIPLE_COIL:������(1)+��ַ(2)+��Ȧ��(2)+�ֽڳ���(1)+Data(variable)
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
	else QICResult<QByteArray>::CreateFailedResult("Invalid functiion code");
	// ������������
	QByteArray header = BuildMBAPHeader(pdu.size());
	return QICResult<QByteArray>::CreateSuccessResult(header + pdu);
}

QICResult<> ModbusTcpNet::ParseWriteBoolResponse(const QByteArray& response)
{
	// MBAP(7)+������(1)+��ַ(2)+ֵ(2)
	if (response.size() < 12) return QICResult<>::CreateFailedResult("Invalid response length");
	// У�鹦����:��鹦�����Ƿ�����쳣��־(��λΪ1)
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80) return QICResult<>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(response[8], 2, 16, QChar('0')));
	// ��֤�������Ƿ�ƥ��
	if (functionCode != ModbusAddress::WRITE_SINGLE_COIL && functionCode != ModbusAddress::WRITE_MULTIPLE_COIL) return QICResult<>::CreateFailedResult("Function code mismath");
	return QICResult<>::CreateSuccessResult();
}
