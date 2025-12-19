#pragma once

#include "Package.h"

class PAKReader final
{
public:
    PAKReader();
    ~PAKReader() = default;

    PAKReader(PAKReader&&) noexcept;
    PAKReader& operator=(PAKReader&&) noexcept;

    PAKReader(const PAKReader&) = delete;
    PAKReader& operator=(const PAKReader&) = delete;

    bool explode(const char* path);
    bool read(const char* filename);
    ByteBuffer readFile(const std::string& name);

    const PackagedFileInfo& operator[](const std::string& name) const;

    void sortFiles();
    const std::vector<PackagedFileInfo>& files() const;

    Package& package();
    void close();
    const std::string& filename() const;

private:
    bool extractFile(const PackagedFileInfo& file, const char* path);

    Package m_package{};
};
