#include "pch.h"
#include "Exception.h"
#include "Iconizer.h"
#include "StringHelper.h"
#include "XmlWrapper.h"

#include <DirectXTex.h>
#include <filesystem>

static constexpr auto COMMIT_SIZE = 1000;

static DirectX::ScratchImage cropIcon(const DirectX::Image* atlas, uint32_t left, uint32_t top, uint32_t width,
                                      uint32_t height)
{
    DirectX::ScratchImage cropped;
    auto hr = cropped.Initialize2D(atlas->format, width, height, 1, 1);
    if (FAILED(hr)) {
        throw Exception("Failed to initialize cropped image");
    }

    const DirectX::Image* dst = cropped.GetImage(0, 0, 0);

    DirectX::Rect rect{left, top, width, height};

    hr = CopyRectangle(*atlas, rect, *dst, DirectX::TEX_FILTER_DEFAULT, 0, 0);
    if (FAILED(hr)) {
        throw Exception("Failed to copy rectangle from atlas");
    }

    return cropped;
}

Iconizer::Iconizer()
{
    auto hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        throw Exception("Failed to initialize COM library");
    }

    auto gameDataPath = m_settings.GetString("Settings", "GameDataPath", "");
    if (gameDataPath.IsEmpty()) {
        throw Exception("Game data path is not set in settings");
    }

    m_gameDataPath = StringHelper::toUTF8(gameDataPath);

    auto iconPakPath = std::filesystem::path(m_gameDataPath) / "Icons.pak";
    if (!exists(iconPakPath)) {
        throw Exception(std::format("Cannot find icon package: {}", iconPakPath.string()));
    }

    m_iconReader.read(iconPakPath.string().c_str());
}

Iconizer::~Iconizer()
{
    close();

    CoUninitialize();
}

void Iconizer::close()
{
    if (m_db) {
        delete m_db;
        m_db = nullptr;
    }
}

bool Iconizer::isOpen() const
{
    return m_db != nullptr;
}

PageableIterator::Ptr Iconizer::newIterator(size_t pageSize)
{
    if (!isOpen()) {
        throw Exception("Database is not open");
    }
    return PageableIterator::create(m_db, pageSize);
}

PageableIterator::Ptr Iconizer::newIterator(const char* key, size_t pageSize)
{
    if (!isOpen()) {
        throw Exception("Database is not open");
    }
    return PageableIterator::create(m_db, key, pageSize);
}

void Iconizer::iconize(const char* pakFile, const char* dbName, bool overwrite)
{
    m_reader.read(pakFile);

    if (m_listener) {
        m_listener->onStart(m_reader.files().size());
    }

    rocksdb::Options options;
    options.create_if_missing = true;

    close();

    if (overwrite) {
        DestroyDB(dbName, options);
    }

    auto status = rocksdb::DB::Open(options, dbName, &m_db);
    if (!status.ok()) {
        throw Exception(std::format("Failed to open RocksDB database: {}", status.ToString()));
    }

    m_batch.Clear();

    auto i = 0;
    for (const auto& file : m_reader.files()) {
        if (m_listener && m_listener->isCancelled()) {
            break;
        }

        if (file.name.ends_with("lsx") || file.name.ends_with("lsf")) {
            if (m_listener) {
                m_listener->onFileIconizing(i, file.name);
            }
        }

        if (file.name.ends_with("lsx")) {
            iconizeLSXFile(file);
        } else if (file.name.ends_with("lsf")) {
            iconizeLSFFile(file);
        }

        ++i;

        auto count = m_batch.Count();
        if (count > 0 && count % COMMIT_SIZE == 0) {
            m_db->Write(rocksdb::WriteOptions(), &m_batch);
            m_batch.Clear();
        }
    }

    m_db->Write(rocksdb::WriteOptions(), &m_batch);
    m_batch.Clear();

    status = m_db->Flush(rocksdb::FlushOptions());
    if (!status.ok()) {
        throw Exception(std::format("Failed to flush RocksDB database: {}", status.ToString()));
    }
}

void Iconizer::open(const char* dbName)
{
    close();

    rocksdb::Options options;

    auto status = rocksdb::DB::Open(options, dbName, &m_db);
    if (!status.ok()) {
        throw Exception("Failed to open RocksDB database: " + status.ToString());
    }
}

void Iconizer::setProgressListener(IIconizerProgressListener* listener)
{
    m_listener = listener;
}

DirectX::ScratchImage Iconizer::loadIconTexture(const std::string& path)
{
    for (const auto& file : m_iconReader.files()) {
        if (file.name.ends_with(path)) {
            auto [ptr, size] = m_iconReader.readFile(file.name);

            DirectX::ScratchImage image;
            auto hr = LoadFromDDSMemory(
                ptr.get(),
                size,
                DirectX::DDS_FLAGS_NONE,
                nullptr,
                image);
            if (FAILED(hr)) {
                throw Exception("Failed to load icon texture from memory");
            }

            return image;
        }
    }

    throw Exception(std::format("Cannot find icon texture: {}", path));
}

