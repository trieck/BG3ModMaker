#include "pch.h"
#include "Exception.h"
#include "ObjectManager.h"

#include <rocksdb/options.h>

static constexpr auto COMMIT_SIZE = 10000;

ObjectManager::ObjectManager()
{
}

void ObjectManager::open(const char* dbName)
{
    rocksdb::Options opts{};
    opts.create_if_missing = true;
    opts.create_missing_column_families = true;
    opts.compression = rocksdb::kZSTD;

    std::vector<rocksdb::ColumnFamilyDescriptor> cfDescs;
    auto addCF = [&](const std::string& name) {
        cfDescs.emplace_back(name, rocksdb::ColumnFamilyOptions());
    };

    addCF(rocksdb::kDefaultColumnFamilyName); // default CF
    addCF("hierarchy"); // hierarchy CF

    std::vector<rocksdb::ColumnFamilyHandle*> handles;
    rocksdb::DB* rawdb = nullptr;
    auto st = rocksdb::DB::Open(opts, dbName, cfDescs, &handles, &rawdb);
    if (!st.ok()) {
        throw Exception(st.ToString());
    }

    m_db.reset(rawdb);
    m_batch.Clear();
    m_cfDefault.reset(handles[0]);
    m_cfHierarchy.reset(handles[1]);
}

void ObjectManager::openReadOnly(const char* dbName)
{
    rocksdb::Options opts{};
    opts.create_if_missing = false;

    std::vector<rocksdb::ColumnFamilyDescriptor> cfDescs;
    auto addCF = [&](const std::string& name) {
        cfDescs.emplace_back(name, rocksdb::ColumnFamilyOptions());
    };
    addCF(rocksdb::kDefaultColumnFamilyName); // default CF
    addCF("hierarchy"); // hierarchy CF

    std::vector<rocksdb::ColumnFamilyHandle*> handles;
    rocksdb::DB* rawdb = nullptr;
    auto st = rocksdb::DB::OpenForReadOnly(opts, dbName, cfDescs, &handles, &rawdb);
    if (!st.ok()) {
        throw Exception(st.ToString());
    }

    m_db.reset(rawdb);
    m_batch.Clear();
    m_cfDefault.reset(handles[0]);
    m_cfHierarchy.reset(handles[1]);
}

bool ObjectManager::isOpen() const
{
    return m_db != nullptr;
}

ObjectManager::~ObjectManager()
{
    close();
}

void ObjectManager::close()
{
    m_cfHierarchy.reset();
    m_cfDefault.reset();
    m_db.reset();
}

void ObjectManager::flush()
{
    if (m_db == nullptr) {
        return;
    }

    flushBatch();

    auto status = m_db->Flush(rocksdb::FlushOptions());
    if (!status.ok()) {
        throw Exception("Failed to flush RocksDB database: " + status.ToString());
    }
}

rocksdb::DB* ObjectManager::getDB() const
{
    return m_db.get();
}

void ObjectManager::flushBatch()
{
    if (m_db == nullptr) {
        return;
    }

    rocksdb::WriteOptions writeOpts{};
    auto st = m_db->Write(writeOpts, &m_batch);
    if (!st.ok()) {
        throw Exception("Failed to write batch to RocksDB database: " + st.ToString());
    }

    m_batch.Clear();
}

bool ObjectManager::insert(const std::string& key, const nlohmann::json& doc)
{
    return insert(key.c_str(), doc);
}

nlohmann::json ObjectManager::get(const std::string& key)
{
    if (m_db == nullptr) {
        throw Exception("Database is not open");
    }

    std::string jsonStr;
    rocksdb::ReadOptions readOpts{};
    auto st = m_db->Get(readOpts, m_cfDefault.get(), key, &jsonStr);
    if (!st.ok()) {
        if (st.IsNotFound()) {
            return {};
        }
        throw Exception("Failed to get object from RocksDB database: " + st.ToString());
    }

    return nlohmann::json::parse(jsonStr);
}

