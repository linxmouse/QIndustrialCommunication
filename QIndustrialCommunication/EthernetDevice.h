#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QEventLoop>
#include <QHostAddress>
#include <QTimer>
#include "QICResult.h"
#include "IEthernetIO.h"

class EthernetDevice : public IEthernetIO
{
public:
	EthernetDevice(QObject *parent = nullptr)
		: IEthernetIO(parent)
	{
		port = 10000;
		connectTimeOut = 10000;
		receiveTimeOut = 5000;
		IsSocketError = false;
		isPersistentConn = true;
		CoreSocket = nullptr;
	}

	EthernetDevice(QString ipAddr, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject *parent = nullptr)
		: ipAddr(ipAddr), port(port), isPersistentConn(isPersistentConn), 
		enableSendRecvLog(enableSendRecvLog), connectTimeOut(connectTimeOut), 
		receiveTimeOut(receiveTimeOut), IEthernetIO(parent)
	{
		IsSocketError = false;
		CoreSocket = nullptr;
	}

	virtual ~EthernetDevice()
	{
	}

protected:
	QICResult<> ConnectServer()
	{
		isPersistentConn = true;
		if (CoreSocket)
		{
			CoreSocket->close();
			delete CoreSocket;
			CoreSocket = nullptr;
		}
		QICResult<QTcpSocket *> result = CreateSocketAndInitialication();
		if (!result.IsSuccess)
		{
			IsSocketError = true;
			if (enableSendRecvLog)
				qDebug() << QString("NetEngine Start Faild: %1").arg(result.Message);
			return QICResult<>::CreateFailedResult(result);
		}
		CoreSocket = result.getContent0();
		if (enableSendRecvLog)
			qDebug() << "NetEngine Started";
		return QICResult<>::CreateSuccessResult();
	}

	QICResult<> CloseConnect()
	{
		QICResult<> result;
		isPersistentConn = false;
		InteractiveMutex.lock();
		result = ReleaseOnDisconnect(CoreSocket);
		if (CoreSocket)
		{
			CoreSocket->close();
			delete CoreSocket;
			CoreSocket = nullptr;
		}
		InteractiveMutex.unlock();
		if (enableSendRecvLog)
			qDebug() << "NetEngine Closed";

		return result;
	}

	virtual QICResult<> InitializationOnConnect(QTcpSocket *socket) = 0;
	virtual QICResult<> ReleaseOnDisconnect(QTcpSocket *socket) = 0;

	QICResult<QTcpSocket *> CreateSocketAndInitialication()
	{
		QICResult<QTcpSocket *> createResult = CreateSocketAndConnect(QHostAddress(ipAddr), port, connectTimeOut);
		if (createResult.IsSuccess)
		{
			QICResult<> initResult = InitializationOnConnect(createResult.getContent0());
			if (!initResult.IsSuccess)
			{
				createResult.getContent0()->close();
				createResult.IsSuccess = initResult.IsSuccess;
				createResult.CopyErrorFromOther(initResult);
			}
		}
		return createResult;
	}

	QICResult<QTcpSocket *> GetAvailableSocket()
	{
		if (isPersistentConn)
		{
			if (IsSocketError || CoreSocket == nullptr)
			{
				QICResult<> result = ConnectServer();
				if (!result.IsSuccess)
				{
					IsSocketError = true;
					return QICResult<QTcpSocket *>::CreateFailedResult(result);
				}
				IsSocketError = false;
				return QICResult<QTcpSocket *>::CreateSuccessResult(CoreSocket);
			}
			return QICResult<QTcpSocket *>::CreateSuccessResult(CoreSocket);
		}
		return CreateSocketAndInitialication();
	}

