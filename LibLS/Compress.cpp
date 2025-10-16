#include "pch.h"

#include "Compress.h"
#include "Exception.h"
#include "LZ4Codec.h"

#include "ICompressor.h"
#include "LZ4Compressor.h"
#include "ZLibCompressor.h"
#include "ZSTDCompressor.h"

namespace Compression {
class CompressorFactory
{
public:
    static ICompressor::Ptr create(CompressionMethod method)
    {
        switch (method) {
        case CompressionMethod::ZLIB:
            return std::make_unique<ZLibCompressor>();
        case CompressionMethod::LZ4:
            return std::make_unique<LZ4Compressor>();
        case CompressionMethod::ZSTD:
            return std::make_unique<ZSTDCompressor>();
        default:
            throw Exception("Invalid compression method");
        }
    }
};

Stream compress(CompressionMethod method, StreamBase& input, LSCompressionLevel level)
{
    auto compressor = CompressorFactory::create(method);
    return compressor->compress(input, level);
}

Stream compress(CompressionMethod method, const uint8_t* data, size_t size, LSCompressionLevel level)
{
    auto compressor = CompressorFactory::create(method);
    return compressor->compress(data, size, level);
}

Stream decompress(CompressionMethod method, StreamBase& input, size_t uncompressedSize, bool chunked)
{
    auto compressor = CompressorFactory::create(method);
    return compressor->decompress(input, uncompressedSize, chunked);
}

Stream decompress(CompressionMethod method, const uint8_t* data, size_t size, size_t uncompressedSize, bool chunked)
{
    auto compressor = CompressorFactory::create(method);
    return compressor->decompress(data, size, uncompressedSize, chunked);
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
    case CompressionMethod::ZSTD:
        return METHOD_ZSTD;
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
    case METHOD_ZSTD:
        return CompressionMethod::ZSTD;
    default:
        throw Exception("Invalid compression flags");
    }
}
} // namespace Compression
