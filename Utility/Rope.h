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
    virtual ~Rope() = default;

    std::string str() const;
    std::string::value_type find(size_t offset) const;
    void insert(size_t offset, const std::string& text);
    void exportDOT(const std::string& filename) const;
    bool isBalanced() const;

private:
    bool isFull(const PNode& node) const;
    PNode insertText(const PNode& node, size_t offset, const std::string& text);
    PNode leafAt(size_t& offset) const;
    PNode leafInsert(PNode& leaf, size_t offset, const std::string& text);
    PNode makeInternal(PNode left, PNode right);
    PNode makeLeaf(const std::string& text);
    PNode split(PNode& node);
    PNode split(PNode& node, const std::string& text);
    PNode split(PNode& node, size_t offset);
    PNode split(PNode& node, size_t offset, const std::string& text);
    size_t nodeSize(PNode node);
    std::string::value_type find(const PNode& node, size_t offset) const;
    void addWeightAndSize(PNode node);
    void printDOT(const PNode& node, std::ostream& os) const;
    void rebalance(PNode node) override;
    void rotateLeft(PNode node);
    void rotateRight(PNode node);
    void stream(PNode node, std::ostream& oss) const;
    void subtractWeightAndSize(PNode node);
    void updateSizes(PNode node);
    void updateWeights(PNode node, int addedChars);

    static constexpr auto MAX_TEXT_SIZE = 3;
};
