#pragma once

#include "NodeAttribute.h"

// TODO: this has to be renamed as it conflicts with the red-black tree

struct RBNode : std::enable_shared_from_this<RBNode>
{
    using Ptr = std::shared_ptr<RBNode>;
    using WeakPtr = std::weak_ptr<RBNode>;

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

