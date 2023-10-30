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
#include "NetworkDevice.h"

class KeyenceNanoSerialOverTcp : public NetworkDevice
{
	Q_OBJECT

public:
	KeyenceNanoSerialOverTcp(QString ipAddress, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut = 3000, int receiveTimeOut = 3000, QObject* parent = nullptr);
	~KeyenceNanoSerialOverTcp();

public:
	/// @brief 让基类中名为 Write 的所有函数在派生类中都是可见的，与派生类自己定义的 Write 函数重载（Overload），而不是被隐藏（Hide）
	using NetworkDevice::Write;
	using NetworkDevice::ReadBool;

	QICResult<QByteArray> Read(const QString& address, ushort length) override
	{
		// 构建读取数据包
		QICResult<QByteArray> buildResult = BuildReadPacket(address, length);
		if (!buildResult.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(buildResult);
		}
		// 从核心服务器读取
		QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.GetContent<0>());
		if (!readResult.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(readResult);
		}
		// 检查PLC写入响应协议格式
		QICResult checkResult = CheckPlcReadResponse(readResult.GetContent<0>());
		if (!checkResult.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(checkResult);
		}
		// 分析地址
		QICResult<QString, int> analysisResult = KvAnalysisAddress(address);
		if (!analysisResult.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(analysisResult);
		}

		// 解析数据
		return ParsedData(analysisResult.GetContent<0>(), readResult.GetContent<0>());
	}

	QICResult<QVector<bool>> ReadBool(const QString& address, ushort length) override
	{
		// 构建读取数据包
		auto buildResult = BuildReadPacket(address, length);
		if (!buildResult.IsSuccess)
		{
			return QICResult<QVector<bool>>::CreateFailedResult(buildResult);
		}
		// 从核心服务器读取
		auto readResult = ReadFromCoreServer(buildResult.GetContent<0>());
		if (!readResult.IsSuccess)
		{
			return QICResult<QVector<bool>>::CreateFailedResult(readResult);
		}
		// 检查PLC写入响应协议格式
		auto checkResult = CheckPlcReadResponse(readResult.GetContent<0>());
		if (!checkResult.IsSuccess)
		{
			return QICResult<QVector<bool>>::CreateFailedResult(checkResult);
		}
		// 地址分析
		auto addressResult = KvAnalysisAddress(address);
		if (!addressResult.IsSuccess)
		{
			return QICResult<QVector<bool>>::CreateFailedResult(addressResult);
		}

		// 解析数据
		return ParsedBoolData(addressResult.GetContent<0>(), readResult.GetContent<0>());
	}

	QICResult<> Write(const QString& address, const QByteArray& value) override
	{
		// 构建写入数据包
		QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
		if (!buildResult.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(buildResult);
		}
		// 从核心服务器读取响应
		QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.GetContent<0>());
		if (!readResult.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(readResult);
		}
		// 检查PLC写入响应协议格式
		QICResult<> checkResult = CheckPlcWriteResponse(readResult.GetContent<0>());
		if (!checkResult.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(checkResult);
		}

		return QICResult<>::CreateSuccessResult();
	}

	QICResult<> Write(const QString& address, bool value) override
	{
		// 构建写入数据包
		QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
		if (!buildResult.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(buildResult);
		}
		// 从核心服务器读取响应
		QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.GetContent<0>());
		if (!readResult.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(readResult);
		}
		// 检查PLC写入响应协议格式
		QICResult<> checkResult = CheckPlcWriteResponse(readResult.GetContent<0>());
		if (!checkResult.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(checkResult);
		}

		return QICResult<>::CreateSuccessResult();
	}

	friend QDebug operator<<(QDebug debug, const KeyenceNanoSerialOverTcp& kvNano)
	{
		// QDebugStateSaver保存了进入代码块时的QDebug状态，并在代码块结束时恢复了这个状态
		QDebugStateSaver saver(debug);
		debug << "KeyenceNanoSerialOverTcp [" << kvNano.ipAddress << ":" << kvNano.port << "]";
		return debug;
	}

protected:
	QICResult<> InitializationOnConnect(QTcpSocket* socket) override
	{
		QByteArray cmd = "CR\r";
		QICResult<QByteArray> result = ReadFromCoreServer(socket, cmd);
		if (!result.IsSuccess) return QICResult<>::CreateFailedResult(result);

		return QICResult<>::CreateSuccessResult();
	}

	QICResult<> ReleaseOnDisconnect(QTcpSocket* socket) override
	{
		QByteArray cmd = "CQ\r";
		QICResult<QByteArray> result = ReadFromCoreServer(socket, cmd);
		if (!result.IsSuccess) return QICResult<>::CreateFailedResult(result);

		return QICResult<>::CreateSuccessResult();
	}

