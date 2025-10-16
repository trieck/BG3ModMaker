#include "pch.h"
#include "Exception.h"
#include "LZ4Codec.h"
#include "LZ4Compressor.h"
#include "LZ4FrameCompressor.h"

#include <lz4.h>

Stream LZ4Compressor::compress(StreamBase& input, LSCompressionLevel level)
{
    auto size = input.size();
    auto data = Stream::makeStream(input).detach();

    return compress(data.first.get(), size, level);
}

Stream LZ4Compressor::compress(const uint8_t* data, size_t size, LSCompressionLevel level)
{
    auto maxCompressedSize = LZ4_compressBound(static_cast<int>(size));

    auto compressedData = std::make_unique<uint8_t[]>(maxCompressedSize);

    int result;
    if (level == LSCompressionLevel::FAST) {
        result = LZ4_compress_fast(reinterpret_cast<const char*>(data),
                                   reinterpret_cast<char*>(compressedData.get()),
                                   static_cast<int>(size),
                                   maxCompressedSize,
                                   1);
    } else {
        result = LZ4_compress_default(reinterpret_cast<const char*>(data),
                                      reinterpret_cast<char*>(compressedData.get()),
                                      static_cast<int>(size),
                                      maxCompressedSize);
    }

    if (result <= 0) {
        throw Exception("LZ4 compression failed");
    }

    return Stream({std::move(compressedData), result});
}

Stream LZ4Compressor::decompress(StreamBase& input, size_t uncompressedSize, bool chunked)
{
    auto data = Stream::makeStream(input).detach();

    return decompress(data.first.get(), data.second, uncompressedSize, chunked);
}

Stream LZ4Compressor::decompress(const uint8_t* data, size_t size, size_t uncompressedSize, bool chunked)
{
    if (chunked) {
        return LZ4FrameCompressor::decompress(data, size, uncompressedSize);
    }

    return LZ4Codec::decode(data, size, uncompressedSize, true);
}
