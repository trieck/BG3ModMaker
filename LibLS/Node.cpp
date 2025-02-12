#include "pch.h"

#include <numeric>
#include "Node.h"

#include <ranges>

int32_t Node::childCount() const
{
    int32_t sum = std::accumulate(children.begin(), children.end(), 0,
        [](int32_t acc, const std::pair<std::string, std::vector<Ptr>>& p) {
            return acc + static_cast<int32_t>(p.second.size());
        });

    return sum;
}

int32_t Node::totalChildCount() const
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

void Node::appendChild(const Ptr& child)
{
    auto [it, inserted] = children.try_emplace(child->name, std::vector<Ptr>{});
    it->second.push_back(child);
}

