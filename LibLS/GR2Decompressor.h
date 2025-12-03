#pragma once

enum GR2CompressionType : uint32_t
{
    COMPRESSION_NONE = 0, /* No compression */
    COMPRESSION_OODLE0,
    COMPRESSION_OODLE1,
    COMPRESSION_BITKNIT1,
    COMPRESSION_BITKNIT2
};

class GR2Decompressor
{
public:
    GR2Decompressor() = default;
    ~GR2Decompressor();

    BOOL Decompress(uint32_t type, uint32_t compressedSize, void* compressedData, void* decompressedData,
                    uint32_t stop0, uint32_t stop1, uint32_t stop2);

    BOOL DecompressBitknit2(uint32_t compressedSize, void* compressedData, uint32_t decompressedSize,
                            void* decompressedData);

    BOOL IsLoaded() const;
    BOOL Load();
    void Unload();

private:
    HMODULE m_hModule = nullptr;
};
