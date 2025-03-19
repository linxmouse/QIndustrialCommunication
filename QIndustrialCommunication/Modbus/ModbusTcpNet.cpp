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
	// ��ַ����
	quint16 startAddr = address.mid(1).toUShort() - 40001;

	// ��������ͷ
	QByteArray pdu;
	// ������: д����Ĵ���
	pdu.append(0x10);
	// ��ʼ��ַ
	pdu.append(reinterpret_cast<char*>(&startAddr), 2);
	// �Ĵ�������
	pdu.append(static_cast<char>(value.size() / WordLenght));

	// �������
	QByteArray formattedData = BytesOrderPtr->PackByteArray(value, 0, value.size());
	pdu.append(formattedData);

	// ����������֤��Ӧ
	QByteArray request = BuildMBAPHeader(pdu.size()) + pdu;
	auto response = ReadFromCoreServer(request);

	// ��֤д����
	// �ɹ���ӦӦ������ͬ��ַ������
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
	// ��ַ����
	auto addr = ModbusAddress::ParseAddress(address, 0x03, m_unitId);
	if (!addr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addr);
	// ����PDU:������(1)+��ַ(2)+����(2)
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
	// У�鹦����
	quint8 functionCode = static_cast<quint8>(response[7]);
	if (functionCode & 0x80) return QICResult<QByteArray>::CreateFailedResult(QString("Modbus exception code: 0x%1").arg(response[8], 2, 16, QChar('0')));
	// ��ȡ��Ч�غ�: FunctionCode(1) | DataLength(1) | Data (variable)
	quint8 dataLenth = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, dataLenth);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteRequest(const QString& address, const QByteArray& value)
{
	return QICResult<QByteArray>::CreateFailedResult("Not Implement");
}
