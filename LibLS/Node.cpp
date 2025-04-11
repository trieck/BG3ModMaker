#include "pch.h"

#include <numeric>
#include "Node.h"

int32_t LSNode::childCount() const
{
    int32_t sum = std::accumulate(children.begin(), children.end(), 0,
        [](int32_t acc, const std::pair<std::string, std::vector<Ptr>>& p) {
            return acc + static_cast<int32_t>(p.second.size());
        });

    return sum;
}

int32_t LSNode::totalChildCount() const
{
    int32_t sum = 0;

    for (const auto& nodeList : children | std::views::values) {
        sum += static_cast<int32_t>(nodeList.size());
        for (const auto& node : nodeList) {
            sum += node->totalChildCount();
        }
    }

    return sum;
}

void LSNode::appendChild(const Ptr& child)
{
    child->parent = shared_from_this();

    auto [it, inserted] = children.try_emplace(child->name, std::vector<Ptr>{});
    it->second.push_back(child);
}
