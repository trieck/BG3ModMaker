#pragma once

/*
==========================================
 Red-Black Tree (RBTree) Node Constraints
==========================================

1) Node Structure Constraints:
--------------------------------
- Each node must store a key (`T data`) that is comparable (<, >),
  trivially constructible and movable
- Each node must maintain references to:
  - Left and Right children (`std::shared_ptr<RBNode<T>>`).
  - Parent (`RBNode* parent;`).
- Each node must store its color (`NodeColor color`).
  - Colors are either RED or BLACK.

2) Red-Black Tree Properties:
-------------------------------
- Every node is either RED or BLACK.
- The root node is always BLACK.
- No two consecutive RED nodes exist (No RED parent-child pairs).
- Every path from the root to a leaf (`nullptr`) contains the same number
  of BLACK nodes (Black Height Property).
- Newly inserted nodes must always start as RED (before rebalancing).

3) Parent Pointer Constraints:
--------------------------------
- Parent pointers must always be updated correctly during:
  - Insertions
  - Rotations (Left/Right)
  - Deletions
- The root node must always have `parent = nullptr`.
- Parent pointers must never be left dangling after modifications.

4) Rotation Requirements (for Balancing):
-------------------------------------------
- Rotate Left & Rotate Right must:
  - Correctly update parent and child pointers.
  - Ensure the new root of the rotated subtree replaces the old root.
  - Maintain the Red-Black properties after rotation.

5) Insert & Delete Constraints:
--------------------------------
- Insertions must maintain Red-Black properties.
  - Fix any RED-RED violations using rotations and recoloring.
- Deletions must maintain the Black Height Property.
  - If a BLACK node is removed, rebalancing is required.

6) Ordering & Search Constraints:
-----------------------------------
- Nodes must follow Binary Search Tree (BST) ordering:
  - Left child < Parent < Right child.
- Search operations must run in O(log n).
  - If ordering is broken, search efficiency degrades.

------------------------------------------
 This comment serves as a reference guide
 for maintaining RBTree correctness.
==========================================
*/

enum class NodeColor
{
    RED,
    BLACK
};

template <typename T>
concept RBType = requires(T a, T b)
{
    std::movable<T>;
    std::default_initializable<T>;
    { a < b } -> std::convertible_to<bool>;
    { a > b } -> std::convertible_to<bool>;
};

template <RBType T>
struct RBNode
{
    RBNode();
    explicit RBNode(T data);

    using Ptr = std::shared_ptr<RBNode>;

    T data{};
    Ptr left, right;
    RBNode* parent;
    NodeColor color;
};

template <RBType T>
RBNode<T>::RBNode()
{
    left = nullptr;
    right = nullptr;
    parent = nullptr;
    color = NodeColor::RED;
}

template <RBType T>
RBNode<T>::RBNode(T data) : data(std::move(data)), parent(nullptr), color(NodeColor::RED)
{
}

template <RBType T>
class RBTree
{
public:
    using PNode = typename RBNode<T>::Ptr;

    RBTree();
    ~RBTree();
    void insert(const T& data);
    void remove(const T& data);
    T find(const T& key) const;

private:
    void fixInsert(PNode& node);
    void fixDelete(PNode& node);
    void rotateLeft(RBNode<T>* node);
    void rotateRight(RBNode<T>* node);
    void swapColor(RBNode<T>* left, RBNode<T>* right);
    void transplant(PNode& u, PNode& v);
    PNode minValueNode(PNode& node);

    PNode m_root = nullptr;
};

template <RBType T>
RBTree<T>::RBTree() : m_root(nullptr)
{
}

template <RBType T>
RBTree<T>::~RBTree()
{
}

