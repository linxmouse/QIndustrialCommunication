#pragma once

#include "EthernetDevice.h"
#include "ModbusAddress.h"
#include <QtMath>

class ModbusTcpNet : public EthernetDevice
{
	Q_OBJECT
	Q_PROPERTY(quint8 m_unitId READ getUnitId WRITE setUnitId)
	//Q_PROPERTY(DataFormat _dataFormat READ getDataFormat WRITE setDataFormat)
public:
	ModbusTcpNet(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut = 3000, int receiveTimeOut = 3000, QObject* parent = nullptr);
	~ModbusTcpNet();

public:
	// ͨ�� EthernetDevice �̳�
	QICResult<> InitializationOnConnect(QTcpSocket* socket) override;
	QICResult<> ReleaseOnDisconnect(QTcpSocket* socket) override;

	// ��д���Ķ�д����
	QICResult<QByteArray> Read(const QString& address, ushort length) override;
	// �û�������Ϊ ReadBool �����к������������ж��ǿɼ��ģ����������Լ������ ReadBool �������أ�Overload���������Ǳ����أ�Hide��
	using IEthernetIO::ReadBool;
	QICResult<QVector<bool>> ReadBool(const QString& address, ushort length) override;
	// �û�������Ϊ Write �����к������������ж��ǿɼ��ģ����������Լ������ Write �������أ�Overload���������Ǳ����أ�Hide��
	using IEthernetIO::Write;
	QICResult<> Write(const QString& address, const QByteArray& value) override;
	QICResult<> Write(const QString& address, const QVector<bool>& values) override;

#pragma region Properties
	quint8 getUnitId() const { return m_unitId; }
	void setUnitId(quint8 value) { m_unitId = value; }
	DataFormat getDataFormat() const { return BytesOrderPtr->dataFormat; }
	void setDataFormat(DataFormat value) { BytesOrderPtr->dataFormat = value; }
#pragma endregion

protected:
	/// @brief �������������ݰ�
	/// @param address ��ȡ��ַ
	/// @param length ��ȡ����
	/// @return ���������ݰ��غ�
	QICResult<QByteArray> BuildReadRequest(const QString& address, ushort length);
	/// @brief ������Ӧ���ݰ�
	/// @param response ��Ӧ���ݰ�
	/// @return ���������غ�
	QICResult<QByteArray> ParseReadResponse(const QByteArray& response);
	/// @brief ��������ɢ�������������ݰ�
	/// @param address ��ȡ��ַ
	/// @param length ��ȡ����
	/// @return ���������ݰ��غ�
	QICResult<QByteArray> BuildReadBoolRequest(const QString& address, ushort length);
	/// @brief ������Ӧ��ɢ���������ݰ�
	/// @param response ��Ӧ���ݰ�
	/// @return ���������غ�
	QICResult<QByteArray> ParseReadBoolResponse(const QByteArray& response);
	/// @brief ����д�Ĵ����������ݰ�
	/// @param address д��ַ
	/// @param value IEthernetIO::BytesOrderPtr::PackByteArray�ֽ�������
	/// @return дд�Ĵ����������ݰ��غ�
	QICResult<QByteArray> BuildWriteRequest(const QString& address, const QByteArray& value);
	/// @brief ������Ӧд�Ĵ����������ݰ�
	/// @param response ��Ӧ���ݰ�
	/// @return дд�Ĵ����Ƿ�ɹ��غ�
	QICResult<> ParseWriteResponse(const QByteArray& response);
	/// @brief ����д��Ȧ�������ݰ�
	/// @param address д��ַ
	/// @param value д��ֵ
	/// @return дд�Ĵ����������ݰ��غ�
	QICResult<QByteArray> BuildWriteBoolRequest(const QString& address, const QVector<bool>& values);
	/// @brief ������Ӧд��Ȧ�������ݰ�
	/// @param response ��Ӧ���ݰ�
	/// @return дд��Ȧ�Ƿ�ɹ��غ�
	QICResult<> ParseWriteBoolResponse(const QByteArray& response);

private:
	/// @brief ��QVector<bool>ת��ΪQByteArray
	/// @param values ���������ֵ
	/// @return ת����Ľ��
	QByteArray BoolListToByteArray(const QVector<bool>& values)
	{
		if (values.isEmpty()) return QByteArray();
		int size = qCeil(values.size() / 8.0);
		QByteArray bytes(size, 0);
		for (int i = 0; i < values.size(); i++)
		{
			if (values.at(i))
			{
				// ��ȡ��ǰ���ֽ�
				quint8 temp = static_cast<quint8>(bytes[i / 8]);
				// �����ֽڵ�λ
				temp |= (1 << (i % 8));
				// ���º���ֽ�д��
				bytes[i / 8] = static_cast<char>(temp);
			}
		}
		return bytes;
	}
	/// @brief ��QByteArrayת����QVector<bool>
	/// @param bytes �ֽ�����
	/// @return ת����Ľ��
	QVector<bool> ByteArrayToBoolList(const QByteArray& bytes)
	{
		if (bytes.isEmpty()) return QVector<bool>();
		int length = bytes.size() * 8;
		QVector<bool> result(length);
		for (int i = 0; i < length; i++)
		{
			result.append((bytes[i / 8] & (1 << (i % 8))) != 0);
		}
		return result;
	}
	
private:
	// �����ʶ��
	quint16 m_transactionId = 0;
	// Э���ʶ��
	const quint16 m_protocolId = 0x0000;
	// Ĭ���豸��ʶ��
	quint8 m_unitId = 0x01;

	/// @brief ����ModbusӦ��Э��ͷ(MBAP)
	/// TransactionID(2) | ProtocolID(2) | Length(2) | UnitIdentifier(1) | FunctionCode(1) | Data(variable)
	/// @param pduLength ModbusЭ�����ݵ�Ԫ����(PDU Length)
	/// Function Code(1) | Data(variable)
	/// Function Code(1)��
	///	0x01����ȡ��ɢ������
	///	0x03����ȡ���ּĴ���
	///	0x04����ȡ����Ĵ���
	///	0x05��д������Ȧ
	///	0x06��д�����Ĵ���
	///	0x0F��д�����Ȧ
	///	0x10��д����Ĵ���
	/// @return MBAP����
	QByteArray BuildMBAPHeader(quint16 pduLength)
	{
		QByteArray header;
		QDataStream stream(&header, QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::BigEndian);
		stream << m_transactionId++
			<< m_protocolId
			// pdu�ĳ���+UnitId(1)
			<< static_cast<quint16>(pduLength + 1)
			<< m_unitId;
		return header;
	}
};