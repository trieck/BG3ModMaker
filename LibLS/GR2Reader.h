#pragma once

#include "FileStream.h"
#include "GR2Decompressor.h"
#include "GR2Stream.h"

enum GR2FileFormat : uint8_t
{
    UNKNOWN_FORMAT = 0x00,
    LITTLE_ENDIAN_32 = 0x01,
    BIG_ENDIAN_32 = 0x02,
    LITTLE_ENDIAN_64 = 0x03,
    BIG_ENDIAN_64 = 0x04,
};

enum GR2NodeType : uint32_t
{
    TYPE_NONE = 0, /* No type */
    TYPE_INLINE = 1, /* Empty node with just children */
    TYPE_REFERENCE = 2, /* Reference to another node */
    TYPE_REFERENCE_TO_ARRAY = 3, /* Reference to an array of nodes */
    TYPE_ARRAY_OF_REFERENCES = 4, /* Array of references to other nodes */
    TYPE_VARIANT_REFERENCE = 5, /* Reference with offset */
    TYPE_REMOVED = 6, /* Removed node */
    TYPE_REFERENCE_TO_VARIANT_ARRAY = 7, /* Reference to an array of variant nodes */
    TYPE_STRING = 8, /* String node */
    TYPE_TRANSFORM = 9, /* Transform node */
    TYPE_REAL32 = 10, /* 32-bit floating point node */
    TYPE_INT8 = 11, /* 8-bit signed integer node */
    TYPE_UINT8 = 12, /* 8-bit unsigned integer node */
    TYPE_BINORMAL_INT8 = 13, /* 8-bit signed integer binormal node */
    TYPE_NORMAL_UINT8 = 14, /* 8-bit unsigned integer normal node */
    TYPE_INT16 = 15, /* 16-bit signed integer node */
    TYPE_UINT16 = 16, /* 16-bit unsigned integer node */
    TYPE_BINORMAL_INT16 = 17, /* 16-bit signed integer binormal node */
    TYPE_NORMAL_UINT16 = 18, /* 16-bit unsigned integer normal node */
    TYPE_INT32 = 19, /* 32-bit signed integer node */
    TYPE_UINT32 = 20, /* 32-bit unsigned integer node */
    TYPE_REAL16 = 21, /* 16-bit floating point node */
    TYPE_EMPTY_REFERENCE = 22, /* Empty reference node */
    TYPE_MAX = 22 /* Maximum node type value */
};

#pragma pack(push, 1)

struct GR2Reference
{
    uint32_t section; // section number
    uint32_t offset; // offset within section
};

struct GR2Header
{
    uint8_t signature[16]; // file signature
    uint32_t headerSize; // size of the header
    uint32_t headerFormat; // header format version
    uint32_t reserved1; // reserved
    uint32_t reserved2; // reserved
    uint32_t version; // file version
    uint32_t fileSize; // total file size
    uint32_t crc; // CRC checksum
    uint32_t sectionOffset; // offset to the section headers
    uint32_t numSections; // number of sections
    GR2Reference type; // reference to the type node
    GR2Reference root; // reference to the root node
    uint32_t tag; // file format version tag
    uint32_t extra[8]; // extra data
};

struct GR2SectionHeader
{
    uint32_t compressType; // compression type
    uint32_t dataOffset; // offset to the data
    uint32_t compressedLen; // length of the compressed data
    uint32_t decompressedLen; // length of the decompressed data
    uint32_t alignment; // decompressed section data aligned to this value
    uint32_t oodleStop0; // first 16 bits of Oodle compression
    uint32_t oodleStop1; // first 8 bits of Oodle compression
    uint32_t fixupOffset; // offset where the fixup data begins relative to the file
    uint32_t fixupSize; // number of fixed up data included in the section
    uint32_t marshallOffset; // offset where the marshall data begins
    uint32_t marshallSize; // number of marshalled data included in the section
};

struct GR2FixUp
{
    uint32_t srcOffset; // position in the section to fix up
    uint32_t dstSection; // section number of the destination
    uint32_t dstOffset; // position in the destination section
};

struct GR2TypeNode
{
    GR2NodeType type; // node type
    uint64_t name; // pointer to the name of the node
    uint64_t fields; // pointer to the fields
    int32_t arraySize; // size of the array if applicable
    uint32_t extra[3]; // extra data
    uint64_t extra4; // pointer size
};

struct GR2TransformData
{
    uint32_t flags;
    float translation[3]{};
    float rotation[4]{};
    float scaleShear[3][3]{};
};

#pragma pack(pop)

struct GR2Object
{
    using Ptr = std::shared_ptr<GR2Object>;
    using WeakPtr = std::weak_ptr<GR2Object>;

    const GR2TypeNode* typeNode;
    WeakPtr parent;

    std::string name;
    std::vector<Ptr> children;

    uint8_t* data{nullptr}; // transient data pointer 
};

struct GR2VarReference : GR2Object
{
    uint64_t offset;
};

struct GR2ArrayReference : GR2Object
{
};

struct GR2ReferenceArray : GR2Object
{
    uint32_t size;
};

struct GR2ReferenceVarArray : GR2Object
{
    uint64_t typePtr; // virtual pointer to the type
    uint32_t size;
};

struct GR2String : GR2Object
{
    std::string value;
};

struct GR2Int16 : GR2Object
{
    std::vector<int16_t> values;
};

struct GR2UInt16 : GR2Object
{
    std::vector<uint16_t> values;
};

struct GR2Int32 : GR2Object
{
    std::vector<int32_t> values;
};

struct GR2UInt32 : GR2Object
{
    std::vector<uint32_t> values;
};

