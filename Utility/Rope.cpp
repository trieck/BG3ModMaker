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
        return split(leaf, offset, text);
    }

    ASSERT(!leaf->value.isFrozen);

    auto spaceLeft = MAX_TEXT_SIZE - leaf->key.weight;
    if (spaceLeft >= text.size()) { // text fits in leaf
        return insertText(leaf, leaf->key.weight, text);
    }

    auto leftTextLen = std::min(spaceLeft, text.size());
    auto rightTextLen = text.size() - leftTextLen;

    auto leftText = text.substr(0, leftTextLen);
    auto rightText = text.substr(text.size() - rightTextLen);

    insertText(leaf, offset, leftText);

    if (rightText.empty()) {
        return leaf;
    }

    return split(leaf, rightText);
}

Rope::PNode Rope::insertText(const PNode& node, size_t offset, const std::string& text)
{
    ASSERT(node && !node->value.isFrozen);
    ASSERT(node->value.isLeaf());
    ASSERT(node->key.weight + text.size() <= MAX_TEXT_SIZE);
    ASSERT(node->value.text.size() + text.size() <= MAX_TEXT_SIZE);

    offset = std::min(offset, node->value.text.size());

    ASSERT(offset <= MAX_TEXT_SIZE);

    node->value.text.insert(offset, text);
    updateWeights(node, static_cast<int>(text.size()));

    return node;
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
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    ofs << "digraph Rope {\n";

    std::ostringstream oss;
    printDOT(root(), oss);

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

    if (l <= 3 && r <=3 ) {
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

    while (node) {
        auto leftSize = static_cast<float>(nodeSize(node->left));
        auto rightSize = static_cast<float>(nodeSize(node->right));

        if (leftSize <= 3 && rightSize <= 3) {
            node = node->parent;
            continue; // too small to worry about rebalancing
        }

        if (leftSize > FIB_RATIO * rightSize) {
            // Left heavy, rotate right
            rotateRight(node);
        }

        if (rightSize > FIB_RATIO * leftSize) {
            // Right heavy, rotate left
            rotateLeft(node);
            break;
        }

        updateSizes(node); // Update sizes after rotation

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

void Rope::addWeightAndSize(PNode node)
{
    if (!node) {
        return;
    }

    auto w = node->key.weight;
    auto s = node->size;

    PNode p = node->parent;

    while (p) {
        if (p->left == node) {
            p->key.weight += w;
        }

        p->size += s;

        node = p;
        p = p->parent;
    }
}

void Rope::subtractWeightAndSize(PNode node)
{
    if (!node) {
        return;
    }

    auto w = node->key.weight;
    auto s = node->size;

    PNode p = node->parent;

    while (p) {
        if (p->left == node) {
            p->key.weight -= w;
        }

        p->size -= s;

        node = p;
        p = p->parent;
    }
}

size_t Rope::nodeSize(PNode node) const
{
    if (!node) {
        return 0;
    }

    return node->size;
}

Rope::PNode Rope::split(PNode& node, const std::string& text)
{
    // This is the recursive split method for inserting text into a full node.
    // It creates a new internal node with the left child containing the original text,
    // and a right child that is empty and unfrozen.

    ASSERT(node->value.isLeaf());
    ASSERT(node->value.text.size() == MAX_TEXT_SIZE);
    ASSERT(node->key.weight == MAX_TEXT_SIZE);
    ASSERT(!node->value.isFrozen);

    node->value.isFrozen = true; // freeze the node

    auto parent = split(node);

    ASSERT(parent->left->value.isLeaf());
    ASSERT(parent->right->value.isLeaf());

    ASSERT(parent->left->value.text.size() == MAX_TEXT_SIZE);
    ASSERT(parent->left->key.weight == MAX_TEXT_SIZE);
    ASSERT(parent->left->value.isFrozen);

    auto leaf = parent->right; // leaf is now the right child of the new parent

    auto spaceLeft = MAX_TEXT_SIZE - leaf->key.weight;
    auto leftTextLen = std::min(spaceLeft, text.size());
    auto rightTextLen = text.size() - leftTextLen;

    auto leftText = text.substr(0, leftTextLen);
    auto rightText = text.substr(text.size() - rightTextLen);

    insertText(leaf, leaf->key.weight, leftText);

    if (rightText.empty()) {
        return parent;
    }

    return split(leaf, rightText);
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
    // A new internal node is created to replace the original node in the tree.
    // The original node is deleted.

    if (!node || node->value.isInternal()) { // not a leaf node
        return nullptr;
    }

    auto isRoot = node == root();

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

    auto newParent = makeInternal(leftNode, rightNode);

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

    updateSizes(newParent);

    // We have inserted a new subtree into the tree,
    // We must ensure the tree remains balanced and the sizes are correct.
    rebalance(newParent);

    return newParent;
}

Rope::PNode Rope::split(PNode& node, size_t offset, const std::string& text)
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
    ASSERT(leaf->key.weight <= MAX_TEXT_SIZE);
    ASSERT(leaf->value.text.size() <= MAX_TEXT_SIZE);

    return leafInsert(leaf, 0, text); // insert the new text into the leaf
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

void Rope::updateSizes(PNode node)
{
    while (node) {
        node->size = 1 + nodeSize(node->left) + nodeSize(node->right);
        node = node->parent;
    }
}

void Rope::updateWeights(PNode node, int addedChars)
{
    if (!node) {
        return;
    }

    node->key.weight += addedChars;

    while (node) {
        auto parent = node->parent;
        if (parent && parent->left == node) {
            parent->key.weight += addedChars;
        }
        node = node->parent;
    }
}

void Rope::rotateLeft(PNode node)
{
    if (!node || !node->right) {
        return; // nothing to rotate
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

    subtractWeightAndSize(M);
    subtractWeightAndSize(B);
    subtractWeightAndSize(A);

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
    addWeightAndSize(B);

    A->parent = B;
    B->left = A;
    addWeightAndSize(A);

    A->right = M;
    if (M) {
        M->parent = A;
    }
    addWeightAndSize(M);
}

void Rope::rotateRight(PNode node)
{
    if (!node || !node->left) {
        return; // nothing to rotate
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

    subtractWeightAndSize(M);
    subtractWeightAndSize(B);
    subtractWeightAndSize(A);

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
    addWeightAndSize(B);

    A->parent = B;
    B->right = A;
    addWeightAndSize(A);

    A->left = M;
    if (M) {
        M->parent = A;
    }

    addWeightAndSize(M);
}

bool Rope::isFull(const PNode& node) const
{
    if (!node) {
        return false;
    }

    if (node->value.isInternal()) {
        return false;
    }

    if (node->key.weight != MAX_TEXT_SIZE) {
        return false;
    }

    ASSERT(node->value.text.size() == MAX_TEXT_SIZE);

    return true;
}

Rope::PNode Rope::makeLeaf(const std::string& text)
{
    RopeValue value(RopeNodeType::Leaf, false, text);

    return new NodeType(RopeKey(text.size()), std::move(value));
}

Rope::PNode Rope::makeInternal(PNode left, PNode right)
{
    auto weight = left->key.weight;

    auto value = RopeValue(RopeNodeType::Internal, true, {});
    auto node = new NodeType(RopeKey(weight), std::move(value));

    node->left = left;
    node->right = right;
    left->parent = node;
    right->parent = node;

    node->size += left->size + right->size;

    return node;
}
