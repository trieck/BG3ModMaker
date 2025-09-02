#pragma once
#include "LSCommon.h"
#include "LSFCommon.h"
#include "Resource.h"
#include "Stream.h"

class LSFReader
{
public:
    LSFReader();
    ~LSFReader();

    Resource::Ptr read(const ByteBuffer& info);

private:
    static AttributeValue readMatrix(const NodeAttribute& attr, Stream& stream);
    NodeAttribute readAttribute(AttributeType type, Stream& reader, uint32_t length) const;
    static AttributeValue readVector(const NodeAttribute& attr, Stream& stream);
    static NodeAttribute readAttribute(AttributeType type, Stream& reader);
    TranslatedFSStringT readTranslatedFSString(Stream& stream) const;
    Stream decompress(uint32_t sizeOnDisk, uint32_t uncompressedSize, const std::string& debugDumpTo, bool allowChunked);
    void readAttributesV2(Stream& stream);
    void readAttributesV3(Stream& stream);
    void readHeader();
    void readKeys(Stream& stream);
    void readNames(Stream& stream);
    void readNode(const LSFNodeInfo& defn, LSNode& node, Stream& attributeReader);
    void readNodes(Stream& stream, bool longNodes);
    void readRegions(const Resource::Ptr& resource);
    std::string readString(Stream& stream, uint32_t length) const;

    Stream m_stream, m_values;
    PackedVersion m_gameVersion;
    LSFVersion m_version;
    LSFMetadataV6 m_metadata{};
    std::vector <std::vector< std::string>> m_names; // hash -> name chain
    std::vector<LSFNodeInfo> m_nodes;
    std::vector<LSNode::Ptr> m_nodeInstances;
    std::vector<LSFAttributeInfo> m_attributes;
};
