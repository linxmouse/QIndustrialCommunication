#pragma once

#include <QDebug>

enum class DataFormat {
    ABCD,  // 大端序 (网络字节序)
    DCBA,  // 小端序
    BADC,  // 中间交换大端序
    CDAB,  // 中间交换小端序
};

QDebug operator<<(QDebug debug, DataFormat format);