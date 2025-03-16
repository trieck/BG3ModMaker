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
    LeftLeaf,
    RightLeaf
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
    bool isLeftLeaf() const;
    bool isRightLeaf() const;

    RopeNodeType type = RopeNodeType::Internal;
    bool isFrozen = false;
    std::string text;
};

class Rope : public FibTree<RopeKey, RopeValue>
{
public:
    Rope() = default;
    ~Rope() = default;

    std::string str() const;
    std::string::value_type find(size_t offset) const;
    void insert(size_t offset, const std::string& text);
    void printTree(std::ostream& os) const;
    void exportDOT(const std::string& filename) const;

private:
    bool isFull(const PNode& node) const;
    PNode insertText(PNode& node, size_t offset, const std::string& text);
    PNode leafAt(size_t& offset) const;
    PNode leafInsert(PNode& leaf, size_t offset, const std::string& text);
    PNode makeInternal(PNode left, PNode right);
    PNode makeLeftLeaf(const std::string& text);
    PNode makeRightLeaf(const std::string& text);
    PNode split(PNode& node);
    PNode split(PNode& node, const std::string& text);
    PNode split(PNode& node, size_t offset);
    PNode split(PNode& node, size_t offset, const std::string& text);
    size_t nodeSize(PNode node);
    std::string::value_type find(const PNode& node, size_t offset) const;
    void printDOT(const PNode& node, std::ostream& os) const;
    void printTree(const PNode& node, size_t level, std::ostream& os) const;
    void stream(PNode node, std::ostream& oss) const;
    void updateSizes(PNode node);
    void updateWeights(PNode node, int addedChars);

    static constexpr auto MAX_TEXT_SIZE = 3;
};
