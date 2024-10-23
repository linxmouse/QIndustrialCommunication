#pragma once

#include <QObject>
#include <QByteArray>
#include <QDataStream>
#include <QVector>
#include <QTextCodec>
#include <QScopedPointer>
#include "QICResult.h"
#include "BytesOrderBase.h"
#include "QICResultTranslator.h"

class IEthernetIO : public QObject
{
public:
	explicit IEthernetIO(QObject *parent = nullptr)
		: WordLenght(1), QObject(parent)
	{
	}
	virtual ~IEthernetIO()
	{
	}

public:
	virtual QICResult<QByteArray> Read(const QString &address, ushort length) = 0;
	virtual QICResult<> Write(const QString &address, const QByteArray &value) = 0;

	virtual QICResult<> Write(const QString &address, QVector<bool> value)
	{
		return QICResult<>::CreateFailedResult("Not Supported.");
	}
	virtual QICResult<> Write(const QString &address, bool value)
	{
		return Write(address, QVector<bool>{value});
	}

	virtual QICResult<QVector<bool>> ReadBool(const QString &address, ushort length)
	{
		return QICResult<QVector<bool>>::CreateFailedResult("Not Supported.");
	}
	virtual QICResult<bool> ReadBool(const QString &address)
	{
		auto result = ReadBool(address, 1);
		if (!result.IsSuccess) return QICResult<bool>::CreateFailedResult(result);
		return QICResult<bool>::CreateSuccessResult(result.getContent<0>().at(0));
	}

