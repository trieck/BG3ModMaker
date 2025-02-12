#pragma once

#include "PAKFormat.h"
#include "FileStream.h"

struct Package final
{
    Package();
    ~Package();

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

class PAKReader final
{
public:
    PAKReader();
    ~PAKReader() = default;

    bool read(const char* filename);
    void close();

    bool explode(const char* path) const;
    void openStreams(uint32_t numParts);

    Package& package();

    const std::vector<PackagedFileInfo>& files() const;

    const PackagedFileInfo& operator[](const std::string& name) const;

    ByteBuffer readFile(const std::string& name) const;

private:
    bool extractFile(const PackagedFileInfo& file, const char* path) const;

    Package m_package{};
};
