#include "pch.h"
#include "ResourceUtils.h"

#include "Exception.h"
#include "FileStream.h"
#include "LSFWriter.h"
#include "LSXReader.h"

Resource::Ptr ResourceUtils::loadResource(const char* filename, ResourceFormat format)
{
    FileStream stream;
    stream.open(filename, "rb");

    ByteBuffer buffer{ std::make_unique<uint8_t[]>(stream.size()), stream.size() };
    stream.read(reinterpret_cast<char*>(buffer.first.get()), stream.size());
    stream.seek(0, SeekMode::Begin);

    switch (format) {
    case LSX: {
        LSXReader reader;
        return reader.read(buffer);
    }
    case LSF: {
        LSXReader reader;
        return reader.read(buffer);
    }

    }

    throw Exception("Unsupported resource format.");
}

void ResourceUtils::saveResource(const char* filename, const Resource::Ptr& resource, ResourceFormat format)
{
    if (format != LSF) {
        throw Exception("Unsupported resource format.");
    }

    FileStream stream;
    stream.open(filename, "wb");

    LSFWriter writer;
    writer.write(stream, *resource);
}
