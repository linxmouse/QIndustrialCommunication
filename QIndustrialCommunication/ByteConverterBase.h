#pragma once

#include <QObject>
#include <QVector>
#include <QtEndian>
#include <QBitArray>
#include <QByteArray>
#include <QTextCodec>
#include <memory>
#include <exception>
#include "DataFormat.h"

class ByteConverterBase
{
public:
	explicit ByteConverterBase(DataFormat dataFormat, bool isStringReverseByteWord = false)
		: dataFormat(dataFormat), isStringReverseByteWord(isStringReverseByteWord) { }

public:
	virtual bool ConvertToBool(const QByteArray& buffer, int index)
	{
		if (index < 0 || index >= buffer.size()) throw std::out_of_range("index is out of range or less than 0");
		return (buffer.at(index) & 1) == 1;
	}
	virtual QVector<bool> ConvertToBool(const QByteArray& buffer, int index, int length)
	{
		if (index < 0 || index >= buffer.size()) throw std::out_of_range("index is out of range or less than 0");
		QVector<bool> result;
		// 获取子数组
		QByteArray subArray = buffer.mid(index, length);
		QBitArray bits(subArray.count() * 8);
		// 转换每个字节到位数组
		for (int i = 0; i < subArray.count(); ++i)
		{
			for (int b = 0; b < 8; ++b)
			{
				bits.setBit(i * 8 + b, subArray.at(i) & (1 << (7 - b)));
			}
		}
		// 从QBitArray转换到QVector<bool>
		for (int i = 0; i < bits.size(); ++i)
		{
			result.push_back(bits.at(i));
		}
		return result;
	}

	virtual short ConvertToInt16(const QByteArray& buffer, int index)
	{
		qint16 value;
		std::memcpy(&value, buffer.constData() + index, sizeof(qint16));
		return value;
	}
	virtual QVector<qint16> ConvertToInt16(const QByteArray& buffer, int index, int length)
	{
		QVector<short> array(length);
		for (int i = 0; i < length; ++i)
		{
			short value = ConvertToInt16(buffer, index + 2 * i);
			array[i] = value;
		}
		return array;
	}

	virtual ushort ConvertToUInt16(const QByteArray& buffer, int index)
	{
		ushort value;
		std::memcpy(&value, buffer.constData() + index, sizeof(ushort));
		return value;
	}
	virtual QVector<ushort> ConvertToUInt16(const QByteArray& buffer, int index, int lenght)
	{
		QVector<ushort> array(lenght);
		for (int i = 0; i < lenght; ++i)
		{
			ushort value = ConvertToUInt16(buffer, index + 2 * i);
			array[i] = value;
		}
		return array;
	}

	virtual int ConvertToInt32(const QByteArray& buffer, int index)
	{
		QByteArray formattedBuffer = ByteTransDataFormat4(buffer, index);
		int value = 0;
		std::memcpy(&value, formattedBuffer.constData(), sizeof(value));
		return value;
	}
	virtual QVector<int> ConvertToInt32(const QByteArray& buffer, int index, int length)
	{
		QVector<int> array;
		for (int i = 0; i < length; ++i)
		{
			array.append(ConvertToInt32(buffer, index + 4 * i));
		}
		return array;
	}

	virtual uint ConvertToUInt32(const QByteArray& buffer, int index)
	{
		QByteArray formattedBuffer = ByteTransDataFormat4(buffer, index);
		uint value = 0;
		std::memcpy(&value, formattedBuffer.constData(), sizeof(value));
		return value;
	}
	virtual QVector<uint> ConvertToUInt32(const QByteArray& buffer, int index, int length)
	{
		QVector<uint> array;
		for (int i = 0; i < length; ++i)
		{
			array.append(ConvertToUInt32(buffer, index + 4 * i));
		}
		return array;
	}

	virtual qint64 ConvertToInt64(const QByteArray& buffer, int index)
	{
		QByteArray formattedBuffer = ByteTransDataFormat8(buffer, index);
		qint64 value = 0;
		std::memcpy(&value, formattedBuffer.constData(), sizeof(value));
		return value;
	}
	virtual QVector<qint64> ConvertToInt64(const QByteArray& buffer, int index, int length)
	{
		QVector<qint64> array;
		for (int i = 0; i < length; ++i)
		{
			array.append(ConvertToInt64(buffer, index + 8 * i));
		}
		return array;
	}

	virtual quint64 ConvertToUInt64(const QByteArray& buffer, int index)
	{
		QByteArray formattedBuffer = ByteTransDataFormat8(buffer, index);
		qint64 value = 0;
		std::memcpy(&value, formattedBuffer.constData(), sizeof(value));
		return value;
	}
	virtual QVector<quint64> ConvertToUInt64(const QByteArray& buffer, int index, int length)
	{
		QVector<quint64> array;
		for (int i = 0; i < length; ++i)
		{
			array.append(ConvertToUInt64(buffer, index + 8 * i));
		}
		return array;
	}

