#pragma once
#include "ICompressor.h"

class LZ4Compressor : public ICompressor
{
public:
    LZ4Compressor() = default;
    ~LZ4Compressor() override = default;

    Stream compress(StreamBase& input, LSCompressionLevel level) override;
    Stream compress(const uint8_t* data, size_t size, LSCompressionLevel level) override;

    Stream decompress(StreamBase& input, size_t uncompressedSize, bool chunked = false) override;
    Stream decompress(const uint8_t* data, size_t size, size_t uncompressedSize, bool chunked = false) override;
};
