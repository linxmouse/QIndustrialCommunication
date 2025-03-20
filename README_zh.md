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
// ��ȡfloat
auto floatValue = modbusTcp->ReadFloat("40000");
qDebug() << floatValue.getContent0();
// ��ȡint
auto intValue = modbusTcp->ReadInt32("40004");
qDebug() << intValue.getContent0();
// ��ȡshort
auto shortValue = modbusTcp->ReadInt16("40006");
qDebug() << shortValue.getContent0();
// ��ȡshort array
auto shortsValue = modbusTcp->ReadInt16("40006", 2);
qDebug() << "0 of shorts:" << shortsValue.getContent0().at(0) << "1 of shorts" << shortsValue.getContent0().at(1);
// ��ȡbool
auto boolsValue = modbusTcp->ReadBool("30000", 12);
qDebug() << boolsValue.getContent0();

// дushort
auto rt = modbusTcp->Write("40000", (ushort)12345);
qDebug() << rt.IsSuccess;
// дushort array
QVector<ushort> ushortValues{ 123, 456 };
auto rt = modbusTcp->Write("40000", ushortValues);
// дint array
QVector<int> intValues{ -123, 456 };
auto rt = modbusTcp->Write("40000", intValues);
// дbool
auto rt = modbusTcp->Write("40000", true);
// дbool array
QVector<bool> boolValues{ true, false };
auto rt = modbusTcp->Write("40000", boolValues);

// д�ַ���
auto rt = modbusTcp->WriteString("30000", QString::fromLocal8Bit("��ã����磬Modbus-TCP�ַ�������!"));
// ��ȡ�ַ���
auto strValue = modbusTcp->ReadString("30000", 20);
qDebug() << strValue.getContent0();
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