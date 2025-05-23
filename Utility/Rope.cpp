#include "pch.h"
#include "MD5.h"
#include "Rope.h"

RopeKey::RopeKey(size_t w) : weight(w)
{
}

RopeKey::RopeKey() : weight(0)
{
}

bool RopeKey::operator<(const RopeKey& other) const
{
    return weight < other.weight;
}

RopeValue::RopeValue(RopeNodeType t, bool f, std::string s) : type(t), isFrozen(f), text(std::move(s))
{
}

bool RopeValue::isInternal() const
{
    return type == RopeNodeType::Internal;
}

bool RopeValue::isLeaf() const
{
    return type == RopeNodeType::Leaf;
}

Rope::Rope(size_t maxTextSize) : m_maxTextSize(maxTextSize)
{
}

void Rope::insert(size_t offset, const std::string& text)
{
    PNode leaf;

    if (isEmpty()) { // no tree
        leaf = makeLeaf({});
        setRoot(leaf);
    } else {
        leaf = leafAt(offset);
    }

    if (!leaf) {
        return; // offset is out of range
    }

    leafInsert(leaf, offset, text);
}

Rope::PNode Rope::leafInsert(PNode& leaf, size_t offset, const std::string& text)
{
    if (!leaf) {
        return nullptr;
    }

    ASSERT(leaf->value.isLeaf());

    if (isFull(leaf)) {
        return splitInsert(leaf, offset, text);
    }

    ASSERT(!leaf->value.isFrozen);

    auto spaceLeft = m_maxTextSize - leaf->key.weight;
    if (spaceLeft >= text.size()) { // text fits in leaf
        return insertText(leaf, offset, text);
    }

    auto leftTextLen = std::min(spaceLeft, text.size());
    auto rightTextLen = text.size() - leftTextLen;

    auto leftText = text.substr(0, leftTextLen);
    auto rightText = text.substr(text.size() - rightTextLen);

    insertText(leaf, offset, leftText);

    if (rightText.empty()) {
        return leaf;
    }

    return splitInsert(leaf, rightText);
}

Rope::PNode Rope::lowestCommonAncestor(PNode a, PNode b) const
{
    std::unordered_set<PNode> ancestors;

    while (a) {
        ancestors.insert(a);
        a = a->parent;
    }

    while (b) {
        if (ancestors.contains(b)) {
            return b;
        }
        b = b->parent;
    }

    return nullptr;
}

Rope::PNode Rope::insertText(const PNode& node, size_t offset, const std::string& text)
{
    ASSERT(node && !node->value.isFrozen);
    ASSERT(node->value.isLeaf());
    ASSERT(node->key.weight + text.size() <= m_maxTextSize);
    ASSERT(node->value.text.size() + text.size() <= m_maxTextSize);

    offset = std::min(offset, node->value.text.size());

    ASSERT(offset <= m_maxTextSize);

    node->value.text.insert(offset, text);

    updateMetaUp(node);

    return node;
}

void Rope::deleteAll(PNode node)
{
    if (!node) {
        return;
    }

    deleteAll(node->left);
    deleteAll(node->right);

    delete node;
}

void Rope::deleteRange(size_t start, size_t end)
{
    if (isEmpty()) {
        return; // nothing to delete
    }

    if (start == end) {
        return; // nothing to delete
    }

    // ensure start < end
    if (start > end) {
        std::swap(start, end);
    }

    auto [left, midRight] = splitNode(root(), start);
    auto [mid, right] = splitNode(midRight, end - start);

    deleteAll(mid);

    if (left && right) {
        setRoot(makeParent(left, right));
    } else {
        setRoot(left ? left : right);
    }

    rebalance(left);
    rebalance(right);
}

void Rope::deleteAll()
{
    if (isEmpty()) {
        return; // nothing to delete
    }

    deleteAll(root());

    setRoot(nullptr); // clear the root
}

Rope::PNodePair Rope::splitNode(PNode node, size_t offset)
{
    if (!node) {
        return {nullptr, nullptr};
    }

    if (node->value.isLeaf()) {
        auto result = splitLeafDel(node, offset);
        delete node;
        return result;
    }

    if (offset < node->key.weight) {
        auto [left, right] = splitNode(node->left, offset);
        node->left = right;
        if (right) {
            right->parent = node;
        }

        auto newParent = makeParent(right, node->right);
        delete node;
        updateMeta(newParent);

        return {left, newParent};
    }

    offset = offset - node->key.weight;
    auto [left, right] = splitNode(node->right, offset);
    node->right = left;
    if (left) {
        left->parent = node;
    }

    auto newParent = makeParent(node->left, left);
    delete node;
    updateMeta(newParent);

    return {newParent, right};
}

