#include "pch.h"
#include "LZ4FrameCompressor.h"
#include "Exception.h"

#include <lz4frame.h>

Stream LZ4FrameCompressor::decompress(StreamBase& stream, size_t decompressedSize)
{
    auto [compressed, sz] = Stream::makeStream(stream).detach();
    return decompress(compressed.get(), sz, decompressedSize);
}

Stream LZ4FrameCompressor::decompress(const uint8_t* data, size_t size, size_t decompressedSize)
{
    auto output = std::make_unique<uint8_t[]>(decompressedSize);

    LZ4F_dctx* dctx;
    if (LZ4F_createDecompressionContext(&dctx, LZ4F_getVersion()) != 0) {
        throw Exception("Failed to create decompression context.");
    }

    auto result = LZ4F_decompress(dctx, output.get(), &decompressedSize, data, &size, nullptr);

    LZ4F_freeDecompressionContext(dctx);

    if (LZ4F_isError(result)) {
        throw Exception(std::format("Failed to decompress data: {}", LZ4F_getErrorName(result)));
    }

    return Stream::makeStream(reinterpret_cast<const char*>(output.get()), decompressedSize);
}
