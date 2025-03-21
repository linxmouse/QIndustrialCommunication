#include "KeyenceNanoSerialOverTcp.h"

KeyenceNanoSerialOverTcp::KeyenceNanoSerialOverTcp(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject *parent)
	: EthernetDevice(ipAddr, port, isPersistentConn, enableSendRecvLog, connectTimeOut, receiveTimeOut, parent)
{
	WordLenght = 1;
	BytesOrderPtr.reset(new BytesOrderBase(DataFormat::DCBA, true));
}

KeyenceNanoSerialOverTcp::~KeyenceNanoSerialOverTcp()
{
	CloseConnect();
}

QICResult<QByteArray> KeyenceNanoSerialOverTcp::Read(const QString &address, ushort length)
{
	// 构建读取数据包
	QICResult<QByteArray> buildResult = BuildReadRequest(address, length);
	if (!buildResult.IsSuccess)
		return QICResult<QByteArray>::CreateFailedResult(buildResult);
	// 从核心服务器读取
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult<> checkResult = ParseReadResponse(readResult.getContent0());
	if (!checkResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(checkResult);
	// 分析地址
	QICResult<QString, int> analysisResult = ParseAddress(address);
	if (!analysisResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(analysisResult);
	// 解析数据
	return ParsedData(analysisResult.getContent0(), readResult.getContent0());
}

QICResult<QVector<bool>> KeyenceNanoSerialOverTcp::ReadBool(const QString &address, ushort length)
{
	// 构建读取数据包
	auto buildResult = BuildReadRequest(address, length);
	if (!buildResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(buildResult);
	// 从核心服务器读取
	auto readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	auto checkResult = ParseReadResponse(readResult.getContent0());
	if (!checkResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(checkResult);
	// 地址分析
	auto addressResult = ParseAddress(address);
	if (!addressResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(addressResult);
	// 解析数据
	return ParsedBoolData(addressResult.getContent0(), readResult.getContent0());
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString &address, const QByteArray &value)
{
	// 构建写入数据包
	QICResult<QByteArray> buildResult = BuildWriteRequest(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// 从核心服务器读取响应
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult<> checkResult = ParseWriteResponse(readResult.getContent0());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString &address, bool value)
{
	// 构建写入数据包
	QICResult<QByteArray> buildResult = BuildWriteRequest(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// 从核心服务器读取响应
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult<> checkResult = ParseWriteResponse(readResult.getContent0());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString &address, const QVector<bool>& values)
{
	// 构建写入数据包
	QICResult<QByteArray> buildResult = BuildWriteRequest(address, values);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// 从核心服务器读取响应
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// 检查PLC写入响应协议格式
	QICResult<> checkResult = ParseWriteResponse(readResult.getContent0());
	return checkResult;
}
