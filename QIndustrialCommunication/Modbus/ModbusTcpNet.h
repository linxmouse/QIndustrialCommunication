#pragma once

#include "EthernetDevice.h"
#include "ModbusAddress.h"

class ModbusTcpNet : public EthernetDevice
{
	Q_OBJECT
	Q_PROPERTY(quint8 m_unitId READ getUnitId WRITE setUnitId)
public:
	ModbusTcpNet(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut = 3000, int receiveTimeOut = 3000, QObject* parent = nullptr);
	~ModbusTcpNet();

public:
	// ͨ�� EthernetDevice �̳�
	QICResult<> InitializationOnConnect(QTcpSocket* socket) override;
	QICResult<> ReleaseOnDisconnect(QTcpSocket* socket) override;

	// ��д���Ķ�д����
	QICResult<QByteArray> Read(const QString& address, ushort length) override;
	QICResult<> Write(const QString& address, const QByteArray& value) override;
	using IEthernetIO::ReadBool;
	QICResult<QVector<bool>> ReadBool(const QString& address, ushort length) override;

#pragma region Properties
	quint8 getUnitId() const { return m_unitId; }
	void setUnitId(quint8 value) { m_unitId = value; }
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
	/// @brief ����д�������ݰ�
	/// @param address д��ַ
	/// @param value д��ֵ
	/// @return д�������ݰ��غ�
	QICResult<QByteArray> BuildWriteRequest(const QString& address, const QByteArray& value);
	
private:
	// �����ʶ��
	quint16 m_transactionId = 0;
	// Э���ʶ��
	const quint16 m_protocolId = 0x0000;
	// Ĭ���豸��ʶ��
	quint8 m_unitId = 0x01;

	/// @brief ����ModbusӦ��Э��ͷ(MBAP)
	/// Transaction ID(2) | Protocol ID(2) | Length(2) | Unit Identifier(1) | Function Code(1) | Data(variable)
	/// @param pduLength ModbusЭ�����ݵ�Ԫ����(PDU Length)
	/// Function Code (1) | Data (variable)
	/// Function Code (1 �ֽ�)��
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