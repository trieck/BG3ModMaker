#pragma once

#include "PAKFormat.h"
#include "FileStream.h"

struct Package
{
    Package();

    void addFile(const PackagedFileInfo& file);
    bool load(const char* filename);
    void seek(int64_t offset, SeekMode mode) const;
    void reset();

    template<typename T>
    T read();

    void read(void* buffer, std::size_t size) const;

    PAKHeader m_header{};
    std::vector<PackagedFileInfo> m_files{};
    std::string m_filename{};
    FileStream m_file{};
};

template <typename T>
T Package::read()
{
    T value;
    m_file.Read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

class PAKReader
{
public:
    PAKReader();
    virtual ~PAKReader() = default;

    bool read(const char* filename);
    bool explode(const char* path);

    void openStreams(uint32_t numParts);

    void addFile(PackagedFileInfo file);
    Package& package();
    

private:
    bool extractFile(const PackagedFileInfo& value, const char* path);

    Package m_package{};
};
