#pragma once

#include <QObject>
#include <QByteArray>
#include <QDataStream>
#include <QVector>
#include <QTextCodec>
#include "QICResult.h"
#include "ByteConverterBase.h"
#include "ByteConverterHelper.h"

class NetworkDeviceBase : public QObject
{
public:
	explicit NetworkDeviceBase(QObject* parent = nullptr)
		: wordLenght(1), byteConverter(nullptr), QObject(parent)
	{
	}
	virtual ~NetworkDeviceBase()
	{
		if (byteConverter) delete byteConverter;
	}

public:
	virtual QICResult<QByteArray> Read(const QString& address, ushort length)
	{
		return QICResult<QByteArray>::CreateFailedResult("Not Supported.");
	}	
	virtual QICResult<> Write(const QString& address, const QByteArray& value)
	{
		return QICResult<>::CreateFailedResult("Not Supported.");
	}
	virtual QICResult<> Write(const QString& address, QVector<bool> value)
	{
		return QICResult<>::CreateFailedResult("Not Supported.");
	}
	virtual QICResult<> Write(const QString& address, bool value)
	{
		return Write(address, QVector<bool>{value});
	}

	virtual QICResult<QVector<bool>> ReadBool(const QString& address, ushort length)
	{
		return QICResult<QVector<bool>>::CreateFailedResult("Not Supported.");
	}
	virtual QICResult<bool> ReadBool(const QString& address)
	{
		auto result = ReadBool(address, 1);
		if (!result.IsSuccess) return QICResult<bool>::CreateFailedResult(result);
		return QICResult<bool>::CreateSuccessResult(result.GetContent1().at(0));
	}

	virtual QICResult<QVector<short>> ReadInt16(QString address, ushort length)
	{
#if 0 // ByteConverterHelper::GetResultFromBytes展开形式
		auto result = Read(address, length * wordLenght);
		if (!result.IsSuccess) return QICResult<QVector<short>>::CreateFailedResult(result);
		auto value = byteConverter->ConvertToInt16(result.GetContent1(), 0, length);
		return QICResult<QVector<short>>::CreateSuccessResult(value);
#endif // ByteConverterHelper::GetResultFromBytes展开形式
		auto result = Read(address, length * wordLenght);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return byteConverter->ConvertToInt16(byteArray, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<short>>(result, lambda);
	}
	QICResult<short> ReadInt16(const QString& address)
	{
		auto result = ReadInt16(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<ushort>> ReadUInt16(const QString& address, ushort length)
	{
		auto result = Read(address, length * wordLenght);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return byteConverter->ConvertToUInt16(byteArray, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<ushort>>(result, lambda);
	}
	QICResult<ushort> ReadUInt16(const QString& address)
	{
		auto result = ReadUInt16(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<int>> ReadInt32(const QString& address, ushort length)
	{
		auto result = Read(address, length * wordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return byteConverter->ConvertToInt32(byteArray, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<int>>(result, lambda);
	}
	QICResult<int> ReadInt32(const QString& address)
	{
		auto result = ReadInt32(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<uint>> ReadUInt32(const QString& address, ushort length)
	{
		auto result = Read(address, length * wordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return byteConverter->ConvertToUInt32(byteArray, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<uint>>(result, lambda);
	}
	QICResult<uint> ReadUInt32(const QString& address)
	{
		auto result = ReadUInt32(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<float>> ReadFloat(const QString& address, ushort length)
	{
		auto result = Read(address, length * wordLenght * 2);
		auto lambda = [=](const QByteArray& ba)
			{
				return byteConverter->ConvertToFloat(ba, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<float>>(result, lambda);
	}
	QICResult<float> ReadFloat(const QString& address)
	{
		auto result = ReadFloat(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<qint64>> ReadInt64(const QString& address, ushort length)
	{
		auto result = Read(address, length * wordLenght * 4);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return byteConverter->ConvertToInt64(byteArray, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<qint64>>(result, lambda);
	}
	QICResult<qint64> ReadInt64(const QString& address)
	{
		auto result = ReadInt64(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<quint64>> ReadUInt64(const QString& address, ushort length)
	{
		auto result = Read(address, length * wordLenght * 4);
		auto lambda = [=](const QByteArray& ba)
			{
				return byteConverter->ConvertToUInt64(ba, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<quint64>>(result, lambda);
	}
	QICResult<quint64> ReadUInt64(const QString& address)
	{
		auto result = ReadUInt64(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<double>> ReadDouble(const QString& address, ushort length)
	{
		auto result = Read(address, length * wordLenght * 4);
		auto lambda = [=](const QByteArray& ba)
			{
				return byteConverter->ConvertToDouble(ba, 0, length);
			};
		return ByteConverterHelper::GetResultFromBytes<QVector<double>>(result, lambda);
	}
	QICResult<double> ReadDouble(const QString& address)
	{
		auto result = ReadDouble(address, 1);
		return ByteConverterHelper::GetResultFromArray(result);
	}

	virtual QICResult<QString> ReadString(const QString& address, ushort length, QTextCodec* codec)
	{
		auto result = Read(address, length);
		auto lambda = [=](const QByteArray& ba)
			{
				return byteConverter->ConvertToString(ba, 0, ba.length(), codec);
			};
		return ByteConverterHelper::GetResultFromBytes<QString>(result, lambda);
	}
	QICResult<QString> ReadString(const QString& address, ushort length) { return ReadString(address, length, QTextCodec::codecForName("ASCII")); }

	virtual QICResult<> Write(const QString& address, QVector<short> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, short value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<ushort> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, ushort value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<int> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, int value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<uint> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, uint value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<float> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, float value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<qint64> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> WriteInt64(const QString& address, qint64 value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<quint64> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, quint64 value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<double> value)
	{
		auto bytes = byteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, double value) { return Write(address, QVector{ value }); }
	
	virtual QICResult<> WriteString(const QString& address, QString value, QTextCodec* codec)
	{
		auto bytes = byteConverter->PackByteArray(value, codec);
		if (wordLenght == 1)
		{
			// 扩展到偶数长度
			if (bytes.size() % 2 != 0) bytes.append('\0');
		}
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString& address, QString value) { return WriteString(address, value, QTextCodec::codecForName("ASCII")); }
	virtual QICResult<> WriteString(const QString& address, QString value, int length, QTextCodec* codec)
	{
		auto bytes = byteConverter->PackByteArray(value, codec);
		if (wordLenght == 1)
		{
			// 扩展到偶数长度
			if (bytes.size() % 2 != 0) bytes.append('\0');
		}
		bytes.resize(length);
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString& address, QString value, int length) { return WriteString(address, value, length, QTextCodec::codecForName("ASCII")); }

protected:
	/// @brief 判断当前平台是否为小端
	/// @return 小端如果为true，大端如果为false
	bool isLittleEndian()
	{
		quint16 number = 1;
		quint8* numPtr = (quint8*)&number;
		return (numPtr[0] == 1);
	}

protected:
	ushort wordLenght;
	ByteConverterBase* byteConverter;
};
