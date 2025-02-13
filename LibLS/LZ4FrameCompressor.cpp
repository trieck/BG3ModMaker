#include "pch.h"

#include <lz4frame.h>
#include "LZ4FrameCompressor.h"

Stream::Ptr LZ4FrameCompressor::decompress(const Stream::Ptr& stream, size_t decompressedSize)
{
    if (!stream) {
        throw std::invalid_argument("Invalid input stream.");
    }

    auto [compressed, sz] = stream->read(stream->size())->bytes();

    auto output = std::make_unique<uint8_t[]>(decompressedSize);

    LZ4F_dctx* dctx;
    if (LZ4F_createDecompressionContext(&dctx, LZ4F_getVersion()) != 0) {
        throw std::runtime_error("Failed to create decompression context.");
    }

    auto result = LZ4F_decompress(dctx, output.get(), &decompressedSize, compressed.get(), &sz, nullptr);

    LZ4F_freeDecompressionContext(dctx);

    if (LZ4F_isError(result)) {
        throw std::runtime_error(std::format("Failed to decompress data: {}", LZ4F_getErrorName(result)));
    }

    return Stream::makeStream(reinterpret_cast<const char*>(output.get()), decompressedSize);
}
