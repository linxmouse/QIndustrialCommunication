#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QEventLoop>
#include <QTimer>
#include "QICResult.h"
#include "NetworkDeviceBase.h"

class NetworkDevice : public NetworkDeviceBase
{
public:
	NetworkDevice(QObject* parent = nullptr)
		: NetworkDeviceBase(parent)
	{
		port = 10000;
		connectTimeOut = 10000;
		receiveTimeOut = 5000;
		IsSocketError = false;
		isPersistentConn = true;
		CoreSocket = nullptr;
	}

	NetworkDevice(QString ipAddress, int port, bool isPersistentConn, bool enableSendRecvLog, int connectTimeOut, int receiveTimeOut, QObject* parent = nullptr)
		:ipAddress(ipAddress), port(port), isPersistentConn(isPersistentConn), enableSendRecvLog(enableSendRecvLog), connectTimeOut(connectTimeOut), receiveTimeOut(receiveTimeOut), NetworkDeviceBase(parent)
	{
		IsSocketError = false;
		CoreSocket = nullptr;
	}

	virtual ~NetworkDevice()
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

		QICResult<QTcpSocket*> result = CreateSocketAndInitialication();
		if (!result.IsSuccess)
		{
			IsSocketError = true;
			if (enableSendRecvLog) qDebug() << QString("NetEngine Start Faild: %1").arg(result.Message);
			return QICResult<>::CreateFailedResult(result);
		}

		CoreSocket = result.GetContent<0>();
		if (enableSendRecvLog) qDebug() << "NetEngine Started";
		return QICResult<>::CreateSuccessResult();
	}

	QICResult<> CloseConnect()
	{
		QICResult<> result;
		isPersistentConn = false;
		InteractiveMutex.lock();
		result = ReleaseOnDisconnect(CoreSocket);
		CoreSocket->close();
		CoreSocket->deleteLater();
		InteractiveMutex.unlock();
		if (enableSendRecvLog) qDebug() << "NetEngine Closed";

		return result;
	}

	virtual QICResult<> InitializationOnConnect(QTcpSocket* socket)
	{
		return QICResult<>::CreateSuccessResult();
	}

	virtual QICResult<> ReleaseOnDisconnect(QTcpSocket* socket)
	{
		return QICResult<>::CreateSuccessResult();
	}

	QICResult<QTcpSocket*> CreateSocketAndInitialication()
	{
		QICResult<QTcpSocket*> operateResult = CreateSocketAndConnect(QHostAddress(ipAddress), port, connectTimeOut);
		if (operateResult.IsSuccess)
		{
			QICResult operateResult2 = InitializationOnConnect(operateResult.GetContent<0>());
			if (!operateResult2.IsSuccess)
			{
				operateResult.GetContent<0>()->close();
				operateResult.IsSuccess = operateResult2.IsSuccess;
				operateResult.CopyErrorFromOther(operateResult2);
			}
		}
		return operateResult;
	}

	QICResult<QTcpSocket*> GetAvailableSocket()
	{
		if (isPersistentConn)
		{
			if (IsSocketError || CoreSocket == nullptr)
			{
				QICResult operateResult = ConnectServer();
				if (!operateResult.IsSuccess)
				{
					IsSocketError = true;
					return QICResult<QTcpSocket*>::CreateFailedResult(operateResult);
				}
				IsSocketError = false;
				return QICResult<QTcpSocket*>::CreateSuccessResult(CoreSocket);
			}
			return QICResult<QTcpSocket*>::CreateSuccessResult(CoreSocket);
		}
		return CreateSocketAndInitialication();
	}

	QICResult<QTcpSocket*> CreateSocketAndConnect(const QHostAddress& address, quint16 port, int timeOut)
	{
		int num = 0;
		static int connectErrorCount = 0;
		while (true)
		{
			num++;
			QTcpSocket* socket = new QTcpSocket();
			QTimer timer;
			timer.setSingleShot(true);

			QEventLoop loop;
			QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
			QObject::connect(socket, &QTcpSocket::connected, &loop, &QEventLoop::quit);

			try
			{
				if (timeOut > 0)
				{
					timer.start(timeOut);
				}
				socket->connectToHost(address, port);
				loop.exec();

				if (socket->state() == QTcpSocket::ConnectedState)
				{
					connectErrorCount = 0;
					return QICResult<QTcpSocket*>::CreateSuccessResult(socket);
				}
				else
				{
					throw QException();
				}
			}
			catch (const QException&)
			{
				socket->deleteLater();
				if (connectErrorCount < 100000000)
				{
					connectErrorCount++;
				}

				if (timer.remainingTime() > (500 - timeOut) && num < 2)
				{
					QThread::msleep(100);
					continue;
				}

				if (timer.isActive() == false)
				{
					qDebug() << QString("Socket Connect Timeout, take %1 ms").arg(timeOut);
					return QICResult<QTcpSocket*>::CreateFailedResult(QString("Connect Timeout, take %1 ms").arg(timeOut));
				}
				else
				{
					qDebug() << "CreateSocketAndConnect Exception";
					return QICResult<QTcpSocket*>::CreateFailedResult("Unknown error");
				}
			}
		}
	}

	QICResult<QByteArray> ReadFromCoreServer(const QByteArray& send)
	{		
		InteractiveMutex.lock();
		QICResult<QTcpSocket*> availableSocket = GetAvailableSocket();
		if (!availableSocket.IsSuccess)
		{
			IsSocketError = true;
			InteractiveMutex.unlock();
			QICResult<QByteArray> result;
			result.CopyErrorFromOther(availableSocket);
			return result;
		}

		QICResult<QByteArray> coreServerResult = ReadFromCoreServer(availableSocket.GetContent<0>(), send);
		QICResult<QByteArray> finalResult;
		if (coreServerResult.IsSuccess)
		{
			IsSocketError = false;
			finalResult.IsSuccess = true;
			finalResult.SetContent<0>(coreServerResult.GetContent<0>());
			finalResult.Message = "Success";
		}
		else
		{
			IsSocketError = true;
			finalResult.CopyErrorFromOther(coreServerResult);
		}
		InteractiveMutex.unlock();
		if (!isPersistentConn && availableSocket.GetContent<0>())
		{
			availableSocket.GetContent<0>()->close();
		}

		return finalResult;
	}

	QICResult<QByteArray> ReadFromCoreServer(QTcpSocket* socket, const QByteArray& send)
	{
		// 发送日志
		if (enableSendRecvLog) qDebug() << "Sending:" << send.toHex(' ');
		// 发送数据
		QICResult<bool> sendResult = Send(socket, send);
		if (!sendResult.IsSuccess)
		{
			return QICResult<QByteArray>::CreateFailedResult(sendResult);
		}
		// 如果超时时间小于0，立即返回成功
		if (receiveTimeOut < 0)
		{
			return QICResult<QByteArray>::CreateSuccessResult(QByteArray());
		}
		// 接收数据
		QICResult<QByteArray> receiveResult = ReceiveByMessage(socket, receiveTimeOut);
		if (!receiveResult.IsSuccess)
		{
			return receiveResult;
		}
		// 接收日志
		if (enableSendRecvLog) qDebug() << "Received:" << receiveResult.GetContent<0>().toHex(' ');

		return QICResult<QByteArray>::CreateSuccessResult(receiveResult.GetContent<0>());
	}

