#pragma once

#include "QICResult.h"
#include <QRegExp>

class ModbusAddress
{
public:
	ModbusAddress()
		: address(0), functionCode(READ_HOLDING_REGISTER), station(1), isOneBaseAddress(false)
	{

	}

	static QICResult<ModbusAddress> ParseAddress(const QString& address, quint8 functionCode, quint8 station, bool isOneBaseAddress)
	{
		try
		{
			// ��λ������0-4
			QRegExp reg("^([0-4])(\\d{4})$");
			if (!reg.exactMatch(address))
			{
				if (isOneBaseAddress) return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format, Must be 5 digits (e.g. 40001)");
				else return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format, Must be 5 digits (e.g. 40000)");
			}
			// ������ַ
			quint16 addr = address.toUShort();
			// ��ȡ��ַ����
			quint16 partAddr = reg.cap(2).toUShort();
			if (isOneBaseAddress)
			{
				if (partAddr < 1) return QICResult<ModbusAddress>::CreateFailedResult("Invalid address value (e.g. 40001)");
				// ת��λ����0���豸Э���ַ
				addr -= 1;
			}
			// 0x01 - ��Ȧ | 0x02 - ��ɢ���� | 0x04 - ����Ĵ��� | 0x03 - ���ּĴ���
			QVector<quint8> functions = { 
				READ_DISCRETE_INPUT,
				READ_HOLDING_REGISTER, 
				READ_INPUT_REGISTER, 
				WRITE_SINGLE_COIL,
				WRITE_SINGLE_REGISTER,
				WRITE_MULTIPLE_COIL,
				WRITE_MULTIPLE_REGISTER
			};
			if (!functions.contains(functionCode)) return QICResult<ModbusAddress>::CreateFailedResult("Unspported address type");
			ModbusAddress modbusAddr;
			modbusAddr.address = addr;
			modbusAddr.functionCode = functionCode;
			modbusAddr.station = station;
			modbusAddr.isOneBaseAddress = isOneBaseAddress;
			return QICResult<ModbusAddress>::CreateSuccessResult(modbusAddr);
		}
		catch (...)
		{
			return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format");
		}
	}

public:
	quint16 address;
	quint8 functionCode;
	quint8 station;
	bool isOneBaseAddress;

	// ��ȡ��ɢ������
	static const quint8 READ_DISCRETE_INPUT = 0x01;
	// ��ȡ���ּĴ���
	static const quint8 READ_HOLDING_REGISTER = 0x03;
	// ��ȡ����Ĵ���
	static const quint8 READ_INPUT_REGISTER = 0x04;
	// д������Ȧ
	static const quint8 WRITE_SINGLE_COIL = 0x05;
	// д�����Ĵ���
	static const quint8 WRITE_SINGLE_REGISTER = 0x06;
	// д�����Ȧ
	static const quint8 WRITE_MULTIPLE_COIL = 0x0f;
	// д�����Ȧ
	static const quint8 WRITE_MULTIPLE_REGISTER = 0x10;
};