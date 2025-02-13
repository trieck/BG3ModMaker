#pragma once

#include "Stream.h"

class LZ4FrameCompressor
{
public:
    LZ4FrameCompressor() = default;
    ~LZ4FrameCompressor() = default;

    static Stream::Ptr decompress(const Stream::Ptr& stream, size_t decompressedSize);
};

