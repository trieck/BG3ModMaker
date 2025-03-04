#pragma once

#include "Package.h"

class PAKWriter
{
public:
    PAKWriter(PackageBuildData build, const char* packagePath);
    ~PAKWriter() = default;

    void write();
    void close();

private:
    std::vector<PackagedFileInfoCommon> packFiles();
    void writePadding();
    PackagedFileInfoCommon writeFile(const PackageBuildInputFile& inputFile);
    void writeCompressedFileList(const std::vector<PackagedFileInfoCommon>& files);
    void archiveHash(uint8_t digest[16]);

    PackageHeaderCommon m_metadata{};
    PackageBuildData m_build{};
    FileStream m_stream;
    Package m_package;
    std::string m_packagePath;
};
