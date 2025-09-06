#include "pch.h"
#include "Cataloger.h"
#include "Exception.h"
#include "LSFReader.h"
#include "XmlWrapper.h"

#include <regex>

using json = nlohmann::json;

static constexpr auto COMMIT_SIZE = 10000;

static bool isUUID(const std::string& s)
{
    static constexpr auto NUL_UUID = "00000000-0000-0000-0000-000000000000";

    static const std::regex uuidRegex(
        "^[0-9a-fA-F]{8}-"
        "[0-9a-fA-F]{4}-"
        "[0-9a-fA-F]{4}-"
        "[0-9a-fA-F]{4}-"
        "[0-9a-fA-F]{12}$",
        std::regex::icase
    );

    return std::regex_match(s, uuidRegex) && s != NUL_UUID;
}

Cataloger::Cataloger()
{
}

Cataloger::~Cataloger()
{
    close();
}

void Cataloger::open(const char* dbName)
{
    close();

    rocksdb::Options options;

    auto status = rocksdb::DB::Open(options, dbName, &m_db);
    if (!status.ok()) {
        throw Exception("Failed to open RocksDB database: " + status.ToString());
    }
}

void Cataloger::close()
{
    if (m_db) {
        delete m_db;
        m_db = nullptr;
    }
}

void Cataloger::catalog(const char* pakFile, const char* dbName, bool overwrite)
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
        throw Exception("Failed to open RocksDB database: " + status.ToString());
    }

    m_batch.Clear();

    auto i = 0;
    for (const auto& file : m_reader.files()) {
        if (m_listener && m_listener->isCancelled()) {
            break;
        }

        if (file.name.ends_with("lsx") || file.name.ends_with("lsf")) {
            if (m_listener) {
                m_listener->onFileCataloging(i, file.name);
            }
        }

        if (file.name.ends_with("lsx")) {
            catalogLSXFile(file);
        } else if (file.name.ends_with("lsf")) {
            catalogLSFFile(file);
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
        throw Exception("Failed to flush RocksDB database: " + status.ToString());
    }
}

nlohmann::json Cataloger::get(const std::string& key)
{
    if (m_db == nullptr) {
        throw Exception("Database is not open");
    }

    std::string value;
    auto s = m_db->Get(rocksdb::ReadOptions(), key, &value);

    if (!s.ok()) {
        if (s.IsNotFound()) {
            return {};
        }
        throw Exception("Failed to read from RocksDB database: " + s.ToString());
    }

    return json::parse(value);
}

bool Cataloger::isOpen() const
{
    return m_db != nullptr;
}

void Cataloger::setProgressListener(ICatalogProgressListener* listener)
{
}

PageableIterator::Ptr Cataloger::newIterator(size_t pageSize)
{
    if (m_db == nullptr) {
        throw Exception("Database is not open");
    }

    return PageableIterator::create(m_db, pageSize);
}

void Cataloger::catalogLSXFile(const PackagedFileInfo& file)
{
    auto buffer = m_reader.readFile(file.name);
    XmlWrapper xmlDoc(buffer);

    auto nodes = xmlDoc.selectNodes("//*[self::node]");
    for (const auto& xpathNode : nodes) {
        auto node = xpathNode.node();
        if (node.attributes().empty()) {
            continue;
        }

        std::string type = node.attribute("id").value();
        if (type != "GameObjects") {
            continue; // not a catalogable node
        }

        json doc;
        doc["source_file"] = file.name;
        doc["type"] = type;

        json attributes = json::array();
        for (const auto& attribute : node.children("attribute")) {
            std::string id = attribute.attribute("id").as_string();
            std::string value = attribute.attribute("value").as_string();
            std::string attrType = attribute.attribute("type").as_string();

            if (id.empty() || value.empty()) {
                continue;
            }

            json attr;
            attr["id"] = id;
            attr["value"] = value;
            attr["type"] = attrType;
            attributes.emplace_back(std::move(attr));
        }

        std::string mapKey;
        for (const auto& attr : attributes) {
            if (attr["id"].get<std::string>() == "MapKey") {
                mapKey = attr["value"].get<std::string>();
                break;
            }
        }

        if (mapKey.empty() || !isUUID(mapKey)) {
            continue; // skip entries without valid MapKey
        }

        doc["attributes"] = attributes;

        auto s = m_batch.Put(mapKey, doc.dump());
        if (!s.ok()) {
            throw Exception("Failed to write to RocksDB database: " + s.ToString());
        }
    }
}

void Cataloger::catalogLSFFile(const PackagedFileInfo& file)
{
    auto buffer = m_reader.readFile(file.name);

    LSFReader reader;
    auto resource = reader.read(buffer);

    for (const auto& val : resource->regions | std::views::values) {
        catalogRegion(file.name, val);
    }
}

void Cataloger::catalogNode(const std::string& filename, const LSNode::Ptr& node)
{
    json doc;
    doc["source_file"] = filename;
    doc["type"] = node->name;

    json attributes = json::array();
    for (const auto& [key, val] : node->attributes) {
        json attr;

        attr["id"] = key;
        attr["value"] = val.str();
        attr["type"] = val.typeStr();

        if (key == "Script") {
            continue; // Skip script content
        }

        attributes.emplace_back(std::move(attr));
    }

    std::string mapKey;
    for (const auto& attr : attributes) {
        if (attr["id"].get<std::string>() == "MapKey") {
            mapKey = attr["value"].get<std::string>();
            break;
        }
    }

    if (isUUID(mapKey) && node->name == "GameObjects") {
        doc["attributes"] = attributes;

        auto s = m_batch.Put(mapKey, doc.dump());
        if (!s.ok()) {
            throw Exception("Failed to write to RocksDB database: " + s.ToString());
        }
    }

    for (const auto& val : node->children | std::views::values) {
        for (const auto& childNode : val) {
            catalogNode(filename, childNode);
        }
    }
}

void Cataloger::catalogNodes(const std::string& filename, const std::vector<LSNode::Ptr>& nodes)
{
    for (const auto& node : nodes) {
        catalogNode(filename, node);
    }
}

void Cataloger::catalogRegion(const std::string& fileName, const Region::Ptr& region)
{
    for (const auto& val : region->children | std::views::values) {
        catalogNodes(fileName, val);
    }
}
