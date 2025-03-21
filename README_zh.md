# Qt ��ҵͨ�ſ�

[English Version](README.md) | ����

## ��Ŀ����

����һ������ Qt �Ĺ�ҵͨ�ſ⣬�ṩ��һ��ǿ��������ʹ�õĽ������������ʹ�ø���Э���빤ҵ PLC ���豸ͨ�š�����Ŀ����� HslCommunication ������˼·��ּ�ڼ� C++ Ӧ�ó����еĹ�ҵͨ�š�

## ��������

- **֧�ֵ�Э��**:
  - ����ʿ Nano ���� TCP ͨ��
  - ������ S7��S1200 �������ͺţ�
  - Modbus TCP

- **��������**:
  - ֧�ֶ����������͵Ķ�д����
  - �����������������͡��ַ�������������֧��
  - �����ֽ���ת��
  - ʹ�� `QICResult` ģ����Ľ�׳������
  - Qt ����޷켯��

## ����Ҫ��

- Qt 5.x �� Qt 6.x
- C++11 ����߰汾
- CMake 3.10+

## ��װ

### ��¡�ֿ�

```bash
git clone https://github.com/linxmouse/QIndustrialCommunication.git
cd qt-industrial-communication
```

### ʹ�� CMake ����

```bash
mkdir build
cd build
cmake ..
make
```

## ʹ��ʾ��

### ����ʿ Nano ���� TCP

```cpp
KeyenceNanoSerialOverTcp overTcp{ "192.168.0.78", 8501, true, false };

// д�벼��ֵ
auto r305w = overTcp.Write("R305", true);
if (r305w.IsSuccess) {
    qDebug() << "R305 д��ɹ�";
}

// ��ȡ16λ�޷�������
auto dm84r = overTcp.ReadUInt16("dm84");
if (dm84r.IsSuccess) {
    qDebug() << "DM84 ֵ: " << dm84r.getContent0();
}
```

### ������ S7

```cpp
SiemensS7Net s7Net(SiemensPLCS::S1200, "127.0.0.1");

// ���ض� DB ��ȡ����
QICResult<int> rInt = s7Net.ReadInt32("DB3400.0");
qDebug() << "��ȡ�� Int32 ֵ: " << rInt.getContent0();

// д�벼��ֵ
auto isWriteSucc = s7Net.Write("db3400.5.1", true);
```

### Modbus-TCP

```cpp
QScopedPointer<ModbusTcpNet> modbusTcp(new ModbusTcpNet("127.0.0.1", 502, true, true));
modbusTcp->setDataFormat(DataFormat::ABCD);
modbusTcp->setIsOneBaseAddress(true);
// дushort
auto rt = modbusTcp->Write("00001", 1.2345f);
rt.IsSuccess ? qDebug() << Qt::endl : qDebug() << rt.Message;
// дint array
QVector<int> intValues{ -123, 456 };
rt = modbusTcp->Write("40001", intValues);
// дushort array
QVector<ushort> ushortValues{ 123, 456 };
rt = modbusTcp->Write("40004", ushortValues);
rt.IsSuccess ? qDebug() << Qt::endl : qDebug() << rt.Message;
// дbool
rt = modbusTcp->Write("30001", true);
rt.IsSuccess ? qDebug() << Qt::endl : qDebug() << rt.Message;
// дbool array
QVector<bool> boolValues{ true, false, true, false, true, false, true, false, true, false, true, false };
rt = modbusTcp->Write("41001", boolValues);
rt.IsSuccess ? qDebug() << Qt::endl : qDebug() << rt.Message;
// д�ַ���
rt = modbusTcp->WriteString("30001", QString::fromLocal8Bit("��ã����磬Modbus-TCP�ַ�������!"));
rt.IsSuccess ? qDebug() << Qt::endl : qDebug() << rt.Message;

// ��ȡfloat
auto floatValue = modbusTcp->ReadFloat("00001");
floatValue.IsSuccess ? qDebug() << floatValue.getContent0() : qDebug() << floatValue.Message;
// ��ȡint
auto intValue = modbusTcp->ReadInt32("40001");
intValue.IsSuccess ? qDebug() << intValue.getContent0() : qDebug() << intValue.Message;
// ��ȡshort
auto shortValue = modbusTcp->ReadInt16("40004");
shortValue.IsSuccess ? qDebug() << shortValue.getContent0() : qDebug() << shortValue.Message;
// ��ȡshort array
auto shortsValue = modbusTcp->ReadInt16("40004", 2);
shortsValue.IsSuccess ? qDebug() << shortsValue.getContent0() : qDebug() << shortsValue.Message;
// ��ȡbool array
auto boolsValue = modbusTcp->ReadBool("41001", 12);
boolsValue.IsSuccess ? qDebug() << boolsValue.getContent0() : qDebug() << boolsValue.Message;
// ��ȡ�ַ���
auto strValue = modbusTcp->ReadString("30001", 20);
strValue.IsSuccess ? qDebug() << strValue.getContent0() : qDebug() << strValue.Message;
```

## �ƻ���������

- [ ] ���� PLC Э��ʵ��
- [ ] ��ǿ��־����Ϲ���
- [ ] �����Ż�

## ����

��ӭ���ף�����ʱ�ύ Pull Request��

## ���֤

[GPL-3.0 license](LICENSE.txt)

## ��л

�����Դ�� HslCommunication �⡣