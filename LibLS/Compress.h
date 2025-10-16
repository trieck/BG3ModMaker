#pragma once

#include "LSCommon.h"
#include "Stream.h"

enum class CompressionMethod : uint8_t
{
    NONE = METHOD_NONE,
    ZLIB = METHOD_ZLIB,
    LZ4 = METHOD_LZ4,
    ZSTD = METHOD_ZSTD,
};

enum class LSCompressionLevel
{
    FAST,
    DEFAULT,
    MAX
};

namespace Compression { // Compression namespace
Stream compress(CompressionMethod method, StreamBase& input, LSCompressionLevel level);
Stream compress(CompressionMethod method, const uint8_t* data, size_t size, LSCompressionLevel level);
Stream decompress(CompressionMethod method, StreamBase& input, size_t uncompressedSize, bool chunked = false);
Stream decompress(CompressionMethod method, const uint8_t* data, size_t size, size_t uncompressedSize,
                  bool chunked = false);

CompressionFlags compressionFlags(CompressionMethod method);
CompressionFlags compressionFlags(LSCompressionLevel level);
CompressionFlags compressionFlags(CompressionMethod method, LSCompressionLevel level);
CompressionMethod compressionMethod(CompressionFlags flags);
} // namespace Compression
