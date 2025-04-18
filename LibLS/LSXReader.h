#pragma once

#include <pugixml.hpp>

#include "Resource.h"

class LSXReader : public pugi::xml_tree_walker
{
public:
    LSXReader();
    ~LSXReader() override;
    Resource::Ptr read(const ByteBuffer& info);

    bool begin(pugi::xml_node& node) override;
    bool end(pugi::xml_node& node) override;
    bool for_each(pugi::xml_node& node) override;

private:
    Region::Ptr m_region;
    std::vector<LSNode::Ptr> m_stack;
    Resource::Ptr m_resource;
};

