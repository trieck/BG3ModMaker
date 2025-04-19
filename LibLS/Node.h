#pragma once

#include "NodeAttribute.h"

struct LSNode : std::enable_shared_from_this<LSNode>
{
    using Ptr = std::shared_ptr<LSNode>;
    using WeakPtr = std::weak_ptr<LSNode>;

    int32_t childCount() const;
    int32_t totalChildCount() const;
    void appendChild(const Ptr& child);

    std::string name;
    WeakPtr parent;
    
    tsl::ordered_map<std::string, NodeAttribute> attributes;
    tsl::ordered_map<std::string, std::vector<Ptr>> children;

    int32_t line{0};
    std::string keyAttribute;
};
