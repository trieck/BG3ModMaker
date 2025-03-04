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

enum class LSCompressionLevel
{
    FAST,
    DEFAULT,
    MAX
};

bool decompressData(CompressionMethod method, const uint8_t* compressedData, uint32_t compressedSize,
    uint8Ptr& decompressedData, uint32_t decompressedSize);

Stream decompressStream(CompressionMethod method, Stream& stream, uint32_t decompressedSize, bool chunked = false);

CompressionFlags compressionFlags(CompressionMethod method);
CompressionFlags compressionFlags(LSCompressionLevel level);
CompressionFlags compressionFlags(CompressionMethod method, LSCompressionLevel level);
CompressionMethod compressionMethod(CompressionFlags flags);