#pragma once

#include "Stream.h"

class LZ4FrameCompressor
{
public:
    LZ4FrameCompressor() = default;
    ~LZ4FrameCompressor() = default;

    static Stream decompress(Stream& stream, size_t decompressedSize);
};

