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
	// ������ȡ���ݰ�
	QICResult<QByteArray> buildResult = BuildReadPacket(address, length);
	if (!buildResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult checkResult = CheckPlcReadResponse(readResult.getContent<0>());
	if (!checkResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(checkResult);
	// ������ַ
	QICResult<QString, int> analysisResult = AnalysisAddress(address);
	if (!analysisResult.IsSuccess) return QICResult<QByteArray>::CreateFailedResult(analysisResult);
	// ��������
	return ParsedData(analysisResult.getContent<0>(), readResult.getContent<0>());
}

QICResult<QVector<bool>> KeyenceNanoSerialOverTcp::ReadBool(const QString& address, ushort length)
{
	// ������ȡ���ݰ�
	auto buildResult = BuildReadPacket(address, length);
	if (!buildResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ
	auto readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	auto checkResult = CheckPlcReadResponse(readResult.getContent<0>());
	if (!checkResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(checkResult);
	// ��ַ����
	auto addressResult = AnalysisAddress(address);
	if (!addressResult.IsSuccess) return QICResult<QVector<bool>>::CreateFailedResult(addressResult);
	// ��������
	return ParsedBoolData(addressResult.getContent<0>(), readResult.getContent<0>());
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString& address, const QByteArray& value)
{
	// ����д�����ݰ�
	QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ��Ӧ
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult<> checkResult = CheckPlcWriteResponse(readResult.getContent<0>());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString& address, bool value)
{
	// ����д�����ݰ�
	QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ��Ӧ
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult<> checkResult = CheckPlcWriteResponse(readResult.getContent<0>());
	return checkResult;
}

QICResult<> KeyenceNanoSerialOverTcp::Write(const QString& address, QVector<bool> value)
{
	// ����д�����ݰ�
	QICResult<QByteArray> buildResult = BuildWritePacket(address, value);
	if (!buildResult.IsSuccess) return QICResult<>::CreateFailedResult(buildResult);
	// �Ӻ��ķ�������ȡ��Ӧ
	QICResult<QByteArray> readResult = ReadFromCoreServer(buildResult.getContent<0>());
	if (!readResult.IsSuccess) return QICResult<>::CreateFailedResult(readResult);
	// ���PLCд����ӦЭ���ʽ
	QICResult<> checkResult = CheckPlcWriteResponse(readResult.getContent<0>());
	return checkResult;
}