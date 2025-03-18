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

QICResult<QByteArray> ModbusTcpNet::BuildReadRequest(const QString& address, ushort length)
{
	// ��ַ����
	auto addr = ParseAddress(address);
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
	// ��ȡ��Ч�غ�
	quint8 byteCount = static_cast<quint8>(response[8]);
	auto responseData = response.mid(9, byteCount);
	return QICResult<QByteArray>::CreateSuccessResult(responseData);
}

QICResult<QByteArray> ModbusTcpNet::BuildWriteRequest(const QString& address, const QByteArray& value)
{
	// ��ַ����
	auto addrInfo = ParseAddress(address);
	if (!addrInfo.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addrInfo);
	// ����PDU
	QByteArray pdu;
	// ������: д��Ĵ���
	pdu.append(0x10);
	qToBigEndian(addrInfo.getContent0().address, pdu.data() + 1);
	quint16 regCount = value.size() / WordLenght;
	qToBigEndian(regCount, pdu.data() + 3);
	// �ֽ���
	pdu.append(static_cast<char>(regCount * WordLenght));
}

QICResult<ModbusAddress> ModbusTcpNet::ParseAddress(const QString& address)
{
	try
	{
		// ��λ������0-4����ֻռ1���ַ�
		QRegExp reg("^([0-4])(\\d+)(?:@(\\d+))?$");
		if (!reg.exactMatch(address)) return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format");
		// ������ַ
		QString typeStr = reg.cap(1);
		quint16 addr = address.toUShort();
		quint8 station = reg.cap(3).isEmpty() ? m_unitId : reg.cap(3).toUShort();
		// ȷ��������
		QMap<QString, quint8> typeMap = {
			{"0", 0x01},// ��Ȧ
			{"1", 0x02},// ��ɢ����
			{"3", 0x04},// ����Ĵ���
			{"4", 0x03},// ���ּĴ���
		};
		if (!typeMap.contains(typeStr)) return QICResult<ModbusAddress>::CreateFailedResult("Unspported address type");
		// ��ַƫ�ƴ���
		if (!m_addressStartWithZero && addr < 1) return QICResult<ModbusAddress>::CreateFailedResult("Address must be greater than 0");
		addr = m_addressStartWithZero ? addr : (addr - 1);
		return QICResult<ModbusAddress>::CreateSuccessResult({ addr, typeMap[typeStr], station });
	}
	catch (...)
	{
		return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format");
	}
}

