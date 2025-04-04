#pragma once

#include <optional>

enum class NodeColor
{
    RED,
    BLACK
};

template <typename T>
concept RBKey = requires(T a, T b)
{
    std::movable<T>;
    std::default_initializable<T>;
    { a < b } -> std::convertible_to<bool>;
};

template <typename T>
concept RBValue = requires
{
    std::movable<T>;
    std::default_initializable<T>;
};

template <RBKey K, RBValue V>
struct RBNode
{
    RBNode();
    explicit RBNode(K k, V v);

    using Ptr = RBNode*;

    K key{};
    V value{};

    Ptr left, right;
    Ptr parent;
    NodeColor color;
};

template <RBKey K, RBValue V>
RBNode<K, V>::RBNode()
{
    left = nullptr;
    right = nullptr;
    parent = nullptr;
    color = NodeColor::RED;
}

template <RBKey K, RBValue V>
RBNode<K, V>::RBNode(K k, V v) : key(std::move(k)), value(std::move(v)), left(nullptr), right(nullptr), parent(nullptr),
                                 color(NodeColor::RED)
{
}

template <RBKey K, RBValue V>
class RBTree
{
public:
    using NodeType = RBNode<K, V>;
    using PNode = typename NodeType::Ptr;
    using KeyType = K;
    using ValueType = V;

    RBTree();
    ~RBTree();
    bool exists(const K& key) const;
    bool find(const K& key, V& value) const;
    bool isEmpty() const;
    PNode root() const;
    std::optional<V> find(const K& key) const;
    uint32_t size() const;
    void insert(K key, V value);
    void remove(const K& key);
    void removeAll();
    void traverse(std::function<void(const K& key, const V& value)> callback) const;

protected:
    void setRoot(PNode node);
    void fixInsert(PNode& node);
    void fixDelete(PNode& node);
    void rotateLeft(PNode node);
    void rotateRight(PNode node);

private:
    void swapColor(PNode left, PNode right);
    void transplant(PNode u, PNode v);
    PNode minValueNode(PNode node);
    void deleteTree(PNode node);

    PNode m_root = nullptr;
    uint32_t m_size = 0;
};

template <RBKey K, RBValue V>
RBTree<K, V>::RBTree() : m_root(nullptr)
{
}

template <RBKey K, RBValue V>
RBTree<K, V>::~RBTree()
{
    removeAll();
}

