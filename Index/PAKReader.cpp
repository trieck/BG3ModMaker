#include "stdafx.h"

#include "Compress.h"
#include "Exception.h"
#include "PAKFormat.h"
#include "PAKReader.h"

// anonymous namespace
namespace {
    constexpr uint32_t PAK_MAGIC = 0x4B50534C;

    std::unique_ptr<std::istream> createMemoryStream(const std::unique_ptr<uint8_t[]>& data, const size_t size)
    {
        return std::make_unique<std::istringstream>(std::string(reinterpret_cast<char*>(data.get()), size));
    }

    template <typename TFile>
    bool readStructs(std::istream& stream, std::vector<TFile>& entries, uint32_t numFiles)
    {
        entries.resize(numFiles);

        for (uint32_t i = 0; i < numFiles; ++i) {
            stream.read(reinterpret_cast<char*>(&entries[i]), sizeof(TFile));
            if (!stream) {
                throw Exception(GetLastError());
            }
        }

        return true;
    }

    template <typename TFileEntry>
    PackagedFileInfo createFromEntry(Package& package, const TFileEntry& entry)
    {
        PackagedFileInfo info;

        info.name = entry.name;
        info.archivePart = entry.archivePart;
        info.crc = 0;
        info.flags = static_cast<CompressionFlags>(entry.flags);
        info.offsetInFile = entry.offsetInFile1 | (static_cast<uint64_t>(entry.offsetInFile2) << 32);
        info.sizeOnDisk = entry.sizeOnDisk;
        info.uncompressedSize = entry.uncompressedSize;

        return info;
    }

    template <typename TFileEntry>
    void readCompressedFileList(PAKReader& reader, int64_t offset)
    {
        auto& package = reader.package();

        package.seek(offset, SeekMode::Begin);
        auto numFiles = package.read<uint32_t>();

        std::unique_ptr<uint8_t[]> compressed;
        uint32_t compressedSize;

        if (reader.package().m_header.version > 13) {
            package.seek(offset + 4, SeekMode::Begin);
            compressedSize = package.read<uint32_t>();

            compressed = std::make_unique<uint8_t[]>(compressedSize);

            package.read(compressed.get(), compressedSize);
        } else {
            compressedSize = reader.package().m_header.fileListSize - 4;
            compressed = std::make_unique<uint8_t[]>(compressedSize);

            package.read(compressed.get(), compressedSize);
        }

        const uint32_t fileBufferSize = sizeof(TFileEntry) * numFiles;

        std::unique_ptr<uint8_t[]> decompressed;
        auto result = decompressData(CompressionMethod::LZ4, compressed.get(), compressedSize, decompressed,
                                     fileBufferSize);
        if (!result) {
            throw std::ios_base::failure("Failed to decompress file list.");
        }

        std::vector<TFileEntry> entries(numFiles);
        const auto stream = createMemoryStream(decompressed, fileBufferSize);

        result = readStructs<TFileEntry>(*stream, entries, numFiles);
        if (!result) {
            throw std::ios_base::failure("Failed to read file list.");
        }

        for (const auto& entry : entries) {
            // TODO: handle archive parts, whatever they are
            auto info = createFromEntry<TFileEntry>(package, entry);
            package.addFile(info);
        }
    }

    template <typename THeader, typename TFileEntry>
    bool readHeader(PAKReader& pakReader, int64_t offset)
    {
        auto& package = pakReader.package();

        // Seek to the header
        package.seek(offset, SeekMode::Begin);

        // Read the header
        THeader header = package.read<THeader>();

        package.m_header = header.commonHeader();

        pakReader.openStreams(header.numParts);

        if (header.version > 10) {
            package.m_header.dataOffset = static_cast<uint32_t>(offset) + sizeof(THeader);
            readCompressedFileList<TFileEntry>(pakReader, package.m_header.fileListOffset);
        } else {
            return false;
        }

        return true;
    }
} // anonymous namespace

PAKReader::PAKReader()
= default;

bool PAKReader::read(const char* filename)
{
    m_package.load(filename);
    m_package.seek(-4, SeekMode::End);

    auto signature = m_package.read<uint32_t>();

    if (signature == PAK_MAGIC) {
        // Read v13 PAK file
        throw std::ios_base::failure("Unsupported PAK version.");
    }

    // Check for v10 PAK file
    m_package.seek(0, SeekMode::Begin);
    signature = m_package.read<uint32_t>();

    if (signature == PAK_MAGIC) {
        // Read v10 PAK file
        auto version = m_package.read<int32_t>();

        if (version != 18) {
            throw std::ios_base::failure("Unsupported PAK version.");
        }

        readHeader<LSPKHeader16, FileEntry18>(*this, 4);

        return true;
    }

    return false;
}

bool PAKReader::explode(const char* path)
{
    for (auto& files = m_package.m_files; const auto& fileInfo : files) {
        if (!extractFile(fileInfo, path)) {
            return false;
        }
    }

    return true;
}

void PAKReader::openStreams(uint32_t numParts)
{
    // TODO: implement
}

void PAKReader::addFile(PackagedFileInfo file)
{
}

Package& PAKReader::package()
{
    return m_package;
}

bool PAKReader::extractFile(const PackagedFileInfo& fileInfo, const char* path)
{
    m_package.seek(static_cast<int64_t>(fileInfo.offsetInFile), SeekMode::Begin);

    std::vector<uint8_t> fileData(fileInfo.sizeOnDisk);
    m_package.read(fileData.data(), fileInfo.sizeOnDisk);

    if (fileInfo.method() != CompressionMethod::NONE) {
        std::unique_ptr<uint8_t[]> decompressed;
        if (auto result = decompressData(fileInfo.method(), fileData.data(), fileInfo.sizeOnDisk, decompressed,
                                         fileInfo.uncompressedSize); !result) {
            throw std::ios_base::failure("Failed to decompress file.");
        }
        fileData = std::vector(decompressed.get(), decompressed.get() + fileInfo.uncompressedSize);
    }

    std::filesystem::path outputPath = std::filesystem::path(path) / fileInfo.name;
    create_directories(outputPath.parent_path()); // Ensure parent directories exist

    FileStream outFile;
    outFile.Open(outputPath.string().c_str(), "wb");
    outFile.Write(fileData.data(), fileInfo.size());

    return true;
}

inline bool Package::load(const char* filename)
{
    reset();

    m_filename = filename;
    m_file.Open(m_filename.c_str(), "rb");

    return true;
}

void Package::seek(int64_t offset, SeekMode mode) const
{
    m_file.Seek(offset, mode);
}

void Package::reset()
{
    m_file.Close();

    m_files.clear();
}

void Package::read(void* buffer, std::size_t size) const
{
    m_file.Read(buffer, size);
}

Package::Package()
= default;

void Package::addFile(const PackagedFileInfo& file)
{
    m_files.emplace_back(file);
}
