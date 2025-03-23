#include "pch.h"
#include "Package.h"

PAKHeader LSPKHeader16::commonHeader() const
{
    PAKHeader header{};

    header.version = version;
    header.dataOffset = 0;
    header.fileListOffset = fileListOffset;
    header.fileListSize = fileListSize;
    header.numParts = 1;
    header.flags = static_cast<PackageFlags>(flags);
    header.priority = priority;

    return header;
}

LSPKHeader16 LSPKHeader16::fromCommon(const PackageHeaderCommon& h)
{
    LSPKHeader16 header{};
    header.version = static_cast<uint32_t>(PackageHeaderCommon::currentVersion);
    header.fileListOffset = h.fileListOffset;
    header.fileListSize = h.fileListSize;
    header.flags = static_cast<uint8_t>(h.flags);
    header.priority = h.priority;
    header.numParts = static_cast<uint16_t>(h.numParts);
    return header;
}

FileEntry18 FileEntry18::fromCommon(const PackagedFileInfoCommon& info)
{
    FileEntry18 entry{};
    strncpy_s(entry.name, info.name.c_str(), sizeof(entry.name));
    entry.offsetInFile1 = static_cast<uint32_t>(info.offsetInFile & 0xFFFFFFFF);
    entry.offsetInFile2 = static_cast<uint16_t>(info.offsetInFile >> 32 & 0xFFFF);
    entry.archivePart = static_cast<uint8_t>(info.archivePart);
    entry.flags = static_cast<uint8_t>(info.flags);
    entry.sizeOnDisk = info.sizeOnDisk;
    entry.uncompressedSize = compressionMethod(info.flags) == CompressionMethod::NONE ? 0 : info.uncompressedSize;
    return entry;
}

