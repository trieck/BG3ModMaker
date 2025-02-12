
#include "pch.h"

#include "Compress.h"
#include <lz4.h>

bool decompressData(CompressionMethod method, const uint8_t* compressedData, uint32_t compressedSize,
    uint8Ptr& decompressedData, uint32_t decompressedSize)
{
    if (method != CompressionMethod::LZ4) {
        return false; // Future expansion for other compression methods
    }

    // Allocate buffer for decompressed data
    decompressedData = std::make_unique<uint8_t[]>(decompressedSize);

    // Perform LZ4 decompression
    int result = LZ4_decompress_safe(reinterpret_cast<const char*>(compressedData),
        reinterpret_cast<char*>(decompressedData.get()),
        static_cast<int>(compressedSize),
        static_cast<int>(decompressedSize));

    return result >= 0; // Return true if decompression was successful
}