Rope::PNodePair Rope::splitLeafDel(PNode node, size_t offset)
{
    // This method is an optimization that may be used
    // for splitting in delete operations.
    if (offset == 0 || offset >= node->value.text.size()) {
        auto newNode = makeLeaf(node->value.text);
        if (isFull(node)) {
            newNode->value.isFrozen = true;
        }

        PNode left, right;
        if (offset == 0) {
            left = nullptr;
            right = newNode;
        } else {
            left = newNode;
            right = nullptr;
        }

        return {left, right};
    }

    return splitLeaf(node, offset);
}

Rope::PNodePair Rope::splitLeaf(PNode node, size_t offset)
{
    // This method splits a leaf node at a specific offset.
    // It creates two new leaf nodes, one containing the text to the left of the offset,
    // and the other containing the text to the right of the offset.
    // The nodes returned are not connected to the tree yet.

    auto length = node->value.text.size();
    auto leftString = node->value.text.substr(0, offset);
    auto rightString = node->value.text.substr(offset, length);

    auto leftNode = makeLeaf(leftString);
    auto rightNode = makeLeaf(rightString);

    if (isFull(leftNode)) {
        leftNode->value.isFrozen = true; // freeze the left node
    }

    if (isFull(rightNode)) {
        rightNode->value.isFrozen = true; // freeze the right node
    }

    return {leftNode, rightNode};
}


Rope::PNode Rope::splitAt(size_t offset)
{
    auto leaf = leafAt(offset);
    if (!leaf) {
        return nullptr; // offset is out of range
    }

    return split(leaf, offset);
}

std::string::value_type Rope::find(size_t offset) const
{
    if (isEmpty()) {
        return '\0';
    }

    return find(root(), offset);
}

std::string::value_type Rope::find(const PNode& node, size_t offset) const
{
    if (!node) {
        return '\0';
    }

    if (node->value.isLeaf()) {
        if (offset >= node->value.text.size()) {
            return '\0';
        }

        return node->value.text[offset];
    }

    if (offset >= node->key.weight) {
        return find(node->right, offset - node->key.weight);
    }

    return find(node->left, offset);
}

void Rope::printDOT(const PNode& node, std::ostream& os) const
{
    if (!node) {
        return;
    }

    MD5 md5;
    auto nodeId = md5.digestString(std::to_string(reinterpret_cast<uintptr_t>(node))).substr(0, 4);

    auto isInternal = node->value.isInternal();
    auto isFrozen = node->value.isFrozen;
    auto isLeaf = node->value.isLeaf();

    std::string text = isInternal ? "" : "\\\"" + node->value.text.substr(0, 10) + "\\\"";

    std::string fillColor = isInternal ? "darkred" : isFrozen ? "lightblue" : "darkgreen";
    std::string fontColor = isFrozen && isLeaf ? "black" : "white";

    std::string shape = "circle";

    os << "\"" << nodeId << "\" [shape=" << shape
        << ", style=filled, fillcolor=\"" << fillColor << "\""
        << ", fontcolor=" << fontColor << ", fontname=\"Segoe UI Semibold\""
        << ", label=\"" << nodeId
        << "\\nW:" << node->key.weight
        << " | S:" << node->size
        << "\\n" << text << "\"];\n";

    if (node->left) {
        auto nodeLeftId = md5.digestString(std::to_string(reinterpret_cast<uintptr_t>(node->left))).substr(0, 4);
        os << "\"" << nodeId << "\" -> \"" << nodeLeftId << "\";\n";
        printDOT(node->left, os);
    }

    if (node->right) {
        auto nodeRightId = md5.digestString(std::to_string(reinterpret_cast<uintptr_t>(node->right))).substr(0, 4);
        os << "\"" << nodeId << "\" -> \"" << nodeRightId << "\";\n";
        printDOT(node->right, os);
    }
}

void Rope::exportDOT(const std::string& filename) const
{
    exportDOT(filename, root());
}

void Rope::exportDOT(const std::string& filename, const PNode& node) const
{
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    ofs << "digraph Rope {\n";

    std::ostringstream oss;
    printDOT(node, oss);

    ofs << oss.str();
    ofs << "}\n";

    ofs.close();
}

bool Rope::isBalanced() const
{
    return isBalanced(root());
}

