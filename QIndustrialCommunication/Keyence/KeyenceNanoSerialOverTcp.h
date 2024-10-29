#pragma once

#include <QObject>
#include <QException>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QEventLoop>
#include <QTimer>
#include <QVector>
#include <QDataStream>
#include <QDebug>
#include <QHostAddress>
#include "QICResult.h"
#include "EthernetDevice.h"

class KeyenceNanoSerialOverTcp : public EthernetDevice
{
	Q_OBJECT

public:
	KeyenceNanoSerialOverTcp(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog,
							 int connectTimeOut = 3000, int receiveTimeOut = 3000, QObject *parent = nullptr);
	~KeyenceNanoSerialOverTcp();

public:
	// 让基类中名为 Write 的所有函数在派生类中都是可见的，与派生类自己定义的 Write 函数重载（Overload），而不是被隐藏（Hide）
	using EthernetDevice::Write;
	// 让基类中名为 ReadBool 的所有函数在派生类中都是可见的，与派生类自己定义的 ReadBool 函数重载（Overload），而不是被隐藏（Hide）
	using EthernetDevice::ReadBool;

	QICResult<QByteArray> Read(const QString &address, ushort length) override;

	QICResult<QVector<bool>> ReadBool(const QString &address, ushort length) override;

	QICResult<> Write(const QString &address, const QByteArray &value) override;

	QICResult<> Write(const QString &address, bool value) override;

	QICResult<> Write(const QString &address, const QVector<bool>& values) override;

	friend QDebug operator<<(QDebug debug, const KeyenceNanoSerialOverTcp &kvNano)
	{
		// QDebugStateSaver保存了进入代码块时的QDebug状态，并在代码块结束时恢复了这个状态
		QDebugStateSaver saver(debug);
		debug << "KeyenceNanoSerialOverTcp [" << kvNano.ipAddr << ":" << kvNano.port << "]";
		return debug;
	}

protected:
	QICResult<> InitializationOnConnect(QTcpSocket *socket) override
	{
		if (!socket || !socket->isValid())
			return QICResult<>::CreateFailedResult("socket is null/invalid.");
		QByteArray cmd = "CR\r";
		QICResult<QByteArray> result = ReadFromCoreServer(socket, cmd);
		if (!result.IsSuccess)
			return QICResult<>::CreateFailedResult(result);
		return QICResult<>::CreateSuccessResult();
	}

	QICResult<> ReleaseOnDisconnect(QTcpSocket *socket) override
	{
		if (!socket || !socket->isValid())
			return QICResult<>::CreateFailedResult("socket is null/invalid.");
		QByteArray cmd = "CQ\r";
		QICResult<QByteArray> result = ReadFromCoreServer(socket, cmd);
		if (!result.IsSuccess)
			return QICResult<>::CreateFailedResult(result);

		return QICResult<>::CreateSuccessResult();
	}

private:
	/// @brief 校验回应包的协议格式
	/// @param ack
	/// @return
	QICResult<> CheckPlcReadResponse(const QByteArray &ack)
	{
		if (ack.isEmpty())
			return QICResult<>::CreateFailedResult(QString::fromLocal8Bit("接收的数据长度为0"));
		// ASCII for 'E'
		if (ack.at(0) == 69)
			return QICResult<>::CreateFailedResult(QString::fromLocal8Bit("PLC反馈信号错误：") + QString::fromStdString(ack.toStdString()));
		// ASCII for '\n' and '\r'
		if (ack.at(ack.size() - 1) != 10 || ack.at(ack.size() - 2) != 13)
			return QICResult<>::CreateFailedResult(QString::fromLocal8Bit("PLC反馈信号错误：") + ack.toHex(' '));

		return QICResult<>::CreateSuccessResult();
	}

	/// @brief 校验回应包的协议格式
	/// @param ack
	/// @return
	QICResult<> CheckPlcWriteResponse(const QByteArray &ack)
	{
		if (ack.isEmpty())
			return QICResult<>::CreateFailedResult(QString::fromLocal8Bit("接收的数据长度为0"));
		// 检查PLC的回应是否是'OK'
		if (ack.at(0) != 79 || ack.at(1) != 75)
			return QICResult<>::CreateFailedResult(QString::fromLocal8Bit("PLC反馈信号错误：") + ack.toHex(' '));

		return QICResult<>::CreateSuccessResult();
	}

