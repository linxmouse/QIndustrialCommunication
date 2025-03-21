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
			// 首位必须是0-4
			QRegExp reg("^([0-4])(\\d{4})$");
			if (!reg.exactMatch(address))
			{
				if (isOneBaseAddress) return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format, Must be 5 digits (e.g. 40001)");
				else return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format, Must be 5 digits (e.g. 40000)");
			}
			// 解析地址
			quint16 addr = address.toUShort();
			// 提取地址部分
			quint16 partAddr = reg.cap(2).toUShort();
			if (isOneBaseAddress)
			{
				if (partAddr < 1) return QICResult<ModbusAddress>::CreateFailedResult("Invalid address value (e.g. 40001)");
				// 转换位基于0的设备协议地址
				addr -= 1;
			}
			// 0x01 - 线圈 | 0x02 - 离散输入 | 0x04 - 输入寄存器 | 0x03 - 保持寄存器
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

	// 读取离散量输入
	static const quint8 READ_DISCRETE_INPUT = 0x01;
	// 读取保持寄存器
	static const quint8 READ_HOLDING_REGISTER = 0x03;
	// 读取输入寄存器
	static const quint8 READ_INPUT_REGISTER = 0x04;
	// 写单个线圈
	static const quint8 WRITE_SINGLE_COIL = 0x05;
	// 写单个寄存器
	static const quint8 WRITE_SINGLE_REGISTER = 0x06;
	// 写多个线圈
	static const quint8 WRITE_MULTIPLE_COIL = 0x0f;
	// 写多个线圈
	static const quint8 WRITE_MULTIPLE_REGISTER = 0x10;
};