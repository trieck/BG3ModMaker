#pragma once

#include "Package.h"

class PAKWriter
{
public:
    using ProgressCallback = std::function<void(size_t current, size_t total, const std::string& filename)>;

    PAKWriter(PackageBuildData build, const char* packagePath, ProgressCallback cb = nullptr);
    ~PAKWriter() = default;

    void write();
    void close();

private:
    bool canCompressFile(const PackageBuildInputFile& inputFile);
    PackagedFileInfoCommon writeFile(const PackageBuildInputFile& inputFile);
    std::vector<PackagedFileInfoCommon> packFiles();
    void archiveHash(uint8_t digest[16]);
    void writeCompressedFileList(const std::vector<PackagedFileInfoCommon>& files);
    void writePadding();

    PackageHeaderCommon m_metadata{};
    PackageBuildData m_build{};
    FileStream m_stream;
    Package m_package;
    std::string m_packagePath;
    ProgressCallback m_cb;
};