	/// @brief 构建读取数据包
	/// @param address 字符串地址
	/// @param length 读取的长度
	/// @return 读取数据包
	QICResult<QByteArray> GenReadBytes(const QString &address, ushort length)
	{
		QICResult<QString, int> result = ParseAddress(address);
		if (!result.IsSuccess)
			return QICResult<QByteArray>::CreateFailedResult(result);
		if ((result.getContent0() == "CTH" || result.getContent0() == "CTC" ||
			 result.getContent0() == "C" || result.getContent0() == "T") &&
			length > 1)
		{
			length = static_cast<ushort>(length / 2);
		}
		QString packet = QString("RDS %1%2 %3\r")
							 .arg(result.getContent0())
							 .arg(result.getContent1())
							 .arg(length);
		QByteArray bytes = packet.toLatin1();
		return QICResult<QByteArray>::CreateSuccessResult(bytes);
	}

	/// @brief 构建写入数据包
	/// @param address 字符串地址
	/// @param value 写入的值
	/// @return 写入数据包
	QICResult<QByteArray> GenWriteBytes(const QString &address, const QByteArray &value)
	{
		QICResult<QString, int> result = ParseAddress(address);
		if (!result.IsSuccess)
			return QICResult<QByteArray>::CreateFailedResult(result);
		QString packet;
		QTextStream stream(&packet);
		stream << "WRS " << result.getContent0() << result.getContent1() << " ";
		if (result.getContent0() == "DM" ||
			result.getContent0() == "CM" ||
			result.getContent0() == "TM" ||
			result.getContent0() == "EM" ||
			result.getContent0() == "FM" ||
			result.getContent0() == "Z")
		{
			int num = value.size() / 2;
			stream << num << " ";
			for (int i = 0; i < num; ++i)
			{
				quint16 val;
				memcpy(&val, value.constData() + i * 2, sizeof(quint16));
				stream << val;
				if (i != num - 1)
					stream << " ";
			}
		}
		else if (result.getContent0() == "T" ||
				 result.getContent0() == "C" ||
				 result.getContent0() == "CTH")
		{
			int num = value.size() / 4;
			stream << num << " ";
			for (int i = 0; i < num; ++i)
			{
				quint32 val;
				memcpy(&val, value.constData() + i * 4, sizeof(quint32));
				stream << val;
				if (i != num - 1)
					stream << " ";
			}
		}
		stream << "\r";
		return QICResult<QByteArray>::CreateSuccessResult(packet.toLatin1());
	}

	/// @brief 构建写入数据包
	/// @param address 字符串地址
	/// @param value 写入的值
	/// @return 写入数据包
	QICResult<QByteArray> GenWriteBytes(const QString &address, bool value)
	{
		QICResult<QString, int> result = ParseAddress(address);
		if (!result.IsSuccess)
			return QICResult<QByteArray>::CreateFailedResult(result);
		QString packet;
		QTextStream stream(&packet);
		if (value)
			stream << "ST ";
		else
			stream << "RS ";
		stream << result.getContent0() << result.getContent1() << "\r";
		return QICResult<QByteArray>::CreateSuccessResult(packet.toLatin1());
	}

	/// @brief 构建写入数据包
	/// @param address 字符串地址
	/// @param value 写入的值
	/// @return 写入数据包
	QICResult<QByteArray> GenWriteBytes(const QString &address, QVector<bool> values)
	{
		QICResult<QString, int> result = ParseAddress(address);
		if (!result.IsSuccess)
			return QICResult<QByteArray>::CreateFailedResult(result);
		QString packet;
		QTextStream stream(&packet);
		stream << "WRS "
			   << result.getContent0()
			   << result.getContent1()
			   << " "
			   << values.length();
		for (auto &value : values)
		{
			auto tf = value ? "1" : "0";
			stream << " " << tf;
		}
		stream << "\r";
		return QICResult<QByteArray>::CreateSuccessResult(packet.toLatin1());
	}

