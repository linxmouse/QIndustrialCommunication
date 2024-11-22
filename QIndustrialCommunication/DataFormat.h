#pragma once

enum class DataFormat {
    ABCD,  // Big-endian (�����ֽ���)
    BADC,  // Byte-swapped big-endian
    CDAB,  // Word-swapped little-endian
    DCBA   // Little-endian
};