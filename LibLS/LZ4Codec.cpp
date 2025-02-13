#include "pch.h"
#include <lz4.h>
#include "LZ4Codec.h"

Stream::Ptr LZ4Codec::decode(const Stream::Ptr& stream, uint32_t inputOffset, size_t inputSize, uint32_t outputOffset,
    size_t outputSize, bool knownOutputSize)
{
    if (!stream) {
        throw std::invalid_argument("Invalid input stream.");
    }

    stream->seek(inputOffset, SeekMode::Begin);
    auto [compressed, sz] = stream->read(inputSize)->bytes();

    if (!knownOutputSize) {
        outputSize = LZ4_decompress_safe(reinterpret_cast<const char*>(compressed.get()), nullptr, 
            static_cast<int>(inputSize), 0);
        if (outputSize <= 0) {
            throw std::runtime_error("Failed to determine output size.");
        }
    }

    auto output = std::make_unique<uint8_t[]>(outputSize);
    auto result = LZ4_decompress_safe(reinterpret_cast<const char*>(compressed.get()),
        reinterpret_cast<char*>(output.get()), static_cast<int>(inputSize), static_cast<int>(outputSize));
    if (result <= 0) {
        throw std::runtime_error("Failed to decompress data.");
    }

    return Stream::makeStream(reinterpret_cast<const char*>(output.get()), result);
}
