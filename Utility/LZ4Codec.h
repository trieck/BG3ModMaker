#pragma once
#include "Stream.h"

class LZ4Codec
{
public:
    LZ4Codec() = default;
    ~LZ4Codec() = default;

    static Stream encode(StreamBase& input, uint32_t inputOffset, size_t inputSize);
    static Stream encode(const uint8_t* data, size_t size);

    static Stream decode(StreamBase& input, uint32_t inputOffset, size_t inputSize, size_t outputSize = 0,
                         bool knownOutputSize = false);
    static Stream decode(const uint8_t* data, size_t size, size_t outputSize = 0, bool knownOutputSize = false);
};