	virtual float ConvertToFloat(const QByteArray& buffer, int index)
	{
		QByteArray formattedBuffer = ByteTransDataFormat4(buffer, index);
		float result = 0;
		std::memcpy(&result, formattedBuffer.constData(), sizeof(float));
		return result;
	}
	virtual QVector<float> ConvertToFloat(const QByteArray& buffer, int index, int length)
	{
		QVector<float> array(length);
		for (int i = 0; i < length; ++i)
		{
			array.append(ConvertToFloat(buffer, index + 4 * i));
		}
		return array;
	}

	virtual double ConvertToDouble(const QByteArray& buffer, int index)
	{
		QByteArray formattedBuffer = ByteTransDataFormat8(buffer, index);
		double result = 0;
		std::memcpy(&result, formattedBuffer.constData(), sizeof(double));
		return result;
	}
	virtual QVector<double> ConvertToDouble(const QByteArray& buffer, int index, int length)
	{
		QVector<double> array;
		for (int i = 0; i < length; ++i)
		{
			array.append(ConvertToDouble(buffer, index + 8 * i));
		}
		return array;
	}

	virtual QString ConvertToString(const QByteArray& buffer, int index, int length, QTextCodec* codec)
	{
		if (index < 0 || index >= buffer.size()) throw std::out_of_range("index is out of range or less than 0");
		QByteArray subArray = buffer.mid(index, length);
		if (isStringReverseByteWord)
		{
			subArray = bytesReverseByWord(subArray);
		}
		return codec->toUnicode(subArray);
	}
	virtual QString ConvertToString(const QByteArray& buffer, QTextCodec* codec)
	{
		return codec->toUnicode(buffer);
	}

	virtual quint8 PackByteArray(const QByteArray& buffer, int index)
	{
		if (index < 0 || index >= buffer.size()) throw std::out_of_range("index is out of range or less than 0");
		return static_cast<quint8>(buffer.at(index));
	}
	virtual QByteArray PackByteArray(const QByteArray& buffer, int index, int length)
	{
		if (index < 0 || index >= buffer.size()) throw std::out_of_range("index is out of range or less than 0");
		return buffer.mid(index, length);
	}

	virtual QByteArray PackByteArray(const QVector<bool>& values)
	{
		int bitCount = values.size();
		QBitArray bits(bitCount);
		// 将QVector<bool>转换为QBitArray
		for (int i = 0; i < bitCount; ++i)
		{
			bits.setBit(i, values[i]);
		}
		// 创建一个足够大的QByteArray来存放位
		// +7用于处理位数不是8的倍数的情况
		QByteArray bytes((bitCount + 7) / 8, 0);
		// 将QBitArray的位填充到QByteArray中
		for (int i = 0; i < bitCount; ++i)
		{
			if (bits.testBit(i))
			{
				bytes[i / 8] = (bytes.at(i / 8) | (1 << (7 - (i % 8))));
			}
		}
		return bytes;
	}
	virtual QByteArray PackByteArray(bool value) { return PackByteArray(QVector<bool>{value}); }

