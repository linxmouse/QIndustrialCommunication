#include "DataFormat.h"

QDebug operator<<(QDebug debug, DataFormat format)
{
    switch (format)
    {
    case DataFormat::ABCD:
        debug << "DataFormat::ABCD";
        break;
    case DataFormat::DCBA:
        debug << "DataFormat::DCBA";
        break;
    case DataFormat::BADC:
        debug << "DataFormat::BADC";
        break;
    case DataFormat::CDAB:
        debug << "DataFormat::CDAB";
        break;
    default:
        debug << "Unknown DataFormat";
        break;
    }
    return debug;
}