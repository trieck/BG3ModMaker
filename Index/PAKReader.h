#pragma once

#include <vector>
#include <fstream>

#include "PAKFormat.h"

struct Package
{
    Package();

    void addFile(const PackagedFileInfo& file);
    bool load(const char* filename);
    void seek(int64_t offset, std::ios_base::seekdir dir);
    void reset();

    template<typename T>
    T read();

    void read(void* buffer, std::streamsize size);

    PAKHeader m_header{};
    std::vector<PackagedFileInfo> m_files{};
    std::string m_filename{};
    std::ifstream m_file{};
};

template <typename T>
T Package::read()
{
    T value;
    m_file.read(reinterpret_cast<char*>(&value), sizeof(T));
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
    Package m_package{};
};
