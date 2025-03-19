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

/**
 * @brief Base class for handling byte order conversions
 * @details This class provides functionality for converting between different byte orders
 *          and data formats, supporting various data types including numeric types and strings.
 *          基类用于处理字节序转换，提供不同字节序和数据格式之间的转换功能，
 *          支持包括数值类型和字符串在内的各种数据类型。
 */
class BytesOrderBase
{
public:
	/**
	 * @brief Constructor for BytesOrderBase
	 * @param dataFormat The data format to be used for conversions (数据格式，用于转换)
	 * @param isStringReverseByteWord Whether to reverse byte order for strings (是否对字符串进行字节序反转)
	 */
	explicit BytesOrderBase(DataFormat dataFormat, bool isStringReverseByteWord = false)
		: dataFormat(dataFormat), isStringReverseByteWord(isStringReverseByteWord) { }

public:
	/**
	 * @brief Convert byte to boolean value
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted boolean value (转换后的布尔值)
	 * @throws std::out_of_range if index is invalid (如果索引无效则抛出异常)
	 */
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

	/**
	 * @brief Convert bytes to 16-bit signed integer
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted 16-bit signed integer (转换后的16位有符号整数)
	 */
	virtual short ConvertToInt16(const QByteArray& buffer, int index)
	{
		QByteArray fmtBuffer = ByteTransDataFormat2(buffer, index);
		qint16 value = 0;
		std::memcpy(&value, fmtBuffer.constData(), sizeof(value));
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

	/**
	 * @brief Convert bytes to 16-bit unsigned integer
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted 16-bit unsigned integer (转换后的16位无符号整数)
	 */
	virtual ushort ConvertToUInt16(const QByteArray& buffer, int index)
	{
		QByteArray fmtBuffer = ByteTransDataFormat2(buffer, index);
		ushort value = 0;
		std::memcpy(&value, fmtBuffer.constData(), sizeof(value));
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

	/**
	 * @brief Convert bytes to 32-bit signed integer
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted 32-bit signed integer (转换后的32位有符号整数)
	 */
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

	/**
	 * @brief Convert bytes to 32-bit unsigned integer
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted 32-bit unsigned integer (转换后的32位无符号整数)
	 */
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

	/**
	 * @brief Convert bytes to 64-bit signed integer
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted 64-bit signed integer (转换后的64位有符号整数)
	 */
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

	/**
	 * @brief Convert bytes to 64-bit unsigned integer
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted 64-bit unsigned integer (转换后的64位无符号整数)
	 */
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

	/**
	 * @brief Convert bytes to single-precision floating point
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted float value (转换后的单精度浮点数)
	 */
	virtual float ConvertToFloat(const QByteArray& buffer, int index)
	{
		QByteArray formattedBuffer = ByteTransDataFormat4(buffer, index);
		float result = 0;
		std::memcpy(&result, formattedBuffer.constData(), sizeof(float));
		return result;
	}
	virtual QVector<float> ConvertToFloat(const QByteArray& buffer, int index, int length)
	{
		QVector<float> array;
		for (int i = 0; i < length; ++i)
		{
			array.append(ConvertToFloat(buffer, index + 4 * i));
		}
		return array;
	}

	/**
	 * @brief Convert bytes to double-precision floating point
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @return Converted double value (转换后的双精度浮点数)
	 */
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

	/**
	 * @brief Convert bytes to string using specified text codec
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index in buffer (缓冲区起始索引)
	 * @param length Number of bytes to convert (要转换的字节数)
	 * @param codec Text codec to use for conversion (用于转换的文本编码器)
	 * @return Converted string (转换后的字符串)
	 * @throws std::out_of_range if index is invalid (如果索引无效则抛出异常)
	 */
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
	/**
	 * @brief Convert entire byte array to string using specified text codec
	 * @param buffer Input byte array (输入字节数组)
	 * @param codec Text codec to use for conversion (用于转换的文本编码器)
	 * @return Converted string (转换后的字符串)
	 */
	virtual QString ConvertToString(const QByteArray& buffer, QTextCodec* codec)
	{
		return codec->toUnicode(buffer);
	}

	/**
	 * @brief Extract single byte from byte array
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Index of byte to extract (要提取的字节索引)
	 * @return Extracted byte value (提取的字节值)
	 * @throws std::out_of_range if index is invalid (如果索引无效则抛出异常)
	 */
	virtual quint8 PackByteArray(const QByteArray& buffer, int index)
	{
		if (index < 0 || index >= buffer.size()) throw std::out_of_range("index is out of range or less than 0");
		return static_cast<quint8>(buffer.at(index));
	}
	/**
	 * @brief Extract multiple bytes from byte array
	 * @param buffer Input byte array (输入字节数组)
	 * @param index Starting index (起始索引)
	 * @param length Number of bytes to extract (要提取的字节数)
	 * @return Extracted bytes (提取的字节数组)
	 * @throws std::out_of_range if index is invalid (如果索引无效则抛出异常)
	 */
	virtual QByteArray PackByteArray(const QByteArray& buffer, int index, int length)
	{
		if (index < 0 || index >= buffer.size()) throw std::out_of_range("index is out of range or less than 0");
		return buffer.mid(index, length);
	}

	/**
	 * @brief Pack boolean values into byte array
	 * @param values Vector of boolean values to pack (要打包的布尔值数组)
	 * @return Packed byte array (打包后的字节数组)
	 */
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
	/**
	 * @brief Pack single boolean value into byte array
	 * @param value Boolean value to pack (要打包的布尔值)
	 * @return Packed byte array (打包后的字节数组)
	 */
	virtual QByteArray PackByteArray(bool value) { return PackByteArray(QVector<bool>{value}); }

	virtual QByteArray PackByteArray(const QVector<short>& values)
	{
		if (values.isEmpty()) return QByteArray();
		QByteArray array;
		for (const short& value : values)
		{
			QByteArray temp = ByteTransDataFormat2(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(short)));
			array.append(temp);
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
			QByteArray temp = ByteTransDataFormat2(QByteArray::fromRawData(reinterpret_cast<const char*>(&value), sizeof(ushort)));
			array.append(temp);
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

	/**
	 * 
	 ABCD (大端序)：
	 第一次转换：不变
	 第二次转换：仍然不变
	 结论：会还原到原始状态
	 DCBA (小端序)：
	 第一次转换：完全反转字节顺序
	 第二次转换：再次完全反转
	 结论：会还原到原始状态
	 BADC (中间交换大端序)：
	 第一次转换：按照特定规则交换
	 第二次转换：按照相同规则交换
	 结论：会还原到原始状态
	 CDAB (中间交换小端序)：
	 第一次转换：按照特定规则交换
	 第二次转换：按照相同规则交换
	 结论：会还原到原始状态
	 */
	template<typename T>
	static void testConversion(T originalValue, DataFormat format) 
	{
		BytesOrderBase converter(format);
		// 将原始值转换为字节数组
		QByteArray originalBytes = QByteArray::fromRawData(reinterpret_cast<const char*>(&originalValue), sizeof(T));
		// 第一次转换
		QByteArray firstConvert;
		if (sizeof(T) == 2) firstConvert = converter.ByteTransDataFormat2(originalBytes);
		else if (sizeof(T) == 4) firstConvert = converter.ByteTransDataFormat4(originalBytes);
		else if (sizeof(T) == 8) firstConvert = converter.ByteTransDataFormat8(originalBytes);
		// 第二次转换
		QByteArray secondConvert;
		if (sizeof(T) == 2) secondConvert = converter.ByteTransDataFormat2(firstConvert);
		else if (sizeof(T) == 4) secondConvert = converter.ByteTransDataFormat4(firstConvert);
		else if (sizeof(T) == 8) secondConvert = converter.ByteTransDataFormat8(firstConvert);
		// 比较原始值和二次转换后的值
		T recoveredValue;
		std::memcpy(&recoveredValue, secondConvert.constData(), sizeof(T));
		qDebug() << "Original Format: " << format;
		qDebug() << "Original Value:  " << QString::number(originalValue, 16).toUpper();
		qDebug() << "Recovered Value: " << QString::number(recoveredValue, 16).toUpper();
		qDebug() << "Match:           " << (originalValue == recoveredValue ? "Yes" : "No");
	}

protected:
	/**
	 * @brief Convert 2-byte data according to specified format
	 * @details Handles endianness conversion for 16-bit values based on dataFormat
	 *          根据dataFormat处理16位值的字节序转换
	 * @param value Input byte array (输入字节数组)
	 * @param index Starting index (起始索引)
	 * @return Formatted byte array (格式化后的字节数组)
	 */
	QByteArray ByteTransDataFormat2(const QByteArray& value, int index = 0)
	{
		QByteArray array(2, 0);
		switch (dataFormat)
		{
		case DataFormat::ABCD:
			array[0] = value[index + 1];
			array[1] = value[index];
			break;
		case DataFormat::BADC:
			array[0] = value[index];
			array[1] = value[index + 1];
			break;
		case DataFormat::CDAB:
			array[0] = value[index + 1];
			array[1] = value[index];
			break;
		case DataFormat::DCBA:
			array[0] = value[index];
			array[1] = value[index + 1];
			break;
		}
		return array;
	}
	/**
	 * @brief Convert 4-byte data according to specified format
	 * @details Handles endianness conversion for 32-bit values based on dataFormat
	 *          根据dataFormat处理32位值的字节序转换
	 * @param value Input byte array (输入字节数组)
	 * @param index Starting index (起始索引)
	 * @return Formatted byte array (格式化后的字节数组)
	 */
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
	/**
	 * @brief Convert 8-byte data according to specified format
	 * @details Handles endianness conversion for 64-bit values based on dataFormat
	 *          根据dataFormat处理64位值的字节序转换
	 * @param value Input byte array (输入字节数组)
	 * @param index Starting index (起始索引)
	 * @return Formatted byte array (格式化后的字节数组)
	 */
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
	/**
	 * @brief Reverses the byte order of the input QByteArray in pairs (i.e., by word).
	 *
	 * This function swaps the order of each consecutive pair of bytes in the input array.
	 * If the length of the array is odd, a null byte ('\0') will be appended to make the
	 * array length even before performing the swap. If the input QByteArray is empty,
	 * the function returns an empty QByteArray.
	 *
	 * Example:
	 * Input: [0x12, 0x34, 0x56]  -> Output: [0x34, 0x12, 0x00, 0x56]
	 *
	 * @param value The input QByteArray to be processed.
	 * @return QByteArray The resulting QByteArray after swapping byte pairs.
	 */
	QByteArray bytesReverseByWord(const QByteArray& value)
	{
		if (value.isEmpty()) return QByteArray();
		QByteArray array = value;
		// 扩展到偶数长度
		if (array.size() % 2 != 0) array.append('\0');
		for (int i = 0; i < array.size() / 2; ++i)
		{
			// 将当前字节与下一个字节交换
			//std::swap(array[i * 2], array[i * 2 + 1]);
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