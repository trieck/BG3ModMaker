#include "pch.h"

#include "Exception.h"
#include "FNVHash.h"
#include "LSFWriter.h"

#include "FileStream.h"

LSFWriter::LSFWriter() : m_stringHashMap(StringHashMapSize)
{
    m_metadata.majorVersion = LSMetadata::currentMajorVersion;
    m_stringHashMap.resize(StringHashMapSize);
}

LSFWriter::~LSFWriter()
{
}

void LSFWriter::write(StreamBase& stream, const Resource& resource)
{   
    m_metadata = resource.metadata;

    writeRegions(resource);
}

void LSFWriter::writeRegions(const Resource& resource)
{
    m_nextNodeIndex = 0;

    for (const auto& node : resource.regions | std::views::values) {
        if (m_version >= LSFVersion::EXTENDED_NODES &&
            m_metadataFormat == LSFMetadataFormat::KEYS_AND_ADJACENCY) {
            writeNodeV3(node);
        } else {
            writeNodeV2(node);
        }
    }
}

void LSFWriter::writeNodeV2(const LSNode::Ptr& node)
{
    LSFNodeEntryV2 nodeInfo;

    if (node->parent.lock() == nullptr) {
        nodeInfo.parentIndex = -1;
    } else {
        nodeInfo.parentIndex = m_nodeIndices[node->parent.lock()];
    }

    nodeInfo.nameHashTableIndex = addStaticString(node->name);

    if (!node->attributes.empty()) {
        nodeInfo.firstAttributeIndex = m_nextAttrIndex;
        writeNodeAttributesV2(node);
    }
}

void LSFWriter::writeNodeAttributesV2(const LSNode::Ptr& node)
{
    auto lastOffset = m_valueStream.tell();

    for (const auto& val : node->attributes | std::views::values) {
        writeAttributeValue(val);
    }
}

void LSFWriter::writeAttributeValue(const NodeAttribute& attr)
{
    switch (attr.type()) {
    case String:
    case FixedString:
    case LSString:
    case WString:
    case LSWString:
        writeString(attr.str());
        break;
    case TranslatedFSString:
        ASSERT(0);
        break;
    case TranslatedString:
        ASSERT(0);
        break;
    case ScratchBuffer:
        ASSERT(0);
        break;
    case Byte:
        m_valueStream.write(std::get<uint8_t>(attr.value())); 
        break;
    case Short:
        m_valueStream.write(std::get<int16_t>(attr.value()));
        break;
    case UShort:
        m_valueStream.write(std::get<uint16_t>(attr.value()));
        break;
    case Int:
        m_valueStream.write(std::get<int32_t>(attr.value()));
        break;
    case UInt:
        m_valueStream.write(std::get<uint32_t>(attr.value()));
        break;
    case Float:
        m_valueStream.write(std::get<float>(attr.value()));
        break;
    case Double:
        m_valueStream.write(std::get<double>(attr.value()));
        break;
    case IVec2:
        m_valueStream.write(std::get<std::array<int32_t, 2>>(attr.value()));
        break;
    case IVec3:
        m_valueStream.write(std::get<std::array<int32_t, 3>>(attr.value()));
        break;
    case IVec4:
        m_valueStream.write(std::get<std::array<int32_t, 4>>(attr.value()));
        break;
    case Vec2:
        m_valueStream.write(std::get<std::array<float, 2>>(attr.value()));
        break;
    case Vec3:
        m_valueStream.write(std::get<std::array<float, 3>>(attr.value()));
        break;
    case Vec4:
        m_valueStream.write(std::get<std::array<float, 4>>(attr.value()));
        break;
    case Mat2:
    case Mat3:
    case Mat3x4:
    case Mat4x3:
    case Mat4:
        ASSERT(0);
        break;
    case Bool:
        m_valueStream.write(std::get<bool>(attr.value()));
        break;
    case ULongLong:
        m_valueStream.write(std::get<uint64_t>(attr.value()));
        break;
    case Long:
        m_valueStream.write(std::get<int64_t>(attr.value()));
        break;
    case Int8:
        m_valueStream.write(std::get<int8_t>(attr.value()));
        break;
    case Uuid:
        m_valueStream.write(std::get<UUIDT>(attr.value()));
        break;
    case Int64:
        m_valueStream.write(std::get<int64_t>(attr.value()));
        break;
    default:
        throw Exception("Unsupported attribute type");
    }
}

void LSFWriter::writeString(const std::string& str)
{
    m_valueStream.write(str.data(), str.size());
}

void LSFWriter::writeNodeV3(const LSNode::Ptr& node)
{
    /*LSFNodeEntryV3 nodeInfo;*/
}

void LSFWriter::writeNodeAttributesV3(const LSNode::Ptr& node)
{
}

uint32_t LSFWriter::addStaticString(const std::string& str)
{
    auto hash = fnvhash::hash(str);

    auto bucket = static_cast<int>((hash & 0x1ff) ^ ((hash >> 9) & 0x1ff) ^ ((hash >> 18) & 0x1ff) ^ ((hash >> 27) & 0x1ff));

    for (auto i = 0ull; i < m_stringHashMap[bucket].size(); ++i) {
        if (m_stringHashMap[bucket][i] == str) {
            return static_cast<uint32_t>((bucket << 16) | i);
        }
    }

    m_stringHashMap[bucket].emplace_back(str);

    return static_cast<uint32_t>((bucket << 16) | (m_stringHashMap[bucket].size() - 1));
}
