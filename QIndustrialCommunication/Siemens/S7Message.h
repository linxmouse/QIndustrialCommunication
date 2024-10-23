#pragma once

#include <QByteArray>

class S7Message
{
public:
	S7Message()
		: protocolHeadBytesLength(4)
	{}

	/**
	 * @brief 验证字节头是否有效
	 * @return 有效 - true 无效 - false
	 */
	bool validHeadBytes()
	{
		if (headBytes.isEmpty()) return false;
		if (headBytes.length() < 2) return false;
		if (headBytes.at(0) == 3 && headBytes.at(1) == 0) return true;
		return false;
	}

	int getContentLengthByHeadBytes()
	{
		QByteArray headBytes = this->headBytes;
		if (!headBytes.isEmpty() && headBytes.length() >= 4)
		{
			return headBytes.at(2) * 256 + headBytes.at(3) - 4;
		}
		return 0;
	}

public:
	int protocolHeadBytesLength;
	QByteArray headBytes;
	QByteArray contentBytes;
	QByteArray sendBytes;
};