#pragma once

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QtMath>
#include "EthernetDevice.h"
#include "SiemensPLCS.h"
#include "S7Address.h"

class SiemensS7Net : public EthernetDevice
{
	Q_OBJECT
	Q_PROPERTY(quint8 plcRack READ getPlcRack WRITE setPlcRack)
	Q_PROPERTY(quint8 plcSlot READ getPlcSlot WRITE setPlcSlot)

public:
	explicit SiemensS7Net(SiemensPLCS siemens, const QString& ipAddr, QObject* parent = nullptr);
	~SiemensS7Net();

#pragma region Properties Implement
	quint8 getPlcRack() const { return plcRack; }
	void setPlcRack(quint8 value)
	{
		plcRack = value;
		plcHead1[21] = plcRack * 32 + plcSlot;
	}

	quint8 getPlcSlot() const { return plcSlot; }
	void setPlcSlot(quint8 value)
	{
		plcSlot = value;
		plcHead1[21] = plcRack * 32 + plcSlot;
	}

	void EnableSendRecvLog() { enableSendRecvLog = true; }
	void DisableSendRecvLog() { enableSendRecvLog = false; }
#pragma endregion

protected:
	QICResult<> InitializationOnConnect(QTcpSocket* socket) override;
	QICResult<> ReleaseOnDisconnect(QTcpSocket* socket) override;

private:
	void Initializetion(SiemensPLCS siemens, const QString& ipAddr);
	QICResult<> checkStartResult(QByteArray content);
	QICResult<> checkStopResult(QByteArray content);
	/**
	 * @brief ��ȡ���S7��ַ������
	 * @param addresses ��Ҫ��ȡ��S7Address�����б�
	 * @return ������ȡ�����QICResult���󣬳ɹ��򷵻ض�ȡ�����ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<QByteArray> Read(const QVector<S7Address>& addresses);
	/**
	 * @brief ��PLCд������
	 * @param bytes ��������
	 * @return �����Ƿ�����ɹ���QICResult����ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<> WritePLC(const QByteArray& bytes);

public:
	QICResult<QString> ReadOrderNumber();
	QICResult<> HotStart();
	QICResult<> ColdStart();
	QICResult<> Stop();

	// �û�������Ϊ Write �����к������������ж��ǿɼ��ģ����������Լ������ Write �������أ�Overload���������Ǳ����أ�Hide��
	using EthernetDevice::Write;

	QICResult<bool> ReadBool(const QString& address) override;
	QICResult<QByteArray> Read(const QString& address, ushort length) override;
	/**
	 * @brief ��ȡָ����ַ�б������
	 * @param addresses ��Ҫ��ȡ��PLC��ַ�б�
	 * @param length ��Ӧÿ����ַ�Ķ�ȡ����
	 * @return ������ȡ�����QICResult���󣬳ɹ��򷵻ض�ȡ�����ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<QByteArray> Read(const QStringList& addresses, const QVector<quint16>& length);
	/**
	 * @brief д�����ݵ�S7��ַ
	 * @param address S7��ַ
	 * @param value ����
	 * @return �����Ƿ�д��ɹ���QICResult����ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<> Write(const QString& address, const QByteArray& value) override;
	/**
	 * @brief д��λ��S7��ַ
	 * @param address S7��ַ
	 * @param value λֵ
	 * @return �����Ƿ�д��ɹ���QICResult����ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<> Write(const QString& address, bool value) override;
	/**
	 * @brief д����λ��S7��ַ
	 * @param address S7��ַ
	 * @param values λֵ����
	 * @return �����Ƿ�д��ɹ���QICResult����ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<> Write(const QString& address, const QVector<bool>& values);
	/**
	 * @brief �������ڶ�ȡ���S7��ַ���������ݰ�
	 * @param addresses ��Ҫ��ȡ��S7��ַ�б�
	 * @return �����������ݰ���QICResult���󣬳ɹ��򷵻�������ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	static QICResult<QByteArray> BuildReadRequest(const QVector<S7Address>& addresses);
	/**
	 * @brief �������ڶ�ȡ���ص��������ݰ�
	 * @param address Ҫ��ȡ��S7��ַ
	 * @return �����������ݰ���QICResult���󣬳ɹ��򷵻�������ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	static QICResult<QByteArray> BuildReadBitRequest(const QString& address);
	/**
	 * @brief ��������д��S7��ַ���������ݰ�
	 * @param address Ҫд���S7��ַ
	 * @param data Ҫд�������
	 * @return �����������ݰ���QICResult���󣬳ɹ��򷵻�������ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	static QICResult<QByteArray> BuildWriteRequest(const S7Address& address, const QByteArray& data);
	/**
	 * @brief ��������д����ص��������ݰ�
	 * @param address Ҫд���S7��ַ
	 * @param data Ҫд���boolֵ
	 * @return �����������ݰ���QICResult���󣬳ɹ��򷵻�������ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	static QICResult<QByteArray> BuildWriteBitRequest(const QString& address, bool value);

private:
	/**
	 * @brief ��ȡS7��ַ��λ����
	 * @param address ��ַ��Ϣ
	 * @return ������ȡ�����QICResult���󣬳ɹ��򷵻ض�ȡ�����ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<QByteArray> ReadAddressBit(const QString& address);
	/**
	 * @brief ��ȡ���S7��ַ�����ݡ�
	 * @param addresses ��Ҫ��ȡ��S7��ַ�б�
	 * @return ������ȡ�����QICResult���󣬳ɹ��򷵻ض�ȡ�����ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	QICResult<QByteArray> ReadAddressData(const QVector<S7Address>& addresses);
	/**
	 * @brief ������PLC���صĶ�ȡ�ֽ�����
	 * @param addresses ��ȡ��S7��ַ�б�
	 * @param content ���ص��ֽ�����
	 * @return �������������QICResult���󣬳ɹ��򷵻ؽ�������ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	static QICResult<QByteArray> ParseReadResponse(const QVector<S7Address>& addresses, const QByteArray& content);
	/**
	 * @brief ������PLC���صĶ�ȡ�ֽ�����
	 * @param address Ҫ��ȡ��S7��ַ
	 * @return �����������ݰ���QICResult���󣬳ɹ��򷵻�������ֽ����飬ʧ���򷵻�ʧ��ԭ��
	 */
	static QICResult<QByteArray> ParseReadBitResponse(const QByteArray& content);
	/**
	 * @brief ������PLC���ص����ݣ��Ӷ��ж�д���Ƿ�ɹ�
	 * @param content ��PLC���ص�����
	 * @return �����Ƿ�����ɹ���QICResult����ʧ���򷵻�ʧ��ԭ��
	 */
	static QICResult<> ParseWriteResponse(const QByteArray& content);
	/**
	 * @brief ��QVector<bool>ת��ΪQByteArray
	 * @param ba ���������ֵ
	 * @return ת����Ľ��
	 */
	QByteArray BoolVectorToByteArray(const QVector<bool>& values)
	{
		if (values.isEmpty()) return QByteArray();
		int size = qCeil(values.size() / 8.0);
		QByteArray result(size, 0);
		for (int i = 0; i < values.size(); i++)
		{
			if (values.at(i))
			{
				// ��ȡ��ǰ���ֽ�
				quint8 currByte = static_cast<quint8>(result[i / 8]);
				// �����ֽڵ�λ
				currByte |= (1 << (i % 8));
				// ���º���ֽ�д��
				result[i / 8] = static_cast<char>(currByte);
			}
		}
		return result;
	}
	/**
	 * @brief ��QByteArrayת����QVector<bool>
	 * @param bytes �ֽ�����
	 * @return ת����Ľ��
	 */
	QVector<bool> ByteArrayToBoolVector(const QByteArray& bytes)
	{
		if (bytes.isEmpty()) return QVector<bool>();
		int length = bytes.size() * 8;
		QVector<bool> result(length);
		for (int i = 0; i < length; i++)
		{
			result.append((bytes[i / 8] & (1 << (i % 8))) != 0);
		}
		return result;
	}

public:
	int pduLength = 0;

private:
	QByteArray plcHead1;
	QByteArray plcHead2;
	QByteArray plcOrderNumber;
	QByteArray plcHead1_200smart;
	QByteArray plcHead2_200smart;
	QByteArray plcHead1_200;
	QByteArray plcHead2_200;
	QByteArray S7_STOP;
	QByteArray S7_HOT_START;
	QByteArray S7_COLD_START;

	quint8 plcRack = 0;
	quint8 plcSlot = 0;
	const quint8 pduStart = 40;
	const quint8 pduStop = 41;
	const quint8 pduAlreadyStarted = 2;
	const quint8 pduAlreadyStopped = 7;
	SiemensPLCS currentPlc = SiemensPLCS::S1200;
};
