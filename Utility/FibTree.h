#pragma once

/*
 * FibTree<K, V> - A Binary Search Tree with Fibonacci-Based Balancing
 *
 * This is a self-balancing binary search tree that maintains balance using
 * Fibonacci-based constraints rather than traditional rotations (as seen in
 * AVL or Red-Black Trees). The goal is to achieve logarithmic search and
 * insertion performance while minimizing disruptive restructuring.
 *
 * Key Features:
 * - Maintains approximate Fibonacci ratios between left and right subtrees.
 * - Uses weight-based balancing instead of rotations.
 * - Ensures a well-proportioned tree structure without enforcing strict balance.
 * - Less frequent and less costly rebalancing compared to AVL or Red-Black Trees.
 *
 * The Fibonacci balancing ensures that:
 * - Each node's left and right subtree sizes approximate Fibonacci numbers.
 * - If a subtree grows too large relative to its sibling, it is split.
 * - If a subtree becomes too small, it merges with its sibling.
 *
 * This balancing method provides O(log n) search and insert operations while
 * maintaining a natural structural efficiency without the complexity of
 * frequent rotations.
 */

#include "Utility.h"

template <typename T>
concept FibKey = requires(T a, T b)
{
    std::movable<T>;
    std::default_initializable<T>;
    { a < b } -> std::convertible_to<bool>;
};

template <typename T>
concept FibValue = requires
{
    std::movable<T>;
    std::default_initializable<T>;
};

template <FibKey K, FibValue V>
struct FibNode
{
    FibNode();
    explicit FibNode(K k, size_t s);
    explicit FibNode(K k, V v);

    using Ptr = FibNode*;

    K key{};
    V value{};

    Ptr left, right;
    Ptr parent;
    size_t size = 1;
};

template <FibKey K, FibValue V>
FibNode<K, V>::FibNode() : left(nullptr), right(nullptr), parent(nullptr)
{
}

template <FibKey K, FibValue V>
FibNode<K, V>::FibNode(K k, size_t s) : key(std::move(k)), left(nullptr), right(nullptr), parent(nullptr), size(s)
{
}

template <FibKey K, FibValue V>
FibNode<K, V>::FibNode(K k, V v) : key(std::move(k)), value(std::move(v)), left(nullptr), right(nullptr),
                                   parent(nullptr)
{
}

template <FibKey K, FibValue V>
class FibTree
{
public:
    using NodeType = FibNode<K, V>;
    using PNode = typename NodeType::Ptr;

    FibTree();
    virtual ~FibTree();

    bool exists(const K& key) const;
    bool find(const K& key, V& value) const;
    bool isEmpty() const;
    PNode root() const;
    void insert(K key, V value);
    void remove(const K& key);
    void removeAll();
    void traverse(std::function<void(const K& key, const V& value)> callback) const;

protected:
    virtual bool isBalanced(PNode node) const;
    PNode minValueNode(PNode node);
    virtual void rebalance(PNode node);
    void deleteTree(PNode node);
    void merge(PNode node);
    void setRoot(PNode node);
    void transplant(PNode u, PNode v);

private:
    PNode m_root = nullptr;
};

template <FibKey K, FibValue V>
FibTree<K, V>::FibTree()
{
}

template <FibKey K, FibValue V>
FibTree<K, V>::~FibTree()
{
    removeAll();
}

template <FibKey K, FibValue V>
void FibTree<K, V>::insert(K key, V value)
{
    auto node = new FibNode<K, V>(std::move(key), std::move(value));

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

    rebalance(node);
}

template <FibKey K, FibValue V>
void FibTree<K, V>::remove(const K& key)
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

    if (z->left == nullptr) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nullptr) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minValueNode(z->right);
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
    }

    delete z;

    if (x != nullptr) {
        rebalance(x);
    }
}

template <FibKey K, FibValue V>
void FibTree<K, V>::removeAll()
{
    deleteTree(m_root);
    m_root = nullptr;
}

template <FibKey K, FibValue V>
bool FibTree<K, V>::find(const K& key, V& value) const
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

template <FibKey K, FibValue V>
bool FibTree<K, V>::exists(const K& key) const
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

template <FibKey K, FibValue V>
void FibTree<K, V>::traverse(std::function<void(const K& key, const V& value)> callback) const
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

template <FibKey K, FibValue V>
bool FibTree<K, V>::isEmpty() const
{
    return m_root == nullptr;
}

template <FibKey K, FibValue V>
typename FibTree<K, V>::PNode FibTree<K, V>::root() const
{
    return m_root;
}

template <FibKey K, FibValue V>
void FibTree<K, V>::setRoot(PNode node)
{
    m_root = node;
}

template <FibKey K, FibValue V>
void FibTree<K, V>::transplant(PNode u, PNode v)
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

template <FibKey K, FibValue V>
bool FibTree<K, V>::isBalanced(PNode node) const
{
    // Check if the tree is balanced
    // The tree is balanced if the size of the left and right subtrees
    // are Fibonacci numbers that are close to each other.
    // The Fibonacci ratio is 1.61803398875.
    // The tree is balanced if the ratio of the left and right subtree sizes
    // is close to this number.

    if (node == nullptr) {
        return true;
    }

    auto totalSize = node->size;

    auto leftSize = node->left ? node->left->size : 0;
    auto rightSize = node->right ? node->right->size : 0;

    auto fib1 = closestFibonacci(totalSize);
    auto fib2 = totalSize - fib1;

    return (leftSize == fib1 && rightSize == fib2) ||
        (leftSize == fib2 && rightSize == fib1);
}

template <FibKey K, FibValue V>
void FibTree<K, V>::merge(PNode node)
{
    if (!node) {
        return;
    }

    // Merge the left and right subtrees of the node
    // to maintain the Fibonacci ratio.
    if (node->left && node->right) {
        auto leftSize = node->left->size;
        auto rightSize = node->right->size;
        if (leftSize > rightSize) {
            transplant(node, node->left);
        } else {
            transplant(node, node->right);
        }
    }
}

template <FibKey K, FibValue V>
void FibTree<K, V>::rebalance(PNode node)
{
    // TODO: placeholder
}

template <FibKey K, FibValue V>
typename FibTree<K, V>::PNode FibTree<K, V>::minValueNode(PNode node)
{
    auto current = node;
    while (current->left != nullptr) {
        current = current->left;
    }
    return current;
}

template <FibKey K, FibValue V>
void FibTree<K, V>::deleteTree(PNode node)
{
    if (node == nullptr) {
        return;
    }

    deleteTree(node->left);
    deleteTree(node->right);

    delete node;
}
