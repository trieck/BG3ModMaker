#include "pch.h"
#include "LSFReader.h"

#include "LSCommon.h"

LSFReader::LSFReader()
= default;

LSFReader::~LSFReader()
= default;

namespace { // anonymous namespace
    constexpr char LSF_MAGIC[4] = {'L', 'S', 'O', 'F'};
} // anonymous namespace

Resource::Ptr LSFReader::read(const ByteBuffer& info)
{
    m_stream = Stream::makeStream(info);

    readHeader();

    auto namesStream = decompress(m_metadata.stringsSizeOnDisk, m_metadata.stringsUncompressedSize, "strings.bin",
                                  false);
    readNames(namesStream);

    m_nodes.clear();

    auto nodesStream = decompress(m_metadata.nodesSizeOnDisk, m_metadata.nodesUncompressedSize, "nodes.bin", true);

    auto hasAdjacencyData = m_version >= LSFVersion::EXTENDED_NODES
        && m_metadata.metadataFormat == KEYS_AND_ADJACENCY;
    readNodes(nodesStream, hasAdjacencyData);

    m_attributes.clear();

    auto attributesStream = decompress(m_metadata.attributesSizeOnDisk, m_metadata.attributesUncompressedSize,
                                       "attributes.bin", true);
    if (hasAdjacencyData) {
        readAttributesV3(attributesStream);
    } else {
        readAttributesV2(attributesStream);
    }

    m_values = decompress(m_metadata.valuesSizeOnDisk, m_metadata.valuesUncompressedSize, "values.bin", true);

    if (m_metadata.metadataFormat == KEYS_AND_ADJACENCY) {
        auto keysStream = decompress(m_metadata.keysSizeOnDisk, m_metadata.keysUncompressedSize, "keys.bin", true);
        readKeys(keysStream);
    }

    auto resource = std::make_unique<Resource>();
    resource->metadataFormat = m_metadata.metadataFormat;

    readRegions(resource);

    resource->metadata.majorVersion = m_gameVersion.major;
    resource->metadata.minorVersion = m_gameVersion.minor;
    resource->metadata.revision = m_gameVersion.revision;
    resource->metadata.buildNumber = m_gameVersion.build;

    return resource;
}

void LSFReader::readHeader()
{
    auto magic = m_stream->read<LSFMagic>();
    if (std::memcmp(magic.magic, LSF_MAGIC, sizeof(LSF_MAGIC)) != 0) {
        throw std::ios_base::failure("Invalid LSF file.");
    }

    if (magic.version < static_cast<uint32_t>(LSFVersion::INITIAL) || magic.version > static_cast<uint32_t>(
        LSFVersion::MAX_READ)) {
        throw std::ios_base::failure("Unsupported LSF version.");
    }

    if (magic.version >= static_cast<uint32_t>(LSFVersion::BG3_EXTENDED_HEADER)) {
        auto [engineVersion] = m_stream->read<LSFExtendedHeader>();
        m_gameVersion = PackedVersion::fromInt64(engineVersion);

        // Workaround for LSF files with missing engine version
        if (m_gameVersion.major == 0) {
            m_gameVersion = {.major = 4, .minor = 0, .revision = 9, .build = 0};
        }
    } else {
        throw std::ios_base::failure("Unsupported LSF version.");
    }

    if (magic.version < static_cast<uint32_t>(LSFVersion::BG3_NODE_KEYS)) {
        throw std::ios_base::failure("Unsupported LSF version.");
    }

    m_version = static_cast<LSFVersion>(magic.version);

    m_metadata = m_stream->read<LSFMetadataV6>();
}

void LSFReader::readNames(const Stream::Ptr& stream)
{
    // Format:
    // 32-bit hash entry count (N)
    //     N x 16-bit chain length (L)
    //         L x 16-bit string length (S)
    //             [S bytes of UTF-8 string data]

    m_names.clear();

    auto numHashEntries = stream->read<uint32_t>();
    while (numHashEntries-- > 0) {
        auto hash = std::vector<std::string>();

        auto numStrings = stream->read<uint16_t>();
        while (numStrings-- > 0) {
            auto nameLen = stream->read<uint16_t>();
            auto nameStream = stream->read(nameLen);
            auto name = std::string(nameStream->str());
            hash.emplace_back(std::move(name));
        }

        m_names.emplace_back(hash);
    }
}

