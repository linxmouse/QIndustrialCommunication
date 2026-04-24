# QIndustrialCommunication - Modbus-TCP 设计思路

## 一、整体架构

项目采用**分层继承**的设计，从抽象接口到具体协议实现，共三层：

```
IEthernetIO（纯接口层）
    └── EthernetDevice（网络通信基础层）
            └── ModbusTcpNet（Modbus TCP 协议层）
```

---

## 二、各层职责

### 1. `IEthernetIO` — 接口层

**文件：** `Core/IEthernetIO.h`

定义所有设备必须实现或可以使用的 **统一 API**，是整个库对外暴露的"门面"。

| 成员 | 说明 |
|---|---|
| `Read(address, length)` | 纯虚函数，读取原始字节，子类必须实现 |
| `Write(address, bytes)` | 纯虚函数，写入原始字节，子类必须实现 |
| `ReadBool / Write(bool)` | 有默认实现，可被子类覆写 |
| `ReadInt16 / ReadUInt16 / ReadInt32 / ReadFloat ...` | **模板方法模式**：调用 `Read()` 拿到原始字节后，通过 `BytesOrderPtr` 做类型转换，子类无需重复实现 |
| `BytesOrderPtr` | `QScopedPointer<BytesOrderBase>`，字节序处理器，子类构造时赋值 |
| `WordsPerAddress` | 每个地址对应的字word数，Modbus=1，S7可能不同 |

**核心思路：** 子类只需实现"读写原始字节"，所有类型转换（int16/float/string...）由基类统一处理。

---

### 2. `EthernetDevice` — TCP 通信基础层

**文件：** `Core/EthernetDevice.h`

封装所有与 TCP Socket 相关的底层细节，子类无需关心连接管理。

| 成员 | 说明 |
|---|---|
| `ConnectServer()` | 建立长连接（持久连接模式） |
| `CloseConnect()` | 断开连接，先调 `ReleaseOnDisconnect()` |
| `GetAvailableSocket()` | 持久连接模式下复用 Socket；短连接模式下每次新建 |
| `ReadFromSocket(send)` | 加锁 → 获取 Socket → 发送 → 接收 → 解锁，是所有上层"收发"操作的入口 |
| `InitializationOnConnect(socket)` | 纯虚，Socket 建立后的协议初始化（如 S7 需要握手，Modbus 无需） |
| `ReleaseOnDisconnect(socket)` | 纯虚，断开前的清理 |
| `InteractiveMutex` | `QMutex`，保证多线程下收发操作的互斥 |
| `isPersistentConn` | 持久连接 / 短连接模式开关 |
| `enableSendRecvLog` | 调试日志开关，收发时打印十六进制数据 |

**连接状态机（持久连接模式）：**
```
初始/错误 ──ReadFromSocket──> GetAvailableSocket ──断线重连──> 正常复用 CoreSocket
```

---

### 3. `ModbusTcpNet` — Modbus TCP 协议层

**文件：** `Modbus/ModbusTcpNet.h / .cpp`

在 `EthernetDevice` 基础上，实现 **Modbus TCP 协议的报文构建与解析**。

#### 3.1 报文结构

Modbus TCP 报文 = MBAP Header(7) + PDU

```
MBAP Header (7 bytes):
  TransactionID (2) | ProtocolID (2, 固定0x0000) | Length (2) | UnitID (1)

PDU:
  FunctionCode (1) | Data (variable)
```

`BuildMBAPHeader(pduLength)` 负责生成 MBAP 头，其中 `m_transactionId` 每次自增，方便收发对应。

#### 3.2 功能码映射（`ModbusAddress`）

| 功能码 | 常量 | 用途 |
|---|---|---|
| 0x01 | `READ_DISCRETE_INPUT` | 读离散量输入（位） |
| 0x03 | `READ_HOLDING_REGISTER` | 读保持寄存器（默认读） |
| 0x04 | `READ_INPUT_REGISTER` | 读输入寄存器 |
| 0x05 | `WRITE_SINGLE_COIL` | 写单个线圈 |
| 0x06 | `WRITE_SINGLE_REGISTER` | 写单个寄存器 |
| 0x0F | `WRITE_MULTIPLE_COIL` | 写多个线圈 |
| 0x10 | `WRITE_MULTIPLE_REGISTER` | 写多个寄存器 |

