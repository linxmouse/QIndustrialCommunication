#include "SiemensS7Net.h"

SiemensS7Net::SiemensS7Net(SiemensPLCS siemens, const QString& ipAddr, QObject* parent)
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

void SiemensS7Net::Initializetion(SiemensPLCS siemens, const QString& ipAddr)
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

QICResult<bool> SiemensS7Net::ReadBool(const QString& address)
{
	auto result = ReadAddressBit(address);
	if (!result.IsSuccess) return QICResult<bool>::CreateFailedResult(result);
	auto content = result.getContent0();
	return QICResult<bool>::CreateSuccessResult(content.at(0) != 0);
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
	// 创建用于存储解析后的S7Address对象的向量，长度与地址列表相同
	QVector<S7Address> s7Addrs(addresses.length());
	// 循环解析每个地址，并检查解析结果是否成功
	for (int i = 0; i < addresses.length(); i++)
	{
		// 使用S7Address的静态方法从地址字符串和长度中解析出S7Address对象
		QICResult<S7Address> r = S7Address::ParseFrom(addresses[i], length[i]);
		// 如果解析失败，返回失败的QICResult对象
		if (!r.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(r);
		// 将成功解析的S7Address存储到s7Addr向量中
		s7Addrs[i] = r.getContent0();
	}
	// 调用内部的Read方法，传入解析后的S7Address列表
	return Read(s7Addrs);
}

QICResult<QByteArray> SiemensS7Net::Read(const QVector<S7Address>& addresses)
{
	// 如果地址数量超过19个，需要分批处理
	if (addresses.size() > 19)
	{
		// 创建结果字节数组
		QByteArray result;
		// 分批读取，每次最多读取19个地址
		for (int i = 0; i < addresses.size(); i += 19)
		{
			// 使用mid方法截取当前的地址子集，每次最多19个地址
			QVector<S7Address> chunk = addresses.mid(i, qMin(19, addresses.size() - i));
			// 读取当前子集的数据
			QICResult<QByteArray> chunkResult = Read(chunk);
			// 如果某一子集读取失败，返回失败的QICResult对象
			if (!chunkResult.IsSuccess) return chunkResult;
			// 将读取到的字节数据追加到结果中
			result.append(chunkResult.getContent0());
		}
		// 返回成功的QICResult对象，包含读取到的完整数据
		return QICResult<QByteArray>::CreateSuccessResult(result);
	}
	// 如果地址数量小于或等于19个，直接读取
	return ReadAddressData(addresses);
}

QICResult<QByteArray> SiemensS7Net::ReadAddressBit(const QString& address)
{
	QICResult<QByteArray> result = GenBitReadBytes(address);
	if (!result.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(result);
	result = ReadFromCoreServer(result.getContent0());
	if (!result.IsSuccess) return result;
	return ParseReadBit(result.getContent0());
}

QICResult<QByteArray> SiemensS7Net::ReadAddressData(const QVector<S7Address>& addresses)
{
	// 构建读取数据包
	QICResult<QByteArray> r = GenReadBytes(addresses);
	// 如果构建失败，返回失败结果
	if (!r.IsSuccess) return r;
	// 发送请求到核心服务器并读取返回数据
	r = ReadFromCoreServer(r.getContent0());
	// 如果读取失败，返回失败结果
	if (!r.IsSuccess) return r;
	// 解析读取的字节数据
	return ParseReadBytes(addresses, r.getContent0());
}

QICResult<QByteArray> SiemensS7Net::GenReadBytes(const QVector<S7Address>& addresses)
{
	// 如果地址列表为空，返回失败结果
	if (addresses.isEmpty()) return QICResult<QByteArray>::CreateFailedResult("S7 Addresses is empty");
	// 如果地址数量超过19个，返回失败结果
	if (addresses.size() > 19) return QICResult<QByteArray>::CreateFailedResult("The number of Addresses read cannot be greater than 19");
	// 初始化字节数组，长度为固定部分19字节加上每个地址12字节
	int addrSize = addresses.size();
	QByteArray bytes(19 + addrSize * 12, Qt::Uninitialized);
	// 填充前19字节的固定请求头信息
	bytes[0]  =	static_cast<char>(3);
	bytes[1]  =	static_cast<char>(0);
	bytes[2]  =	static_cast<char>(bytes.length() / 256);
	bytes[3]  =	static_cast<char>(bytes.length() % 256);
	bytes[4]  =	static_cast<char>(2);
	bytes[5]  =	static_cast<char>(240);
	bytes[6]  =	static_cast<char>(128);
	bytes[7]  =	static_cast<char>(50);
	bytes[8]  =	static_cast<char>(1);
	bytes[9]  =	static_cast<char>(0);
	bytes[10] = static_cast<char>(0);
	bytes[11] = static_cast<char>(0);
	bytes[12] = static_cast<char>(1);
	bytes[13] = static_cast<char>((bytes.length() - 17) / 256);
	bytes[14] = static_cast<char>((bytes.length() - 17) % 256);
	bytes[15] = static_cast<char>(0);
	bytes[16] = static_cast<char>(0);
	bytes[17] = static_cast<char>(4);
	bytes[18] = static_cast<char>(addrSize);
	// 填充每个S7地址的信息
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
	// 返回成功的QICResult对象，包含构建好的数据包
	return QICResult<QByteArray>::CreateSuccessResult(bytes);
}

QICResult<QByteArray> SiemensS7Net::GenBitReadBytes(const QString& address)
{
	// 解析地址字符串为S7Address对象
	QICResult<S7Address> r = S7Address::ParseFrom(address);
	// 如果解析失败，返回失败的QICResult对象
	if (!r.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(r);
	// 初始化请求的字节数组
	QByteArray bytes(31, Qt::Uninitialized);
	// 填充请求数据包的固定头部信息
	bytes[0]  = static_cast<char>(3);
	bytes[1]  = static_cast<char>(0);
	bytes[2]  = static_cast<char>(bytes.length() / 256);
	bytes[3]  = static_cast<char>(bytes.length() % 256);
	bytes[4]  = static_cast<char>(2);
	bytes[5]  = static_cast<char>(240);
	bytes[6]  = static_cast<char>(128);
	bytes[7]  = static_cast<char>(50);
	bytes[8]  = static_cast<char>(1);
	bytes[9]  = static_cast<char>(0);
	bytes[10] = static_cast<char>(0);
	bytes[11] = static_cast<char>(0);
	bytes[12] = static_cast<char>(1);
	bytes[13] = static_cast<char>((bytes.length() - 17) / 256);
	bytes[14] = static_cast<char>((bytes.length() - 17) % 256);
	bytes[15] = static_cast<char>(0);
	bytes[16] = static_cast<char>(0);
	bytes[17] = static_cast<char>(4);
	bytes[18] = static_cast<char>(1);
	// 填充解析出的S7Address信息
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
	// 返回成功的QICResult对象，包含构建好的数据包
	return QICResult<QByteArray>::CreateSuccessResult(bytes);
}

QICResult<QByteArray> SiemensS7Net::ParseReadBytes(const QVector<S7Address>& addresses, const QByteArray& content)
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

QICResult<> SiemensS7Net::WritePLC(const QByteArray& bytes)
{
	auto rcvRt = ReadFromCoreServer(bytes);
	if (!rcvRt.IsSuccess) return QICResult<>::CreateFailedResult(rcvRt);
	return ParseWrite(rcvRt.getContent0());
}

QICResult<> SiemensS7Net::Write(const QString& address, const QByteArray& value)
{
	QICResult<S7Address> addressRt = S7Address::ParseFrom(address);
	if (!addressRt.IsSuccess) return QICResult<>::CreateFailedResult(addressRt);
	
	int totalBytesNum = value.size();
	int writedBytesNum = 0;
	S7Address s7Address = addressRt.getContent0();
	while (writedBytesNum < totalBytesNum)
	{
		ushort toWriteBytesNum = qMin(totalBytesNum - writedBytesNum, pduLength);
		auto bytes = value.mid(writedBytesNum, toWriteBytesNum);
		QICResult<QByteArray> packedBytesRt = GenWriteBytes(s7Address, bytes);
		if (!packedBytesRt.IsSuccess) return QICResult<>::CreateFailedResult(packedBytesRt);
		QICResult<> isSuc = WritePLC(packedBytesRt.getContent0());
		if (!isSuc.IsSuccess) return isSuc;
		writedBytesNum += toWriteBytesNum;
		s7Address.addressStart += toWriteBytesNum * 8;
	}
	return QICResult<>::CreateSuccessResult();
}

QICResult<> SiemensS7Net::Write(const QString& address, bool value)
{
	QICResult<QByteArray> packedBytesRt = GenBitWriteBytes(address, value);
	if (!packedBytesRt.IsSuccess) return QICResult<>::CreateFailedResult(packedBytesRt);
	return WritePLC(packedBytesRt.getContent0());
}

QICResult<> SiemensS7Net::Write(const QString& address, const QVector<bool>& values)
{
	QByteArray bytes = BoolVectorToByteArray(values);
	return Write(address, bytes);
}

QICResult<QByteArray> SiemensS7Net::GenWriteBytes(const S7Address& address, const QByteArray& data)
{
	QByteArray array(35 + data.size(), Qt::Uninitialized);
	array[0]  = static_cast<char>(3);
	array[1]  = static_cast<char>(0);
	array[2]  = static_cast<char>((35 + data.size()) / 256);
	array[3]  = static_cast<char>((35 + data.size()) % 256);
	array[4]  = static_cast<char>(2);
	array[5]  = static_cast<char>(240);
	array[6]  = static_cast<char>(128);
	array[7]  = static_cast<char>(50);
	array[8]  = static_cast<char>(1);
	array[9]  = static_cast<char>(0);
	array[10] = static_cast<char>(0);
	array[11] = static_cast<char>(0);
	array[12] = static_cast<char>(1);
	array[13] = static_cast<char>(0);
	array[14] = static_cast<char>(14);
	array[15] = static_cast<char>((4 + data.size()) / 256);
	array[16] = static_cast<char>((4 + data.size()) % 256);
	array[17] = static_cast<char>(5);
	array[18] = static_cast<char>(1);
	array[19] = static_cast<char>(18);
	array[20] = static_cast<char>(10);
	array[21] = static_cast<char>(16);
	array[22] = static_cast<char>(2);
	array[23] = static_cast<char>(data.size() / 256);
	array[24] = static_cast<char>(data.size() % 256);
	array[25] = static_cast<char>(address.dbBlock / 256);
	array[26] = static_cast<char>(address.dbBlock % 256);
	array[27] = static_cast<char>(address.dataCode);
	array[28] = static_cast<char>(address.addressStart / 256 / 256 % 256);
	array[29] = static_cast<char>(address.addressStart / 256 % 256);
	array[30] = static_cast<char>(address.addressStart % 256);
	array[31] = static_cast<char>(0);
	array[32] = static_cast<char>(4);
	array[33] = static_cast<char>(data.size() * 8 / 256);
	array[34] = static_cast<char>(data.size() * 8 % 256);
	array.replace(35, data.size(), data);
	return QICResult<QByteArray>::CreateSuccessResult(array);
}

QICResult<QByteArray> SiemensS7Net::GenBitWriteBytes(const QString& address, bool value)
{
	QICResult<S7Address> addressRt = S7Address::ParseFrom(address);
	if (!addressRt.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(addressRt);
	
	S7Address s7Addr = addressRt.getContent0();
	QByteArray data(1, value ? 1 : 0);
	QByteArray array(35 + data.size(), Qt::Uninitialized);
	array[0]  = static_cast<char>(3);
	array[1]  = static_cast<char>(0);
	array[2]  = static_cast<char>((35 + data.size()) / 256);
	array[3]  = static_cast<char>((35 + data.size()) % 256);
	array[4]  = static_cast<char>(2);
	array[5]  = static_cast<char>(240);
	array[6]  = static_cast<char>(128);
	array[7]  = static_cast<char>(50);
	array[8]  = static_cast<char>(1);
	array[9]  = static_cast<char>(0);
	array[10] = static_cast<char>(0);
	array[11] = static_cast<char>(0);
	array[12] = static_cast<char>(1);
	array[13] = static_cast<char>(0);
	array[14] = static_cast<char>(14);
	array[15] = static_cast<char>((4 + data.size()) / 256);
	array[16] = static_cast<char>((4 + data.size()) % 256);
	array[17] = static_cast<char>(5);
	array[18] = static_cast<char>(1);
	array[19] = static_cast<char>(18);
	array[20] = static_cast<char>(10);
	array[21] = static_cast<char>(16);
	array[22] = static_cast<char>(1);
	array[23] = static_cast<char>(data.size() / 256);
	array[24] = static_cast<char>(data.size() % 256);
	array[25] = static_cast<char>(s7Addr.dbBlock / 256);
	array[26] = static_cast<char>(s7Addr.dbBlock % 256);
	array[27] = static_cast<char>(s7Addr.dataCode);
	array[28] = static_cast<char>(s7Addr.addressStart / 256 / 256 % 256);
	array[29] = static_cast<char>(s7Addr.addressStart / 256 % 256);
	array[30] = static_cast<char>(s7Addr.addressStart % 256);
	if (s7Addr.dataCode == 28)
	{
		array[31] = static_cast<char>(0);
		array[32] = static_cast<char>(9);
	}
	else 
	{
		array[31] = static_cast<char>(0);
		array[32] = static_cast<char>(3);
	}
	array[33] = static_cast<char>(data.size() * 8 / 256);
	array[34] = static_cast<char>(data.size() * 8 % 256);
	array.replace(35, data.size(), data);
	return QICResult<QByteArray>::CreateSuccessResult(array);
}

QICResult<> SiemensS7Net::ParseWrite(const QByteArray& content)
{
	quint8 ch = content.at(content.size() - 1);
	if (ch != 0xFF)
	{
		QString message = QString("Write data exception, \nData: %1").arg(QString::fromLatin1(content.toHex(' ')));
		return QICResult<>(false, message, ch);
	}
	return QICResult<>::CreateSuccessResult();
}
