#pragma once

enum GR2MagicFlags : uint8_t
{
    FLAG_NONE = 0,
    FLAG_BIG_ENDIAN = 1 << 0,
    FLAG_64_BIT = 1 << 1,
    FLAG_EXTRA_16 = 1 << 2,
};

struct GR2MagicInfo
{
    uint8_t flags;
    uint32_t magic[4];
};

static constexpr GR2MagicInfo MAGIC[] = {
    /* Little Endian 32-bit File Format 6 */
    {
        .flags = FLAG_NONE,
        .magic = {3400558520, 263286264, 2123133572, 503322974}
    },
    /* Big Endian 32-bit File Format 6 */
    {
        .flags = FLAG_BIG_ENDIAN,
        .magic = {3093803210, 4167938319, 2222099582, 1578696734}
    },
    {
        .flags = FLAG_EXTRA_16,
        .magic = {3228360233, 726901946, 2780296485, 4007814902},
    },
    /* Little Endian 32-bit File Format 7 (Granny 2.9) */
    {
        .flags = FLAG_EXTRA_16 | FLAG_64_BIT,
        .magic = {1581882341, 337601391, 2850755358, 3303915152},
    }
};

enum GR2CompressionType : uint32_t
{
    COMPRESSION_NONE = 0, /* No compression */
    COMPRESSION_OODLE0,
    COMPRESSION_OODLE1,
    COMPRESSION_BITKNIT1,
    COMPRESSION_BITKNIT2
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

struct GR2Header
{
    uint32_t magic[4];
    uint32_t size; // size of the file including sectors
    uint32_t format; // format of the file
    uint8_t extra[8]; // extra data
};

struct GR2Reference
{
    uint32_t section; // section number
    uint32_t offset; // offset within section
};

struct GR2FileInfo
{
    int32_t format; // format of the file
    uint32_t totalSize; // total size of the file
    uint32_t crc32; // CRC32 checksum
    uint32_t fileInfoSize; // size of the file info
    uint32_t sectionCount; // number of section
    GR2Reference type; // reference where the type node is located
    GR2Reference root; // reference where the root node is located
    uint32_t tag; // version of the type node
    uint8_t extra[32]; // extra data
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

    uint32_t rootOffset{0}; // instance offset for root objects
    uint8_t* data{nullptr}; // transient data pointer while traversing
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
    uint64_t offset;
    uint32_t size;
};

struct GR2String : GR2Object
{
    std::string value;
};

struct GRInt16 : GR2Object
{
    std::vector<int16_t> values;
};

struct GRInt32 : GR2Object
{
    std::vector<int32_t> values;
};

struct GRFloat : GR2Object
{
    std::vector<float> values;
};

struct GRUInt8 : GR2Object
{
    std::vector<uint8_t> values;
};

struct GRTransform : GR2Object
{
    GR2TransformData transform{};
};

class GR2Reader
{
public:
    GR2Reader() = default;
    ~GR2Reader() = default;

    void read(const char* filename);
    void traverse();

private:
    template <typename T>
    T get(const GR2Reference& ref);

    template <typename T>
    T resolve(uint64_t vptr);

    template <typename T>
    T resolve(const GR2Reference& ref);

    template <typename T = uint8_t*>
    T rootGet();

    template <typename T = uint8_t*>
    T rootResolve();

    template <typename T>
    void printValues(const std::vector<T>& values);

    bool is64Bit() const;
    bool isExtra16() const;
    bool isValid(const GR2TypeNode* node) const;
    GR2Object::Ptr makeArrayReference(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level);
    GR2Object::Ptr makeFloat(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeInline(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeInt16(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeInt32(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeObject(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level);
    GR2Object::Ptr makeReference(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeReferenceArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level);
    GR2Object::Ptr makeReferenceVarArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level);
    GR2Object::Ptr makeString(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeTransform(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level);
    GR2Object::Ptr makeUInt8(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr makeVarReference(const GR2TypeNode* node, const GR2Object::Ptr& parent);
    GR2Object::Ptr traverse(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level);
    std::string typeToString(GR2NodeType type);
    void addFixup(uint32_t srcSection, GR2FixUp& fixup);
    void readFileInfo();
    void readHeader();
    void readSections();
    void traverseFields(const GR2TypeNode* fields, const GR2Object::Ptr& parent, uint32_t level);

    ByteBuffer m_data;
    GR2FileInfo m_fileInfo{};
    GR2Header m_header{};
    std::unordered_set<const GR2TypeNode*> m_visitedNodes;
    std::vector<GR2SectionHeader> m_sectionHeaders;
    std::vector<void*> m_fixups;

    uint8_t m_flags{0};
    uint32_t m_rootOffset{0};

    std::vector<GR2Object::Ptr> m_rootObjects;
};

template <typename T>
T GR2Reader::get(const GR2Reference& ref)
{
    // Bounds check section index
    if (ref.section >= m_sectionHeaders.size()) {
        return nullptr;
    }

    auto* base = m_data.first.get() + m_sectionHeaders[ref.section].dataOffset;

    return reinterpret_cast<T>(base + ref.offset);
}

// Resolve a virtual pointer to an actual pointer
template <typename T>
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

template <typename T>
T GR2Reader::resolve(const GR2Reference& ref)
{
    auto vptr = get<uint64_t*>(ref); // TODO: has to handle 32-bit pointers too

    if (vptr == nullptr || *vptr == 0) {
        return nullptr;
    }

    return resolve<T>(*vptr);
}

template <typename T>
T GR2Reader::rootGet()
{
    return get<T>(
        {.section = m_fileInfo.root.section, .offset = m_rootOffset}
    );
}

template <typename T>
T GR2Reader::rootResolve()
{
    return resolve<T>(
        {.section = m_fileInfo.root.section, .offset = m_rootOffset}
    );
}

template <typename T>
void GR2Reader::printValues(const std::vector<T>& values)
{
    if (values.size() == 1) {
        if constexpr (std::is_floating_point_v<T>) {
            std::cout << std::fixed << values[0];
        } else {
            std::cout << +values[0];
        }
    } else {
        std::cout << "[ ";
        for (auto i = 0u; i < values.size(); ++i) {
            if constexpr (std::is_floating_point_v<T>) {
                std::cout << std::fixed;
            }

            if constexpr (std::is_floating_point_v<T>) {
                std::cout << std::fixed << values[i];
            } else {
                std::cout << +values[i];
            }

            if (i < values.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << " ]";
    }

    if constexpr (std::is_floating_point_v<T>) {
        std::cout.unsetf(std::ios::floatfield);
    }
}
