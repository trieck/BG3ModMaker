#pragma once
#include "Resource.h"

class LSXReader
{
public:
    LSXReader();
    ~LSXReader();
    Resource::Ptr read(const ByteBuffer& info);

private:
    Region::Ptr m_region;
    std::vector<LSNode::Ptr> m_stack;
};

