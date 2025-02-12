#include "pch.h"
#include "PAKFormat.h"

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

void FileEntry18::toCommon(PackagedFileInfoCommon& info) const
{
}

