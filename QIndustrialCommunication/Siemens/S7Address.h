#pragma once

#include <QObject>
#include "QICResult.h"

/**
 * @brief S7Address 类，用于表示西门子S7 PLC的地址
 *        S7Address class represents the address in Siemens S7 PLC
 */
class S7Address
{
public:
    /**
     * @brief 构造函数，初始化成员变量
     *        Constructor to initialize member variables
     */
    S7Address()
        : addressStart(0), length(0), dataCode(0), dbBlock(0)
    {}

    /**
     * @brief 解析给定的字符串地址，并赋值给当前实例
     *        Parse the given string address and assign values to the current instance
     *
     * @param address 字符串形式的PLC地址
     *                The address string of the PLC
     * @param length 数据的长度
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
     * @brief 计算地址的起始偏移量
     *        Calculate the starting offset of the address
     *
     * @param address 字符串形式的PLC地址
     *                The address string of the PLC
     * @param isCT 是否是计时器/计数器地址，默认为false
     *             Whether it is a timer/counter address, default is false
     * @return int 返回计算出的地址起始偏移量
     *             Returns the calculated start offset of the address
     */
    static int CalculateAddressStarted(const QString& address, bool isCT = false)
    {
        // 没有小数点的情况下
        // If there is no dot (.) in the address
        if (address.indexOf('.') < 0)
        {
            // 如果是计时器或计数器地址，直接返回数字
            // If it's a timer/counter address, return the number directly
            if (isCT) return  address.toInt();
            // 否则乘以8来表示位偏移
            // Otherwise, multiply by 8 to represent the bit offset
            return address.toInt() * 8;
        }

        // 有小数点的情况下，分割并计算偏移量
        // If there's a dot, split and calculate offset
        QStringList sa = address.split(".");
        return sa[0].toInt() * 8 + sa[1].toInt();
    }

    /**
     * @brief 解析地址，使用默认长度0
     *        Parse the address using default length of 0
     *
     * @param address 字符串形式的PLC地址
     *                The address string of the PLC
     * @return QICResult<S7Address> 返回解析结果
     *         Returns the result of the parsing
     */
    static QICResult<S7Address> ParseFrom(const QString& address)
    {
        return ParseFrom(address, 0);
    }

    /**
     * @brief 解析地址，根据输入的字符串地址和长度
     *        Parse the address based on the input string address and length
     *
     * @param address 字符串形式的PLC地址
     *                The address string of the PLC
     * @param length 数据的长度
     *               The length of the data
     * @return QICResult<S7Address> 返回解析结果
     *         Returns the result of the parsing
     */
    static QICResult<S7Address> ParseFrom(const QString& address, quint16 length)
    {
        S7Address s7Address;
        s7Address.length = length;
        s7Address.dbBlock = 0;

        if (address.isEmpty()) return QICResult<S7Address>::CreateFailedResult("Address is empty");

        QChar firstChar = address.at(0).toUpper();
        // 输入地址以 I 开头，表示输入区（输入信号）
        // Address starts with 'I', indicating input area (input signals)
        if (firstChar == 'I')
        {
            s7Address.dataCode = 129;  // 输入区的数据编码
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        // 输出区的地址处理
        // Address handling for output area
        else if (firstChar == 'Q')
        {
            s7Address.dataCode = 130;  // 输出区的数据编码
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        // 内存区的地址处理
        // Address handling for memory area
        else if (firstChar == 'M')
        {
            s7Address.dataCode = 131;  // 内存区的数据编码
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        // 数据块区的地址处理
        // Address handling for data block area
        else if (firstChar == 'D' || address.mid(0, 2).toUpper() == "DB")
        {
            s7Address.dataCode = 132;  // 数据块区的数据编码
            QStringList sa = address.split('.');
            if (address.at(1).toUpper() == 'B') s7Address.dbBlock = sa[0].mid(2).toUShort();
            else s7Address.dbBlock = sa[0].mid(1).toUShort();
            s7Address.addressStart = CalculateAddressStarted(address.mid(address.indexOf('.') + 1));
        }
        // 计时器的地址处理
        // Address handling for timers
        else if (firstChar == 'T')
        {
            s7Address.dataCode = 29;  // 计时器的数据编码
            s7Address.addressStart = CalculateAddressStarted(address.mid(1), true);
        }
        // 计数器的地址处理
        // Address handling for counters
        else if (firstChar == 'C')
        {
            s7Address.dataCode = 28;  // 计数器的数据编码
            s7Address.addressStart = CalculateAddressStarted(address.mid(1), true);
        }
        // 辅助寄存器的地址处理
        // Address handling for auxiliary registers
        else if (firstChar == 'V')
        {
            s7Address.dataCode = 132;
            s7Address.dbBlock = 1;  // 使用DB1
            s7Address.addressStart = CalculateAddressStarted(address.mid(1));
        }
        else
        {
            return QICResult<S7Address>::CreateFailedResult("Not support address type");
        }
        return QICResult<S7Address>::CreateSuccessResult(s7Address);
    }

public:
    // 地址起始偏移量
    // Address start offset
    int addressStart;

    // 数据长度
    // Data length
    quint16 length;

    // 数据编码，用于指示具体区域（输入/输出/内存等）
    // Data code to indicate specific area (input/output/memory, etc.)
    quint8 dataCode;

    // 数据块编号
    // Data block number
    quint16 dbBlock;
};