	/// @brief 解析数据
	/// @param addressType 地址类型
	/// @param response QTcpSocket读取的完整报文
	/// @return 携带纯数据部分的QICResult<QByteArray>
	QICResult<QByteArray> ParsedData(const QString &addressType, const QByteArray &response)
	{
		try
		{
			QByteArray tempData = response.left(response.size() - 2);
			QString stringData = QString::fromStdString(tempData.toStdString());
			QStringList strList = stringData.split(' ', Qt::SkipEmptyParts);

			int num;
			if (addressType == "DM" ||
				addressType == "CM" ||
				addressType == "TM" ||
				addressType == "EM" ||
				addressType == "FM")
				num = 1;
			else if (addressType == "Z")
				num = 1;
			// 对于其他类型，初始化为0
			else
				num = 0;

			QByteArray resultArray;
			if (num != 0)
			{
				resultArray.resize(strList.size() * 2);
				for (int i = 0; i < strList.size(); ++i)
				{
					quint16 value = strList[i].toUShort();
					memcpy(resultArray.data() + i * 2, &value, 2);
				}
				return QICResult<QByteArray>::CreateSuccessResult(resultArray);
			}

			if (addressType == "AT" ||
				addressType == "CTC" ||
				addressType == "T" ||
				addressType == "C" ||
				addressType == "CTH")
			{
				resultArray.resize(strList.size() * 4);
				for (int i = 0; i < strList.size(); ++i)
				{
					QStringList subList = strList[i].split(',', Qt::SkipEmptyParts);
					quint32 value = subList[1].toUInt();
					memcpy(resultArray.data() + i * 4, &value, 4);
				}
				return QICResult<QByteArray>::CreateSuccessResult(resultArray);
			}

			return QICResult<QByteArray>::CreateFailedResult("输入的类型不支持，请重新输入");
		}
		catch (const QException &ex)
		{
			return QICResult<QByteArray>::CreateFailedResult("Extract Msg: " + QString(ex.what()) + "\nData: " + response.toHex(' '));
		}
	}

	/// @brief 解析Boolean数据
	/// @param addressType 地址类型
	/// @param response QTcpSocket读取的完整报文
	/// @return 携带纯数据部分的QICResult<QVector<bool>>
	QICResult<QVector<bool>> ParsedBoolData(const QString &addressType, const QByteArray &response)
	{
		try
		{
			QByteArray stringData = response.left(response.size() - 2);
			QString str = QString::fromUtf8(stringData);
			int num;
			if (addressType == "R" ||
				addressType == "CR" ||
				addressType == "MR")
				num = 1;
			else
				num = (addressType == "LR" ? 1 : 0);
			if (num != 0)
			{
				QStringList strList = str.split(' ', Qt::SkipEmptyParts);
				QVector<bool> boolArray;
				for (const auto &item : strList)
				{
					boolArray.push_back(item == "1");
				}
				return QICResult<QVector<bool>>::CreateSuccessResult(boolArray);
			}

			int num2;
			if (addressType == "T" ||
				addressType == "C" ||
				addressType == "CTH")
				num2 = 1;
			else
				num2 = (addressType == "CTC" ? 1 : 0);
			if (num2 != 0)
			{
				QStringList strList = str.split(' ', Qt::SkipEmptyParts);
				QVector<bool> boolArray;
				for (const auto &item : strList)
				{
					boolArray.push_back(item.startsWith("1"));
				}
				return QICResult<QVector<bool>>::CreateSuccessResult(boolArray);
			}

			return QICResult<QVector<bool>>::CreateFailedResult("输入的类型不支持，请重新输入");
		}
		catch (const std::exception &ex)
		{
			return QICResult<QVector<bool>>::CreateFailedResult("Extract Msg: " + QString(ex.what()) + "\nData: " + response.toHex(' '));
		}
	}

	/// @brief 分析地址信息
	/// @param address
	/// @return
	QICResult<QString, int> ParseAddress(QString address)
	{
		try
		{
			if (address.startsWith("CTH", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("CTH", address.mid(3).toInt());
			if (address.startsWith("CTC", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("CTC", address.mid(3).toInt());
			if (address.startsWith("CR", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("CR", address.mid(2).toInt());
			if (address.startsWith("MR", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("MR", address.mid(2).toInt());
			if (address.startsWith("LR", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("LR", address.mid(2).toInt());
			if (address.startsWith("DM", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("DM", address.mid(2).toInt());
			if (address.startsWith("CM", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("CM", address.mid(2).toInt());
			if (address.startsWith("TM", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("TM", address.mid(2).toInt());
			if (address.startsWith("EM", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("EM", address.mid(2).toInt());
			if (address.startsWith("FM", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("FM", address.mid(2).toInt());
			if (address.startsWith("AT", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("AT", address.mid(2).toInt());
			if (address.startsWith("Z", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("Z", address.mid(1).toInt());
			if (address.startsWith("R", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("R", address.mid(1).toInt());
			if (address.startsWith("T", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("T", address.mid(1).toInt());
			if (address.startsWith("C", Qt::CaseInsensitive))
				return QICResult<QString, int>::CreateSuccessResult("C", address.mid(1).toInt());

			throw QException();
		}
		catch (const QException &ex)
		{
			return QICResult<QString, int>::CreateFailedResult(ex.what());
		}
	}
};
