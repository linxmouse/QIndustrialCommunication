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
	// 通过 EthernetDevice 继承
	QICResult<> InitializationOnConnect(QTcpSocket* socket) override;
	QICResult<> ReleaseOnDisconnect(QTcpSocket* socket) override;

	// 重写核心读写方法
	QICResult<QByteArray> Read(const QString& address, ushort length) override;
	// 让基类中名为 ReadBool 的所有函数在派生类中都是可见的，与派生类自己定义的 ReadBool 函数重载（Overload），而不是被隐藏（Hide）
	using IEthernetIO::ReadBool;
	QICResult<QVector<bool>> ReadBool(const QString& address, ushort length) override;
	// 让基类中名为 Write 的所有函数在派生类中都是可见的，与派生类自己定义的 Write 函数重载（Overload），而不是被隐藏（Hide）
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
	/// @brief 构建读请求数据包
	/// @param address 读取地址
	/// @param length 读取长度
	/// @return 读请求数据包载荷
	QICResult<QByteArray> BuildReadRequest(const QString& address, ushort length);
	/// @brief 解析响应数据包
	/// @param response 响应数据包
	/// @return 数据内容载荷
	QICResult<QByteArray> ParseReadResponse(const QByteArray& response);
	/// @brief 构建读离散量输入请求数据包
	/// @param address 读取地址
	/// @param length 读取长度
	/// @return 读请求数据包载荷
	QICResult<QByteArray> BuildReadBoolRequest(const QString& address, ushort length);
	/// @brief 解析响应离散量输入数据包
	/// @param response 响应数据包
	/// @return 数据内容载荷
	QICResult<QByteArray> ParseReadBoolResponse(const QByteArray& response);
	/// @brief 构建写寄存器请求数据包
	/// @param address 写地址
	/// @param value IEthernetIO::BytesOrderPtr::PackByteArray字节序数据
	/// @return 写写寄存器请求数据包载荷
	QICResult<QByteArray> BuildWriteRequest(const QString& address, const QByteArray& value);
	/// @brief 解析响应写寄存器请求数据包
	/// @param response 响应数据包
	/// @return 写写寄存器是否成功载荷
	QICResult<> ParseWriteResponse(const QByteArray& response);
	/// @brief 构建写线圈请求数据包
	/// @param address 写地址
	/// @param value 写入值
	/// @return 写写寄存器请求数据包载荷
	QICResult<QByteArray> BuildWriteBoolRequest(const QString& address, const QVector<bool>& values);
	/// @brief 解析响应写线圈请求数据包
	/// @param response 响应数据包
	/// @return 写写线圈是否成功载荷
	QICResult<> ParseWriteBoolResponse(const QByteArray& response);

private:
	/// @brief 将QVector<bool>转换为QByteArray
	/// @param values 布尔数组的值
	/// @return 转换后的结果
	QByteArray BoolListToByteArray(const QVector<bool>& values)
	{
		if (values.isEmpty()) return QByteArray();
		int size = qCeil(values.size() / 8.0);
		QByteArray bytes(size, 0);
		for (int i = 0; i < values.size(); i++)
		{
			if (values.at(i))
			{
				// 获取当前的字节
				quint8 temp = static_cast<quint8>(bytes[i / 8]);
				// 更新字节的位
				temp |= (1 << (i % 8));
				// 更新后的字节写回
				bytes[i / 8] = static_cast<char>(temp);
			}
		}
		return bytes;
	}
	/// @brief 将QByteArray转换回QVector<bool>
	/// @param bytes 字节数据
	/// @return 转换后的结果
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
	// 事务标识符
	quint16 m_transactionId = 0;
	// 协议标识符
	const quint16 m_protocolId = 0x0000;
	// 默认设备标识符
	quint8 m_unitId = 0x01;

	/// @brief 生成Modbus应用协议头(MBAP)
	/// TransactionID(2) | ProtocolID(2) | Length(2) | UnitIdentifier(1) | FunctionCode(1) | Data(variable)
	/// @param pduLength Modbus协议数据单元长度(PDU Length)
	/// Function Code(1) | Data(variable)
	/// Function Code(1)：
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