	QICResult<QTcpSocket *> CreateSocketAndConnect(const QHostAddress &address, quint16 port, int timeOut)
	{
		int num = 0;
		static int connectErrorCount = 0;
		while (true)
		{
			num++;
			QTcpSocket *socket = new QTcpSocket();
			try
			{
				socket->connectToHost(address, port);
				if (!socket->waitForConnected(timeOut))
				{
					throw std::exception(socket->errorString().toLatin1());
				}
				connectErrorCount = 0;
				return QICResult<QTcpSocket *>::CreateSuccessResult(socket);
			}
			catch (const std::exception &)
			{
				socket->deleteLater();
				if (connectErrorCount < 100000000)
					connectErrorCount++;
				if (num < 2)
				{
					QThread::msleep(100);
					continue;
				}
				if (socket->state() != QTcpSocket::ConnectedState)
				{
					qDebug() << QString("Socket Connect Timeout, take %1 ms").arg(timeOut);
					return QICResult<QTcpSocket *>::CreateFailedResult(QString("Connect Timeout, take %1 ms").arg(timeOut));
				}
				else
				{
					qDebug() << "CreateSocketAndConnect Exception";
					return QICResult<QTcpSocket *>::CreateFailedResult("Unknown error");
				}
			}
		}
	}

	QICResult<QByteArray> ReadFromCoreServer(const QByteArray &send)
	{
		InteractiveMutex.lock();
		QICResult<QTcpSocket *> availableSocket = GetAvailableSocket();
		if (!availableSocket.IsSuccess)
		{
			IsSocketError = true;
			InteractiveMutex.unlock();
			QICResult<QByteArray> result;
			result.CopyErrorFromOther(availableSocket);
			return result;
		}
		QICResult<QByteArray> coreServerResult = ReadFromCoreServer(availableSocket.getContent0(), send);
		QICResult<QByteArray> finalResult;
		if (coreServerResult.IsSuccess)
		{
			IsSocketError = false;
			finalResult.IsSuccess = true;
			finalResult.setContent0(coreServerResult.getContent0());
			finalResult.Message = "Success";
		}
		else
		{
			IsSocketError = true;
			finalResult.CopyErrorFromOther(coreServerResult);
		}
		InteractiveMutex.unlock();
		if (!isPersistentConn && availableSocket.getContent0())
		{
			availableSocket.getContent0()->close();
		}
		return finalResult;
	}

	QICResult<QByteArray> ReadFromCoreServer(QTcpSocket *socket, const QByteArray &send)
	{
		// 发送日志
		if (enableSendRecvLog)
			qDebug() << "Sending:" << send.toHex(' ');
		// 发送数据
		QICResult<bool> sendResult = Send(socket, send);
		if (!sendResult.IsSuccess)
			return QICResult<QByteArray>::CreateFailedResult(sendResult);
		// 如果超时时间小于0，立即返回成功
		if (receiveTimeOut < 0)
			return QICResult<QByteArray>::CreateSuccessResult(QByteArray());
		// 接收数据
		QICResult<QByteArray> receiveResult = Receive(socket, receiveTimeOut);
		if (!receiveResult.IsSuccess)
			return receiveResult;
		// 接收日志
		if (enableSendRecvLog)
			qDebug() << "Received:" << receiveResult.getContent0().toHex(' ');
		return QICResult<QByteArray>::CreateSuccessResult(receiveResult.getContent0());
	}

private:
	/// @brief 发送数据包
	/// @param socket QTcpSocket*
	/// @param data 将要发送的QByteArray&数据
	/// @return QICResult<bool>
	QICResult<bool> Send(QTcpSocket *socket, const QByteArray &data)
	{
		qint64 bytesSent = socket->write(data);
		if (bytesSent == -1 || bytesSent < data.size())
			return QICResult<bool>::CreateFailedResult("Failed to send all data.");
		return QICResult<bool>::CreateSuccessResult(true);
	}

	/// @brief 读取数据包
	/// @param socket QTcpSocket*
	/// @param timeOut 读取超时时间,单位ms
	/// @return QICResult<QByteArray>
	QICResult<QByteArray> Receive(QTcpSocket *socket, int timeOut)
	{
		QByteArray receivedData;
		if (socket->waitForReadyRead(timeOut))
			receivedData = socket->readAll();
		else
			return QICResult<QByteArray>::CreateFailedResult("Receive timed out.");
		if (receivedData.isEmpty())
			return QICResult<QByteArray>::CreateFailedResult("Received empty data.");
		return QICResult<QByteArray>::CreateSuccessResult(receivedData);
	}

protected:
	QString ipAddr = "127.0.0.1";
	quint16 port = 10000;
	int connectTimeOut = 10000;
	int receiveTimeOut = 5000;
	bool IsSocketError;
	bool isPersistentConn = true;
	bool enableSendRecvLog = true;
	QTcpSocket *CoreSocket;
	QMutex InteractiveMutex;
};