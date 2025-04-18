#pragma once

#include "LSFCommon.h"
#include "Resource.h"
#include "Stream.h"

class LSFWriter
{
public:
    LSFWriter();
    ~LSFWriter();

    void write(StreamBase& stream, const Resource& resource);
private:
    void writeRegions(const Resource& resource);
    void writeNodeV2(const LSNode::Ptr& node);
    void writeNodeAttributesV2(const LSNode::Ptr& node);
    void writeNodeV3(const LSNode::Ptr& node);
    void writeNodeAttributesV3(const LSNode::Ptr& node);

    void writeAttributeValue(const NodeAttribute& attr);
    void writeString(const std::string& str);

    uint32_t addStaticString(const std::string& str);

    Stream m_stream, m_nodeStream, m_attrStream, m_valueStream, m_keyStream;
    LSFVersion m_version = LSFVersion::MAX_WRITE;
    LSFMetadataFormat m_metadataFormat = LSFMetadataFormat::NONE;
    CompressionMethod m_compressionMethod = CompressionMethod::NONE;
    LSCompressionLevel m_compressionLevel = LSCompressionLevel::DEFAULT;
    LSMetadata m_metadata{};
    int32_t m_nextNodeIndex = 0;
    int32_t m_nextAttrIndex = 0;

    static constexpr auto StringHashMapSize = 0x200;
    using StringHashMapT = std::vector<std::vector<std::string>>;

    StringHashMapT m_stringHashMap;

    std::unordered_map<LSNode::Ptr, int32_t> m_nodeIndices;
    std::vector<int32_t> m_nextSiblingIndices;
};
