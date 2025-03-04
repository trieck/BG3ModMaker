#pragma once

#include <unordered_map>
#include "Package.h"

class PAKReader final
{
public:
    PAKReader();
    ~PAKReader() = default;

    bool read(const char* filename);
    void close();

    bool explode(const char* path);
    void openStreams(uint32_t numParts);

    Package& package();

    const std::vector<PackagedFileInfo>& files() const;

    const PackagedFileInfo& operator[](const std::string& name) const;

    ByteBuffer readFile(const std::string& name);

private:
    bool extractFile(const PackagedFileInfo& file, const char* path);

    Package m_package{};
};
