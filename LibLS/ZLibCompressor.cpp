#include "pch.h"
#include "ZLibCompressor.h"

#include <zlib.h>

#include "Exception.h"

Stream ZLibCompressor::compress(StreamBase& input, LSCompressionLevel level)
{
    auto size = input.size();
    auto data = Stream::makeStream(input).detach();
    return compress(data.first.get(), size, level);
}

Stream ZLibCompressor::compress(const uint8_t* data, size_t size, LSCompressionLevel level)
{
    auto zlibLevel = Z_DEFAULT_COMPRESSION;

    switch (level) {
    case LSCompressionLevel::FAST:
        zlibLevel = Z_BEST_SPEED;
        break;
    case LSCompressionLevel::DEFAULT:
        zlibLevel = Z_DEFAULT_COMPRESSION;
        break;
    case LSCompressionLevel::MAX:
        zlibLevel = Z_BEST_COMPRESSION;
        break;
    }

    auto maxCompressedSize = compressBound(static_cast<uLong>(size));

    auto compressedData = std::make_unique<uint8_t[]>(maxCompressedSize);

    auto result = compress2(reinterpret_cast<Bytef*>(compressedData.get()), &maxCompressedSize,
                            reinterpret_cast<const Bytef*>(data), static_cast<uLong>(size),
                            zlibLevel);
    if (result != Z_OK) {
        auto* errMsg = "Unknown error";
        switch (result) {
        case Z_MEM_ERROR:
            errMsg = "Z_MEM_ERROR";
            break;
        case Z_BUF_ERROR:
            errMsg = "Z_BUF_ERROR";
            break;
        case Z_STREAM_ERROR:
            errMsg = "Z_STREAM_ERROR";
            break;
        default:
            break;
        }
        throw Exception(std::format("ZLib compression failed ({}): {}", result, errMsg));
    }

    return Stream({std::move(compressedData), maxCompressedSize});
}

Stream ZLibCompressor::decompress(StreamBase& input, size_t uncompressedSize, bool chunked)
{
    auto data = Stream::makeStream(input).detach();
    return decompress(data.first.get(), data.second, uncompressedSize, chunked);
}

Stream ZLibCompressor::decompress(const uint8_t* data, size_t size, size_t uncompressedSize, bool /*chunked*/)
{
    auto uncompressedData = std::make_unique<uint8_t[]>(uncompressedSize);

    auto destLen = static_cast<uLongf>(uncompressedSize);

    auto result = uncompress(reinterpret_cast<Bytef*>(uncompressedData.get()), &destLen,
                             reinterpret_cast<const Bytef*>(data), static_cast<uLong>(size));

    if (result != Z_OK) {
        const auto* errMsg = "Unknown error";
        switch (result) {
        case Z_MEM_ERROR: errMsg = "Z_MEM_ERROR";
            break;
        case Z_BUF_ERROR: errMsg = "Z_BUF_ERROR";
            break;
        case Z_DATA_ERROR: errMsg = "Z_DATA_ERROR";
            break;
        case Z_STREAM_ERROR: errMsg = "Z_STREAM_ERROR";
            break;
        default:
            break;
        }
        throw Exception(std::format("ZLib decompression failed ({}): {}", result, errMsg));
    }

    return Stream({std::move(uncompressedData), destLen});
}
