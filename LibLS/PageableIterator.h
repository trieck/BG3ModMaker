#pragma once
#include <rocksdb/db.h>

class PageableIterator
{
    PageableIterator(rocksdb::DB* db, size_t pageSize);

public:
    using Ptr = std::unique_ptr<PageableIterator>;

    static Ptr create(rocksdb::DB* db, size_t pageSize);

    bool first();
    bool last();
    bool next();
    bool prev();

    std::vector<std::string> keys() const;

    size_t currentPage() const;
    size_t totalPages() const;
    size_t pageSize() const;
    size_t totalEntries() const;

private:
    enum Direction
    {
        Forward,
        Backward
    };

    void buildKeys(Direction dir);
    std::vector<std::string> m_currentKeys;

    rocksdb::DB* m_db;
    std::unique_ptr<rocksdb::Iterator> m_it;
    std::string m_firstKey, m_lastKey;
    size_t m_pageSize = 0;
    size_t m_currentPage = 1;
    size_t m_totalPages = 0;
    size_t m_totalEntries = 0;
};
