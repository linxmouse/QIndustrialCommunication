# Qt Industrial Communication Library

[中文版本](README_zh.md) | English

## Overview

This Qt-based industrial communication library provides a robust and easy-to-use solution for communicating with industrial PLCs and devices using various protocols. Inspired by the HslCommunication library, this project aims to simplify industrial communication in C++ applications.

## Features

- **Supported Protocols**:
  - Keyence Nano Serial over TCP
  - Siemens S7 (S1200 and other variants)
  - Modbus TCP

- **Key Capabilities**:
  - Read/write operations for various data types
  - Support for boolean, integer, float, string, and array data
  - Network byte order conversion
  - Robust error handling with `QICResult` template class
  - Qt framework integration

## Requirements

- Qt 5.x or Qt 6.x
- C++11 or later
- CMake 3.10+

## Installation

### Clone the Repository

```bash
git clone https://github.com/linxmouse/QIndustrialCommunication.git
cd qt-industrial-communication
```

### Build with CMake

```bash
mkdir build
cd build
cmake ..
make
```

## Usage Examples

### Keyence Nano Serial over TCP

```cpp
KeyenceNanoSerialOverTcp overTcp{ "192.168.0.78", 8501, true, false };

// Write a boolean value
auto r305w = overTcp.Write("R305", true);
if (r305w.IsSuccess) {
    qDebug() << "Write to R305 successful";
}

// Read a 16-bit unsigned integer
auto dm84r = overTcp.ReadUInt16("dm84");
if (dm84r.IsSuccess) {
    qDebug() << "DM84 value: " << dm84r.getContent0();
}
```

### Siemens S7

```cpp
SiemensS7Net s7Net(SiemensPLCS::S1200, "127.0.0.1");

// Read an integer from a specific DB
QICResult<int> rInt = s7Net.ReadInt32("DB3400.0");
qDebug() << "Read Int32 value: " << rInt.getContent0();

// Write a boolean value
auto isWriteSucc = s7Net.Write("db3400.5.1", true);
```

### Modbus-TCP

```cpp
QScopedPointer<ModbusTcpNet> modbusTcp(new ModbusTcpNet("127.0.0.1", 502, true, true));
modbusTcp->setDataFormat(DataFormat::ABCD);
// 读取float
auto floatValue = modbusTcp->ReadFloat("40000");
qDebug() << floatValue.getContent0();
// 读取int
auto intValue = modbusTcp->ReadInt32("40004");
qDebug() << intValue.getContent0();
// 读取short
auto shortValue = modbusTcp->ReadInt16("40006");
qDebug() << shortValue.getContent0();
// 读取short array
auto shortsValue = modbusTcp->ReadInt16("40006", 2);
qDebug() << "0 of shorts:" << shortsValue.getContent0().at(0) << "1 of shorts" << shortsValue.getContent0().at(1);
// 读取bool
auto boolsValue = modbusTcp->ReadBool("30000", 12);
qDebug() << boolsValue.getContent0();

// 写ushort
auto rt = modbusTcp->Write("40000", (ushort)12345);
qDebug() << rt.IsSuccess;
// 写ushort array
QVector<ushort> ushortValues{ 123, 456 };
auto rt = modbusTcp->Write("40000", ushortValues);
// 写int array
QVector<int> intValues{ -123, 456 };
auto rt = modbusTcp->Write("40000", intValues);
// 写bool
auto rt = modbusTcp->Write("40000", true);
// 写bool array
QVector<bool> boolValues{ true, false };
auto rt = modbusTcp->Write("40000", boolValues);

// 写字符串
auto rt = modbusTcp->WriteString("30000", QString::fromLocal8Bit("你好，世界，Modbus-TCP字符串测试!"));
// 读取字符串
auto strValue = modbusTcp->ReadString("30000", 20);
qDebug() << strValue.getContent0();
```

## Planned Features

- [ ] Additional PLC protocol implementations
- [ ] Enhanced logging and diagnostics
- [ ] Performance optimizations

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

[GPL-3.0 license](LICENSE.txt)

## Acknowledgements

Inspired by the HslCommunication library.