struct GR2Float : GR2Object
{
    std::vector<float> values;
};

struct GR2UInt8 : GR2Object
{
    std::vector<uint8_t> values;
};

struct GR2Transform : GR2Object
{
    GR2TransformData transform{};
};

struct GR2ObjectInfo
{
    GR2Object::Ptr object;
    uint32_t level;
};

using GR2Callback = std::function<void(const GR2ObjectInfo& info)>;

template <class T>
concept TPtr = std::is_pointer_v<T>;

template <typename T>
concept VPtr = std::same_as<T, uint32_t> || std::same_as<T, uint64_t>;

class GR2Reader
{
public:
    GR2Reader() = default;
    ~GR2Reader() = default;

    void load(const char* filename, const GR2Callback& callback = {});
    void read(const char* filename);
    void build(const GR2Callback& callback = {});

    const std::vector<GR2Object::Ptr>& rootObjects() const;
    static bool isGR2(FileStream& stream);
    static bool isGR2(const ByteBuffer& contents);

private:
    bool is64Bit() const;
    bool isValid(const GR2TypeNode* node) const;
    GR2Object::Ptr build(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                         const GR2Callback& callback = {});
    GR2Object::Ptr makeArrayReference(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                      const GR2Callback& callback = {});
    GR2Object::Ptr makeFloat(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeInline(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeInt16(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeInt32(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeObject(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                              const GR2Callback& callback = {});
    GR2Object::Ptr makeReference(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeReferenceArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                      const GR2Callback& callback = {});
    GR2Object::Ptr makeReferenceVarArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                         const GR2Callback& callback = {});
    GR2Object::Ptr makeString(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeTransform(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeUInt16(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeUInt32(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeUInt8(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeVarReference(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    static GR2FileFormat readFormat(const GR2Header& header);
    void addFixup(uint32_t srcSection, GR2FixUp& fixup);
    void buildFields(const GR2TypeNode* fields, const GR2Object::Ptr& parent, uint32_t level,
                     const GR2Callback& callback = {});
    bool isArrayType(const GR2TypeNode& node) const;
    GR2RefStream getStream(const GR2Object::Ptr& parent);
    void makeRootStream();
    void readHeader();
    void readSections();

    template <TPtr T>
    T get(const GR2Reference& ref);

    template <TPtr T>
    T get(GR2RefStream& stream);

    template <TPtr T>
    T resolve(uint64_t vptr);

    template <TPtr T>
    T resolve(const GR2Reference& ref);

    template <TPtr T>
    T resolve(GR2RefStream& stream);

    template <VPtr T>
    GR2Object::Ptr processArrayReference(const GR2ArrayReference::Ptr& obj, uint32_t size, const GR2TypeNode* fields,
                                         uint32_t level, const GR2Callback& callback);

    ByteBuffer m_data;
    GR2Decompressor m_decompressor;
    GR2FileFormat m_format{UNKNOWN_FORMAT};
    GR2Header m_header{};
    GR2Stream m_rootStream;
    std::vector<GR2Object::Ptr> m_rootObjects;
    std::vector<GR2SectionHeader> m_sectionHeaders;
    std::vector<void*> m_fixups;
};

template <TPtr T>
T GR2Reader::get(const GR2Reference& ref)
{
    // Bounds check section index
    if (ref.section >= m_sectionHeaders.size()) {
        return nullptr;
    }

    auto* base = m_data.first.get() + m_sectionHeaders[ref.section].dataOffset;

    return reinterpret_cast<T>(base + ref.offset);
}

template <TPtr T>
T GR2Reader::get(GR2RefStream& stream)
{
    if (stream.isNull()) {
        return nullptr;
    }

    return stream.read<T>();
}

// Resolve a virtual pointer to an actual pointer
template <TPtr T>
T GR2Reader::resolve(uint64_t vptr)
{
    if (vptr == 0) { // null pointer
        return nullptr;
    }

    if (vptr >= m_fixups.size() + 1) {
        return nullptr;
    }

    auto ptr = m_fixups[vptr - 1];

    return static_cast<T>(ptr);
}

template <TPtr T>
T GR2Reader::resolve(const GR2Reference& ref)
{
    if (is64Bit()) {
        auto vptr = get<uint64_t*>(ref);
        if (vptr == nullptr || *vptr == 0) {
            return nullptr;
        }

        return resolve<T>(*vptr);
    }

    auto vptr = get<uint32_t*>(ref);
    if (vptr == nullptr || *vptr == 0) {
        return nullptr;
    }

    return resolve<T>(*vptr);
}

template <TPtr T>
T GR2Reader::resolve(GR2RefStream& stream)
{
    if (stream.isNull()) {
        return nullptr;
    }

    if (is64Bit()) {
        auto vptr = stream.read<uint64_t>();
        if (vptr == 0) {
            return nullptr;
        }
        return resolve<T>(vptr);
    }

    auto vptr = stream.read<uint32_t>();
    if (vptr == 0) {
        return nullptr;
    }

    return resolve<T>(vptr);
}

template <VPtr T>
GR2Object::Ptr GR2Reader::processArrayReference(const GR2ArrayReference::Ptr& obj, uint32_t size,
                                                const GR2TypeNode* fields, uint32_t level,
                                                const GR2Callback& callback)
{
    ATLASSERT(isValid(fields));

    auto* table = reinterpret_cast<T*>(obj->data);

    for (auto i = 0u; i < size; ++i) {
        auto ptr = table[i];
        obj->data = resolve<uint8_t*>(ptr);
        buildFields(fields, obj, level + 1, callback);
    }

    return obj;
}