bool ObjectManager::insert(const char* key, const nlohmann::json& doc)
{
    std::string jsonStr = doc.dump();

    auto st = m_batch.Put(m_cfDefault.get(), key, jsonStr);
    if (!st.ok()) {
        return false;
    }

    auto parent = findParent(doc);
    if (!parent.empty()) {
        addRelation(key, parent);
    } else {
        addRoot(key);
    }

    addType(doc, key, parent);

    auto count = m_batch.Count();
    if (count > 0 && count % COMMIT_SIZE == 0) {
        m_db->Write(rocksdb::WriteOptions(), &m_batch);
        m_batch.Clear();
    }

    return true;
}

std::string ObjectManager::getParent(const std::string& child) const
{
    std::string parent;

    if (m_db == nullptr || child.empty()) {
        return parent;
    }

    rocksdb::ReadOptions readOpts{};
    m_db->Get(readOpts, m_cfHierarchy.get(), "parent:" + child, &parent);

    return parent;
}

PrefixIterator::Ptr ObjectManager::getChildren(const std::string& parent) const
{
    if (m_db == nullptr || parent.empty()) {
        return nullptr;
    }

    std::string prefix = "child:" + parent + ":";

    return PrefixIterator::create(m_db.get(), m_cfHierarchy.get(), prefix.c_str());
}

PrefixIterator::Ptr ObjectManager::getRoots() const
{
    if (m_db == nullptr) {
        return nullptr;
    }

    return PrefixIterator::create(m_db.get(), m_cfHierarchy.get(), "root:");
}

PrefixIterator::Ptr ObjectManager::getRoots(const char* type) const
{
    if (m_db == nullptr) {
        return nullptr;
    }

    if (type == nullptr || *type == '\0') {
        return getRoots();
    }

    std::string prefix = "type_root:" + std::string(type) + ":";

    return PrefixIterator::create(m_db.get(), m_cfHierarchy.get(), prefix.c_str());
}

PrefixIterator::Ptr ObjectManager::getTypes() const
{
    if (m_db == nullptr) {
        return nullptr;
    }
    return PrefixIterator::create(m_db.get(), m_cfHierarchy.get(), "types:");
}

void ObjectManager::addRelation(const std::string& child, const std::string& parent)
{
    if (m_db == nullptr || child.empty() || parent.empty()) {
        return;
    }

    // child -> parent (getParent)
    m_batch.Put(m_cfHierarchy.get(), "parent:" + child, parent);

    // parent -> child (getChildren)
    m_batch.Put(m_cfHierarchy.get(), "child:" + parent + ":" + child, child);
}

void ObjectManager::addRoot(const std::string& key)
{
    if (m_db == nullptr || key.empty()) {
        return;
    }

    // root objects (getRoots)
    m_batch.Put(m_cfHierarchy.get(), "root:" + key, key);
}

void ObjectManager::addType(const nlohmann::json& doc, const std::string& key, const std::string& parent)
{
    if (m_db == nullptr || key.empty()) {
        return;
    }

    std::string type = findType(doc);
    if (!type.empty()) {
        m_batch.Put(m_cfHierarchy.get(), "types:" + type, type);
        m_batch.Put(m_cfHierarchy.get(), "type:" + type + ":" + key, key);

        if (parent.empty()) { // root
            m_batch.Put(m_cfHierarchy.get(), "type_root:" + type + ":" + key, key);
        }
    }
}

std::string ObjectManager::findType(const nlohmann::json& doc)
{
    ASSERT(doc.is_object());
    ASSERT(doc.contains("attributes"));
    ASSERT(doc["attributes"].is_array());

    static const std::unordered_set<std::string> typeKeys = {
        "Type"
    };

    for (const auto& attr : doc["attributes"]) {
        auto id = attr.value("id", "");
        auto val = attr.value("value", "");

        if (val.empty()) {
            continue;
        }

        if (typeKeys.contains(id)) {
            return val;
        }
    }

    return "";
}


std::string ObjectManager::findParent(const nlohmann::json& doc)
{
    ASSERT(doc.is_object());
    ASSERT(doc.contains("attributes"));
    ASSERT(doc["attributes"].is_array());

    static const std::unordered_set<std::string> parentKeys = {
        "ParentTemplateId", "TemplateName", "RootTemplate"
    };

    for (const auto& attr : doc["attributes"]) {
        auto id = attr.value("id", "");
        auto val = attr.value("value", "");

        if (val.empty()) {
            continue;
        }

        if (parentKeys.contains(id)) {
            return val;
        }
    }

    return "";
}
