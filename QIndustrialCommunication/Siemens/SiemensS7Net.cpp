#include "SiemensS7Net.h"

SiemensS7Net::SiemensS7Net(SiemensPLCS siemens, QString ipAddr, QObject* parent)
	: EthernetDevice(parent),
	plcHead1(QByteArray::fromRawData("\x03\x00\x00\x16\x11\xE0\x00\x00\x00\x01\x00\xC0\x01\x0A\xC1\x02\x01\x02\xC2\x02\x01\x00", 22)),
	plcHead2(QByteArray::fromRawData("\x03\x00\x00\x19\x02\xF0\x80\x32\x01\x00\x00\x04\x00\x00\x08\x00\x00\xF0\x00\x00\x01\x00\x01\x01\xE0", 25)),
	plcOrderNumber(QByteArray::fromRawData("\x03\x00\x00\x21\x02\xF0\x80\x32\x07\x00\x00\x00\x01\x00\x08\x00\x08\x00\x01\x12\x04\x11\x44\x01\x00\xFF\x09\x00\x04\x00\x11\x00\x00", 33)),
	plcHead1_200smart(QByteArray::fromRawData("\x03\x00\x00\x16\x11\xE0\x00\x00\x00\x01\x00\xC1\x02\x10\x00\xC2\x02\x03\x00\xC0\x01\x0A", 22)),
	plcHead2_200smart(QByteArray::fromRawData("\x03\x00\x00\x19\x02\xF0\x80\x32\x01\x00\x00\xCC\xC1\x00\x08\x00\x00\xF0\x00\x00\x01\x00\x01\x03\xC0", 25)),
	plcHead1_200(QByteArray::fromRawData("\x03\x00\x00\x16\x11\xE0\x00\x00\x00\x01\x00\xC1\x02\x4D\x57\xC2\x02\x4D\x57\xC0\x01\x09", 22)),
	plcHead2_200(QByteArray::fromRawData("\x03\x00\x00\x19\x02\xF0\x80\x32\x01\x00\x00\x00\x00\x00\x08\x00\x00\xF0\x00\x00\x01\x00\x01\x03\xC0", 25)),
	S7_STOP(QByteArray::fromRawData("\x03\x00\x00\x21\x02\xF0\x80\x32\x01\x00\x00\x0E\x00\x00\x10\x00\x00\x29\x00\x00\x00\x00\x00\x09\x50\x5F\x50\x52\x4F\x47\x52\x41\x4D", 33)),
	S7_HOT_START(QByteArray::fromRawData("\x03\x00\x00\x25\x02\xF0\x80\x32\x01\x00\x00\x0C\x00\x00\x14\x00\x00\x28\x00\x00\x00\x00\x00\x00\xFD\x00\x00\x09\x50\x5F\x50\x52\x4F\x47\x52\x41\x4D", 37)),
	S7_COLD_START(QByteArray::fromRawData("\x03\x00\x00\x27\x02\xF0\x80\x32\x01\x00\x00\x0F\x00\x00\x16\x00\x00\x28\x00\x00\x00\x00\x00\x00\xFD\x00\x02\x43\x20\x09\x50\x5F\x50\x52\x4F\x47\x52\x41\x4D", 39))
{
	Initializetion(siemens, ipAddr);
}

SiemensS7Net::~SiemensS7Net()
{
}

void SiemensS7Net::Initializetion(SiemensPLCS siemens, QString ipAddr)
{
	EthernetDevice::WordLenght = 2;
	this->ipAddr = ipAddr;
	this->port = 102;
	this->currentPlc = siemens;
	BytesOrderPtr.reset(new BytesOrderBase(DataFormat::ABCD, false));
	switch (siemens)
	{
	case SiemensPLCS::S1200:
		plcHead1[21] = 0;
		break;
	case SiemensPLCS::S300:
		plcHead1[21] = 2;
		break;
	case SiemensPLCS::S400:
		plcHead1[21] = 3;
		plcHead1[17] = 0;
		break;
	case SiemensPLCS::S1500:
		plcHead1[21] = 0;
		break;
	case SiemensPLCS::S200Smart:
		plcHead1 = plcHead1_200smart;
		plcHead2 = plcHead2_200smart;
		break;
	case SiemensPLCS::S200:
		plcHead1 = plcHead1_200;
		plcHead2 = plcHead2_200;
		break;
	default:
		plcHead1[18] = 0;
		break;
	}
}

