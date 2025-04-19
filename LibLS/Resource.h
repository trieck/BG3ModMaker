#pragma once
#include "LSCommon.h"
#include "LSFCommon.h"
#include "Node.h"

struct Region : LSNode
{
    std::string regionName;
    using Ptr = std::shared_ptr<Region>;
};

struct Resource
{
    LSMetadata metadata{};
    LSFMetadataFormat metadataFormat{LSFMetadataFormat::NONE };
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

enum ResourceFormat
{
    LSX,
    LSF
};