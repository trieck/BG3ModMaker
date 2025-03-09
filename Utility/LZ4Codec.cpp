#include "pch.h"
#include <lz4.h>
#include "LZ4Codec.h"

Stream LZ4Codec::decode(Stream& stream, uint32_t inputOffset, size_t inputSize, uint32_t outputOffset,
                        size_t outputSize, bool knownOutputSize)
{
    stream.seek(inputOffset, SeekMode::Begin);
    auto [compressed, sz] = stream.read(inputSize).detach();

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

    return Stream::makeStream(std::move(output), result);
}

Stream LZ4Codec::encode(Stream& stream, uint32_t inputOffset, size_t inputSize)
{
    stream.seek(inputOffset, SeekMode::Begin);

    auto [input, sz] = stream.read(inputSize).detach();

    auto outputSize = LZ4_compressBound(static_cast<int>(inputSize));
    auto output = std::make_unique<uint8_t[]>(outputSize);

    auto result = LZ4_compress_default(reinterpret_cast<const char*>(input.get()), reinterpret_cast<char*>(output.get()),
        static_cast<int>(inputSize), static_cast<int>(outputSize));
    if (result <= 0) {
        throw std::runtime_error("Failed to compress data.");
    }

    return Stream::makeStream(std::move(output), result);
}