private:
	/// @brief 校验回应包的协议格式
	/// @param ack 
	/// @return 
	QICResult<> CheckPlcReadResponse(const QByteArray& ack)
	{
		if (ack.isEmpty()) return QICResult<>::CreateFailedResult("接收的数据长度为0");
		// ASCII for 'E'
		if (ack.at(0) == 69) return QICResult<>::CreateFailedResult("PLC反馈信号错误：" + QString::fromStdString(ack.toStdString()));
		// ASCII for '\n' and '\r'
		if (ack.at(ack.size() - 1) != 10 || ack.at(ack.size() - 2) != 13) return QICResult<>::CreateFailedResult("PLC反馈信号错误：" + ack.toHex(' '));

		return QICResult<>::CreateSuccessResult();
	}

	/// @brief 校验回应包的协议格式
	/// @param ack 
	/// @return 
	QICResult<> CheckPlcWriteResponse(const QByteArray& ack)
	{
		if (ack.isEmpty()) return QICResult<>::CreateFailedResult("接收的数据长度为0");
		// 检查PLC的回应是否是'OK'
		if (ack.at(0) != 79 || ack.at(1) != 75) return QICResult<>::CreateFailedResult("PLC反馈信号错误：" + ack.toHex(' '));

		return QICResult<>::CreateSuccessResult();
	}

	/// @brief 构建读取数据包
	/// @param address 字符串地址
	/// @param length 读取的长度
	/// @return 读取数据包
	QICResult<QByteArray> BuildReadPacket(const QString& address, ushort length)
	{
		QICResult<QString, int> result = KvAnalysisAddress(address);
		if (!result.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(result);
		}

		if ((result.GetContent<0>() == "CTH" || result.GetContent<0>() == "CTC" ||
			result.GetContent<0>() == "C" || result.GetContent<0>() == "T") && length > 1)
		{
			length = static_cast<ushort>(length / 2);
		}

		QString packet = QString("RDS %1%2 %3\r")
			.arg(result.GetContent<0>())
			.arg(result.GetContent<1>())
			.arg(length);
		QByteArray bytes = packet.toLatin1();

		return QICResult<QByteArray>::CreateSuccessResult(bytes);
	}

	/// @brief 构建写入数据包
	/// @param address 字符串地址
	/// @param value 写入的值
	/// @return 写入数据包
	QICResult<QByteArray> BuildWritePacket(const QString& address, const QByteArray& value)
	{
		QICResult<QString, int> result = KvAnalysisAddress(address);
		if (!result.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(result);
		}

		QString packet;
		QTextStream stream(&packet);
		stream << "WRS " << result.GetContent<0>() << result.GetContent<1>() << " ";
		if (result.GetContent<0>() == "DM" ||
			result.GetContent<0>() == "CM" ||
			result.GetContent<0>() == "TM" ||
			result.GetContent<0>() == "EM" ||
			result.GetContent<0>() == "FM" ||
			result.GetContent<0>() == "Z")
		{
			int num = value.size() / 2;
			stream << num << " ";
			for (int i = 0; i < num; ++i)
			{
				quint16 val;
				memcpy(&val, value.constData() + i * 2, sizeof(quint16));
				stream << val;
				if (i != num - 1) stream << " ";
			}
		}
		else if (result.GetContent<0>() == "T" ||
			result.GetContent<0>() == "C" ||
			result.GetContent<0>() == "CTH")
		{
			int num = value.size() / 4;
			stream << num << " ";
			for (int i = 0; i < num; ++i)
			{
				quint32 val;
				memcpy(&val, value.constData() + i * 4, sizeof(quint32));
				stream << val;
				if (i != num - 1) stream << " ";
			}
		}
		stream << "\r";

		return QICResult<QByteArray>::CreateSuccessResult(packet.toLatin1());
	}

	/// @brief 构建写入数据包
	/// @param address 字符串地址
	/// @param value 写入的值
	/// @return 写入数据包
	QICResult<QByteArray> BuildWritePacket(const QString& address, bool value)
	{
		QICResult<QString, int> result = KvAnalysisAddress(address);
		if (!result.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(result);
		}

		QString packet;
		QTextStream stream(&packet);
		if (value) stream << "ST ";
		else stream << "RS ";
		stream << result.GetContent<0>() << result.GetContent<1>() << "\r";

		return QICResult<QByteArray>::CreateSuccessResult(packet.toLatin1());
	}

	/// @brief 解析数据
	/// @param addressType 地址类型
	/// @param response QTcpSocket读取的完整报文
	/// @return 携带纯数据部分的QICResult<QByteArray>
	QICResult<QByteArray> ParsedData(const QString& addressType, const QByteArray& response)
	{
		try
		{
			QByteArray tempData = response.left(response.size() - 2);
			QString stringData = QString::fromStdString(tempData.toStdString());
			QStringList strList = stringData.split(' ', QString::SkipEmptyParts);

			int num;
			if (addressType == "DM" ||
				addressType == "CM" ||
				addressType == "TM" ||
				addressType == "EM" ||
				addressType == "FM") num = 1;
			else if (addressType == "Z") num = 1;
			// 对于其他类型，初始化为0
			else num = 0;

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
					QStringList subList = strList[i].split(',', QString::SkipEmptyParts);
					quint32 value = subList[1].toUInt();
					memcpy(resultArray.data() + i * 4, &value, 4);
				}
				return QICResult<QByteArray>::CreateSuccessResult(resultArray);
			}

			return QICResult<QByteArray>::CreateFailedResult("输入的类型不支持，请重新输入");
		}
		catch (const QException& ex)
		{
			return QICResult<QByteArray>::CreateFailedResult("Extract Msg: " + QString(ex.what()) + "\nData: " + response.toHex(' '));
		}
	}

	/// @brief 解析Boolean数据
	/// @param addressType 地址类型
	/// @param response QTcpSocket读取的完整报文
	/// @return 携带纯数据部分的QICResult<QVector<bool>>
	QICResult<QVector<bool>> ParsedBoolData(const QString& addressType, const QByteArray& response)
	{
		try
		{
			QByteArray stringData = response.left(response.size() - 2);
			QString str = QString::fromUtf8(stringData);
			int num;
			if (addressType == "R" ||
				addressType == "CR" ||
				addressType == "MR") num = 1;
			else num = (addressType == "LR" ? 1 : 0);
			if (num != 0)
			{
				QStringList strList = str.split(' ', QString::SkipEmptyParts);
				QVector<bool> boolArray;
				for (const auto& item : strList)
				{
					boolArray.push_back(item == "1");
				}
				return QICResult<QVector<bool>>::CreateSuccessResult(boolArray);
			}

			int num2;
			if (addressType == "T" ||
				addressType == "C" ||
				addressType == "CTH") num2 = 1;
			else num2 = (addressType == "CTC" ? 1 : 0);
			if (num2 != 0)
			{
				QStringList strList = str.split(' ', QString::SkipEmptyParts);
				QVector<bool> boolArray;
				for (const auto& item : strList)
				{
					boolArray.push_back(item.startsWith("1"));
				}
				return QICResult<QVector<bool>>::CreateSuccessResult(boolArray);
			}

			return QICResult<QVector<bool>>::CreateFailedResult("输入的类型不支持，请重新输入");
		}
		catch (const std::exception& ex)
		{
			return QICResult<QVector<bool>>::CreateFailedResult("Extract Msg: " + QString(ex.what()) + "\nData: " + response.toHex(' '));
		}
	}

	/// @brief 分析地址信息
	/// @param address 
	/// @return 
	QICResult<QString, int> KvAnalysisAddress(QString address)
	{
		try
		{
			if (address.startsWith("CTH", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("CTH", address.mid(3).toInt());
			if (address.startsWith("CTC", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("CTC", address.mid(3).toInt());
			if (address.startsWith("CR", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("CR", address.mid(2).toInt());
			if (address.startsWith("MR", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("MR", address.mid(2).toInt());
			if (address.startsWith("LR", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("LR", address.mid(2).toInt());
			if (address.startsWith("DM", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("DM", address.mid(2).toInt());
			if (address.startsWith("CM", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("CM", address.mid(2).toInt());
			if (address.startsWith("TM", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("TM", address.mid(2).toInt());
			if (address.startsWith("EM", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("EM", address.mid(2).toInt());
			if (address.startsWith("FM", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("FM", address.mid(2).toInt());
			if (address.startsWith("AT", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("AT", address.mid(2).toInt());
			if (address.startsWith("Z", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("Z", address.mid(1).toInt());
			if (address.startsWith("R", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("R", address.mid(1).toInt());
			if (address.startsWith("T", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("T", address.mid(1).toInt());
			if (address.startsWith("C", Qt::CaseInsensitive)) return QICResult<QString, int>::CreateSuccessResult("C", address.mid(1).toInt());

			throw QException();
		}
		catch (const QException& ex)
		{
			return QICResult<QString, int>::CreateFailedResult(ex.what());
		}
	}
};