QICResult<> SiemensS7Net::checkStartResult(QByteArray content)
{
	if (content.length() < 19) return QICResult<>::CreateFailedResult("Receive error");
	if (static_cast<unsigned char>(content.at(19)) != 40) return QICResult<>::CreateFailedResult("Can not start PLC");
	if (static_cast<unsigned char>(content.at(20)) != 2) return QICResult<>::CreateFailedResult("Can not start PLC");
	return QICResult<>::CreateSuccessResult();
}

QICResult<> SiemensS7Net::checkStopResult(QByteArray content)
{
	if (content.length() < 19) return QICResult<>::CreateFailedResult("Receive error");
	if (static_cast<unsigned char>(content.at(19)) != 41) return QICResult<>::CreateFailedResult("Can not stop PLC");
	if (static_cast<unsigned char>(content.at(20)) != 7) return QICResult<>::CreateFailedResult("Can not stop PLC");
	return QICResult<>::CreateSuccessResult();
}

QICResult<> SiemensS7Net::InitializationOnConnect(QTcpSocket* socket)
{
	QICResult<QByteArray> r = ReadFromCoreServer(socket, plcHead1);
	if (!r.IsSuccess) return QICResult<>::CreateFailedResult(r);
	r = ReadFromCoreServer(socket, plcHead2);
	if (!r.IsSuccess) return QICResult<>::CreateFailedResult(r);
	QByteArray content = r.getContent0();
	pduLength = this->BytesOrderPtr->ConvertToUInt16(content.mid(content.count() - 2), 0) - 20;
	if (pduLength < 200) pduLength = 200;
	return QICResult<>::CreateSuccessResult();
}

QICResult<> SiemensS7Net::ReleaseOnDisconnect(QTcpSocket* socket)
{
	return QICResult<>();
}

QICResult<QString> SiemensS7Net::ReadOrderNumber()
{
	auto r = ReadFromCoreServer(plcOrderNumber);
	if (!r.IsSuccess) return QICResult<QString>::CreateFailedResult(r);
	auto orderNumber = r.getContent0().mid(71, 20);
	return QICResult<QString>::CreateSuccessResult(QString::fromLatin1(orderNumber));
}

QICResult<> SiemensS7Net::HotStart()
{
	auto r = ReadFromCoreServer(S7_HOT_START);
	if (!r.IsSuccess) return QICResult<>::CreateFailedResult(r);
	return checkStartResult(r.getContent0());
}

QICResult<> SiemensS7Net::ColdStart()
{
	auto r = ReadFromCoreServer(S7_COLD_START);
	if (!r.IsSuccess) return QICResult<>::CreateFailedResult(r);
	return checkStartResult(r.getContent0());
}

QICResult<> SiemensS7Net::Stop()
{
	auto r = ReadFromCoreServer(S7_STOP);
	if (!r.IsSuccess) return QICResult<>::CreateFailedResult(r);
	return checkStopResult(r.getContent0());
}