	virtual QICResult<QVector<short>> ReadInt16(QString address, ushort length)
	{
#if 1 // 不使用QICResultTranslator
		
		auto result = Read(address, length * WordLenght);
		if (!result.IsSuccess) return QICResult<QVector<short>>::CreateFailedResult(result);
		auto value = BytesOrderPtr->ConvertToInt16(result.getContent<0>(), 0, length);
		return QICResult<QVector<short>>::CreateSuccessResult(value);
#else
		auto result = Read(address, length * WordLenght);
		auto lambda = [=](const QByteArray &byteArray)
		{
			return BytesOrderPtr->ConvertToInt16(byteArray, 0, length);
		};
		return QICResultTranslator::GetResultFromBytes<QVector<short>>(result, lambda);
#endif
	}
	QICResult<short> ReadInt16(const QString &address)
	{
#if 1 // 不使用QICResultTranslator
		auto result = ReadInt16(address, 1);
		if (!result.IsSuccess) return QICResult<short>::CreateFailedResult(result);
		auto content = result.getContent<0>();
		return QICResult<short>::CreateSuccessResult(content.at(0));
#else
		auto result = ReadInt16(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
#endif
	}

	virtual QICResult<QVector<ushort>> ReadUInt16(const QString &address, ushort length)
	{
#if 1 // 不使用QICResultTranslator
		auto result = Read(address, length * WordLenght);
		if (!result.IsSuccess) return QICResult<QVector<ushort>>::CreateFailedResult(result);
		auto fmtedResult = BytesOrderPtr->ConvertToUInt16(result.getContent<0>(), 0, length);
		return QICResult<QVector<ushort>>::CreateSuccessResult(fmtedResult);
#else
		auto result = Read(address, length * WordLenght);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return BytesOrderPtr->ConvertToUInt16(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<ushort>>(result, lambda);
#endif // 1

	}
	QICResult<ushort> ReadUInt16(const QString &address)
	{
#if 1 // 不使用QICResultTranslator
		auto result = ReadUInt16(address, 1);
		if (!result.IsSuccess) return QICResult<ushort>::CreateFailedResult(result);
		auto content = result.getContent<0>();
		return QICResult<ushort>::CreateSuccessResult(content.at(0));
#else
		auto result = ReadUInt16(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
#endif // 1

	}

	virtual QICResult<QVector<int>> ReadInt32(const QString &address, ushort length)
	{
#if 1 // 不使用QICResultTranslator
		auto result = Read(address, length * WordLenght * 2);
		if (!result.IsSuccess) return QICResult<QVector<int>>::CreateFailedResult(result);
		auto fmtedResult = BytesOrderPtr->ConvertToInt32(result.getContent<0>(), 0, length);
		return QICResult<QVector<int>>::CreateSuccessResult(fmtedResult);
#else
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return BytesOrderPtr->ConvertToInt32(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<int>>(result, lambda);
#endif // 1

	}
	QICResult<int> ReadInt32(const QString &address)
	{
#if 1 // 不使用QICResultTranslator
		auto result = ReadInt32(address, 1);
		if (!result.IsSuccess) return QICResult<int>::CreateFailedResult(result);
		auto content = result.getContent<0>();
		return QICResult<int>::CreateSuccessResult(content.at(0));
#else
		auto result = ReadInt32(address, 1);
		if (!result.IsSuccess) return QICResult<int>::CreateFailedResult(result);
		return QICResult<int>::CreateSuccessResult(result.getContent<0>().at(0));
#endif // 1

	}

	virtual QICResult<QVector<uint>> ReadUInt32(const QString &address, ushort length)
	{
#if 1 // 不使用QICResultTranslator
		auto result = Read(address, length * WordLenght * 2);
		if (!result.IsSuccess) return QICResult<QVector<uint>>::CreateFailedResult(result);
		auto fmtedResult = BytesOrderPtr->ConvertToUInt32(result.getContent<0>(), 0, length);
#else
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return BytesOrderPtr->ConvertToUInt32(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<uint>>(result, lambda);
#endif // 1

	}
	QICResult<uint> ReadUInt32(const QString &address)
	{
		auto result = ReadUInt32(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<float>> ReadFloat(const QString &address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray &ba)
		{
			return BytesOrderPtr->ConvertToFloat(ba, 0, length);
		};
		return QICResultTranslator::GetResultFromBytes<QVector<float>>(result, lambda);
	}
	QICResult<float> ReadFloat(const QString &address)
	{
		auto result = ReadFloat(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<qint64>> ReadInt64(const QString &address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray &byteArray)
		{
			return BytesOrderPtr->ConvertToInt64(byteArray, 0, length);
		};
		return QICResultTranslator::GetResultFromBytes<QVector<qint64>>(result, lambda);
	}
	QICResult<qint64> ReadInt64(const QString &address)
	{
		auto result = ReadInt64(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<quint64>> ReadUInt64(const QString &address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray &ba)
		{
			return BytesOrderPtr->ConvertToUInt64(ba, 0, length);
		};
		return QICResultTranslator::GetResultFromBytes<QVector<quint64>>(result, lambda);
	}
	QICResult<quint64> ReadUInt64(const QString &address)
	{
		auto result = ReadUInt64(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<double>> ReadDouble(const QString &address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray &ba)
		{
			return BytesOrderPtr->ConvertToDouble(ba, 0, length);
		};
		return QICResultTranslator::GetResultFromBytes<QVector<double>>(result, lambda);
	}
	QICResult<double> ReadDouble(const QString &address)
	{
		auto result = ReadDouble(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QString> ReadString(const QString &address, ushort length, QTextCodec *codec)
	{
		auto result = Read(address, length);
		auto lambda = [=](const QByteArray &ba)
		{
			return BytesOrderPtr->ConvertToString(ba, 0, ba.length(), codec);
		};
		return QICResultTranslator::GetResultFromBytes<QString>(result, lambda);
	}
	QICResult<QString> ReadString(const QString &address, ushort length) { return ReadString(address, length, QTextCodec::codecForName("ASCII")); }

	virtual QICResult<> Write(const QString &address, QVector<short> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString &address, short value) { return Write(address, QVector{value}); }

	virtual QICResult<> Write(const QString &address, QVector<ushort> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString &address, ushort value) { return Write(address, QVector{value}); }

	virtual QICResult<> Write(const QString &address, QVector<int> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString &address, int value) { return Write(address, QVector{value}); }

	virtual QICResult<> Write(const QString &address, QVector<uint> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString &address, uint value) { return Write(address, QVector{value}); }

	virtual QICResult<> Write(const QString &address, QVector<float> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString &address, float value) { return Write(address, QVector{value}); }

	virtual QICResult<> Write(const QString &address, QVector<qint64> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> WriteInt64(const QString &address, qint64 value) { return Write(address, QVector{value}); }

	virtual QICResult<> Write(const QString &address, QVector<quint64> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString &address, quint64 value) { return Write(address, QVector{value}); }

	virtual QICResult<> Write(const QString &address, QVector<double> value)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString &address, double value) { return Write(address, QVector{value}); }

	virtual QICResult<> WriteString(const QString &address, QString value, QTextCodec *codec)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value, codec);
		if (WordLenght == 1)
		{
			// 扩展到偶数长度
			if (bytes.size() % 2 != 0)
				bytes.append('\0');
		}
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString &address, QString value) { return WriteString(address, value, QTextCodec::codecForName("ASCII")); }
	virtual QICResult<> WriteString(const QString &address, QString value, int length, QTextCodec *codec)
	{
		auto bytes = BytesOrderPtr->PackByteArray(value, codec);
		if (WordLenght == 1)
		{
			// 扩展到偶数长度
			if (bytes.size() % 2 != 0)
				bytes.append('\0');
		}
		bytes.resize(length);
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString &address, QString value, int length) { return WriteString(address, value, length, QTextCodec::codecForName("ASCII")); }

public:
	ushort WordLenght;
	QScopedPointer<BytesOrderBase> BytesOrderPtr;
};
