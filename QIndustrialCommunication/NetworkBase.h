#pragma once

#include <QObject>
#include <QByteArray>
#include <QDataStream>
#include <QVector>
#include <QTextCodec>
#include "QICResult.h"
#include "ConverterBase.h"
#include "QICResultTranslator.h"

class NetworkBase : public QObject
{
public:
	explicit NetworkBase(QObject* parent = nullptr)
		: WordLenght(1), Converter(nullptr), QObject(parent)
	{
	}
	virtual ~NetworkBase()
	{
		if (Converter) delete Converter;
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
		// չ����ʽ
		auto result = Read(address, length * wordLenght);
		if (!result.IsSuccess) return QICResult<QVector<short>>::CreateFailedResult(result);
		auto value = byteConverter->ConvertToInt16(result.GetContent<0>(), 0, length);
		return QICResult<QVector<short>>::CreateSuccessResult(value);
#else
		auto result = Read(address, length * WordLenght);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return Converter->ConvertToInt16(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<short>>(result, lambda);
#endif
	}
	QICResult<short> ReadInt16(const QString& address)
	{
#if 0
		// չ����ʽ
		auto result = ReadInt16(address, 1);
		if (!result.IsSuccess) return QICResult<short>::CreateFailedResult(result);
		auto content = result.GetContent<0>();
		return QICResult<short>::CreateSuccessResult(content.at(0));
#else
		auto result = ReadInt16(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
#endif
	}

	virtual QICResult<QVector<ushort>> ReadUInt16(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return Converter->ConvertToUInt16(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<ushort>>(result, lambda);
	}
	QICResult<ushort> ReadUInt16(const QString& address)
	{
		auto result = ReadUInt16(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<int>> ReadInt32(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return Converter->ConvertToInt32(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<int>>(result, lambda);
	}
	QICResult<int> ReadInt32(const QString& address)
	{
		auto result = ReadInt32(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<uint>> ReadUInt32(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return Converter->ConvertToUInt32(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<uint>>(result, lambda);
	}
	QICResult<uint> ReadUInt32(const QString& address)
	{
		auto result = ReadUInt32(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<float>> ReadFloat(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 2);
		auto lambda = [=](const QByteArray& ba)
			{
				return Converter->ConvertToFloat(ba, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<float>>(result, lambda);
	}
	QICResult<float> ReadFloat(const QString& address)
	{
		auto result = ReadFloat(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<qint64>> ReadInt64(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray& byteArray)
			{
				return Converter->ConvertToInt64(byteArray, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<qint64>>(result, lambda);
	}
	QICResult<qint64> ReadInt64(const QString& address)
	{
		auto result = ReadInt64(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<quint64>> ReadUInt64(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray& ba)
			{
				return Converter->ConvertToUInt64(ba, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<quint64>>(result, lambda);
	}
	QICResult<quint64> ReadUInt64(const QString& address)
	{
		auto result = ReadUInt64(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QVector<double>> ReadDouble(const QString& address, ushort length)
	{
		auto result = Read(address, length * WordLenght * 4);
		auto lambda = [=](const QByteArray& ba)
			{
				return Converter->ConvertToDouble(ba, 0, length);
			};
		return QICResultTranslator::GetResultFromBytes<QVector<double>>(result, lambda);
	}
	QICResult<double> ReadDouble(const QString& address)
	{
		auto result = ReadDouble(address, 1);
		return QICResultTranslator::GetResultFromArray(result);
	}

	virtual QICResult<QString> ReadString(const QString& address, ushort length, QTextCodec* codec)
	{
		auto result = Read(address, length);
		auto lambda = [=](const QByteArray& ba)
			{
				return Converter->ConvertToString(ba, 0, ba.length(), codec);
			};
		return QICResultTranslator::GetResultFromBytes<QString>(result, lambda);
	}
	QICResult<QString> ReadString(const QString& address, ushort length) { return ReadString(address, length, QTextCodec::codecForName("ASCII")); }

	virtual QICResult<> Write(const QString& address, QVector<short> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, short value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<ushort> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, ushort value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<int> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, int value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<uint> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, uint value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<float> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, float value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<qint64> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> WriteInt64(const QString& address, qint64 value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<quint64> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, quint64 value) { return Write(address, QVector{ value }); }

	virtual QICResult<> Write(const QString& address, QVector<double> value)
	{
		auto bytes = Converter->PackByteArray(value);
		return Write(address, bytes);
	}
	QICResult<> Write(const QString& address, double value) { return Write(address, QVector{ value }); }
	
	virtual QICResult<> WriteString(const QString& address, QString value, QTextCodec* codec)
	{
		auto bytes = Converter->PackByteArray(value, codec);
		if (WordLenght == 1)
		{
			// ��չ��ż������
			if (bytes.size() % 2 != 0) bytes.append('\0');
		}
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString& address, QString value) { return WriteString(address, value, QTextCodec::codecForName("ASCII")); }
	virtual QICResult<> WriteString(const QString& address, QString value, int length, QTextCodec* codec)
	{
		auto bytes = Converter->PackByteArray(value, codec);
		if (WordLenght == 1)
		{
			// ��չ��ż������
			if (bytes.size() % 2 != 0) bytes.append('\0');
		}
		bytes.resize(length);
		return Write(address, bytes);
	}
	QICResult<> WriteString(const QString& address, QString value, int length) { return WriteString(address, value, length, QTextCodec::codecForName("ASCII")); }

public:
	ushort WordLenght;
	ConverterBase* Converter;
};