	virtual QByteArray PackByteArray(const QVector<short>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const short& value : values)
		{
			array.append(reinterpret_cast<const char*>(&value), sizeof(short));
		}
		return array;
	}
	virtual QByteArray PackByteArray(short value) { return PackByteArray(QVector<short>{value}); }

	virtual QByteArray PackByteArray(const QVector<ushort>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const ushort& value : values)
		{
			array.append(reinterpret_cast<const char*>(&value), sizeof(ushort));
		}
		return array;
	}
	virtual QByteArray PackByteArray(ushort value) { return PackByteArray(QVector<ushort>{value}); }

	virtual QByteArray PackByteArray(const QVector<int>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const int& value : values)
		{
			QByteArray temp = ByteTransDataFormat4(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(int)));
			array.append(temp);
		}
		return array;
	}
	virtual QByteArray PackByteArray(int value) { return PackByteArray(QVector<int>{value}); }

	virtual QByteArray PackByteArray(const QVector<uint>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const uint& value : values)
		{
			QByteArray temp = ByteTransDataFormat4(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(uint)));
			array.append(temp);
		}
		return array;
	}
	virtual QByteArray PackByteArray(uint value) { return PackByteArray(QVector<uint>{value}); }

	virtual QByteArray PackByteArray(const QVector<qint64>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const qint64& value : values)
		{
			QByteArray temp = ByteTransDataFormat8(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(qint64)));
			array.append(temp);
		}
		return array;
	}
	virtual QByteArray PackByteArray(qint64 value) { return PackByteArray(QVector<qint64>{value}); }

	virtual QByteArray PackByteArray(const QVector<quint64>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const quint64& value : values)
		{
			QByteArray temp = ByteTransDataFormat8(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(quint64)));
			array.append(temp);
		}
		return array;
	}
	virtual QByteArray PackByteArray(quint64 value) { return PackByteArray(QVector<quint64>{value}); }

	virtual QByteArray PackByteArray(const QVector<float>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const float& value : values)
		{
			QByteArray temp = ByteTransDataFormat4(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(float)));
			array.append(temp);
		}
		return array;
	}
	virtual QByteArray PackByteArray(float value) { return PackByteArray(QVector<float>{value}); }

	virtual QByteArray PackByteArray(const QVector<double>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const double& value : values)
		{
			QByteArray temp = ByteTransDataFormat8(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(double)));
			array.append(temp);
		}
		return array;
	}
	virtual QByteArray PackByteArray(double value) { return PackByteArray(QVector<double>{value}); }

	virtual QByteArray PackByteArray(const QString& value, QTextCodec* codec)
	{
		if (value.isNull()) throw std::out_of_range("the data to be written cannot be null.");
		QByteArray bytes = codec->fromUnicode(value);
		return isStringReverseByteWord ? bytesReverseByWord(bytes) : bytes;
	}
	virtual QByteArray PackByteArray(const QString& value, int length, QTextCodec* codec)
	{
		if (value.isNull()) throw std::out_of_range("the data to be written cannot be null.");
		QByteArray bytes = codec->fromUnicode(value);
		if (isStringReverseByteWord) bytes = bytesReverseByWord(bytes);
		bytes.resize(length);
		return bytes;
	}

protected:
	QByteArray ByteTransDataFormat4(const QByteArray& value, int index = 0)
	{
		QByteArray array(4, 0);
		switch (dataFormat)
		{
		case DataFormat::ABCD:
			array[0] = value[index + 3];
			array[1] = value[index + 2];
			array[2] = value[index + 1];
			array[3] = value[index];
			break;
		case DataFormat::BADC:
			array[0] = value[index + 2];
			array[1] = value[index + 3];
			array[2] = value[index];
			array[3] = value[index + 1];
			break;
		case DataFormat::CDAB:
			array[0] = value[index + 1];
			array[1] = value[index];
			array[2] = value[index + 3];
			array[3] = value[index + 2];
			break;
		case DataFormat::DCBA:
			array[0] = value[index];
			array[1] = value[index + 1];
			array[2] = value[index + 2];
			array[3] = value[index + 3];
			break;
		}
		return array;
	}

	QByteArray ByteTransDataFormat8(const QByteArray& value, int index = 0)
	{
		QByteArray array(8, 0);
		switch (dataFormat)
		{
		case DataFormat::ABCD:
			array[0] = value[index + 7];
			array[1] = value[index + 6];
			array[2] = value[index + 5];
			array[3] = value[index + 4];
			array[4] = value[index + 3];
			array[5] = value[index + 2];
			array[6] = value[index + 1];
			array[7] = value[index];
			break;
		case DataFormat::BADC:
			array[0] = value[index + 6];
			array[1] = value[index + 7];
			array[2] = value[index + 4];
			array[3] = value[index + 5];
			array[4] = value[index + 2];
			array[5] = value[index + 3];
			array[6] = value[index];
			array[7] = value[index + 1];
			break;
		case DataFormat::CDAB:
			array[0] = value[index + 1];
			array[1] = value[index];
			array[2] = value[index + 3];
			array[3] = value[index + 2];
			array[4] = value[index + 5];
			array[5] = value[index + 4];
			array[6] = value[index + 7];
			array[7] = value[index + 6];
			break;
		case DataFormat::DCBA:
			array[0] = value[index];
			array[1] = value[index + 1];
			array[2] = value[index + 2];
			array[3] = value[index + 3];
			array[4] = value[index + 4];
			array[5] = value[index + 5];
			array[6] = value[index + 6];
			array[7] = value[index + 7];
			break;
		}
		return array;
	}

private:
	QByteArray bytesReverseByWord(const QByteArray& value)
	{
		if (value.isEmpty()) return QByteArray();
		QByteArray array = value;
		// 扩展到偶数长度
		if (array.size() % 2 != 0) array.append('\0');
		for (int i = 0; i < array.size() / 2; ++i)
		{
			char temp = array[i * 2];
			array[i * 2] = array[i * 2 + 1];
			array[i * 2 + 1] = temp;
		}
		return array;
	}

public:
	DataFormat dataFormat;
	bool isStringReverseByteWord;
};