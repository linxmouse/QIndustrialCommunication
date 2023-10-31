#include "KeyenceNanoSerialOverTcp.h"

KeyenceNanoSerialOverTcp::KeyenceNanoSerialOverTcp(QString ipAddress, int port, bool isPersistentConn, 
	bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject* parent)
	: DeviceBase(ipAddress, port, isPersistentConn, enableSendRecvLog, connectTimeOut, receiveTimeOut, parent)
{
	ByteConverter = new ByteConverterBase(DataFormat::DCBA, true);
}

KeyenceNanoSerialOverTcp::~KeyenceNanoSerialOverTcp()
{
	CloseConnect();
}

QICResult<QByteArray> KeyenceNanoSerialOverTcp::Read(const QString& address, ushort length)
{
	// 构建读取数据包
	QICResult<QByteArray> buildResult = BuildReadPacket(address, length);
	if (!buildResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(buildResult);
	// 从核心服务器读取
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult checkResult = CheckPlcReadResponse(readResult.getContent<0>());
	if (!checkResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(checkResult);
	// 分析地址
	QICResult<QString, int> analysisResult = AnalysisAddress(address);
	if (!analysisResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(analysisResult);
	// 解析数据
	return ParsedData(analysisResult.getContent<0>(), readResult.getContent<0>());
}

QICResult<QVector<bool>> KeyenceNanoSerialOverTcp::ReadBool(const QString& address, ushort length)
{
	// 构建读取数据包
	auto buildResult = BuildReadPacket(address, length);
	if (!buildResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(buildResult);
	// 从核心服务器读取
	auto readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	auto checkResult = CheckPlcReadResponse(readResult.getContent<0>());
	if (!checkResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(checkResult);
	// 地址分析
	auto addressResult = AnalysisAddress(address);
	if (!addressResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(addressResult);
	// 解析数据
	return ParsedBoolData(addressResult.getContent<0>(), readResult.getContent<0>());
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString& address, const QByteArray& value)
{
	// 构建写入数据包
	QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// 从核心服务器读取响应
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult<> checkResult = CheckPlcWriteResponse(readResult.getContent<0>());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString& address, bool value)
{
	// 构建写入数据包
	QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// 从核心服务器读取响应
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult<> checkResult = CheckPlcWriteResponse(readResult.getContent<0>());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString& address, QVector<bool> value)
{
	// 构建写入数据包
	QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// 从核心服务器读取响应
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult<> checkResult = CheckPlcWriteResponse(readResult.getContent<0>());
	return checkResult;
}