bool Rope::isBalanced(PNode node) const
{
    if (!node) {
        return true;
    }

    if (node->value.isLeaf()) {
        return true; // leaf nodes are balanced
    }

    auto l = nodeSize(node->left);
    auto r = nodeSize(node->right);

    if (l <= 3 && r <= 3) {
        return true;
    }

    auto ratio = static_cast<float>(std::max(l, r)) / static_cast<float>(std::min(l, r));

    return ratio <= std::numbers::phi_v<float> &&
        isBalanced(node->left) &&
        isBalanced(node->right);
}

void Rope::rebalance(PNode node)
{
    constexpr float FIB_RATIO = std::numbers::phi_v<float>;

    // Rebalance propagates upward through left-heavy imbalance,
    // but only performs one left rotation to fix right-heavy skew.
    // This produces shallower, more stable trees after edits.

    while (node) {
        float leftSize = static_cast<float>(nodeSize(node->left));
        float rightSize = static_cast<float>(nodeSize(node->right));

        if (leftSize <= 3 && rightSize <= 3) {
            node = node->parent;
            continue;
        }

        if (leftSize > FIB_RATIO * rightSize) {
            node = rotateRight(node);
            continue; // don't stop
        }

        if (rightSize > FIB_RATIO * leftSize) {
            rotateLeft(node);
            break; // stop rebalancing
        }

        node = node->parent;
    }
}

std::string Rope::str() const
{
    std::ostringstream oss;
    stream(root(), oss);
    return oss.str();
}

void Rope::stream(PNode node, std::ostream& oss) const
{
    if (!node) {
        return;
    }

    if (node->left) {
        stream(node->left, oss);
    }

    if (node->value.isLeaf()) {
        oss << node->value.text;
    }

    if (node->right) {
        stream(node->right, oss);
    }
}

size_t Rope::nodeSize(PNode node) const
{
    if (!node) {
        return 0;
    }

    return node->size;
}

size_t Rope::weightOf(PNode node) const
{
    if (!node) {
        return 0;
    }

    return node->key.weight;
}

size_t Rope::totalWeight(PNode node) const
{
    if (!node) {
        return 0;
    }

    if (node->value.isLeaf()) {
        return node->key.weight;
    }

    return totalWeight(node->left) + totalWeight(node->right);
}

Rope::PNode Rope::splitInsert(PNode& node, const std::string& text)
{
    // This is the recursive split method for inserting text into a full node.
    // It creates a new internal node with the left child containing the original text,
    // and a right child that is empty and unfrozen and recursively splits until
    // all the text is inserted.

    ASSERT(node->value.isLeaf());
    ASSERT(node->value.text.size() == m_maxTextSize);
    ASSERT(node->key.weight == m_maxTextSize);
    ASSERT(!node->value.isFrozen);

    node->value.isFrozen = true; // freeze the node

    auto parent = split(node);

    ASSERT(parent->left->value.isLeaf());
    ASSERT(parent->right->value.isLeaf());

    ASSERT(parent->left->value.text.size() == m_maxTextSize);
    ASSERT(parent->left->key.weight == m_maxTextSize);
    ASSERT(parent->left->value.isFrozen);

    auto leaf = parent->right; // leaf is now the right child of the new parent

    auto spaceLeft = m_maxTextSize - leaf->key.weight;
    auto leftTextLen = std::min(spaceLeft, text.size());
    auto rightTextLen = text.size() - leftTextLen;

    auto leftText = text.substr(0, leftTextLen);
    auto rightText = text.substr(text.size() - rightTextLen);

    insertText(leaf, leaf->key.weight, leftText);

    if (rightText.empty()) {
        return parent;
    }

    return splitInsert(leaf, rightText);
}

Rope::PNode Rope::split(PNode& node)
{
    // This method is for "right" splitting a node that is full.
    // It creates a new internal node with the left child containing
    // the text of the original node, but is now frozen.
    // The right child is a new leaf node that is empty and unfrozen.

    return split(node, node->value.text.size());
}

Rope::PNode Rope::split(PNode& node, size_t offset)
{
    // This method is for splitting a node at a specific offset.
    // The offset is the position in the text where the split should occur.

    // The result of the split is two new nodes, one containing the text to the left of the offset,
    // and the other containing the text to the right of the offset.
    // A new internal parent node is created to replace the original node in the tree.
    // The original node is deleted.

    if (!node || node->value.isInternal()) { // not a leaf node
        return nullptr;
    }

    auto isRoot = node == root();

    auto [leftNode, rightNode] = splitLeaf(node, offset);

    auto newParent = makeParent(leftNode, rightNode);

    auto grandparent = node->parent;
    if (grandparent != nullptr) {
        if (node == grandparent->left) {
            grandparent->left = newParent;
        } else {
            grandparent->right = newParent;
        }
        newParent->parent = grandparent;
    }

    if (isRoot) {
        setRoot(newParent);
    }

    delete node; // goodbye old node
    node = nullptr;

    updateMetaUp(newParent);
    rebalance(newParent);

    return newParent;
}

