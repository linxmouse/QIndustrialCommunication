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
	// 1. ������ַ�ַ���,��ȡ S7Address ����
	QICResult<S7Address> rAddr = S7Address::ParseFrom(address, length);
	if (!rAddr.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(rAddr);	// ��ַ����ʧ��,����ʧ�ܽ��
	QByteArray result;																// �洢���ն�ȡ������
	quint16 readedBytesNum = 0;														// �Ѷ�ȡ���ֽ���
	auto s7Addr = rAddr.getContent0();												// ��ȡ������ĵ�ַ

	// 2. ѭ����ȡ����,ÿ�ζ�ȡ pduLength ���ȵ�����
	while (readedBytesNum < length) 
	{
		quint16 toReadBytesNum = qMin(length - readedBytesNum, pduLength);	// ���ζ�ȡ�ĳ���
		s7Addr.length = toReadBytesNum;										// ���ñ��ζ�ȡ�ĳ���
		QICResult<QByteArray> rRead = Read(QVector<S7Address>{ s7Addr });	// ��ȡ����
		if (!rRead.IsSuccess) return rRead;									// ��ȡʧ��,����ʧ�ܽ��

		result.append(rRead.getContent0());									// ����ȡ������׷�ӵ������
		readedBytesNum += toReadBytesNum;									// �����Ѷ�ȡ���ֽ���
		s7Addr.addressStart += toReadBytesNum * 8;							// �����´ζ�ȡ����ʼ��ַ
	}

	// 3. ���سɹ����
	return QICResult<QByteArray>::CreateSuccessResult(result);
}

