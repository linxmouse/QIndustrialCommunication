#pragma once

#include <QByteArray>
#include <QVector>
#include <QTextCodec>
#include <cstdint>
#include <vector>

#include "DataFormat.h"

class BytesOrderConverter
{
public:
	/**
	 * @brief 通用模板转换函数
	 * @tparam T
	 * @param value 需要转换的类型
	 * @param df 字节序
	 * @return 返回大端序
	 */
	template<typename T>
	static T Convert(T value, DataFormat df)
	{
		// 如果是简单传输，直接返回
		if (df == DataFormat::ABCD) return value;
		union {
			T val;
			uint8_t bytes[sizeof(T)];
		}src, dst;
		src.val = value;
		switch (df)
		{
		case DataFormat::DCBA:
			// 完全反转
			for (size_t i = 0; i < sizeof(T); i++)
			{
				dst.bytes[i] = src.bytes[sizeof(T) - 1 - i];
			}
			break;
		case DataFormat::BADC:
			// 16位/32位字交换
			if (sizeof(T) == 2)
			{
				dst.bytes[0] = src.bytes[1];
				dst.bytes[1] = src.bytes[0];
			}
			else if (sizeof(T) == 4)
			{
				dst.bytes[0] = src.bytes[2];
				dst.bytes[1] = src.bytes[3];
				dst.bytes[2] = src.bytes[0];
				dst.bytes[3] = src.bytes[1];
			}
			break;
		case DataFormat::CDAB:
			// 16位/32位字内部交换
			if (sizeof(T) == 2)
			{
				dst.bytes[0] = src.bytes[1];
				dst.bytes[1] = src.bytes[0];
			}
			else if (sizeof(T) == 4)
			{
				dst.bytes[0] = src.bytes[1];
				dst.bytes[1] = src.bytes[0];
				dst.bytes[2] = src.bytes[3];
				dst.bytes[3] = src.bytes[2];
			}
			break;
		default:
			return value;
		}
		return dst.val;
	}

	/**
	 * @brief 数组批量转换
	 * @tparam T
	 * @param values 需要转换的std::vector数组
	 * @param df 字节序
	 * @return 返回大端序数组
	 */
	template<typename T>
	static std::vector<T> ConvertArray(const std::vector<T>& values, DataFormat df)
	{
		std::vector<T> result;
		for (const auto& val : values)
		{
			result.push_back(Convert(val, df));
		}
		return result;
	}

	/**
	 * @brief 数组批量转换
	 * @tparam T 
	 * @param values 需要转换的QVector数组
	 * @param df 字节序
	 * @return 返回大端序数组
	 */
	template<typename T>
	static QVector<T> ConvertArray(const QVector<T>& values, DataFormat df)
	{
		QVector<T> result;
		result.reserve(values.size());
		for (const auto& val: values)
		{
			result.append(Convert(val, df));
		}
		return result;
	}

	/**
	 * @brief 字节数组转换
	 * @param bytes 字节数组
	 * @param df 字节序
	 * @return 大端字节序
	 */
	static QByteArray ConvertByteArray(const QByteArray& bytes, DataFormat df)
	{
		if (bytes.isEmpty()) return bytes;
		QByteArray result = bytes;
		switch (df)
		{
		case DataFormat::DCBA:
			std::reverse(result.begin(), result.end());
			break;
		case DataFormat::BADC:
			for (int i = 0; i < result.size(); i += 2)
			{
				if (i + 1 < result.size())
				{
					std::swap(result[i], result[i + 1]);
				}
			}
			break;
		case DataFormat::CDAB:
			for (int i = 0; i < result.size(); i += 2)
			{
				if (i + 1 < result.size())
				{
					std::swap(result[i], result[i + 1]);
				}
			}
			break;
		default:
			break;
		}
		return result;
	}

	/**
	 * @brief 字符串转换
	 * @param str 字符串
	 * @param codec 字符编码
	 * @param isReverseByteWord 是否对字反转
	 * @return Unicode String
	 */
	static QString ConvertString(const QString& str, QTextCodec* codec, bool isReverseByteWord)
	{
		if (str.isEmpty() || !codec) return str;
		QByteArray bytes = codec->fromUnicode(str);
		if (isReverseByteWord)
		{
			for (int i = 0; i < bytes.size(); i += 2)
			{
				if (i + 1 < bytes.size())
				{
					std::swap(bytes[i], bytes[i + 1]);
				}
			}
		}
		return codec->toUnicode(bytes);
	}
};