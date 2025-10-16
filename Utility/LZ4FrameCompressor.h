#pragma once

#include "Stream.h"
#include "StreamBase.h"

class LZ4FrameCompressor
{
public:
    LZ4FrameCompressor() = default;
    ~LZ4FrameCompressor() = default;

    static Stream decompress(StreamBase& stream, size_t decompressedSize);
    static Stream decompress(const uint8_t* data, size_t size, size_t decompressedSize);
};