QICResult<QByteArray> SiemensS7Net::Read(const QStringList& addresses, const QVector<quint16>& length)
{
	// �������ڴ洢�������S7Address������������������ַ�б���ͬ
	QVector<S7Address> s7Addrs(addresses.length());
	// ѭ������ÿ����ַ��������������Ƿ�ɹ�
	for (int i = 0; i < addresses.length(); i++)
	{
		// ʹ��S7Address�ľ�̬�����ӵ�ַ�ַ����ͳ����н�����S7Address����
		QICResult<S7Address> r = S7Address::ParseFrom(addresses[i], length[i]);
		// �������ʧ�ܣ�����ʧ�ܵ�QICResult����
		if (!r.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(r);
		// ���ɹ�������S7Address�洢��s7Addr������
		s7Addrs[i] = r.getContent0();
	}
	// �����ڲ���Read����������������S7Address�б�
	return Read(s7Addrs);
}

QICResult<QByteArray> SiemensS7Net::Read(const QVector<S7Address>& addresses)
{
	// �����ַ��������19������Ҫ��������
	if (addresses.size() > 19)
	{
		// ��������ֽ�����
		QByteArray result;
		// ������ȡ��ÿ������ȡ19����ַ
		for (int i = 0; i < addresses.size(); i += 19)
		{
			// ʹ��mid������ȡ��ǰ�ĵ�ַ�Ӽ���ÿ�����19����ַ
			QVector<S7Address> chunk = addresses.mid(i, qMin(19, addresses.size() - i));
			// ��ȡ��ǰ�Ӽ�������
			QICResult<QByteArray> chunkResult = Read(chunk);
			// ���ĳһ�Ӽ���ȡʧ�ܣ�����ʧ�ܵ�QICResult����
			if (!chunkResult.IsSuccess) return chunkResult;
			// ����ȡ�����ֽ�����׷�ӵ������
			result.append(chunkResult.getContent0());
		}
		// ���سɹ���QICResult���󣬰�����ȡ������������
		return QICResult<QByteArray>::CreateSuccessResult(result);
	}
	// �����ַ����С�ڻ����19����ֱ�Ӷ�ȡ
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
	// ������ȡ���ݰ�
	QICResult<QByteArray> r = GenReadBytes(addresses);
	// �������ʧ�ܣ�����ʧ�ܽ��
	if (!r.IsSuccess) return r;
	// �������󵽺��ķ���������ȡ��������
	r = ReadFromCoreServer(r.getContent0());
	// �����ȡʧ�ܣ�����ʧ�ܽ��
	if (!r.IsSuccess) return r;
	// ������ȡ���ֽ�����
	return ParseReadBytes(addresses, r.getContent0());
}

QICResult<QByteArray> SiemensS7Net::GenReadBytes(const QVector<S7Address>& addresses)
{
	// �����ַ�б�Ϊ�գ�����ʧ�ܽ��
	if (addresses.isEmpty()) return QICResult<QByteArray>::CreateFailedResult("S7 Addresses is empty");
	// �����ַ��������19��������ʧ�ܽ��
	if (addresses.size() > 19) return QICResult<QByteArray>::CreateFailedResult("The number of Addresses read cannot be greater than 19");
	// ��ʼ���ֽ����飬����Ϊ�̶�����19�ֽڼ���ÿ����ַ12�ֽ�
	int addrSize = addresses.size();
	QByteArray bytes(19 + addrSize * 12, Qt::Uninitialized);
	// ���ǰ19�ֽڵĹ̶�����ͷ��Ϣ
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
	// ���ÿ��S7��ַ����Ϣ
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
	// ���سɹ���QICResult���󣬰��������õ����ݰ�
	return QICResult<QByteArray>::CreateSuccessResult(bytes);
}

QICResult<QByteArray> SiemensS7Net::GenBitReadBytes(const QString& address)
{
	// ������ַ�ַ���ΪS7Address����
	QICResult<S7Address> r = S7Address::ParseFrom(address);
	// �������ʧ�ܣ�����ʧ�ܵ�QICResult����
	if (!r.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(r);
	// ��ʼ��������ֽ�����
	QByteArray bytes(31, Qt::Uninitialized);
	// ����������ݰ��Ĺ̶�ͷ����Ϣ
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
	// ����������S7Address��Ϣ
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
	// ���سɹ���QICResult���󣬰��������õ����ݰ�
	return QICResult<QByteArray>::CreateSuccessResult(bytes);
}

QICResult<QByteArray> SiemensS7Net::ParseReadBytes(const QVector<S7Address>& addresses, const QByteArray& content)
{
	// �������е�ַ���ȵ��ܺ�
	int totalLength = 0;
	for (const auto& addr : addresses) totalLength += addr.length;
	// ������ݳ����Ƿ��㹻�����ҵ�ַ�����Ƿ�ƥ��
	if (content.length() >= 21 && 
		static_cast<unsigned char>(content.at(20)) == addresses.size())
	{
		// ��ʼ��������飬��СΪ���е�ַ���ȵ��ܺ�
		QByteArray result(totalLength, Qt::Uninitialized);
		// ��ʼ�����λ��ָ��͵�ַ����
		int resultPos = 0, addressIndex = 0;
		// �������ݣ�������21���ֽڿ�ʼ
		for (int i = 21; i < content.length(); i++)
		{
			// ȷ������Խ�������������
			if (i + 1 < content.length())
			{
				// ����Ƿ�Ϊ���ݿ�Ŀ�ʼ��־ 0xFF 04
				if (static_cast<unsigned char>(content.at(i)) == 0xFF && 
					static_cast<unsigned char>(content.at(i + 1)) == 4)
				{
					// ����������ȡ��Ӧ���ȵ����ݲ����Ƶ��������
					result.replace(resultPos, addresses[addressIndex].length, content.mid(i + 4, addresses[addressIndex].length));
					// �����Ѵ�����ֽ�
					i += addresses[addressIndex].length + 3;
					// ���½��λ��ָ��
					resultPos += addresses[addressIndex].length;
					// �ƶ�����һ����ַ
					addressIndex++;
				}
				// ����Ƿ񳬳���PLC���õĶ�ȡ��Χ
				else if (static_cast<unsigned char>(content.at(i)) == 5 && 
					static_cast<unsigned char>(content.at(i + 1)) == 0)
				{
					// ����ʧ�ܽ��������������Ϣ
					return QICResult<QByteArray>::CreateFailedResult("The range of data read is beyond the PLC setting");
				}
			}
		}
		// ���سɹ���������������������
		return QICResult<QByteArray>::CreateSuccessResult(result);
	}
	// ������ݳ��Ȼ��ַ������֤ʧ�ܣ�����ʧ�ܽ��
	return QICResult<QByteArray>::CreateFailedResult("Data block length verification fails. Check whether put/get and db block optimization are enabled");
}

QICResult<QByteArray> SiemensS7Net::ParseReadBit(const QByteArray& content)
{
	// ��ʼ�����س���Ϊ1
	int numBits = 1;
	// ������ݳ����Ƿ��㹻�����ҵ�ַ�����Ƿ�Ϊ1
	if (content.length() >= 21 && static_cast<unsigned char>(content.at(20)) == 1)
	{
		// ��ʼ��������飬��СΪ1�ֽڣ����ڴ洢��ȡ�ı�������
		QByteArray result(numBits, Qt::Uninitialized);
		// ��������Ƿ��������22���ֽڣ�����֤���ݿ����ʼ��־ 0xFF 03
		if (22 < content.length() && static_cast<unsigned char>(content.at(21)) == 0xFF && 
			static_cast<unsigned char>(content.at(22)) == 3)
		{
			// ����������㣬����25���ֽڵ����ݸ��Ƶ��������
			result[0] = content.at(25);
		}
		// ���سɹ����������������ı�������
		return QICResult<QByteArray>::CreateSuccessResult(result);
	}
	// ������ݳ��Ȼ��ַ������֤ʧ�ܣ�����ʧ�ܽ��
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
