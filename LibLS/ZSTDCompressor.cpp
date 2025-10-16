#include "pch.h"
#include "ZSTDCompressor.h"

#include <zstd.h>

#include "Exception.h"

Stream ZSTDCompressor::compress(StreamBase& input, LSCompressionLevel level)
{
    auto size = input.size();
    auto data = Stream::makeStream(input).detach();
    return compress(data.first.get(), size, level);
}

Stream ZSTDCompressor::compress(const uint8_t* data, size_t size, LSCompressionLevel level)
{
    Stream output;

    auto zstdLevel = 0;

    switch (level) {
    case LSCompressionLevel::FAST:
        zstdLevel = 1;
        break;
    case LSCompressionLevel::DEFAULT:
        zstdLevel = 3;
        break;
    case LSCompressionLevel::MAX:
        zstdLevel = 12;
        break;
    }

    auto maxCompressedSize = ZSTD_compressBound(size);

    auto compressedData = std::make_unique<uint8_t[]>(maxCompressedSize);

    auto compressedSize = ZSTD_compress(compressedData.get(), maxCompressedSize,
                                        data, size, zstdLevel);

    if (ZSTD_isError(compressedSize)) {
        throw Exception(std::format("ZSTD compression failed: {}", ZSTD_getErrorName(compressedSize)));
    }

    return Stream({std::move(compressedData), compressedSize});
}

Stream ZSTDCompressor::decompress(StreamBase& input, size_t uncompressedSize, bool chunked)
{
    auto data = Stream::makeStream(input).detach();

    return decompress(data.first.get(), data.second, uncompressedSize, chunked);
}

Stream ZSTDCompressor::decompress(const uint8_t* data, size_t size, size_t uncompressedSize, bool /*chunked*/)
{
    auto uncompressedData = std::make_unique<uint8_t[]>(uncompressedSize);

    auto result = ZSTD_decompress(uncompressedData.get(), uncompressedSize, data, size);
    if (ZSTD_isError(result)) {
        throw Exception(std::format("ZSTD decompression failed: {}", ZSTD_getErrorName(result)));
    }

    return Stream({std::move(uncompressedData), result});
}