void LSFReader::readNodes(const Stream::Ptr& stream, bool longNodes)
{
    auto size = stream->size();
    while (stream->tell() < size) {
        auto resolved = LSFNodeInfo{};

        if (longNodes) {
            auto item = stream->read<LSFNodeEntryV3>();
            resolved.parentIndex = item.parentIndex;
            resolved.nameIndex = item.nameIndex();
            resolved.nameOffset = item.nameOffset();
            resolved.firstAttributeIndex = item.firstAttributeIndex;
        } else {
            auto item = stream->read<LSFNodeEntryV2>();
            resolved.parentIndex = item.parentIndex;
            resolved.nameIndex = item.nameIndex();
            resolved.nameOffset = item.nameOffset();
            resolved.firstAttributeIndex = item.firstAttributeIndex;
        }

        m_nodes.emplace_back(resolved);
    }
}

void LSFReader::readAttributesV2(const Stream::Ptr& stream)
{
    std::vector<int32_t> prevAttributeRefs;

    uint32_t dataOffset = 0;
    int32_t index = 0;

    auto size = stream->size();
    while (stream->tell() < size) {
        auto attribute = stream->read<LSFAttributeEntryV2>();

        LSFAttributeInfo resolved;
        resolved.nameIndex = attribute.nameIndex();
        resolved.nameOffset = attribute.nameOffset();
        resolved.typeId = attribute.type();
        resolved.length = attribute.length();
        resolved.dataOffset = dataOffset;
        resolved.nextAttributeIndex = -1;

        auto nodeIndex = attribute.nodeIndex + 1;
        if (prevAttributeRefs.size() > static_cast<size_t>(nodeIndex)) {
            if (prevAttributeRefs[nodeIndex] != -1) {
                m_attributes[prevAttributeRefs[nodeIndex]].nextAttributeIndex = index;
            }

            prevAttributeRefs[nodeIndex] = index;
        } else {
            while (prevAttributeRefs.size() < static_cast<size_t>(nodeIndex)) {
                prevAttributeRefs.emplace_back(-1);
            }
            prevAttributeRefs.emplace_back(index);
        }

        dataOffset += resolved.length;
        m_attributes.emplace_back(resolved);
        ++index;
    }
}

void LSFReader::readAttributesV3(const Stream::Ptr& stream)
{
    auto size = stream->size();
    while (stream->tell() < size) {
        auto attribute = stream->read<LSFAttributeEntryV3>();

        LSFAttributeInfo resolved;
        resolved.nameIndex = attribute.nameIndex();
        resolved.nameOffset = attribute.nameOffset();
        resolved.typeId = attribute.type();
        resolved.length = attribute.length();
        resolved.dataOffset = attribute.offset;
        resolved.nextAttributeIndex = attribute.nextAttributeIndex;

        m_attributes.emplace_back(resolved);
    }
}

void LSFReader::readKeys(const Stream::Ptr& stream)
{
    auto size = stream->size();
    while (stream->tell() < size) {
        auto key = stream->read<LSFKeyEntry>();
        auto keyNameIndex = key.keyNameIndex();
        auto keyNameOffset = key.keyNameOffset();

        auto keyAttribute = m_names[keyNameIndex][keyNameOffset];
        auto& node = m_nodes[key.nodeIndex];
        node.keyAttribute = keyAttribute;
    }
}

NodeAttribute LSFReader::readAttribute(AttributeType type, const Stream::Ptr& reader, uint32_t length)
{
    NodeAttribute attr(type);
    TranslatedString_T str;

    int32_t handleLength;

    switch (type) {
    case String:
    case Path:
    case FixedString:
    case LSString:
    case WString:
    case LSWString:
    case ScratchBuffer:
        attr.setValue(reader->read(length)->str());
        break;
    case TranslatedString:
        if (m_version >= LSFVersion::BG3 || (m_gameVersion.major > 4 ||
            (m_gameVersion.major == 4 && m_gameVersion.revision > 0) || 
            (m_gameVersion.major == 4 && m_gameVersion.revision == 0 && m_gameVersion.build >= 0x1a))) {
            str.version = reader->read<uint16_t>();
        } else {
            str.version = 0;
            auto valueLength = reader->read<int32_t>();
            str.value = reader->read(valueLength)->str();
        }

        handleLength = reader->read<int32_t>();
        str.handle = reader->read(handleLength)->str();
        attr.setValue(str.str());
        break;
    case TranslatedFSString:
        // TODO: Implement
        break;
    default:
        attr = readAttribute(type, reader);
        break;
    }

    return attr;
}

