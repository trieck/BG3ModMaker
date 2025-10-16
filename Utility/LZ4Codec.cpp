#include "pch.h"
#include "Exception.h"
#include "LZ4Codec.h"

#include <lz4.h>

Stream LZ4Codec::encode(StreamBase& input, uint32_t inputOffset, size_t inputSize)
{
    input.seek(inputOffset, SeekMode::Begin);

    auto [data, sz] = Stream::makeStream(input).detach();

    return encode(data.get(), inputSize);
}

Stream LZ4Codec::encode(const uint8_t* data, size_t size)
{
    auto outputSize = LZ4_compressBound(static_cast<int>(size));
    auto output = std::make_unique<uint8_t[]>(outputSize);

    auto result = LZ4_compress_default(reinterpret_cast<const char*>(data),
                                       reinterpret_cast<char*>(output.get()),
                                       static_cast<int>(size), static_cast<int>(outputSize));
    if (result <= 0) {
        throw Exception("Failed to compress data.");
    }

    return Stream::makeStream(std::move(output), result);
}

Stream LZ4Codec::decode(StreamBase& input, uint32_t inputOffset, size_t inputSize, size_t outputSize,
                        bool knownOutputSize)
{
    input.seek(inputOffset, SeekMode::Begin);
    auto [compressed, sz] = Stream::makeStream(input).detach();

    return decode(compressed.get(), inputSize, outputSize, knownOutputSize);
}

Stream LZ4Codec::decode(const uint8_t* data, size_t size, size_t outputSize, bool knownOutputSize)
{
    if (!knownOutputSize) {
        outputSize = LZ4_decompress_safe(reinterpret_cast<const char*>(data), nullptr,
                                         static_cast<int>(size), 0);
        if (outputSize <= 0) {
            throw Exception("Failed to determine output size.");
        }
    }

    auto output = std::make_unique<uint8_t[]>(outputSize);
    auto result = LZ4_decompress_safe(reinterpret_cast<const char*>(data),
                                      reinterpret_cast<char*>(output.get()), static_cast<int>(size),
                                      static_cast<int>(outputSize));
    if (result <= 0) {
        throw Exception("Failed to decompress data.");
    }

    return Stream::makeStream(std::move(output), result);
}