Rope::PNode Rope::splitInsert(PNode& node, size_t offset, const std::string& text)
{
    ASSERT(node && node->value.isLeaf());

    auto parentNode = split(node, offset);
    if (!parentNode) {
        return nullptr; // split failed
    }

    PNode leaf;
    if (parentNode->left->value.isFrozen) {
        leaf = parentNode->right;
    } else {
        leaf = parentNode->left;
    }

    ASSERT(leaf && leaf->value.isLeaf());
    ASSERT(!leaf->value.isFrozen);
    ASSERT(leaf->key.weight <= m_maxTextSize);
    ASSERT(leaf->value.text.size() <= m_maxTextSize);

    return leafInsert(leaf, offset, text); // insert the new text into the leaf
}

Rope::PNode Rope::leafAt(size_t& offset) const
{
    auto node = root();

    while (node) {
        if (node->value.isLeaf()) {
            break; // found a leaf node even if out of range
        }

        auto weight = node->key.weight;

        if (offset < weight) {
            node = node->left;
        } else if (offset >= weight) {
            offset -= weight;
            node = node->right;
        }
    }

    return node;
}

void Rope::updateMeta(PNode node)
{
    if (!node) {
        return;
    }

    if (node->value.isLeaf()) {
        node->key.weight = node->value.text.size();
        node->size = 1;
    } else {
        node->key.weight = totalWeight(node->left);
        node->size = 1 + nodeSize(node->left) + nodeSize(node->right);
    }
}

void Rope::updateMetaUp(PNode node)
{
    while (node) {
        updateMeta(node);
        node = node->parent;
    }
}

Rope::PNode Rope::rotateLeft(PNode node)
{
    if (!node || !node->right) {
        return node; // nothing to rotate
    }

    // Before rotation, the tree looks like this:
    //      A
    //     / \
    //    L   B
    //       / \
    //      M   R

    // After rotation, it will look like this:
    //      B
    //     / \
    //    A   R
    //   / \
    //  L   M

    auto A = node;
    auto B = node->right;
    auto M = B->left;

    auto grandparent = A->parent;
    if (grandparent) {
        if (grandparent->left == A) {
            grandparent->left = B;
        } else {
            grandparent->right = B;
        }
    } else {
        setRoot(B);
    }

    B->parent = grandparent;

    A->parent = B;
    B->left = A;

    A->right = M;
    if (M) {
        M->parent = A;
    }

    updateMeta(A);
    updateMeta(B);

    return B;
}

Rope::PNode Rope::rotateRight(PNode node)
{
    if (!node || !node->left) {
        return node; // nothing to rotate
    }

    // Before rotation, the tree looks like this:
    //        A
    //       / \
    //      B   R
    //     / \
    //    L   M

    // After rotation, it will look like this:
    //        B
    //       / \
    //      L   A
    //         / \
    //        M   R

    auto A = node;
    auto B = node->left;
    auto M = B->right;

    auto grandparent = A->parent;
    if (grandparent) {
        if (grandparent->left == A) {
            grandparent->left = B;
        } else {
            grandparent->right = B;
        }
    } else {
        setRoot(B);
    }

    B->parent = grandparent;

    A->parent = B;
    B->right = A;

    A->left = M;
    if (M) {
        M->parent = A;
    }

    updateMeta(A);
    updateMeta(B);

    return B;
}

bool Rope::isFull(const PNode& node) const
{
    if (!node) {
        return false;
    }

    if (node->value.isInternal()) {
        return false;
    }

    if (node->key.weight != m_maxTextSize) {
        return false;
    }

    ASSERT(node->value.text.size() == m_maxTextSize);

    return true;
}

Rope::PNode Rope::makeLeaf(const std::string& text)
{
    RopeValue value(RopeNodeType::Leaf, false, text);

    return new NodeType(RopeKey(text.size()), std::move(value));
}

Rope::PNode Rope::makeParent(PNode left, PNode right)
{
    auto weight = weightOf(left);

    auto value = RopeValue(RopeNodeType::Internal, true, {});
    auto node = new NodeType(RopeKey(weight), std::move(value));

    node->left = left;
    node->right = right;

    if (left) {
        left->parent = node;
    }

    if (right) {
        right->parent = node;
    }

    updateMeta(node);

    return node;
}
