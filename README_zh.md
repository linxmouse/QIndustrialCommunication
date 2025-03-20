# Qt 工业通信库

[English Version](README.md) | 中文

## 项目概述

这是一个基于 Qt 的工业通信库，提供了一个强大且易于使用的解决方案，用于使用各种协议与工业 PLC 和设备通信。该项目借鉴了 HslCommunication 库的设计思路，旨在简化 C++ 应用程序中的工业通信。

## 功能特性

- **支持的协议**:
  - 基恩士 Nano 串行 TCP 通信
  - 西门子 S7（S1200 及其他型号）
  - Modbus TCP

- **核心能力**:
  - 支持多种数据类型的读写操作
  - 布尔、整数、浮点型、字符串和数组数据支持
  - 网络字节序转换
  - 使用 `QICResult` 模板类的健壮错误处理
  - Qt 框架无缝集成

## 环境要求

- Qt 5.x 或 Qt 6.x
- C++11 或更高版本
- CMake 3.10+

## 安装

### 克隆仓库

```bash
git clone https://github.com/linxmouse/QIndustrialCommunication.git
cd qt-industrial-communication
```

### 使用 CMake 构建

```bash
mkdir build
cd build
cmake ..
make
```

## 使用示例

### 基恩士 Nano 串行 TCP

```cpp
KeyenceNanoSerialOverTcp overTcp{ "192.168.0.78", 8501, true, false };

// 写入布尔值
auto r305w = overTcp.Write("R305", true);
if (r305w.IsSuccess) {
    qDebug() << "R305 写入成功";
}

// 读取16位无符号整数
auto dm84r = overTcp.ReadUInt16("dm84");
if (dm84r.IsSuccess) {
    qDebug() << "DM84 值: " << dm84r.getContent0();
}
```

### 西门子 S7

```cpp
SiemensS7Net s7Net(SiemensPLCS::S1200, "127.0.0.1");

// 从特定 DB 读取整数
QICResult<int> rInt = s7Net.ReadInt32("DB3400.0");
qDebug() << "读取的 Int32 值: " << rInt.getContent0();

// 写入布尔值
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

## 计划开发功能

- [ ] 更多 PLC 协议实现
- [ ] 增强日志和诊断功能
- [ ] 性能优化

## 贡献

欢迎贡献！请随时提交 Pull Request。

## 许可证

[GPL-3.0 license](LICENSE.txt)

## 鸣谢

灵感来源于 HslCommunication 库。