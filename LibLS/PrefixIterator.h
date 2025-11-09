#pragma once
#include <rocksdb/db.h>

class PrefixIterator
{
    PrefixIterator(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* cf, const char* prefix);

public:
    using Ptr = std::unique_ptr<PrefixIterator>;

    static Ptr create(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* cf, const char* prefix);

    bool isValid() const;
    void next();
    std::string key() const;
    std::string value() const;

private:
    rocksdb::DB* m_db;
    std::unique_ptr<rocksdb::Iterator> m_it;
    std::string m_firstKey, m_lastKey;
    std::string m_prefix;
    std::string m_upperBoundStr;
    rocksdb::Slice m_lowerBoundSlice;
    rocksdb::Slice m_upperBoundSlice;
};
