#pragma once
#include "LSCommon.h"
#include "LSFCommon.h"
#include "Stream.h"

class LSFReader
{
public:
    LSFReader();
    ~LSFReader();

    Resource::Ptr read(const ByteBuffer& info);

private:
    void readHeader();
    void readNames(Stream& stream);
    void readNodes(Stream& stream, bool longNodes);
    void readAttributesV2(Stream& stream);
    void readAttributesV3(Stream& stream);
    void readKeys(Stream& stream);
    static std::string readVector(const NodeAttribute& attr, Stream& stream);
    std::string readTranslatedFSString(Stream& stream) const;
    static std::string readMatrix(const NodeAttribute& attr, Stream& stream);
    NodeAttribute readAttribute(AttributeType type, Stream& reader, uint32_t length) const;
    static NodeAttribute readAttribute(AttributeType type, Stream& reader);
    void readNode(const LSFNodeInfo& defn, RBNode& node, Stream& attributeReader);
    void readRegions(const Resource::Ptr& resource);

    Stream decompress(uint32_t sizeOnDisk, uint32_t uncompressedSize, std::string debugDumpTo, bool allowChunked);

    Stream m_stream, m_values;
    PackedVersion m_gameVersion;
    LSFVersion m_version;
    LSFMetadataV6 m_metadata{};
    std::vector <std::vector< std::string>> m_names; // hash -> name chain
    std::vector<LSFNodeInfo> m_nodes;
    std::vector<RBNode::Ptr> m_nodeInstances;
    std::vector<LSFAttributeInfo> m_attributes;
};
