#include "pch.h"

#include <lz4.h>

#include "Compress.h"
#include "Exception.h"
#include "LZ4Codec.h"
#include "LZ4FrameCompressor.h"

bool decompressData(CompressionMethod method, const uint8_t* compressedData, uint32_t compressedSize,
                    UInt8Ptr& decompressedData, uint32_t decompressedSize)
{
    if (method != CompressionMethod::LZ4) {
        throw Exception("Unsupported compression method");
    }

    // Allocate buffer for decompressed data
    decompressedData = std::make_unique<uint8_t[]>(decompressedSize);

    // Perform LZ4 decompression
    int result = LZ4_decompress_safe(reinterpret_cast<const char*>(compressedData),
        reinterpret_cast<char*>(decompressedData.get()),
        static_cast<int>(compressedSize),
        static_cast<int>(decompressedSize));

    return result >= 0; // Return true if decompression was successful
}

Stream decompressStream(CompressionMethod method, Stream& stream, uint32_t decompressedSize, bool chunked)
{
    Stream decompressed;

    if (method != CompressionMethod::LZ4) {
        throw Exception("Unsupported compression method");
    }

    if (chunked) {
        decompressed = LZ4FrameCompressor::decompress(stream, decompressedSize);
    } else {
        decompressed = LZ4Codec::decode(stream, 0, stream.size(), 0, decompressedSize, true);
        if (decompressed.size() != decompressedSize) {
            throw Exception("Decompressed size mismatch");
        }
    }

    return decompressed;
}

CompressionFlags compressionFlags(CompressionMethod method)
{
    switch (method) {
    case CompressionMethod::NONE:
        return METHOD_NONE;
    case CompressionMethod::ZLIB:
        return METHOD_ZLIB;
    case CompressionMethod::LZ4:
        return METHOD_LZ4;
    case CompressionMethod::LZSTD:
        return METHOD_LZSTD;
    }

    throw Exception("Invalid compression method");
}

CompressionFlags compressionFlags(LSCompressionLevel level)
{
    switch (level) {
    case LSCompressionLevel::FAST:
        return FAST_COMPRESS;
    case LSCompressionLevel::DEFAULT:
        return DEFAULT_COMPRESS;
    case LSCompressionLevel::MAX:
        return MAX_COMPRESS;
    }

    throw Exception("Invalid compression level");
}

CompressionFlags compressionFlags(CompressionMethod method, LSCompressionLevel level)
{
    if (method == CompressionMethod::NONE) {
        return METHOD_NONE;
    }

    return static_cast<CompressionFlags>(compressionFlags(method) | compressionFlags(level));
}

CompressionMethod compressionMethod(CompressionFlags flags)
{
    switch (flags & 0x0F) {
    case METHOD_NONE:
        return CompressionMethod::NONE;
    case METHOD_ZLIB:
        return CompressionMethod::ZLIB;
    case METHOD_LZ4:
        return CompressionMethod::LZ4;
    case METHOD_LZSTD:
        return CompressionMethod::LZSTD;
    default:
        throw Exception("Invalid compression flags");
    }
}