private:
	/// @brief 发送数据包
	/// @param socket QTcpSocket*
	/// @param data 将要发送的QByteArray&数据
	/// @return QICResult<bool>
	QICResult<bool> Send(QTcpSocket* socket, const QByteArray& data)
	{
		qint64 bytesSent = socket->write(data);
		if (bytesSent == -1 || bytesSent < data.size())
		{
			return QICResult<bool>::CreateFailedResult("Failed to send all data.");
		}
		return QICResult<bool>::CreateSuccessResult(true);
	}

	/// @brief 读取数据包
	/// @param socket QTcpSocket*
	/// @param timeOut 读取超时时间,单位ms
	/// @return QICResult<QByteArray>
	QICResult<QByteArray> ReceiveByMessage(QTcpSocket* socket, int timeOut)
	{
		QByteArray receivedData;
		if (socket->waitForReadyRead(timeOut)) receivedData = socket->readAll();
		else return QICResult<QByteArray>::CreateFailedResult("Receive timed out.");
		if (receivedData.isEmpty()) return QICResult<QByteArray>::CreateFailedResult("Received empty data.");

		return QICResult<QByteArray>::CreateSuccessResult(receivedData);
	}

protected:
	QString ipAddress = "127.0.0.1";
	int port = 10000;
	int connectTimeOut = 10000;
	int receiveTimeOut = 5000;
	bool IsSocketError;
	bool isPersistentConn = true;
	bool enableSendRecvLog = true;
	QTcpSocket* CoreSocket;
	QMutex InteractiveMutex;
};