template <RBType T>
void RBTree<T>::insert(const T& data)
{
    auto node = std::make_shared<RBNode<T>>(data);

    RBNode<T>* parent = nullptr;

    auto current = m_root;

    while (current != nullptr) {
        parent = current.get();
        if (node->data < current->data) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    node->parent = parent;
    if (parent == nullptr) {
        m_root = node;
    } else if (node->data < parent->data) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    fixInsert(node);
}

template <RBType T>
void RBTree<T>::remove(const T& data)
{
    auto node = m_root;
    PNode x, y, z;

    while (node != nullptr) {
        if (node->data == data) {
            z = node;
        }

        if (node->data <= data) {
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
        if (y->parent == z.get()) {
            if (x != nullptr) {
                x->parent = y.get();
            }
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y.get();
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y.get();
        y->color = z->color;

        if (yOriginalColor == NodeColor::BLACK) {
            fixDelete(x);
        }
    }
}

template <RBType T>
T RBTree<T>::find(const T& key) const
{
    auto current = m_root;

    while (current != nullptr) {
        if (key < current->data) {
            current = current->left;
        } else if (key > current->data) {
            current = current->right;
        } else {
            return current->data;
        }
    }

    return T();
}

template <RBType T>
void RBTree<T>::fixInsert(PNode& node)
{
    RBNode<T>* parent = nullptr;
    RBNode<T>* grandparent = nullptr;

    while (node != m_root && node->color == NodeColor::RED) {
        if (node->parent == nullptr || node->parent->color == NodeColor::BLACK) {
            break;
        }

        parent = node->parent;
        grandparent = parent->parent;

        if (parent == grandparent->left.get()) {
            auto uncle = grandparent->right;
            if (uncle != nullptr && uncle->color == NodeColor::RED) {
                grandparent->color = NodeColor::RED;
                parent->color = NodeColor::BLACK;
                uncle->color = NodeColor::BLACK;
                node.reset(grandparent);
            } else {
                if (node == parent->right) {
                    rotateLeft(parent);
                    node.reset(parent);
                    parent = node->parent;
                }
                rotateRight(grandparent);
                swapColor(parent, grandparent);
                node.reset(parent);
            }
        } else {
            auto uncle = grandparent->left;
            if (uncle != nullptr && uncle->color == NodeColor::RED) {
                grandparent->color = NodeColor::RED;
                parent->color = NodeColor::BLACK;
                uncle->color = NodeColor::BLACK;
                node.reset(grandparent);
            } else {
                if (node == parent->left) {
                    rotateRight(parent);
                    node.reset(parent);
                    parent = node->parent;
                }
                rotateLeft(grandparent);
                swapColor(parent, grandparent);
                node.reset(parent);
            }
        }
    }

    m_root->color = NodeColor::BLACK;
}

template <RBType T>
void RBTree<T>::fixDelete(PNode& node)
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
                (sibling->right  == nullptr || sibling->right->color == NodeColor::BLACK)) {
                sibling->color = NodeColor::RED;
                node.reset(node->parent);
            } else {
                if (sibling->right == nullptr || sibling->right->color == NodeColor::BLACK) {
                    if (sibling->left != nullptr) {
                        sibling->left->color = NodeColor::BLACK;
                    }
                    sibling->color = NodeColor::RED;
                    rotateRight(sibling.get());
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
                rotateLeft(sibling.get());
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

template <RBType T>
void RBTree<T>::rotateLeft(RBNode<T>* node)
{
    auto child = node->right;
    node->right = node->left;

    if (node->right != nullptr) {
        node->right->parent = node;
    }

    child->parent = node->parent;
    if (node->parent == nullptr) {
        m_root = child;
    } else if (node == node->parent->left.get()) {
        node->parent->left = child;
    } else {
        node->parent->right = child;
    }

    child->left.reset(node);
    node->parent = child.get();
}

template <RBType T>
void RBTree<T>::rotateRight(RBNode<T>* node)
{
    auto child = node->left;
    node->left = child->right;

    if (node->left != nullptr) {
        node->left->parent = node;
    }
    child->parent = node->parent;

    if (node->parent == nullptr) {
        m_root = child;
    } else if (node == node->parent->left.get()) {
        node->parent->left = child;
    } else {
        node->parent->right = child;
    }

    child->right.reset(node);

    node->parent = child.get();
}

template <RBType T>
void RBTree<T>::swapColor(RBNode<T>* left, RBNode<T>* right)
{
    auto temp = left->color;
    left->color = right->color;
    right->color = temp;
}

template <RBType T>
void RBTree<T>::transplant(PNode& u, PNode& v)
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

template <RBType T>
typename RBTree<T>::PNode RBTree<T>::minValueNode(PNode& node)
{
    auto current = node;
    while (current->left != nullptr) {
        current = current->left;
    }

    return current;
}
