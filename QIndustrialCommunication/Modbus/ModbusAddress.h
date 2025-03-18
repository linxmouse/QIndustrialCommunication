#pragma once

#include "ModbusDataType.h"

struct ModbusAddress
{
	quint16 address;
	quint8 functionCode;
	quint8 station;
	ModbusDataType dataType;
};