#pragma once

#include <QObject>
#include <QByteArray>
#include <QDataStream>
#include <QVector>
#include <QTextCodec>
#include "QICResult.h"
#include "ByteConverterBase.h"
#include "GetResultHelper.h"

class NetworkBase : public QObject
{
public:
	explicit NetworkBase(QObject* parent = nullptr)
		: WordLenght(1), ByteConverter(nullptr), QObject(parent)
	{
	}
	virtual ~NetworkBase()
	{
		if (ByteConverter) delete ByteConverter;
	}

public:
	virtual QICResult<QByteArray> Read(const QString& address, ushort length) = 0;
	virtual QICResult<> Write(const QString& address, const QByteArray& value) = 0;
	
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
		return QICResult<bool>::CreateSuccessResult(result.getContent<0>().at(0));
	}

	virtual QICResult<QVector<short>> ReadInt16(QString address, ushort length)
	{
#if 0 
		// 展开形式
		auto result = Read(address, length * wordLenght);
		if (!result.IsSuccess) return QICResult<QVector<short>>::CreateFailedResult(result);
		auto value = byteConverter->ConvertToInt16(result.GetContent<0>(), 0, length);
		return QICResult<QVector<short>>::CreateSuccessResult(value);
#else
		auto result = Read(address, length * WordLenght);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return ByteConverter->ConvertToInt16(byteArray, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<short>>(result, lambda);
#endif
	}
	QICResult<short> ReadInt16(const QString& address)
	{
#if 0
		// 展开形式
		auto result = ReadInt16(address, 1);
		if (!result.IsSuccess) return QICResult<short>::CreateFailedResult(result);
		auto content = result.GetContent<0>();
		return QICResult<short>::CreateSuccessResult(content.at(0));
#else
		auto result = ReadInt16(address, 1);
		return GetResultHelper::GetResultFromArray(result);
#endif
	}

	virtual QICResult<QVector<ushort>> ReadUInt16(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return ByteConverter->ConvertToUInt16(byteArray, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<ushort>>(result, lambda);
	}
	QICResult<ushort> ReadUInt16(const QString& address)
	{
		auto result = ReadUInt16(address, 1);
		return GetResultHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<int>> ReadInt32(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return ByteConverter->ConvertToInt32(byteArray, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<int>>(result, lambda);
	}
	QICResult<int> ReadInt32(const QString& address)
	{
		auto result = ReadInt32(address, 1);
		return GetResultHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<uint>> ReadUInt32(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return ByteConverter->ConvertToUInt32(byteArray, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<uint>>(result, lambda);
	}
	QICResult<uint> ReadUInt32(const QString& address)
	{
		auto result = ReadUInt32(address, 1);
		return GetResultHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<float>> ReadFloat(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& ba)
			{
				return ByteConverter->ConvertToFloat(ba, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<float>>(result, lambda);
	}
	QICResult<float> ReadFloat(const QString& address)
	{
		auto result = ReadFloat(address, 1);
		return GetResultHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<qint64>> ReadInt64(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return ByteConverter->ConvertToInt64(byteArray, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<qint64>>(result, lambda);
	}
	QICResult<qint64> ReadInt64(const QString& address)
	{
		auto result = ReadInt64(address, 1);
		return GetResultHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<quint64>> ReadUInt64(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray& ba)
			{
				return ByteConverter->ConvertToUInt64(ba, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<quint64>>(result, lambda);
	}
	QICResult<quint64> ReadUInt64(const QString& address)
	{
		auto result = ReadUInt64(address, 1);
		return GetResultHelper::GetResultFromArray(result);
	}

	virtual QICResult<QVector<double>> ReadDouble(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray& ba)
			{
				return ByteConverter->ConvertToDouble(ba, 0, length);
			};
		return GetResultHelper::GetResultFromBytes<QVector<double>>(result, lambda);
	}
	QICResult<double> ReadDouble(const QString& address)
	{
		auto result = ReadDouble(address, 1);
		return GetResultHelper::GetResultFromArray(result);
	}

	virtual QICResult<QString> ReadString(const QString& address, ushort length, QTextCodec* codec)
	{
		auto result = Read(address, length);
		auto lambda = [=](const QByteArray& ba)
			{
				return ByteConverter->ConvertToString(ba, 0, ba.length(), codec);
			};
		return GetResultHelper::GetResultFromBytes<QString>(result, lambda);
	}
	QICResult<QString> ReadString(const QString& address, ushort length) { return ReadString(address, length, QTextCodec::codecForName("ASCII")); }

	virtual QICResult<> Write(const QString& address, QVector<short> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, short value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<ushort> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, ushort value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<int> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, int value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<uint> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, uint value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<float> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, float value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<qint64> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> WriteInt64(const QString& address, qint64 value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<quint64> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, quint64 value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<double> value)
	{
		auto bytes = ByteConverter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, double value) { return Write(address, QVector{ value }); }
	
	virtual QICResult<> WriteString(const QString& address, QString value, QTextCodec* codec)
	{
		auto bytes = ByteConverter->PackByteArray(value, codec);
		if (WordLenght == 1)
		{
			// 扩展到偶数长度
			if (bytes.size() % 2 != 0) bytes.append('\0');
		}
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString& address, QString value) { return WriteString(address, value, QTextCodec::codecForName("ASCII")); }
	virtual QICResult<> WriteString(const QString& address, QString value, int length, QTextCodec* codec)
	{
		auto bytes = ByteConverter->PackByteArray(value, codec);
		if (WordLenght == 1)
		{
			// 扩展到偶数长度
			if (bytes.size() % 2 != 0) bytes.append('\0');
		}
		bytes.resize(length);
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString& address, QString value, int length) { return WriteString(address, value, length, QTextCodec::codecForName("ASCII")); }

public:
	ushort WordLenght;
	ByteConverterBase* ByteConverter;
};