template <RBKey K, RBValue V>
void RBTree<K, V>::insert(K key, V value)
{
    auto node = new RBNode<K, V>(std::move(key), std::move(value));

    PNode parent = nullptr;

    auto current = m_root;

    while (current != nullptr) {
        parent = current;
        if (node->key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    node->parent = parent;
    if (parent == nullptr) {
        m_root = node;
    } else if (node->key < parent->key) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    fixInsert(node);

    m_size++;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::remove(const K& key)
{
    auto node = m_root;
    PNode x = nullptr;
    PNode y = nullptr;
    PNode z = nullptr;

    while (node != nullptr) {
        if (node->key == key) {
            z = node;
            break;
        }

        if (node->key <= key) {
            node = node->right;
        } else {
            node = node->left;
        }
    }

    if (z == nullptr) {
        return; // not found
    }

    y = z;

    auto yOriginalColor = y->color;

    if (z->left == nullptr) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nullptr) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minValueNode(z->right);
        yOriginalColor = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x != nullptr) {
                x->parent = y;
            }
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    delete z;

    if (x != nullptr && yOriginalColor == NodeColor::BLACK) {
        fixDelete(x);
    }

    m_size--;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::removeAll()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

template <RBKey K, RBValue V>
bool RBTree<K, V>::find(const K& key, V& value) const
{
    auto current = m_root;

    while (current != nullptr) {
        if (key < current->key) {
            current = current->left;
        } else if (current->key < key) {
            current = current->right;
        } else {
            value = current->value;
            return true;
        }
    }

    return false;
}

template <RBKey K, RBValue V>
std::optional<V> RBTree<K, V>::find(const K& key) const
{
    V value;

    if (find(key, value)) {
        return value;
    }

    return std::nullopt;
}

template <RBKey K, RBValue V>
uint32_t RBTree<K, V>::size() const
{
    return m_size;
}

template <RBKey K, RBValue V>
bool RBTree<K, V>::exists(const K& key) const
{
    auto current = m_root;

    while (current != nullptr) {
        if (key < current->key) {
            current = current->left;
        } else if (current->key < key) {
            current = current->right;
        } else {
            return true;
        }
    }

    return false;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::traverse(std::function<void(const K& key, const V& value)> callback) const
{
    std::function<void(PNode)> inOrder = [&](PNode node)
    {
        if (node == nullptr) {
            return;
        }
        inOrder(node->left);
        callback(node->key, node->value);
        inOrder(node->right);
    };

    inOrder(m_root);
}

template <RBKey K, RBValue V>
bool RBTree<K, V>::isEmpty() const
{
    return m_root == nullptr;
}

template <RBKey K, RBValue V>
typename RBTree<K, V>::PNode RBTree<K, V>::root() const
{
    return m_root;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::setRoot(PNode node)
{
    node->color = NodeColor::BLACK;
    m_root = node;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::fixInsert(PNode& node)
{
    PNode parent = nullptr;
    PNode grandparent = nullptr;

    while (node != m_root && node->color == NodeColor::RED) {
        if (node->parent == nullptr || node->parent->color == NodeColor::BLACK) {
            break;
        }

        parent = node->parent;
        grandparent = parent->parent;

        if (parent == grandparent->left) {
            auto uncle = grandparent->right;
            if (uncle != nullptr && uncle->color == NodeColor::RED) {
                grandparent->color = NodeColor::RED;
                parent->color = NodeColor::BLACK;
                uncle->color = NodeColor::BLACK;
                node = grandparent;
            } else {
                if (node == parent->right) {
                    rotateLeft(parent);
                    node = parent;
                    parent = node->parent;
                }
                rotateRight(grandparent);
                swapColor(parent, grandparent);
                node = parent;
            }
        } else {
            auto uncle = grandparent->left;
            if (uncle != nullptr && uncle->color == NodeColor::RED) {
                grandparent->color = NodeColor::RED;
                parent->color = NodeColor::BLACK;
                uncle->color = NodeColor::BLACK;
                node = grandparent;
            } else {
                if (node == parent->left) {
                    rotateRight(parent);
                    node = parent;
                    parent = node->parent;
                }
                rotateLeft(grandparent);
                swapColor(parent, grandparent);
                node = parent;
            }
        }
    }

    m_root->color = NodeColor::BLACK;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::fixDelete(PNode& node)
{
    while (node != m_root && node->color == NodeColor::BLACK) {
        if (node == node->parent->left) {
            auto sibling = node->parent->right;
            if (sibling->color == NodeColor::RED) {
                sibling->color = NodeColor::BLACK;
                node->parent->color = NodeColor::RED;
                rotateLeft(node->parent);
                sibling = node->parent->right;
            }
            if (sibling->left == nullptr || sibling->left->color == NodeColor::BLACK &&
                (sibling->right == nullptr || sibling->right->color == NodeColor::BLACK)) {
                sibling->color = NodeColor::RED;
                node = node->parent;
            } else {
                if (sibling->right == nullptr || sibling->right->color == NodeColor::BLACK) {
                    if (sibling->left != nullptr) {
                        sibling->left->color = NodeColor::BLACK;
                    }
                    sibling->color = NodeColor::RED;
                    rotateRight(sibling);
                    sibling = node->parent->right;
                }
                sibling->color = node->parent->color;
                node->parent->color = NodeColor::BLACK;
                if (sibling->right != nullptr) {
                    sibling->right->color = NodeColor::BLACK;
                }
                rotateLeft(node->parent);
                node = m_root;
            }
        } else {
            auto sibling = node->parent->left;
            if (sibling->color == NodeColor::RED) {
                sibling->color = NodeColor::BLACK;
                node->parent->color = NodeColor::RED;
                rotateRight(node->parent);
                sibling = node->parent->left;
            }
            if ((sibling->left == nullptr || sibling->left->color == NodeColor::BLACK) && (sibling->right == nullptr ||
                sibling->right->color == NodeColor::BLACK)) {
                sibling->color = NodeColor::RED;
                rotateLeft(sibling);
                sibling = node->parent->left;
            }
            sibling->color = node->parent->color;
            node->parent->color = NodeColor::BLACK;
            if (sibling->left != nullptr) {
                sibling->left->color = NodeColor::BLACK;
            }
            rotateRight(node->parent);
            node = m_root;
        }
    }
    node->color = NodeColor::BLACK;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::rotateLeft(PNode node)
{
    auto child = node->right;
    node->right = node->left;

    if (node->right != nullptr) {
        node->right->parent = node;
    }

    child->parent = node->parent;
    if (node->parent == nullptr) {
        m_root = child;
    } else if (node == node->parent->left) {
        node->parent->left = child;
    } else {
        node->parent->right = child;
    }

    child->left = node;
    node->parent = child;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::rotateRight(PNode node)
{
    auto child = node->left;
    node->left = child->right;

    if (node->left != nullptr) {
        node->left->parent = node;
    }
    child->parent = node->parent;

    if (node->parent == nullptr) {
        m_root = child;
    } else if (node == node->parent->left) {
        node->parent->left = child;
    } else {
        node->parent->right = child;
    }

    child->right = node;

    node->parent = child;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::swapColor(PNode left, PNode right)
{
    auto temp = left->color;
    left->color = right->color;
    right->color = temp;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::transplant(PNode u, PNode v)
{
    if (u->parent == nullptr) {
        m_root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }

    if (v != nullptr) {
        v->parent = u->parent;
    }
}

template <RBKey K, RBValue V>
typename RBTree<K, V>::PNode RBTree<K, V>::minValueNode(PNode node)
{
    auto current = node;
    while (current->left != nullptr) {
        current = current->left;
    }

    return current;
}

template <RBKey K, RBValue V>
void RBTree<K, V>::deleteTree(PNode node)
{
    if (node == nullptr) {
        return;
    }

    deleteTree(node->left);
    deleteTree(node->right);

    delete node;
}
