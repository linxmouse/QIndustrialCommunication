#pragma once

#include "QICResult.h"
#include <QRegExp>

class ModbusAddress
{
public:
	ModbusAddress()
		: address(0), functionCode(0x03), station(1)
	{

	}

	static QICResult<ModbusAddress> ParseAddress(const QString& address, quint8 functionCode, quint8 station)
	{
		try
		{
			// 首位必须是0-4，且只占1个字符
			QRegExp reg("^([0-4])(\\d+)$");
			if (!reg.exactMatch(address)) return QICResult<ModbusAddress>::CreateFailedResult("Invalid address format");
			// 解析地址
			quint16 addr = address.toUShort();
			// 0x01 - 线圈 | 0x02 - 离散输入 | 0x04 - 输入寄存器 | 0x03 - 保持寄存器
			QVector<quint8> functions = { 0x01, 0x02, 0x04, 0x03 };
			if (!functions.contains(functionCode)) return QICResult<ModbusAddress>::CreateFailedResult("Unspported address type");
			ModbusAddress modbusAddr;
			modbusAddr.address = addr;
			modbusAddr.functionCode = functionCode;
			modbusAddr.station = station;
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
};