#pragma once

#include <cstdint>
#include <lz4.h>
#include <memory>

enum class CompressionMethod : uint8_t
{
    NONE = 0x00,
    ZLIB = 0x01,
    LZ4 = 0x02,
    LZSTD = 0x03,
};


bool decompressData(CompressionMethod method, const uint8_t* compressedData, uint32_t compressedSize,
    std::unique_ptr<uint8_t[]>& decompressedData, uint32_t decompressedSize);
