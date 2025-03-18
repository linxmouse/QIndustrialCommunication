#pragma once

enum class ModbusDataType
{
	// 保持寄存器
	HoldingRegister,
	// 输入寄存器
	InputRegister,
	// 线圈
	Coil,
	// 离散输入
	DiscreteInput,
};