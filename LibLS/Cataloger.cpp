#include "pch.h"
#include "Cataloger.h"
#include "Exception.h"
#include "LSFReader.h"
#include "XmlWrapper.h"

#include <regex>

using json = nlohmann::json;

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

    m_objectManager.open(dbName);
}

void Cataloger::openReadOnly(const char* dbName)
{
    close();

    m_objectManager.openReadOnly(dbName);
}

void Cataloger::close()
{
    m_objectManager.close();
}

void Cataloger::catalog(const char* pakFile, const char* dbName, bool overwrite)
{
    m_reader.read(pakFile);

    if (m_listener) {
        m_listener->onStart(m_reader.files().size());
    }

    close();

    if (overwrite) {
        DestroyDB(dbName, rocksdb::Options());
    }

    open(dbName);

    auto i = 0;
    for (const auto& file : m_reader.files()) {
        if (m_listener && m_listener->isCancelled()) {
            break;
        }

        if (file.name.ends_with("lsx") || file.name.ends_with("lsf")) {
            if (m_listener) {
                m_listener->onFile(i, file.name);
            }
        }

        if (file.name.ends_with("lsx")) {
            catalogLSXFile(file);
        } else if (file.name.ends_with("lsf")) {
            catalogLSFFile(file);
        }

        ++i;
    }

    m_objectManager.flush();

    if (m_listener) {
        if (m_listener->isCancelled()) {
            m_listener->onCancel();
        } else {
            m_listener->onFinished(i);
        }
    }
}

nlohmann::json Cataloger::get(const std::string& key)
{
    return m_objectManager.get(key);
}

bool Cataloger::isOpen() const
{
    return m_objectManager.isOpen();
}

void Cataloger::setProgressListener(IFileProgressListener* listener)
{
    m_listener = listener;
}

PageableIterator::Ptr Cataloger::newIterator(const char* key, size_t pageSize)
{
    if (!m_objectManager.isOpen()) {
        throw Exception("Database is not open");
    }

    return PageableIterator::create(m_objectManager.getDB(), key, pageSize);
}

PageableIterator::Ptr Cataloger::newIterator(size_t pageSize)
{
    if (!m_objectManager.isOpen()) {
        throw Exception("Database is not open");
    }

    return PageableIterator::create(m_objectManager.getDB(), pageSize);
}

PrefixIterator::Ptr Cataloger::getChildren(const char* parent) const
{
    return m_objectManager.getChildren(parent);
}

PrefixIterator::Ptr Cataloger::getRoots() const
{
    return m_objectManager.getRoots();
}

PrefixIterator::Ptr Cataloger::getRoots(const char* type) const
{
    return m_objectManager.getRoots(type);
}

PrefixIterator::Ptr Cataloger::getTypes() const
{
    return m_objectManager.getTypes();
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

            if (attr["id"].get<std::string>() == "ValueUUID") {
                mapKey = attr["value"].get<std::string>();
                break;
            }
        }

        if (mapKey.empty() || !isUUID(mapKey)) {
            continue; // skip entries without valid MapKey
        }

        doc["attributes"] = attributes;

        m_objectManager.insert(mapKey, doc);
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

        if (attr["id"].get<std::string>() == "ValueUUID") {
            mapKey = attr["value"].get<std::string>();
            break;
        }
    }

    if (isUUID(mapKey) && node->name == "GameObjects") {
        doc["attributes"] = attributes;

        m_objectManager.insert(mapKey, doc);
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