static std::string readUUID(const Stream::Ptr& ptr)
{
    std::string uuid;

    for (auto i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            uuid += "-";
        }
        uuid += std::format("{:02x}", ptr->read<uint8_t>());
    }

    return uuid;
}

NodeAttribute LSFReader::readAttribute(AttributeType type, const Stream::Ptr& reader)
{
    NodeAttribute attr(type);

    switch (type) {
    case None:
        break;
    case Byte:
        attr.setValue(std::to_string(reader->read<uint8_t>()));
        break;
    case Int8:
        attr.setValue(std::to_string(reader->read<int8_t>()));
        break;
    case Short:
        attr.setValue(std::to_string(reader->read<int16_t>()));
        break;
    case UShort:
        attr.setValue(std::to_string(reader->read<uint16_t>()));
        break;
    case Int:
        attr.setValue(std::to_string(reader->read<int32_t>()));
        break;
    case UInt:
        attr.setValue(std::to_string(reader->read<uint32_t>()));
        break;
    case Long:
        attr.setValue(std::to_string(reader->read<int64_t>()));
        break;
    case Int64:
        attr.setValue(std::to_string(reader->read<int64_t>()));
        break;
    case ULongLong:
        attr.setValue(std::to_string(reader->read<uint64_t>()));
        break;
    case Float:
        attr.setValue(std::to_string(reader->read<float>()));
        break;
    case Double:
        attr.setValue(std::to_string(reader->read<double>()));
        break;
    case Bool:
        attr.setValue(reader->read<uint8_t>() != 0 ? "true" : "false");
        break;
    case UUID:
        attr.setValue(readUUID(reader));
        break;
    default:
        attr.setValue("Unsupported attribute type.");
    }

    return attr;
}

void LSFReader::readNode(const LSFNodeInfo& defn, Node& node, const Stream::Ptr& attributeReader)
{
    node.name = m_names[defn.nameIndex][defn.nameOffset];

    if (defn.firstAttributeIndex != -1) {
        auto attribute = m_attributes[defn.firstAttributeIndex];
        while (true) {
            m_values->seek(attribute.dataOffset, SeekMode::Begin);
            auto value = readAttribute(static_cast<AttributeType>(attribute.typeId), attributeReader, attribute.length);

            node.attributes[m_names[attribute.nameIndex][attribute.nameOffset]] = value;

            if (attribute.nextAttributeIndex == -1) {
                break;
            }

            attribute = m_attributes[attribute.nextAttributeIndex];
        }
    }
}

void LSFReader::readRegions(const Resource::Ptr& resource)
{
    auto& attrReader = m_values;

    m_nodeInstances.clear();

    for (const auto& defn : m_nodes) {
        if (defn.parentIndex == -1) {
            auto region = std::make_shared<Region>();
            readNode(defn, *region, attrReader);
            region->keyAttribute = defn.keyAttribute;
            m_nodeInstances.emplace_back(region);
            region->regionName = region->name;
            resource->regions[region->regionName] = std::move(region);
        } else {
            auto node = std::make_shared<Node>();
            readNode(defn, *node, attrReader);
            node->keyAttribute = defn.keyAttribute;
            node->parent = m_nodeInstances[defn.parentIndex];
            m_nodeInstances.emplace_back(node);
            m_nodeInstances[defn.parentIndex]->children[node->name].emplace_back(node);
        }
    }
}

Stream::Ptr LSFReader::decompress(uint32_t sizeOnDisk, uint32_t uncompressedSize, std::string debugDumpTo,
                                  bool allowChunked) const
{
    if (sizeOnDisk == 0 && uncompressedSize != 0) {
        auto stream = m_stream->read(uncompressedSize);

        return stream;
    }
    throw std::ios_base::failure("Unsupported stream");

    if (sizeOnDisk == 0 || uncompressedSize == 0) {
        return std::make_unique<Stream>(); // empty stream
    }

    return nullptr;
}
