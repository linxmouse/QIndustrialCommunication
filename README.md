# Qt Industrial Communication Library

[ÖÐÎÄ°æ±¾](README_zh.md) | English

## Overview

This Qt-based industrial communication library provides a robust and easy-to-use solution for communicating with industrial PLCs and devices using various protocols. Inspired by the HslCommunication library, this project aims to simplify industrial communication in C++ applications.

## Features

- **Supported Protocols**:
  - Keyence Nano Serial over TCP
  - Siemens S7 (S1200 and other variants)
  - Modbus TCP (Planned)

- **Key Capabilities**:
  - Read/write operations for various data types
  - Support for boolean, integer, string, and array data
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
git clone https://github.com/yourusername/qt-industrial-communication.git
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

## Planned Features

- [ ] Modbus TCP support
- [ ] Additional PLC protocol implementations
- [ ] Enhanced logging and diagnostics
- [ ] Performance optimizations

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

[Specify your license here, e.g., MIT, Apache 2.0]

## Acknowledgements

Inspired by the HslCommunication library.