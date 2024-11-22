#pragma once

enum class DataFormat {
    ABCD,  // Big-endian (ÍøÂç×Ö½ÚÐò)
    BADC,  // Byte-swapped big-endian
    CDAB,  // Word-swapped little-endian
    DCBA   // Little-endian
};