#pragma once

// Osiris objects (nodes, databases, rules, etc.) are identified by 1-based
// numeric indices defined by the file format. Index 0 is reserved by Osiris
// to mean "no reference".
//
// OsiTable provides a dense, index-based storage abstraction over std::vector
// that preserves these semantics directly:
//   - indices are 1-based
//   - index 0 is invalid / NO_INDEX
//   - lookup by index is O(1)
//   - iteration never exposes index 0
//
// The intent is to encode Osiris invariants once, centrally, and make them
// impossible to forget or misuse throughout the codebase.

template <typename T>
class OsiTable
{
public:
    using iterator = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;

    iterator begin() noexcept
    {
        return m_items.begin() + 1;
    }

    iterator end() noexcept
    {
        return m_items.end();
    }

    const_iterator begin() const noexcept
    {
        return m_items.begin() + 1;
    }

    const_iterator end() const noexcept
    {
        return m_items.end();
    }

    const_iterator cbegin() const noexcept
    {
        return m_items.cbegin() + 1;
    }

    const_iterator cend() const noexcept
    {
        return m_items.cend();
    }

    using Index = uint32_t;
    static constexpr Index NO_INDEX = 0;

    T& operator[](Index idx)
    {
        if (idx == NO_INDEX) {
            throw std::out_of_range("OsiTable: index 0 (NO_REF) is invalid");
        }

        if (idx >= m_items.size()) {
            throw std::out_of_range("OsiTable: index out of range");
        }

        return m_items[idx];
    }

    const T& operator[](Index idx) const
    {
        if (idx == NO_INDEX) {
            throw std::out_of_range("OsiTable: index 0 (NO_REF) is invalid");
        }

        if (idx >= m_items.size()) {
            throw std::out_of_range("OsiTable: index out of range");
        }

        return m_items[idx];
    }

    void set(Index idx, std::unique_ptr<T> value)
    {
        assert(idx != NO_INDEX);
        assert(idx < m_items.size());
        m_items[idx] = std::move(value);
    }

    void resize(Index maxIndex)
    {
        m_items.resize(maxIndex + 1);
    }

    void reserve(Index maxIndex)
    {
        m_items.reserve(maxIndex + 1);
    }

    void clear() noexcept
    {
        m_items.clear();
    }

    bool empty() const noexcept
    {
        return m_items.size() <= 1;
    }

    std::size_t size() const noexcept
    {
        return m_items.size() > 0 ? m_items.size() - 1 : 0;
    }

private:
    std::vector<T> m_items;
};
