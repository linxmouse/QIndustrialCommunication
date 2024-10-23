#pragma once

#include <QObject>
#include <QVector>
#include <QStringList>
#include "EthernetDevice.h"
#include "SiemensPLCS.h"
#include "S7Address.h"

class SiemensS7Net : public EthernetDevice
{
	Q_OBJECT
	Q_PROPERTY(quint8 plcRack READ getPlcRack WRITE setPlcRack)
	Q_PROPERTY(quint8 plcSlot READ getPlcSlot WRITE setPlcSlot)

public:
	explicit SiemensS7Net(SiemensPLCS siemens, QString ipAddr, QObject *parent = nullptr);
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
	void Initializetion(SiemensPLCS siemens, QString ipAddr);	
	QICResult<> checkStartResult(QByteArray content);
	QICResult<> checkStopResult(QByteArray content);

public:
	QICResult<QString> readOrderNumber();
	QICResult<> hotStart();
	QICResult<> coldStart();
	QICResult<> stop();

	QICResult<QByteArray> Read(const QString& address, ushort length) override;
	QICResult<QByteArray> Read(const QStringList& addresses, const QVector<quint16>& length);
	QICResult<QByteArray> Read(const QVector<S7Address>& addresses);

	QICResult<> Write(const QString& address, const QByteArray& value) override;

	static QICResult<QByteArray> BuildReadPacket(const QVector<S7Address>& addresses);
	static QICResult<QByteArray> BuildBitReadPacket(const QString& address);

private:
	QICResult<QByteArray> ReadS7Address(const QVector<S7Address>& addresses);
	static QICResult<QByteArray> ParseReadByte(const QVector<S7Address>& addresses, const QByteArray& content);
	static QICResult<QByteArray> ParseReadBit(const QByteArray& content);

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