QICResult<QByteArray> SiemensS7Net::Read(const QString& address, ushort length)
{
	// 1. 解析地址字符串,获取 S7Address 对象
	QICResult<S7Address> rAddr = S7Address::ParseFrom(address, length);
	if (!rAddr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(rAddr);	// 地址解析失败,返回失败结果
	QByteArray result;																// 存储最终读取的数据
	quint16 readedBytesNum = 0;														// 已读取的字节数
	auto s7Addr = rAddr.getContent0();												// 获取解析后的地址

	// 2. 循环读取数据,每次读取 pduLength 长度的数据
	while (readedBytesNum < length) 
	{
		quint16 toReadBytesNum = qMin(length - readedBytesNum, pduLength);	// 本次读取的长度
		s7Addr.length = toReadBytesNum;										// 设置本次读取的长度
		QICResult<QByteArray> rRead = Read(QVector<S7Address>{ s7Addr });	// 读取数据
		if (!rRead.IsSuccess) return rRead;									// 读取失败,返回失败结果

		result.append(rRead.getContent0());									// 将读取的数据追加到结果中
		readedBytesNum += toReadBytesNum;									// 更新已读取的字节数
		s7Addr.addressStart += toReadBytesNum * 8;							// 更新下次读取的起始地址
	}

	// 3. 返回成功结果
	return QICResult<QByteArray>::CreateSuccessResult(result);
}

QICResult<QByteArray> SiemensS7Net::Read(const QStringList& addresses, const QVector<quint16>& length)
{
	QVector<S7Address> s7Addr(addresses.length());
	for (int i = 0; i < addresses.length(); i++)
	{
		QICResult<S7Address> r = S7Address::ParseFrom(addresses[i], length[i]);
		if (!r.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(r);
		s7Addr[i] = r.getContent0();
	}
	return Read(s7Addr);
}

QICResult<QByteArray> SiemensS7Net::Read(const QVector<S7Address>& addresses)
{
	if (addresses.size() > 19)
	{
		QByteArray result;
		for (int i = 0; i < addresses.size(); i += 19)
		{
			QVector<S7Address> chunk = addresses.mid(i, qMin(19, addresses.size() - i));
			QICResult<QByteArray> chunkResult = Read(chunk);
			if (!chunkResult.IsSuccess) return chunkResult;
			result.append(chunkResult.getContent0());
		}
		return QICResult<QByteArray>::CreateSuccessResult(result);
	}
	return ReadS7Address(addresses);
}

QICResult<> SiemensS7Net::Write(const QString& address, const QByteArray& value)
{
	return QICResult<>();
}

QICResult<QByteArray> SiemensS7Net::BuildReadPacket(const QVector<S7Address>& addresses)
{
	if (addresses.isEmpty()) return QICResult<QByteArray>::CreateFailedResult("S7 Addresses is empty");
	if (addresses.size() > 19) return QICResult<QByteArray>::CreateFailedResult("The number of Addresses read cannot be greater than 19");
	int addrSize = addresses.size();
	QByteArray bytes(19 + addrSize * 12, Qt::Uninitialized);
	bytes[0] =	static_cast<char>(3);
	bytes[1] =	static_cast<char>(0);
	bytes[2] =	static_cast<char>(bytes.length() / 256);
	bytes[3] =	static_cast<char>(bytes.length() % 256);
	bytes[4] =	static_cast<char>(2);
	bytes[5] =	static_cast<char>(240);
	bytes[6] =	static_cast<char>(128);
	bytes[7] =	static_cast<char>(50);
	bytes[8] =	static_cast<char>(1);
	bytes[9] =	static_cast<char>(0);
	bytes[10] = static_cast<char>(0);
	bytes[11] = static_cast<char>(0);
	bytes[12] = static_cast<char>(1);
	bytes[13] = static_cast<char>((bytes.length() - 17) / 256);
	bytes[14] = static_cast<char>((bytes.length() - 17) % 256);
	bytes[15] = static_cast<char>(0);
	bytes[16] = static_cast<char>(0);
	bytes[17] = static_cast<char>(4);
	bytes[18] = static_cast<char>(addrSize);
	for (int i = 0; i < addrSize; i++)
	{
		bytes[19 + i * 12] = static_cast<char>(18);
		bytes[20 + i * 12] = static_cast<char>(10);
		bytes[21 + i * 12] = static_cast<char>(16);
		bytes[22 + i * 12] = static_cast<char>(2);
		bytes[23 + i * 12] = static_cast<char>(addresses[i].length / 256);
		bytes[24 + i * 12] = static_cast<char>(addresses[i].length % 256);
		bytes[25 + i * 12] = static_cast<char>(addresses[i].dbBlock / 256);
		bytes[26 + i * 12] = static_cast<char>(addresses[i].dbBlock % 256);
		bytes[27 + i * 12] = static_cast<char>(addresses[i].dataCode);
		bytes[28 + i * 12] = static_cast<char>(addresses[i].addressStart / 256 / 256 % 256);
		bytes[29 + i * 12] = static_cast<char>(addresses[i].addressStart / 256 % 256);
		bytes[30 + i * 12] = static_cast<char>(addresses[i].addressStart % 256);
	}
	return QICResult<QByteArray>::CreateSuccessResult(bytes);
}

QICResult<QByteArray> SiemensS7Net::BuildBitReadPacket(const QString& address)
{
	QICResult<S7Address> r = S7Address::ParseFrom(address);
	if (!r.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(r);
	QByteArray bytes(31, Qt::Uninitialized);
	bytes[0] =	static_cast<char>(3);
	bytes[1] =	static_cast<char>(0);
	bytes[2] =	static_cast<char>(bytes.length() / 256);
	bytes[3] =	static_cast<char>(bytes.length() % 256);
	bytes[4] =	static_cast<char>(2);
	bytes[5] =	static_cast<char>(240);
	bytes[6] =	static_cast<char>(128);
	bytes[7] =	static_cast<char>(50);
	bytes[8] =	static_cast<char>(1);
	bytes[9] =	static_cast<char>(0);
	bytes[10] = static_cast<char>(0);
	bytes[11] = static_cast<char>(0);
	bytes[12] = static_cast<char>(1);
	bytes[13] = static_cast<char>((bytes.length() - 17) / 256);
	bytes[14] = static_cast<char>((bytes.length() - 17) % 256);
	bytes[15] = static_cast<char>(0);
	bytes[16] = static_cast<char>(0);
	bytes[17] = static_cast<char>(4);
	bytes[18] = static_cast<char>(1);
	bytes[19] = static_cast<char>(18);
	bytes[20] = static_cast<char>(10);
	bytes[21] = static_cast<char>(16);
	bytes[22] = static_cast<char>(1);
	bytes[23] = static_cast<char>(0);
	bytes[24] = static_cast<char>(1);
	bytes[25] = static_cast<char>(r.getContent0().dbBlock / 256);
	bytes[26] = static_cast<char>(r.getContent0().dbBlock % 256);
	bytes[27] = static_cast<char>(r.getContent0().dataCode);
	bytes[28] = static_cast<char>(r.getContent0().addressStart / 256 / 256 % 256);
	bytes[29] = static_cast<char>(r.getContent0().addressStart / 256 % 256);
	bytes[30] = static_cast<char>(r.getContent0().addressStart % 256);
	return QICResult<QByteArray>::CreateSuccessResult(bytes);
}

QICResult<QByteArray> SiemensS7Net::ReadS7Address(const QVector<S7Address>& addresses)
{
	QICResult<QByteArray> r = BuildReadPacket(addresses);
	if (!r.IsSuccess) return r;
	r = ReadFromCoreServer(r.getContent0());
	if (!r.IsSuccess) return r;
	return ParseReadByte(addresses, r.getContent0());
}

QICResult<QByteArray> SiemensS7Net::ParseReadByte(const QVector<S7Address>& addresses, const QByteArray& content)
{
	// 计算所有地址长度的总和
	int totalLength = 0;
	for (const auto& addr : addresses) totalLength += addr.length;
	// 检查内容长度是否足够，并且地址数量是否匹配
	if (content.length() >= 21 && 
		static_cast<unsigned char>(content.at(20)) == addresses.size())
	{
		// 初始化结果数组，大小为所有地址长度的总和
		QByteArray result(totalLength, Qt::Uninitialized);
		// 初始化结果位置指针和地址索引
		int resultPos = 0, addressIndex = 0;
		// 遍历内容，从索引21的字节开始
		for (int i = 21; i < content.length(); i++)
		{
			// 确保不会越界访问内容数组
			if (i + 1 < content.length())
			{
				// 检查是否为数据块的开始标志 0xFF 04
				if (static_cast<unsigned char>(content.at(i)) == 0xFF && 
					static_cast<unsigned char>(content.at(i + 1)) == 4)
				{
					// 从内容中提取对应长度的数据并复制到结果数组
					result.replace(resultPos, addresses[addressIndex].length, content.mid(i + 4, addresses[addressIndex].length));
					// 跳过已处理的字节
					i += addresses[addressIndex].length + 3;
					// 更新结果位置指针
					resultPos += addresses[addressIndex].length;
					// 移动到下一个地址
					addressIndex++;
				}
				// 检查是否超出了PLC设置的读取范围
				else if (static_cast<unsigned char>(content.at(i)) == 5 && 
					static_cast<unsigned char>(content.at(i + 1)) == 0)
				{
					// 返回失败结果，包含错误信息
					return QICResult<QByteArray>::CreateFailedResult("The range of data read is beyond the PLC setting");
				}
			}
		}
		// 返回成功结果，包含解析后的数据
		return QICResult<QByteArray>::CreateSuccessResult(result);
	}
	// 如果数据长度或地址数量验证失败，返回失败结果
	return QICResult<QByteArray>::CreateFailedResult("Data block length verification fails. Check whether put/get and db block optimization are enabled");
}

QICResult<QByteArray> SiemensS7Net::ParseReadBit(const QByteArray& content)
{
	// 初始化比特长度为1
	int numBits = 1;
	// 检查内容长度是否足够，并且地址数量是否为1
	if (content.length() >= 21 && static_cast<unsigned char>(content.at(20)) == 1)
	{
		// 初始化结果数组，大小为1字节，用于存储读取的比特数据
		QByteArray result(numBits, Qt::Uninitialized);
		// 检查数据是否包含至少22个字节，并验证数据块的起始标志 0xFF 03
		if (22 < content.length() && static_cast<unsigned char>(content.at(21)) == 0xFF && 
			static_cast<unsigned char>(content.at(22)) == 3)
		{
			// 如果条件满足，将第25个字节的数据复制到结果数组
			result[0] = content.at(25);
		}
		// 返回成功结果，包含解析后的比特数据
		return QICResult<QByteArray>::CreateSuccessResult(result);
	}
	// 如果数据长度或地址数量验证失败，返回失败结果
	return QICResult<QByteArray>::CreateFailedResult("Data block length verification fails. Check whether put/get and db block optimization are enabled");
}
