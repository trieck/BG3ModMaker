#include "pch.h"
#include "PageableIterator.h"

PageableIterator::PageableIterator(rocksdb::DB* db, size_t pageSize) : m_db(db), m_pageSize(pageSize)
{
    if (m_db == nullptr) {
        throw std::invalid_argument("Database pointer cannot be null");
    }

    if (m_pageSize == 0) {
        throw std::invalid_argument("Page size must be greater than zero");
    }

    m_it = std::unique_ptr<rocksdb::Iterator>(m_db->NewIterator(rocksdb::ReadOptions()));

    uint64_t count = 0;
    if (m_db->GetIntProperty("rocksdb.estimate-num-keys", &count)) {
        m_totalEntries = static_cast<size_t>(count);
        m_totalPages = (m_totalEntries + m_pageSize - 1) / m_pageSize;
    }

    first();
}

PageableIterator::Ptr PageableIterator::create(rocksdb::DB* db, size_t pageSize)
{
    return std::unique_ptr<PageableIterator>(new PageableIterator(db, pageSize));
}

bool PageableIterator::first()
{
    if (m_it == nullptr) {
        return false;
    }

    m_it->SeekToFirst();
    m_currentPage = 1;

    buildKeys(Forward);

    return !m_currentKeys.empty();
}

bool PageableIterator::last()
{
    if (m_it == nullptr) {
        return false;
    }

    m_it->SeekToLast();

    buildKeys(Backward);

    if (m_currentKeys.empty()) {
        return false;
    }

    if (m_totalPages > 0) {
        m_currentPage = m_totalPages;
    } else {
        m_currentPage = (m_totalEntries + m_pageSize - 1) / m_pageSize;
    }

    return true;
}

bool PageableIterator::next()
{
    if (m_it == nullptr || m_lastKey.empty()) {
        return false;
    }

    m_it->Seek(m_lastKey);
    if (!m_it->Valid()) {
        return false;
    }

    m_it->Next(); // Move to the next entry after the last key of the current page
    if (!m_it->Valid()) {
        return false;
    }

    buildKeys(Forward);

    if (m_currentKeys.empty()) {
        return false; // no more pages
    }

    m_currentPage = std::min(m_totalPages, m_currentPage + 1);

    return true;
}

bool PageableIterator::prev()
{
    if (m_it == nullptr || m_firstKey.empty()) {
        return false;
    }

    m_it->Seek(m_firstKey);

    if (!m_it->Valid()) {
        return false;
    }

    m_it->Prev(); // Move to the previous entry before the first key of the current page

    if (!m_it->Valid()) {
        return false;
    }

    buildKeys(Backward);

    m_currentPage = std::max<int>(1, static_cast<int>(m_currentPage) - 1);

    return true;
}

std::vector<std::string> PageableIterator::keys() const
{
    return m_currentKeys;
}

size_t PageableIterator::currentPage() const
{
    return m_currentPage;
}

size_t PageableIterator::totalPages() const
{
    return m_totalPages;
}

size_t PageableIterator::pageSize() const
{
    return m_pageSize;
}

size_t PageableIterator::totalEntries() const
{
    return m_totalEntries;
}

void PageableIterator::buildKeys(Direction dir)
{
    m_currentKeys.clear();
    if (m_it == nullptr || !m_it->Valid()) {
        return;
    }

    m_firstKey.clear();
    m_lastKey.clear();

    size_t count = 0;
    while (m_it->Valid() && count < m_pageSize) {
        m_currentKeys.push_back(m_it->key().ToString());

        if (dir == Forward) {
            m_it->Next();
        } else {
            m_it->Prev();
        }

        ++count;
    }

    if (dir == Backward) {
        std::ranges::reverse(m_currentKeys);
    }

    if (!m_currentKeys.empty()) {
        m_firstKey = m_currentKeys.front();
        m_lastKey = m_currentKeys.back();
    }
}
