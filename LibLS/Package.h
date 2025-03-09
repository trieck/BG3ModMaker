#pragma once

#include "Compress.h"
#include "FileStream.h"

struct PackagedFileInfoCommon;

constexpr uint32_t PAK_MAGIC = 0x4B50534C;

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

enum class PackageVersion
{
    V7 = 7, // D:OS 1
    V9 = 9, // D:OS 1 EE
    V10 = 10, // D:OS 2
    V13 = 13, // D:OS 2 DE
    V15 = 15, // BG3 EA
    V16 = 16, // BG3 EA Patch4
    V18 = 18 // BG3 Release
};

struct PackageHeaderCommon
{
    static auto constexpr currentVersion = PackageVersion::V18;
    static auto constexpr signature = PAK_MAGIC;

    uint32_t version;
    uint64_t fileListOffset;

    // size of file list; used for legacy (<= v10) packages only
    uint32_t fileListSize;

    // number of packed files; used for legacy (<= v10) packages only
    uint32_t numFiles;
    uint32_t numParts;

    // offset of packed data in archive part 0; used for legacy (<= v10) packages only
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

    static LSPKHeader16 fromCommon(const PackageHeaderCommon& h);
};

struct FileEntry18 {
    char name[256];
    uint32_t offsetInFile1;
    uint16_t offsetInFile2;
    uint8_t archivePart;
    uint8_t flags;
    uint32_t sizeOnDisk;
    uint32_t uncompressedSize;

    static FileEntry18 fromCommon(const PackagedFileInfoCommon& info);
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

struct PackageBuildInputFile
{
    std::string filename;
    std::string name;
};

struct PackageBuildData final
{
    PackageVersion version{PackageHeaderCommon::currentVersion };
    CompressionMethod compression{CompressionMethod::NONE};
    LSCompressionLevel compressionLevel{LSCompressionLevel::DEFAULT};
    PackageFlags flags{0};
    bool hash{false};

    std::vector<PackageBuildInputFile> files;
    bool excludeHidden{true};
    uint8_t priority{0};
};

struct Package final
{
    Package();
    ~Package();

    void addFile(const PackagedFileInfo& file);
    bool load(const char* filename);
    void seek(int64_t offset, SeekMode mode);
    void reset();

    template <typename T>
    T read();

    void read(void* buffer, std::size_t size);

    PAKHeader m_header{};
    std::vector<PackagedFileInfo> m_files{};
    std::unordered_map<std::string, size_t> m_filemap{};
    std::string m_filename{};
    FileStream m_file{};
};

template <typename T>
T Package::read()
{
    T value;
    m_file.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

class Packager
{
public:
    Packager(const PackageBuildData& build, const char* packagePath);
    ~Packager();
    bool build();
};
