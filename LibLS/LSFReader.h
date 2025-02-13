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
    void readNames(const Stream::Ptr& stream);
    void readNodes(const Stream::Ptr& stream, bool longNodes);
    void readAttributesV2(const Stream::Ptr& stream);
    void readAttributesV3(const Stream::Ptr& stream);
    void readKeys(const Stream::Ptr& stream);
    static std::string readVector(const NodeAttribute& attr, const Stream::Ptr& stream);
    std::string readTranslatedFSString(const Stream::Ptr& stream) const;
    static std::string readMatrix(const NodeAttribute& attr, const Stream::Ptr& stream);
    NodeAttribute readAttribute(AttributeType type, const Stream::Ptr& reader, uint32_t length) const;
    static NodeAttribute readAttribute(AttributeType type, const Stream::Ptr& reader);
    void readNode(const LSFNodeInfo& defn, Node& node, const Stream::Ptr& attributeReader);
    void readRegions(const Resource::Ptr& resource);

    Stream::Ptr decompress(uint32_t sizeOnDisk, uint32_t uncompressedSize, std::string debugDumpTo, bool allowChunked) const;

    Stream::Ptr m_stream, m_values;
    PackedVersion m_gameVersion;
    LSFVersion m_version;
    LSFMetadataV6 m_metadata{};
    std::vector <std::vector< std::string>> m_names; // hash -> name chain
    std::vector<LSFNodeInfo> m_nodes;
    std::vector<Node::Ptr> m_nodeInstances;
    std::vector<LSFAttributeInfo> m_attributes;
};

