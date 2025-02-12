#pragma once
#include <unordered_map>

#include "LSCommon.h"
#include "Node.h"

enum LSFMetadataFormat : uint32_t {

    NONE = 0,
    KEYS_AND_ADJACENCY = 1,
    NONE2 = 2
};

#pragma pack(push, 1)
struct LSFMetadataV6
{
    uint32_t stringsUncompressedSize;
    uint32_t stringsSizeOnDisk;
    uint32_t keysUncompressedSize;
    uint32_t keysSizeOnDisk;
    uint32_t nodesUncompressedSize;
    uint32_t nodesSizeOnDisk;
    uint32_t attributesUncompressedSize;
    uint32_t attributesSizeOnDisk;
    uint32_t valuesUncompressedSize;
    uint32_t valuesSizeOnDisk;
    CompressionFlags compressionFlags;
    int8_t unknown2;
    uint16_t unknown3;
    LSFMetadataFormat metadataFormat;
};

struct LSFMagic
{
    uint8_t magic[4];
    uint32_t version;
};

struct LSFExtendedHeader
{
    int64_t engineVersion;
};

struct LSFNodeEntryV2
{
    uint32_t nameHashTableIndex;    // name of this node (16-bit MSB: hash table index, 16-bit LSB: offset in hash chain)
    int32_t firstAttributeIndex;   // index of the first attribute of the node, -1 if none
    int32_t parentIndex;           // index of the parent node, -1 if root
    int32_t nameIndex() const
    {
        return static_cast<int32_t>(nameHashTableIndex >> 16);
    }
    int32_t nameOffset() const
    {
        return static_cast<int32_t>(nameHashTableIndex & 0xFFFF);
    }
};

struct LSFNodeEntryV3
{
    uint32_t nameHashTableIndex;    // name of this node (16-bit MSB: hash table index, 16-bit LSB: offset in hash chain)
    int32_t parentIndex;           // index of the parent node, -1 if root
    int32_t nextSiblingIndex;      // index of the next sibling node, -1 if last
    int32_t firstAttributeIndex;   // index of the first attribute of the node, -1 if none
    int32_t nameIndex() const
    {
        return static_cast<int32_t>(nameHashTableIndex >> 16);
    }
    int32_t nameOffset() const
    {
        return static_cast<int32_t>(nameHashTableIndex & 0xFFFF);
    }
};

struct LSFAttributeEntryV2
{
    uint32_t nameHashTableIndex;    // name of this attribute (16-bit MSB: hash table index, 16-bit LSB: offset in hash chain)
    uint32_t typeAndLength;         // 6-bit LSB: type of this attribute, 26-bit MSB: length of this attribute
    int32_t nodeIndex;              // index of the node this attribute belongs to
    int32_t nameIndex() const
    {
        return static_cast<int32_t>(nameHashTableIndex >> 16);
    }
    int32_t nameOffset() const
    {
        return static_cast<int32_t>(nameHashTableIndex & 0xFFFF);
    }
    uint32_t type() const
    {
        return typeAndLength & 0x3F;
    }
    uint32_t length() const
    {
        return typeAndLength >> 6;
    }

};

struct LSFKeyEntry
{
    uint32_t nodeIndex; // index of the node
    uint32_t keyName;   // name of key attribute (16-bit MSB: index into hash table, 16-bit LSB: offset in hash chain)
    int32_t keyNameIndex() const
    {
        return static_cast<int32_t>(keyName >> 16);
    }
    int32_t keyNameOffset() const
    {
        return static_cast<int32_t>(keyName & 0xFFFF);
    }
    
};
#pragma pack(pop)

enum class LSFVersion : uint32_t
{
    INITIAL = 0x01,
    CHUNKED_COMPRESS = 0x02,
    EXTENDED_NODES = 0x03,
    BG3 = 0x04,
    BG3_EXTENDED_HEADER = 0x05,
    BG3_NODE_KEYS = 0x06,
    BG3_PATCH3 = 0x07,
    MAX_READ = 0x07,
    MAX_WRITE = 0x07
};

struct LSFNodeInfo
{
    int32_t parentIndex;            // index of the parent node, -1 if root
    int32_t nameIndex;              // index of the name in the name hash
    int32_t nameOffset;             // offset of the name in the hash chain
    int32_t firstAttributeIndex;    // index of the first attribute of the node, -1 if none
    std::string keyAttribute{};     // key attribute of the node
};

struct LSFAttributeInfo
{
    int32_t nameIndex;              // index of the name in the name hash
    int32_t nameOffset;             // offset of the name in the hash chain
    uint32_t typeId;                // type of the attribute
    uint32_t length;                // length of the attribute
    uint32_t dataOffset;            // absolute position of the attribute data in the values section
    int32_t nextAttributeIndex;     // index of the next attribute of the node, -1 if last
};

struct Region : Node
{
    std::string regionName;
    using Ptr = std::shared_ptr<Region>;
};

struct Resource
{
    LSMetadata metadata{};
    LSFMetadataFormat metadataFormat{ NONE };
    std::unordered_map<std::string, Region::Ptr> regions;

    using Ptr = std::unique_ptr<Resource>;

    Resource()
    {
        metadata.majorVersion = 3;
    }
};