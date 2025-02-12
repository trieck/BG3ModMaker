#pragma once

#include "Compress.h"
#include "LSCommon.h"

struct PackagedFileInfoCommon;

enum class PackageFlags : uint8_t {
    allowMemoryMapping = 0x02,  // Allow memory-mapped access to the files in this archive.
    Solid = 0x04, // All files are compressed into a single LZ4 stream
    Preload = 0x08 // Archive contents should be preloaded on game startup.
};

struct PAKHeader {
    uint32_t version;
    uint64_t fileListOffset;
    uint32_t fileListSize;
    uint32_t numFiles;
    uint32_t numParts;
    uint32_t dataOffset;
    PackageFlags flags;
    uint8_t priority;
    uint8_t md5[16];
};

#pragma pack(push, 1)

struct LSPKHeader16 {
    uint32_t version;
    uint64_t fileListOffset;
    uint32_t fileListSize;
    uint8_t flags;
    uint8_t priority;
    uint8_t md5[16];
    uint16_t numParts;

    PAKHeader commonHeader() const;
};

struct FileEntry18 {
    char name[256];
    uint32_t offsetInFile1;
    uint16_t offsetInFile2;
    uint8_t archivePart;
    uint8_t flags;
    uint32_t sizeOnDisk;
    uint32_t uncompressedSize;

    void toCommon(PackagedFileInfoCommon& info) const;
};

#pragma pack(pop)

struct PackagedFileInfoCommon
{
    std::string name;
    uint32_t archivePart;
    uint32_t crc;
    CompressionFlags flags;
    uint64_t offsetInFile;
    uint32_t sizeOnDisk;
    uint32_t uncompressedSize;
};

struct PackagedFileInfo : PackagedFileInfoCommon
{
    CompressionMethod method() const
    {
        return static_cast<CompressionMethod>(flags & 0x0F);
    }

    std::uint32_t size() const
    {
        return method() == CompressionMethod::NONE ? sizeOnDisk : uncompressedSize;
    }
};
