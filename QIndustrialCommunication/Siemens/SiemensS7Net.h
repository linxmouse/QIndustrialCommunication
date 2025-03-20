#pragma once

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QtMath>
#include "EthernetDevice.h"
#include "SiemensPLCS.h"
#include "S7Address.h"

class SiemensS7Net : public EthernetDevice
{
	Q_OBJECT
	Q_PROPERTY(quint8 plcRack READ getPlcRack WRITE setPlcRack)
	Q_PROPERTY(quint8 plcSlot READ getPlcSlot WRITE setPlcSlot)

public:
	explicit SiemensS7Net(SiemensPLCS siemens, const QString& ipAddr, QObject* parent = nullptr);
	~SiemensS7Net();

#pragma region Properties Implement
	quint8 getPlcRack() const { return plcRack; }
	void setPlcRack(quint8 value)
	{
		plcRack = value;
		plcHead1[21] = plcRack * 32 + plcSlot;
	}

	quint8 getPlcSlot() const { return plcSlot; }
	void setPlcSlot(quint8 value)
	{
		plcSlot = value;
		plcHead1[21] = plcRack * 32 + plcSlot;
	}

	void EnableSendRecvLog() { enableSendRecvLog = true; }
	void DisableSendRecvLog() { enableSendRecvLog = false; }
#pragma endregion

protected:
	QICResult<> InitializationOnConnect(QTcpSocket* socket) override;
	QICResult<> ReleaseOnDisconnect(QTcpSocket* socket) override;

private:
	void Initializetion(SiemensPLCS siemens, const QString& ipAddr);
	QICResult<> checkStartResult(QByteArray content);
	QICResult<> checkStopResult(QByteArray content);
	/**
	 * @brief 读取多个S7地址的数据
	 * @param addresses 需要读取的S7Address对象列表
	 * @return 包含读取结果的QICResult对象，成功则返回读取到的字节数组，失败则返回失败原因
	 */
	QICResult<QByteArray> Read(const QVector<S7Address>& addresses);
	/**
	 * @brief 向PLC写入数据
	 * @param bytes 数据内容
	 * @return 包含是否请求成功的QICResult对象，失败则返回失败原因
	 */
	QICResult<> WritePLC(const QByteArray& bytes);

public:
	QICResult<QString> ReadOrderNumber();
	QICResult<> HotStart();
	QICResult<> ColdStart();
	QICResult<> Stop();

	// 让基类中名为 Write 的所有函数在派生类中都是可见的，与派生类自己定义的 Write 函数重载（Overload），而不是被隐藏（Hide）
	using EthernetDevice::Write;