void Iconizer::iconizeLSXFile(const PackagedFileInfo& file)
{
    auto buffer = m_reader.readFile(file.name);
    XmlWrapper xmlDoc(buffer);

    auto size = xmlDoc.selectNode("//region[@id='TextureAtlasInfo']//node[@id='TextureAtlasIconSize']");
    if (size.node().empty()) {
        return; // no texture atlas info size
    }

    auto iconWidth = 0u;
    auto iconHeight = 0u;

    auto attributes = size.node().children("attribute");
    for (const auto& attr : attributes) {
        std::string id = attr.attribute("id").value();
        std::string value = attr.attribute("value").value();
        if (id == "Height") {
            iconHeight = std::stoul(value);
        } else if (id == "Width") {
            iconWidth = std::stoul(value);
        }
    }

    if (iconWidth == 0 || iconHeight == 0) {
        return; // missing required attributes
    }

    std::string path, pathUUID;
    auto pathNode = xmlDoc.selectNode("//region[@id='TextureAtlasInfo']//node[@id='TextureAtlasPath']");
    attributes = pathNode.node().children("attribute");
    for (const auto& attr : attributes) {
        std::string id = attr.attribute("id").value();
        std::string value = attr.attribute("value").value();
        if (id == "Path") {
            path = value;
        } else if (id == "UUID") {
            pathUUID = value;
        }
    }

    if (path.empty() || pathUUID.empty()) {
        return; // missing required attributes
    }

    auto textureWidth = 0u;
    auto textureHeight = 0u;

    pathNode = xmlDoc.selectNode("//region[@id='TextureAtlasInfo']//node[@id='TextureAtlasTextureSize']");
    attributes = pathNode.node().children("attribute");
    for (const auto& attr : attributes) {
        std::string id = attr.attribute("id").value();
        std::string value = attr.attribute("value").value();
        if (id == "Width") {
            textureWidth = std::stoul(value);
        } else if (id == "Height") {
            textureHeight = std::stoul(value);
        }
    }

    if (textureWidth == 0 || textureHeight == 0) {
        return; // missing required attributes
    }

    auto texture = loadIconTexture(path);
    if (texture.GetMetadata().width != textureWidth ||
        texture.GetMetadata().height != textureHeight) {
        throw Exception("Texture size does not match the size specified in the LSX file");
    }

    auto const* srcImage = texture.GetImage(0, 0, 0);

    DirectX::ScratchImage decompressed;
    if (DirectX::IsCompressed(srcImage->format)) {
        auto hr = Decompress(
            texture.GetImage(0, 0, 0), 1, texture.GetMetadata(),
            DXGI_FORMAT_R8G8B8A8_UNORM, decompressed);
        if (FAILED(hr)) {
            throw Exception("Failed to decompress atlas DDS");
        }

        srcImage = decompressed.GetImage(0, 0, 0);
    }

    auto nodes = xmlDoc.selectNodes("//node[@id='IconUV']");
    for (const auto& xpathNode : nodes) {
        auto node = xpathNode.node();

        float u1 = std::numeric_limits<float>::min();
        float u2 = std::numeric_limits<float>::min();
        float v1 = std::numeric_limits<float>::min();
        float v2 = std::numeric_limits<float>::min();

        std::string mapKey;
        attributes = node.children("attribute");
        for (const auto& attr : attributes) {
            std::string id = attr.attribute("id").value();
            std::string value = attr.attribute("value").value();
            if (id == "U1") {
                u1 = std::stof(value);
            } else if (id == "U2") {
                u2 = std::stof(value);
            } else if (id == "V1") {
                v1 = std::stof(value);
            } else if (id == "V2") {
                v2 = std::stof(value);
            } else if (id == "MapKey") {
                mapKey = value;
            }
        }

        if (mapKey.empty() ||
            u1 <= std::numeric_limits<float>::min() ||
            u2 <= std::numeric_limits<float>::min() ||
            v1 <= std::numeric_limits<float>::min() ||
            v2 <= std::numeric_limits<float>::min()) {
            continue; // missing required attributes
        }

        std::cout << mapKey << std::endl;

        auto left = static_cast<uint32_t>(u1 * textureWidth);
        auto top = static_cast<uint32_t>(v1 * textureHeight);

        auto icon = cropIcon(srcImage, left, top, iconWidth, iconHeight);

        DirectX::Blob blob;
        auto hr = SaveToDDSMemory(*icon.GetImage(0, 0, 0), DirectX::DDS_FLAGS_NONE, blob);
        if (FAILED(hr)) {
            throw Exception("Failed to save icon to DDS memory");
        }

        rocksdb::Slice key(mapKey);
        rocksdb::Slice value(reinterpret_cast<const char*>(blob.GetBufferPointer()), blob.GetBufferSize());

        auto s = m_batch.Put(key, value);
        if (!s.ok()) {
            throw Exception(std::format("Failed to write to RocksDB database: {}", s.ToString()));
        }
    }
}

void Iconizer::iconizeLSFFile(const PackagedFileInfo& file)
{
}