写单个 vs 写多个 的**自动判断逻辑**：
- `Write(bytes)`: `value.size() == 2` → 用 0x06，否则用 0x10
- `Write(bools)`: `values.size() == 1` → 用 0x05，否则用 0x0F

#### 3.3 地址格式（`ModbusAddress::ParseAddress`）

地址格式为 **5位十进制字符串**，首位表示区域：
```
0xxxx → Coil（线圈）
1xxxx → DiscreteInput（离散输入）
3xxxx → InputRegister（输入寄存器）
4xxxx → HoldingRegister（保持寄存器）
```
支持 `isOneBaseAddress`：为 `true` 时地址从 1 开始（协议地址自动 -1）。

#### 3.4 读写流程（以 `Read` 为例）

```
Read(address, length)
  ├─ BuildReadRequest()    → 地址解析 + 构建PDU + 拼接MBAP = 完整报文
  ├─ ReadFromSocket()      → 加锁 → 发送 → 接收原始响应（EthernetDevice层）
  └─ ParseReadResponse()   → 校验长度 → 检查异常位(0x80) → 提取载荷字节
```

#### 3.5 线圈位操作工具函数

| 函数 | 说明 |
|---|---|
| `BoolListToByteArray` | `QVector<bool>` → `QByteArray`，低位在前（LSB first） |
| `ByteArrayToBoolList` | `QByteArray` → `QVector<bool>`，按 length 截断 |

---

## 三、通用返回值 `QICResult<T...>`

**文件：** `Core/QICResult.h`

所有函数的返回类型，携带：
- `IsSuccess`：是否成功
- `Message`：错误信息
- `ErrorCode`：错误代码
- `Content`：`std::tuple<T...>` 实际数据，用 `getContent0()` 取第一个元素

`QICResult<>` 表示无返回内容（只关心成功与否）。

**错误传递惯用法：**
```cpp
// 失败时将上一级错误透传下去
return QICResult<>::CreateFailedResult(buildResult);  // 从另一个Result复制错误
return QICResult<QByteArray>::CreateFailedResult("message");  // 直接填错误消息
```

---

## 四、字节序处理 `BytesOrderBase`

**文件：** `Core/BytesOrderBase.h`，配合 `DataFormat.h`

支持四种字节序（32位数值）：

| DataFormat | 内存字节顺序 | 说明 |
|---|---|---|
| `ABCD` | AB CD | 大端（网络字节序） |
| `DCBA` | DC BA | 小端 |
| `BADC` | BA DC | 中间交换大端 |
| `CDAB` | CD AB | 中间交换小端（**Modbus 默认**） |

`ModbusTcpNet` 构造时设置：
```cpp
BytesOrderPtr.reset(new BytesOrderBase(DataFormat::CDAB, false));
```

`IEthernetIO` 的所有 `ReadInt16/ReadFloat/...` 都通过 `BytesOrderPtr->ConvertToXxx()` 完成转换，与协议层解耦。

---

## 五、关键设计决策总结

| 决策 | 原因 |
|---|---|
| 三层继承（接口→通信→协议） | 每层职责单一，新增协议只需实现最顶层 |
| `using IEthernetIO::Write` | 防止子类定义 `Write(bool[])` 后隐藏基类的 `Write(bool)` 重载 |
| `QMutex InteractiveMutex` | 保证多线程调用时 socket 收发的原子性 |
| `ReadFromSocket` 统一入口 | 连接管理、重连、日志都在此一处完成，协议层专注报文 |
| `QICResult` 贯穿所有层 | 避免异常机制，错误可沿调用链透传，接口统一清晰 |
| `BuildXxx / ParseXxx` 分离 | 报文构建与解析独立，便于单独测试和复用 |
