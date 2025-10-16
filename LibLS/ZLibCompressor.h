#pragma once
#include "ICompressor.h"

class ZLibCompressor : public ICompressor
{
public:
    ZLibCompressor() = default;
    ~ZLibCompressor() override = default;

    Stream compress(StreamBase& input, LSCompressionLevel level) override;
    Stream compress(const uint8_t* data, size_t size, LSCompressionLevel level) override;
    Stream decompress(StreamBase& input, size_t uncompressedSize, bool chunked = false) override;
    Stream decompress(const uint8_t* data, size_t size, size_t uncompressedSize, bool chunked = false) override;
};