	QICResult<bool> ReadBool(const QString& address) override;
	QICResult<QByteArray> Read(const QString& address, ushort length) override;
	/**
	 * @brief 读取指定地址列表的内容
	 * @param addresses 需要读取的PLC地址列表
	 * @param length 对应每个地址的读取长度
	 * @return 包含读取结果的QICResult对象，成功则返回读取到的字节数组，失败则返回失败原因
	 */
	QICResult<QByteArray> Read(const QStringList& addresses, const QVector<quint16>& length);
	/**
	 * @brief 写入数据到S7地址
	 * @param address S7地址
	 * @param value 数据
	 * @return 包含是否写入成功的QICResult对象，失败则返回失败原因
	 */
	QICResult<> Write(const QString& address, const QByteArray& value) override;
	/**
	 * @brief 写入位到S7地址
	 * @param address S7地址
	 * @param value 位值
	 * @return 包含是否写入成功的QICResult对象，失败则返回失败原因
	 */
	QICResult<> Write(const QString& address, bool value) override;
	/**
	 * @brief 写入多个位到S7地址
	 * @param address S7地址
	 * @param values 位值数组
	 * @return 包含是否写入成功的QICResult对象，失败则返回失败原因
	 */
	QICResult<> Write(const QString& address, const QVector<bool>& values);
	/**
	 * @brief 构建用于读取多个S7地址的请求数据包
	 * @param addresses 需要读取的S7地址列表
	 * @return 包含请求数据包的QICResult对象，成功则返回请求的字节数组，失败则返回失败原因
	 */
	static QICResult<QByteArray> BuildReadRequest(const QVector<S7Address>& addresses);
	/**
	 * @brief 构建用于读取比特的请求数据包
	 * @param address 要读取的S7地址
	 * @return 包含请求数据包的QICResult对象，成功则返回请求的字节数组，失败则返回失败原因
	 */
	static QICResult<QByteArray> BuildReadBitRequest(const QString& address);
	/**
	 * @brief 构建用于写入S7地址的请求数据包
	 * @param address 要写入的S7地址
	 * @param data 要写入的数据
	 * @return 包含请求数据包的QICResult对象，成功则返回请求的字节数组，失败则返回失败原因
	 */
	static QICResult<QByteArray> BuildWriteRequest(const S7Address& address, const QByteArray& data);
	/**
	 * @brief 构建用于写入比特的请求数据包
	 * @param address 要写入的S7地址
	 * @param data 要写入的bool值
	 * @return 包含请求数据包的QICResult对象，成功则返回请求的字节数组，失败则返回失败原因
	 */
	static QICResult<QByteArray> BuildWriteBitRequest(const QString& address, bool value);

private:
	/**
	 * @brief 读取S7地址的位数据
	 * @param address 地址信息
	 * @return 包含读取结果的QICResult对象，成功则返回读取到的字节数组，失败则返回失败原因
	 */
	QICResult<QByteArray> ReadAddressBit(const QString& address);
	/**
	 * @brief 读取多个S7地址的数据。
	 * @param addresses 需要读取的S7地址列表。
	 * @return 包含读取结果的QICResult对象，成功则返回读取到的字节数组，失败则返回失败原因
	 */
	QICResult<QByteArray> ReadAddressData(const QVector<S7Address>& addresses);
	/**
	 * @brief 解析从PLC返回的读取字节数据
	 * @param addresses 读取的S7地址列表
	 * @param content 返回的字节数据
	 * @return 包含解析结果的QICResult对象，成功则返回解析后的字节数组，失败则返回失败原因
	 */
	static QICResult<QByteArray> ParseReadResponse(const QVector<S7Address>& addresses, const QByteArray& content);
	/**
	 * @brief 解析从PLC返回的读取字节数据
	 * @param address 要读取的S7地址
	 * @return 包含请求数据包的QICResult对象，成功则返回请求的字节数组，失败则返回失败原因
	 */
	static QICResult<QByteArray> ParseReadBitResponse(const QByteArray& content);
	/**
	 * @brief 解析从PLC返回的数据，从而判断写入是否成功
	 * @param content 从PLC返回的数据
	 * @return 包含是否请求成功的QICResult对象，失败则返回失败原因
	 */
	static QICResult<> ParseWriteResponse(const QByteArray& content);
	/**
	 * @brief 将QVector<bool>转换为QByteArray
	 * @param ba 布尔数组的值
	 * @return 转换后的结果
	 */
	QByteArray BoolVectorToByteArray(const QVector<bool>& values)
	{
		if (values.isEmpty()) return QByteArray();
		int size = qCeil(values.size() / 8.0);
		QByteArray result(size, 0);
		for (int i = 0; i < values.size(); i++)
		{
			if (values.at(i))
			{
				// 获取当前的字节
				quint8 currByte = static_cast<quint8>(result[i / 8]);
				// 更新字节的位
				currByte |= (1 << (i % 8));
				// 更新后的字节写回
				result[i / 8] = static_cast<char>(currByte);
			}
		}
		return result;
	}
	/**
	 * @brief 将QByteArray转换回QVector<bool>
	 * @param bytes 字节数据
	 * @return 转换后的结果
	 */
	QVector<bool> ByteArrayToBoolVector(const QByteArray& bytes)
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

public:
	int pduLength = 0;

private:
	QByteArray plcHead1;
	QByteArray plcHead2;
	QByteArray plcOrderNumber;
	QByteArray plcHead1_200smart;
	QByteArray plcHead2_200smart;
	QByteArray plcHead1_200;
	QByteArray plcHead2_200;
	QByteArray S7_STOP;
	QByteArray S7_HOT_START;
	QByteArray S7_COLD_START;

	quint8 plcRack = 0;
	quint8 plcSlot = 0;
	const quint8 pduStart = 40;
	const quint8 pduStop = 41;
	const quint8 pduAlreadyStarted = 2;
	const quint8 pduAlreadyStopped = 7;
	SiemensPLCS currentPlc = SiemensPLCS::S1200;
};
