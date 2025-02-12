#pragma once

#include <unordered_map>

#include "NodeAttribute.h"

struct Node : std::enable_shared_from_this<Node>
{
    using Ptr = std::shared_ptr<Node>;
    using WeakPtr = std::weak_ptr<Node>;

    int32_t childCount() const;
    int32_t totalChildCount() const;
    void appendChild(const Ptr& child);

    std::string name;
    WeakPtr parent;

    std::unordered_map<std::string, NodeAttribute> attributes;
    std::unordered_map<std::string, std::vector<Ptr>> children;

    int32_t line{0};
    std::string keyAttribute;
};

