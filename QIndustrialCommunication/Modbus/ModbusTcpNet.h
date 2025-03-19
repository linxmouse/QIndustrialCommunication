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
	// 通过 EthernetDevice 继承
	QICResult<> InitializationOnConnect(QTcpSocket* socket) override;
	QICResult<> ReleaseOnDisconnect(QTcpSocket* socket) override;

	// 重写核心读写方法
	QICResult<QByteArray> Read(const QString& address, ushort length) override;
	QICResult<> Write(const QString& address, const QByteArray& value) override;
	using IEthernetIO::ReadBool;
	QICResult<QVector<bool>> ReadBool(const QString& address, ushort length) override;

#pragma region Properties
	quint8 getUnitId() const { return m_unitId; }
	void setUnitId(quint8 value) { m_unitId = value; }
#pragma endregion

protected:
	/// @brief 构建读请求数据包
	/// @param address 读取地址
	/// @param length 读取长度
	/// @return 读请求数据包载荷
	QICResult<QByteArray> BuildReadRequest(const QString& address, ushort length);
	/// @brief 解析响应数据包
	/// @param response 响应数据包
	/// @return 数据内容载荷
	QICResult<QByteArray> ParseReadResponse(const QByteArray& response);
	/// @brief 构建写请求数据包
	/// @param address 写地址
	/// @param value 写入值
	/// @return 写请求数据包载荷
	QICResult<QByteArray> BuildWriteRequest(const QString& address, const QByteArray& value);
	
private:
	// 事务标识符
	quint16 m_transactionId = 0;
	// 协议标识符
	const quint16 m_protocolId = 0x0000;
	// 默认设备标识符
	quint8 m_unitId = 0x01;

	/// @brief 生成Modbus应用协议头(MBAP)
	/// Transaction ID(2) | Protocol ID(2) | Length(2) | Unit Identifier(1) | Function Code(1) | Data(variable)
	/// @param pduLength Modbus协议数据单元长度(PDU Length)
	/// Function Code (1) | Data (variable)
	/// Function Code (1 字节)：
	///	0x01：读取离散量输入
	///	0x03：读取保持寄存器
	///	0x04：读取输入寄存器
	///	0x05：写单个线圈
	///	0x06：写单个寄存器
	///	0x0F：写多个线圈
	///	0x10：写多个寄存器
	/// @return MBAP数据
	QByteArray BuildMBAPHeader(quint16 pduLength)
	{
		QByteArray header;
		QDataStream stream(&header, QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::BigEndian);
		stream << m_transactionId++
			<< m_protocolId
			// pdu的长度+UnitId(1)
			<< static_cast<quint16>(pduLength + 1)
			<< m_unitId;
		return header;
	}
};