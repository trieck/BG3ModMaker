
#include "PAKFormat.h"
#include <xmemory>

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

    memset(header.md5, 0, sizeof(header.md5));

    return header;
}

void FileEntry18::toCommon(PackagedFileInfoCommon& info) const
{
}

