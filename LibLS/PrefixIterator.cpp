#include "pch.h"
#include "PrefixIterator.h"

PrefixIterator::PrefixIterator(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* cf, const char* prefix)
{
    if (db == nullptr) {
        throw std::invalid_argument("Database pointer cannot be null");
    }

    m_db = db;
    m_prefix = prefix ? prefix : "";

    rocksdb::ReadOptions ro;
    if (!m_prefix.empty()) {
        m_lowerBoundSlice = rocksdb::Slice(m_prefix);
        // increment last char to form the exclusive upper bound
        m_upperBoundStr = m_prefix;
        m_upperBoundStr.back()++;
        m_upperBoundSlice = rocksdb::Slice(m_upperBoundStr);
        ro.iterate_lower_bound = &m_lowerBoundSlice;
        ro.iterate_upper_bound = &m_upperBoundSlice;
    }

    m_it = std::unique_ptr<rocksdb::Iterator>(m_db->NewIterator(ro, cf));
    if (!m_prefix.empty()) {
        m_it->Seek(m_prefix);
    } else {
        m_it->SeekToFirst();
    }
}

PrefixIterator::Ptr PrefixIterator::create(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* cf, const char* prefix)
{
    return std::unique_ptr<PrefixIterator>(new PrefixIterator(db, cf, prefix));
}

bool PrefixIterator::isValid() const
{
    return m_it != nullptr && m_it->Valid();
}

void PrefixIterator::next()
{
    if (m_it != nullptr) {
        m_it->Next();
    }
}

std::string PrefixIterator::key() const
{
    if (m_it != nullptr && m_it->Valid()) {
        return m_it->key().ToString();
    }

    return "";
}

std::string PrefixIterator::value() const
{
    if (m_it != nullptr && m_it->Valid()) {
        return m_it->value().ToString();
    }

    return "";
}
