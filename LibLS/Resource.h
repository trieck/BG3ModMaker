#pragma once
#include "LSCommon.h"
#include "Node.h"

enum LSMetadataFormat : uint32_t {

    NONE = 0,
    KEYS_AND_ADJACENCY = 1,
    NONE2 = 2
};

struct Region : LSNode
{
    std::string regionName;
    using Ptr = std::shared_ptr<Region>;
};

struct Resource
{
    LSMetadata metadata{};
    LSMetadataFormat metadataFormat{ NONE };
    std::unordered_map<std::string, Region::Ptr> regions;

    using Ptr = std::unique_ptr<Resource>;

    Resource()
    {
        metadata.majorVersion = 3;
    }
};

struct AttributeTypeMaps
{
    static AttributeType typeToId(const std::string& type);
    static std::string idToType(AttributeType type);
};
