#pragma once

template <typename T>
concept BTreeKey = requires(T a, T b)
{
    std::movable<T>;
    std::default_initializable<T>;
    { a < b } -> std::convertible_to<bool>;
};

template <typename T>
concept BTreeValue = requires
{
    std::movable<T>;
    std::default_initializable<T>;
};

template <BTreeKey K, BTreeValue V>
struct BTreePage;

template <BTreeKey K, BTreeValue V>
struct BTreeNode
{
    K key;
    V value;
    std::unique_ptr<BTreePage<K, V>> next;
};

template <BTreeKey K, BTreeValue V>
struct BTreePage
{
    enum PageType
    {
        Internal,
        Leaf
    };

    using Ptr = std::unique_ptr<BTreePage>;

    bool isLeaf() const;
    bool isInternal() const;

    PageType type = Leaf;
    std::vector<BTreeNode<K, V>> nodes;
};

template <BTreeKey K, BTreeValue V>
class BTree
{
public:
    explicit BTree(uint32_t maxKeys);
    ~BTree();

    class iterator
    {
    public:
        iterator() = default;

        iterator(BTreePage<K, V>* p, size_t i)
            : page(p), index(i)
        {
        }

        const K& key() const
        {
            return page->nodes[index].key;
        }

        V& value() const
        {
            return page->nodes[index].value;
        }

        bool operator==(const iterator& other) const
        {
            return page == other.page && index == other.index;
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        BTreePage<K, V>* page = nullptr;
        size_t index = 0;
    };

    void insert(const K& key, const V& value);
    iterator find(const K& key) const;
    iterator begin() const;
    iterator end() const;

private:
    using Page = BTreePage<K, V>;
    using PPage = Page*;

    using Node = BTreeNode<K, V>;
    using PNode = Node*;

    Page::Ptr insertR(PPage page, const K& key, const V& value);
    iterator findR(PPage page, const K& key) const;
    Page::Ptr split(PPage page);

    Page::Ptr m_pRoot = nullptr;
    uint32_t m_maxKeys; // Maximum number of keys per page
};

template <BTreeKey K, BTreeValue V>
bool BTreePage<K, V>::isLeaf() const
{
    return type == Leaf;
}

template <BTreeKey K, BTreeValue V>
bool BTreePage<K, V>::isInternal() const
{
    return type == Internal;
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::BTree(uint32_t maxKeys) : m_maxKeys(maxKeys)
{
    m_pRoot = std::make_unique<Page>();
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::~BTree()
{
}

template <BTreeKey K, BTreeValue V>
void BTree<K, V>::insert(const K& key, const V& value)
{
    auto u = insertR(m_pRoot.get(), key, value);
    if (u == nullptr) {
        return; // no need to split root
    }

    // split root

    // take ownership of the old root
    auto oldRoot = std::move(m_pRoot);

    // create new root
    auto newRoot = std::make_unique<Page>();
    newRoot->type = Page::Internal;

    // left child = old root
    Node a;
    a.key = oldRoot->nodes[0].key;
    a.next = std::move(oldRoot);

    // right child = split result
    Node b;
    b.key = u->nodes[0].key;
    b.next = std::move(u);

    // assemble new root
    newRoot->nodes.emplace_back(std::move(a));
    newRoot->nodes.emplace_back(std::move(b));

    // install new root
    m_pRoot = std::move(newRoot);
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::iterator BTree<K, V>::end() const
{
    return iterator{};
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::iterator BTree<K, V>::find(const K& key) const
{
    return findR(m_pRoot.get(), key);
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::iterator BTree<K, V>::begin() const
{
    auto p = m_pRoot.get();

    while (p && p->isInternal()) {
        p = p->nodes.front().next.get();
    }

    return p && !p->nodes.empty() ? iterator(p, 0) : end();
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::Page::Ptr BTree<K, V>::insertR(PPage page, const K& key, const V& value)
{
    uint32_t j = 0;
    Node t{.key = key};

    if (page->isLeaf()) { // leaf page
        for (; j < page->nodes.size(); ++j) {
            if (key == page->nodes[j].key) {
                page->nodes[j].value = value;
                return nullptr; // no insert, no split
            }
            if (key < page->nodes[j].key) {
                break;
            }
        }
        t.value = value;
    } else { // internal page
        for (; j < page->nodes.size(); ++j) {
            if (j + 1 == page->nodes.size() || key < page->nodes[j + 1].key) {
                // find the next subpage to descend into
                auto* next = page->nodes[j++].next.get();
                auto u = insertR(next, key, value);
                if (u == nullptr) {
                    return nullptr;
                }

                t.next = std::move(u);
                break;
            }
        }
    }

    page->nodes.insert(page->nodes.begin() + j, std::move(t));

    if (page->nodes.size() <= m_maxKeys) {
        return nullptr;
    }

    return split(page);
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::iterator BTree<K, V>::findR(PPage page, const K& key) const
{
    uint32_t j = 0;

    if (page->isLeaf()) { // leaf page
        for (; j < page->nodes.size(); ++j) {
            if (key == page->nodes[j].key) {
                return iterator(page, j);
            }
            if (key < page->nodes[j].key) {
                break;
            }
        }

        return end();
    }

    // internal page
    for (; j < page->nodes.size(); ++j) {
        if (j + 1 == page->nodes.size() || key < page->nodes[j + 1].key) {
            // find the next subpage to descend into
            auto* next = page->nodes[j].next.get();
            return findR(next, key);
        }
    }

    return end();
}

template <BTreeKey K, BTreeValue V>
BTree<K, V>::Page::Ptr BTree<K, V>::split(PPage page)
{
    assert(page->nodes.size() == m_maxKeys + 1);

    auto newPage = std::make_unique<Page>();
    newPage->type = page->type;
    auto midPoint = page->nodes.size() / 2;

    newPage->nodes.assign(
        std::make_move_iterator(page->nodes.begin() + midPoint),
        std::make_move_iterator(page->nodes.end())
    );

    page->nodes.erase(page->nodes.begin() + midPoint, page->nodes.end());

    return newPage;
}
