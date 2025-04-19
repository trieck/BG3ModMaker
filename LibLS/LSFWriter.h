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
    uint32_t addStaticString(const std::string& str);
    void writeAttributeValue(const NodeAttribute& attr);
    void writeNodeAttributesV2(const LSNode::Ptr& node);
    void writeNodeAttributesV3(const LSNode::Ptr& node);
    void writeNodeChildren(const LSNode::Ptr& node);
    void writeNodeV2(const LSNode::Ptr& node);
    void writeNodeV3(const LSNode::Ptr& node);
    void writeRegions(const Resource& resource);
    void writeStaticString(Stream& stream, const std::string& str);
    void writeStaticStrings(Stream& stream);
    void writeString(const std::string& str);
    void writeStringWithLength(const std::string& value);
    void writeTranslatedFSString(const TranslatedFSStringT& str);
    void writeTranslatedString(const TranslatedStringT& str);

    Stream m_nodeStream, m_attrStream, m_valueStream, m_keyStream;
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
