#pragma once

#include <cstdint>
#include <lz4.h>

#include "LSCommon.h"
#include "Stream.h"

enum class CompressionMethod : uint8_t
{
    NONE = METHOD_NONE,
    ZLIB = METHOD_ZLIB,
    LZ4 = METHOD_LZ4,
    LZSTD = METHOD_LZSTD,
};

bool decompressData(CompressionMethod method, const uint8_t* compressedData, uint32_t compressedSize,
    uint8Ptr& decompressedData, uint32_t decompressedSize);

Stream::Ptr decompressStream(CompressionMethod method, const Stream::Ptr& stream, uint32_t decompressedSize, bool chunked = false);
