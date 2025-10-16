#pragma once

#include "StreamBase.h"
#include "Compress.h"

class ICompressor
{
public:
    virtual ~ICompressor() = default;

    using Ptr = std::unique_ptr<ICompressor>;

    virtual Stream compress(StreamBase& input, LSCompressionLevel level) = 0;
    virtual Stream compress(const uint8_t* data, size_t size, LSCompressionLevel level) = 0;
    virtual Stream decompress(StreamBase& input, size_t uncompressedSize, bool chunked = false) = 0;
    virtual Stream decompress(const uint8_t* data, size_t size, size_t uncompressedSize, bool chunked = false) = 0;
};
