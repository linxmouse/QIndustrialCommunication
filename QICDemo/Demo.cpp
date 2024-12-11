#include <QtMath>

#include "KeyenceNanoSerialOverTcp.h"
#include "SiemensS7Net.h"
#include "BytesOrderHelper.h"

int main(int argc, char* argv[])
{
	// User Code
	QICResult<int, QString> result = QICResult<int, QString>::CreateSuccessResult(42, "Hello");
	qDebug() << result.ToMessageShowString();
	qDebug() << "Content0: " << result.getContent0(); // 输出 42
	qDebug() << "Content1: " << result.getContent1(); // 输出 "Hello"

#if 0 // 基恩士测试
	KeyenceNanoSerialOverTcp overTcp{ "192.168.0.78", 8501, true, false };
	qDebug() << overTcp;

	// 如果R305 = true, 那么DM82为float(88.88), DM84为ushort(77)
	// 如果R306 = true, 那么DM82、DM84的值都为0
	auto r305w = overTcp.Write("R305", true);
	if (r305w.IsSuccess) qDebug() << "Write true to R305 success!";
	auto r305r = overTcp.ReadBool("R305");
	if (r305r.IsSuccess) qDebug() << QString("Read R305's value: %1").arg(r305r.getContent0());
	r305w = overTcp.Write("R305", false);
	if (r305w.IsSuccess) qDebug() << "Write true to R305 success!";
	r305r = overTcp.ReadBool("R305");
	if (r305r.IsSuccess) qDebug() << QString("Read R305's value: %1").arg(r305r.getContent0());
	r305w = overTcp.Write("R305", true);
	if (r305w.IsSuccess) qDebug() << "Write true to R305 success!";
	r305r = overTcp.ReadBool("R305");
	if (r305r.IsSuccess) qDebug() << QString("Read R305's value: %1").arg(r305r.getContent0());
	auto r305batchw = overTcp.Write("R305", QVector<bool>{false, false});

	auto dm84r = overTcp.ReadUInt16("dm84");
	if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.getContent0();
	auto wr = overTcp.Write("dm84", (ushort)251);
	if (wr.IsSuccess) qDebug() << "Write 251 to DM84 success";
	dm84r = overTcp.ReadUInt16("dm84");
	if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.getContent0();
	wr = overTcp.Write("dm84", (ushort)456);
	if (wr.IsSuccess) qDebug() << "Write 456 to DM84 success";
	dm84r = overTcp.ReadUInt16("dm84");
	if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.getContent0();
	wr = overTcp.Write("dm84", (ushort)65530);
	if (wr.IsSuccess) qDebug() << "Write 65530 to DM84 success";
	dm84r = overTcp.ReadUInt16("dm84");
	if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.getContent0();

	wr = overTcp.Write("dm86", 12348765);
	if (wr.IsSuccess) qDebug() << "Write 12348765 to DM86 success";
	auto dm86r = overTcp.ReadUInt32("dm86");
	if (dm86r.IsSuccess) qDebug() << "dm86's value: " << dm86r.getContent0();
	wr = overTcp.Write("dm86", 98761234);
	if (wr.IsSuccess) qDebug() << "Write 98761234 to DM86 success";
	dm86r = overTcp.ReadUInt32("dm86");
	if (dm86r.IsSuccess) qDebug() << "dm86's value: " << dm86r.getContent0();
	wr = overTcp.Write("dm86", 65535781);
	if (wr.IsSuccess) qDebug() << "Write 65535781 to DM86 success";
	dm86r = overTcp.ReadUInt32("dm86");
	if (dm86r.IsSuccess) qDebug() << "dm86's value: " << dm86r.getContent0();

	auto dm88r = overTcp.ReadString("dm88", 2);
	if (dm88r.IsSuccess) qDebug() << "dm88's value: " << dm88r.getContent0();
	else qWarning() << dm88r.Message;
	auto dm88w = overTcp.WriteString("dm88", "abcd");
	if (dm88w.IsSuccess) qDebug() << "write 'abcd' to dm88 success!";
	else qWarning() << dm88w.Message;
	dm88r = overTcp.ReadString("dm88", 2);
	if (dm88r.IsSuccess) qDebug() << "dm88's value: " << dm88r.getContent0();
	else qWarning() << dm88r.Message;
#endif // 0 // 基恩士测试

#if 0 // 西门子S7测试
	SiemensS7Net s7Net(SiemensPLCS::S1200, "127.0.0.1");
	s7Net.DisableSendRecvLog();

	qDebug() << "PLC OrderNumber: " << s7Net.ReadOrderNumber().getContent0();

	QICResult<int> rInt = s7Net.ReadInt32("DB3400.0");
	qDebug() << "Readed Int32's value: " << rInt.getContent0();

	QICResult<QVector<int>> rInts = s7Net.ReadInt32("DB3400.0", 2);
	int index = 0;
	for (const auto& v : rInts.getContent0())
	{
		qDebug() << QString("Array's index %1 value ").arg(index++) << v;
	}

	QICResult<bool> rBoolean = s7Net.ReadBool("db3400.5");
	qDebug() << "Read db3400.5 boolean's value: " << rBoolean.getContent0();
	rBoolean = s7Net.ReadBool("db3400.6");
	qDebug() << "Read db3400.6 boolean's value: " << rBoolean.getContent0();
	rBoolean = s7Net.ReadBool("db3400.7");
	qDebug() << "Read db3400.7 boolean's value: " << rBoolean.getContent0();

	auto isWriteSucc = s7Net.Write("db3400.5.1", true);
	isWriteSucc = s7Net.Write("db3400.10", (int)123456);
	isWriteSucc = s7Net.Write("db3400.20", (uint)123456789);

	QVector<bool> values{ true, false, true, false, true, false };
	isWriteSucc = s7Net.Write("db3400.30", values);

	auto rBooleans = s7Net.ReadBool("db3400.5.1");
#endif // 1 // 西门子S7测试

#if 0 // 测试本地字节序到网络字节序的转换
	if (BytesOrderHelper::isLittleEndian()) qDebug() << QString::fromLocal8Bit("运行在小端系统上");
	quint16 value = 0x1234;
	quint16 networkValue = BytesOrderHelper::toNetworkOrder(value);
	quint16 rcvValue = BytesOrderHelper::fromNetworkOrder(networkValue);
#endif // 1

#if 1 // 测试不同大小和不同格式的数据转换的可逆性
	int intValue = 0x12345678;
	short shortValue = 0x1234;
	qint64 longValue = 0x123456789ABCDEF0LL;

	BytesOrderBase::testConversion<int>(intValue, DataFormat::ABCD);
	BytesOrderBase::testConversion<int>(intValue, DataFormat::BADC);
	BytesOrderBase::testConversion<int>(intValue, DataFormat::CDAB);
	BytesOrderBase::testConversion<int>(intValue, DataFormat::DCBA);

	BytesOrderBase::testConversion<short>(shortValue, DataFormat::ABCD);
	BytesOrderBase::testConversion<short>(shortValue, DataFormat::DCBA);
	BytesOrderBase::testConversion<short>(shortValue, DataFormat::BADC);
	BytesOrderBase::testConversion<short>(shortValue, DataFormat::CDAB);

	BytesOrderBase::testConversion<long>(longValue, DataFormat::ABCD);
	BytesOrderBase::testConversion<long>(longValue, DataFormat::DCBA);
	BytesOrderBase::testConversion<long>(longValue, DataFormat::BADC);
	BytesOrderBase::testConversion<long>(longValue, DataFormat::CDAB);
#endif

	system("pause");
}
