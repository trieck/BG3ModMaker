#pragma once

#include "PrefixIterator.h"

#include <nlohmann/json.hpp>

class ObjectManager
{
public:
    ObjectManager();
    ~ObjectManager();

    bool insert(const char* key, const nlohmann::json& doc);
    bool insert(const std::string& key, const nlohmann::json& doc);
    bool isOpen() const;
    nlohmann::json get(const std::string& key);
    rocksdb::DB* getDB() const;
    std::string getParent(const std::string& child) const;
    PrefixIterator::Ptr getChildren(const std::string& parent) const;
    PrefixIterator::Ptr getRoots() const;
    PrefixIterator::Ptr getRoots(const char* type) const;
    PrefixIterator::Ptr getTypes() const;
    void close();
    void flush();
    void open(const char* dbName);
    void openReadOnly(const char* dbName);

private:
    std::string findParent(const nlohmann::json& doc);
    std::string findType(const nlohmann::json& doc);
    void addRelation(const std::string& child, const std::string& parent);
    void addRoot(const std::string& key);
    void addType(const nlohmann::json& doc, const std::string& key, const std::string& parent);
    void flushBatch();

    rocksdb::WriteBatch m_batch;
    std::unique_ptr<rocksdb::DB> m_db;
    std::unique_ptr<rocksdb::ColumnFamilyHandle> m_cfDefault;
    std::unique_ptr<rocksdb::ColumnFamilyHandle> m_cfHierarchy;
};

