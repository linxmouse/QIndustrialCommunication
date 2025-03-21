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
	// ������ȡ���ݰ�
	QICResult<QByteArray> buildResult = BuildReadRequest(address, length);
	if (!buildResult.IsSuccess)
		return QICResult<QByteArray>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult<> checkResult = ParseReadResponse(readResult.getContent0());
	if (!checkResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(checkResult);
	// ������ַ
	QICResult<QString, int> analysisResult = ParseAddress(address);
	if (!analysisResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(analysisResult);
	// ��������
	return ParsedData(analysisResult.getContent0(), readResult.getContent0());
}

QICResult<QVector<bool>> KeyenceNanoSerialOverTcp::ReadBool(const QString &address, ushort length)
{
	// ������ȡ���ݰ�
	auto buildResult = BuildReadRequest(address, length);
	if (!buildResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ
	auto readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	auto checkResult = ParseReadResponse(readResult.getContent0());
	if (!checkResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(checkResult);
	// ��ַ����
	auto addressResult = ParseAddress(address);
	if (!addressResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(addressResult);
	// ��������
	return ParsedBoolData(addressResult.getContent0(), readResult.getContent0());
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString &address, const QByteArray &value)
{
	// ����д�����ݰ�
	QICResult<QByteArray> buildResult = BuildWriteRequest(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ��Ӧ
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult<> checkResult = ParseWriteResponse(readResult.getContent0());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString &address, bool value)
{
	// ����д�����ݰ�
	QICResult<QByteArray> buildResult = BuildWriteRequest(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ��Ӧ
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult<> checkResult = ParseWriteResponse(readResult.getContent0());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString &address, const QVector<bool>& values)
{
	// ����д�����ݰ�
	QICResult<QByteArray> buildResult = BuildWriteRequest(address, values);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ��Ӧ
	QICResult<QByteArray> readResult = ReadFromSocket(buildResult.getContent0());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult<> checkResult = ParseWriteResponse(readResult.getContent0());
	return checkResult;
}
