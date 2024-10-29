#pragma once

#include <QObject>
#include "QICResult.h"

/**
 * @brief S7Address �࣬���ڱ�ʾ������S7 PLC�ĵ�ַ
 *        S7Address class represents the address in Siemens S7 PLC
 */
class S7Address
{
public:
    /**
     * @brief ���캯������ʼ����Ա����
     *        Constructor to initialize member variables
     */
    S7Address()
        : addressStart(0), length(0), dataCode(0), dbBlock(0)
    {}

    /**
     * @brief �����������ַ�����ַ������ֵ����ǰʵ��
     *        Parse the given string address and assign values to the current instance
     *
     * @param address �ַ�����ʽ��PLC��ַ
     *                The address string of the PLC
     * @param length ���ݵĳ���
     *               The length of the data
     */
    void Parse(QString address, quint16 length)
    {
        QICResult<S7Address> result = ParseFrom(address, length);
        if (result.IsSuccess)
        {
            this->addressStart = result.getContent0().addressStart;
            this->length = result.getContent0().length;
            this->dataCode = result.getContent0().dataCode;
            this->dbBlock = result.getContent0().dbBlock;
        }
    }

    /**
     * @brief �����ַ����ʼƫ����
     *        Calculate the starting offset of the address
     *
     * @param address �ַ�����ʽ��PLC��ַ
     *                The address string of the PLC
     * @param isCT �Ƿ��Ǽ�ʱ��/��������ַ��Ĭ��Ϊfalse
     *             Whether it is a timer/counter address, default is false
     * @return int ���ؼ�����ĵ�ַ��ʼƫ����
     *             Returns the calculated start offset of the address
     */
    static int CalculateAddressStarted(const QString& address, bool isCT = false)
    {
        // û��С����������
        // If there is no dot (.) in the address
        if (address.indexOf('.') < 0)
        {
            // ����Ǽ�ʱ�����������ַ��ֱ�ӷ�������
            // If it's a timer/counter address, return the number directly
            if (isCT) return  address.toInt();
            // �������8����ʾλƫ��
            // Otherwise, multiply by 8 to represent the bit offset
            return address.toInt() * 8;
        }

        // ��С���������£��ָ����ƫ����
        // If there's a dot, split and calculate offset
        QStringList sa = address.split(".");
        return sa[0].toInt() * 8 + sa[1].toInt();
    }

    /**
     * @brief ������ַ��ʹ��Ĭ�ϳ���0
     *        Parse the address using default length of 0
     *
     * @param address �ַ�����ʽ��PLC��ַ
     *                The address string of the PLC
     * @return QICResult<S7Address> ���ؽ������
     *         Returns the result of the parsing
     */
    static QICResult<S7Address> ParseFrom(const QString& address)
    {
        return ParseFrom(address, 0);
    }

    /**
     * @brief ������ַ������������ַ�����ַ�ͳ���
     *        Parse the address based on the input string address and length
     *
     * @param address �ַ�����ʽ��PLC��ַ
     *                The address string of the PLC
     * @param length ���ݵĳ���
     *               The length of the data
     * @return QICResult<S7Address> ���ؽ������
     *         Returns the result of the parsing
     */
    static QICResult<S7Address> ParseFrom(const QString& address, quint16 length)
    {
        S7Address s7Address;
        s7Address.length = length;
        s7Address.dbBlock = 0;

        if (address.isEmpty()) return QICResult<S7Address>::CreateFailedResult("Address is empty");

        QChar firstChar = address.at(0).toUpper();
        // �����ַ�� I ��ͷ����ʾ�������������źţ�
        // Address starts with 'I', indicating input area (input signals)
        if (firstChar == 'I')
        {
            s7Address.dataCode = 129;  // �����������ݱ���
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        // ������ĵ�ַ����
        // Address handling for output area
        else if (firstChar == 'Q')
        {
            s7Address.dataCode = 130;  // ����������ݱ���
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        // �ڴ����ĵ�ַ����
        // Address handling for memory area
        else if (firstChar == 'M')
        {
            s7Address.dataCode = 131;  // �ڴ��������ݱ���
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        // ���ݿ����ĵ�ַ����
        // Address handling for data block area
        else if (firstChar == 'D' || address.mid(0, 2).toUpper() == "DB")
        {
            s7Address.dataCode = 132;  // ���ݿ��������ݱ���
            QStringList sa = address.split('.');
            if (address.at(1).toUpper() == 'B') s7Address.dbBlock = sa[0].mid(2).toUShort();
            else s7Address.dbBlock = sa[0].mid(1).toUShort();
            s7Address.addressStart = CalculateAddressStarted(address.mid(address.indexOf('.') + 1));
        }
        // ��ʱ���ĵ�ַ����
        // Address handling for timers
        else if (firstChar == 'T')
        {
            s7Address.dataCode = 29;  // ��ʱ�������ݱ���
            s7Address.addressStart = CalculateAddressStarted(address.mid(1), true);
        }
        // �������ĵ�ַ����
        // Address handling for counters
        else if (firstChar == 'C')
        {
            s7Address.dataCode = 28;  // �����������ݱ���
            s7Address.addressStart = CalculateAddressStarted(address.mid(1), true);
        }
        // �����Ĵ����ĵ�ַ����
        // Address handling for auxiliary registers
        else if (firstChar == 'V')
        {
            s7Address.dataCode = 132;
            s7Address.dbBlock = 1;  // ʹ��DB1
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        else
        {
            return QICResult<S7Address>::CreateFailedResult("Not support address type");
        }
        return QICResult<S7Address>::CreateSuccessResult(s7Address);
    }

public:
    // ��ַ��ʼƫ����
    // Address start offset
    int addressStart;

    // ���ݳ���
    // Data length
    quint16 length;

    // ���ݱ��룬����ָʾ������������/���/�ڴ�ȣ�
    // Data code to indicate specific area (input/output/memory, etc.)
    quint8 dataCode;

    // ���ݿ���
    // Data block number
    quint16 dbBlock;
};