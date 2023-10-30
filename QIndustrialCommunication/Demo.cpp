#include <QCoreApplication>
#include "KeyenceNanoSerialOverTcp.h"

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	// User Code
	QICResult<int, QString> result = QICResult<int, QString>::CreateSuccessResult(42, "Hello");
	qDebug() << result.ToMessageShowString();
	qDebug() << "Content1: " << result.GetContent<0>(); // 输出 42
	qDebug() << "Content2: " << result.GetContent<1>(); // 输出 "Hello"

	KeyenceNanoSerialOverTcp overTcp{ "192.168.0.78", 8501, true, false };
	qDebug() << overTcp;

	//// 如果R305 = true, 那么DM82为float(88.88), DM84为ushort(77)
	//// 如果R306 = true, 那么DM82、DM84的值都为0
	//auto r305w = overTcp.Write("R305", true);
	//if (r305w.IsSuccess) qDebug() << "Write true to R305 success!";
	//auto r305r = overTcp.ReadBool("R305");
	//if (r305r.IsSuccess) qDebug() << QString("Read R305's value: %1").arg(r305r.GetContent1());
	//r305w = overTcp.Write("R305", false);
	//if (r305w.IsSuccess) qDebug() << "Write true to R305 success!";
	//r305r = overTcp.ReadBool("R305");
	//if (r305r.IsSuccess) qDebug() << QString("Read R305's value: %1").arg(r305r.GetContent1());
	//r305w = overTcp.Write("R305", true);
	//if (r305w.IsSuccess) qDebug() << "Write true to R305 success!";
	//r305r = overTcp.ReadBool("R305");
	//if (r305r.IsSuccess) qDebug() << QString("Read R305's value: %1").arg(r305r.GetContent1());

	//auto dm84r = overTcp.ReadUInt16("dm84");
	//if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.GetContent1();
	//auto wr = overTcp.Write("dm84", (ushort)251);
	//if (wr.IsSuccess) qDebug() << "Write 251 to DM84 success";
	//dm84r = overTcp.ReadUInt16("dm84");
	//if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.GetContent1();
	//wr = overTcp.Write("dm84", (ushort)456);
	//if (wr.IsSuccess) qDebug() << "Write 456 to DM84 success";
	//dm84r = overTcp.ReadUInt16("dm84");
	//if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.GetContent1();
	//wr = overTcp.Write("dm84", (ushort)65530);
	//if (wr.IsSuccess) qDebug() << "Write 65530 to DM84 success";
	//dm84r = overTcp.ReadUInt16("dm84");
	//if (dm84r.IsSuccess) qDebug() << "dm84's value: " << dm84r.GetContent1();
	
	//auto wr = overTcp.Write("dm86", 12348765);
	//if (wr.IsSuccess) qDebug() << "Write 12348765 to DM86 success";
	//auto dm86r = overTcp.ReadUInt32("dm86");
	//if (dm86r.IsSuccess) qDebug() << "dm86's value: " << dm86r.GetContent1();
	//wr = overTcp.Write("dm86", 98761234);
	//if (wr.IsSuccess) qDebug() << "Write 98761234 to DM86 success";
	//dm86r = overTcp.ReadUInt32("dm86");
	//if (dm86r.IsSuccess) qDebug() << "dm86's value: " << dm86r.GetContent1();
	//wr = overTcp.Write("dm86", 65535781);
	//if (wr.IsSuccess) qDebug() << "Write 65535781 to DM86 success";
	//dm86r = overTcp.ReadUInt32("dm86");
	//if (dm86r.IsSuccess) qDebug() << "dm86's value: " << dm86r.GetContent1();

	auto dm88r = overTcp.ReadString("dm88", 2);
	if (dm88r.IsSuccess) qDebug() << "dm88's value: " << dm88r.GetContent1();
	else qWarning() << dm88r.Message;
	auto dm88w = overTcp.WriteString("dm88", "abcd");
	if (dm88w.IsSuccess) qDebug() << "write 'abcd' to dm88 success!";
	else qWarning() << dm88w.Message;
	dm88r = overTcp.ReadString("dm88", 2);
	if (dm88r.IsSuccess) qDebug() << "dm88's value: " << dm88r.GetContent1();
	else qWarning() << dm88r.Message;

	return app.exec();
}
