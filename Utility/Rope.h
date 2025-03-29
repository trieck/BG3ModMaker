#pragma once

#include "FibTree.h"

struct RopeKey
{
    RopeKey();
    explicit RopeKey(size_t w);
    RopeKey(const RopeKey&) = default;
    RopeKey(RopeKey&&) = default;
    RopeKey& operator=(const RopeKey&) = default;
    RopeKey& operator=(RopeKey&&) = default;
    ~RopeKey() = default;

    bool operator<(const RopeKey& other) const;

    size_t weight;
};

enum class RopeNodeType {
    Internal,
    Leaf
};

struct RopeValue
{
    RopeValue() = default;
    explicit RopeValue(RopeNodeType t, bool f, std::string s);

    RopeValue(const RopeValue&) = default;
    RopeValue(RopeValue&&) = default;
    RopeValue& operator=(const RopeValue&) = default;
    RopeValue& operator=(RopeValue&&) = default;
    ~RopeValue() = default;

    bool isInternal() const;
    bool isLeaf() const;

    RopeNodeType type = RopeNodeType::Internal;
    bool isFrozen = false;
    std::string text;
};

class Rope : public FibTree<RopeKey, RopeValue>
{
public:
    Rope() = default;
    ~Rope() override = default;

    void insert(size_t offset, const std::string& text);
    void exportDOT(const std::string& filename) const;
    void deleteRange(size_t start, size_t end);
    void deleteAll();
    std::string::value_type find(size_t offset) const;
    std::string str() const;
    bool isBalanced() const;

private:
    static constexpr auto MAX_TEXT_SIZE = 3;
    using PNodePair = std::pair<PNode, PNode>;

    bool isBalanced(PNode node) const override;
    bool isFull(const PNode& node) const;
    PNode insertText(const PNode& node, size_t offset, const std::string& text);
    PNode leafAt(size_t& offset) const;
    PNode leafInsert(PNode& leaf, size_t offset, const std::string& text);
    PNode lowestCommonAncestor(PNode a, PNode b) const;
    PNode makeLeaf(const std::string& text);
    PNode makeParent(PNode left, PNode right);
    PNode rotateLeft(PNode node);
    PNode rotateRight(PNode node);
    PNode split(PNode& node);
    PNode split(PNode& node, size_t offset);
    PNode splitAt(size_t offset);
    PNode splitInsert(PNode& node, const std::string& text);
    PNode splitInsert(PNode& node, size_t offset, const std::string& text);
    PNodePair splitLeaf(PNode node, size_t offset);
    PNodePair splitLeafDel(PNode node, size_t offset);
    PNodePair splitNode(PNode node, size_t offset);
    size_t nodeSize(PNode node) const;
    size_t totalWeight(PNode node) const;
    size_t weightOf(PNode node) const;
    std::string::value_type find(const PNode& node, size_t offset) const;
    void addWeightAndSize(PNode node);
    void deleteAll(PNode node);
    void exportDOT(const std::string& filename, const PNode& node) const;
    void printDOT(const PNode& node, std::ostream& os) const;
    void rebalance(PNode node) override;
    void stream(PNode node, std::ostream& oss) const;
    void subtractWeightAndSize(PNode node);
    void updateSizes(PNode node);
    void updateWeights(PNode node, int addedChars);
};
