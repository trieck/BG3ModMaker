#pragma once
#include "Stream.h"

class LZ4Codec
{
public:
    LZ4Codec() = default;
    ~LZ4Codec() = default;
    static Stream::Ptr decode(const Stream::Ptr& stream, 
        uint32_t inputOffset, 
        size_t inputSize,
        uint32_t outputOffset, 
        size_t outputSize = 0,
        bool knownOutputSize = false);
};

