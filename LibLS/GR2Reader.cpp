#include "pch.h"
#include "Exception.h"
#include "FileStream.h"
#include "GR2Reader.h"

void GR2Reader::read(const char* filename)
{
    FileStream stream;
    stream.open(filename, "rb");
    m_data = stream.read();

    readHeader();
    readFileInfo();
    readSections();
    makeRootStream();
}

bool GR2Reader::isValid(const GR2TypeNode* node) const
{
    return node != nullptr && node->type > TYPE_NONE && node->type <= TYPE_MAX;
}

void GR2Reader::traverse(const GR2Callback& callback)
{
    auto* node = get<GR2TypeNode*>(m_fileInfo.type);
    while (isValid(node)) {
        traverse(node, nullptr, 0, callback);
        node++;
    }
}

const std::vector<GR2Object::Ptr>& GR2Reader::rootObjects() const
{
    return m_rootObjects;
}

GR2Object::Ptr GR2Reader::traverse(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                   const GR2Callback& callback)
{
    if (!isValid(node)) {
        return nullptr;
    }

    auto obj = makeObject(node, parent, level, callback);

    if (!isArrayType(*node)) { // Arrays handle their own fields and callbacks
        if (callback) {
            callback({.object = obj, .level = level});
        }

        auto* fields = resolve<GR2TypeNode*>(node->fields);
        traverseFields(fields, obj, level + 1, callback);
    }

    return obj;
}

void GR2Reader::traverseFields(const GR2TypeNode* fields, const GR2Object::Ptr& parent, uint32_t level,
                               const GR2Callback& callback)
{
    while (isValid(fields)) {
        traverse(fields, parent, level, callback);
        fields++;
    }
}

void GR2Reader::makeRootStream()
{
    const auto& root = m_fileInfo.root;
    auto* base = m_data.first.get() + m_sectionHeaders[root.section].dataOffset + root.offset;

    m_rootStream = GR2Stream(base);
}

GR2RefStream GR2Reader::getStream(const GR2Object::Ptr& parent)
{
    auto* data = parent == nullptr ? &m_rootStream.data() : &parent->data;

    return GR2RefStream(data);
}

bool GR2Reader::isArrayType(const GR2TypeNode& node) const
{
    return node.type == TYPE_ARRAY_OF_REFERENCES || node.type == TYPE_REFERENCE_TO_ARRAY ||
        node.type == TYPE_REFERENCE_TO_VARIANT_ARRAY;
}

GR2Object::Ptr GR2Reader::makeObject(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                     const GR2Callback& callback)
{
    GR2Object::Ptr obj = nullptr;

    switch (node->type) {
    case TYPE_INLINE:
        obj = makeInline(node, parent);
        break;
    case TYPE_REFERENCE:
        obj = makeReference(node, parent);
        break;
    case TYPE_VARIANT_REFERENCE:
        obj = makeVarReference(node, parent);
        break;
    case TYPE_ARRAY_OF_REFERENCES:
        obj = makeArrayReference(node, parent, level, callback);
        break;
    case TYPE_REFERENCE_TO_ARRAY:
        obj = makeReferenceArray(node, parent, level, callback);
        break;
    case TYPE_REFERENCE_TO_VARIANT_ARRAY:
        obj = makeReferenceVarArray(node, parent, level, callback);
        break;
    case TYPE_STRING:
        obj = makeString(node, parent);
        break;
    case TYPE_INT16:
        obj = makeInt16(node, parent);
        break;
    case TYPE_INT32:
        obj = makeInt32(node, parent);
        break;
    case TYPE_REAL32:
        obj = makeFloat(node, parent);
        break;
    case TYPE_UINT8:
        obj = makeUInt8(node, parent);
        break;
    case TYPE_TRANSFORM:
        obj = makeTransform(node, parent);
        break;
    default:
        throw Exception(std::format("Unsupported GR2 node type: {}", static_cast<uint32_t>(node->type)));
    }

    if (parent == nullptr) {
        m_rootObjects.emplace_back(obj);
    } else {
        parent->children.emplace_back(obj);
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeReference(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_REFERENCE);

    auto stream = getStream(parent);

    auto obj = std::make_shared<GR2Object>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->data = resolve<uint8_t*>(stream);

    return obj;
}

GR2Object::Ptr GR2Reader::makeReferenceArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                             const GR2Callback& callback)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_REFERENCE_TO_ARRAY);

    auto obj = std::make_shared<GR2ReferenceArray>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto stream = getStream(parent);
    if (!stream.isNull()) {
        obj->size = stream.read<uint32_t>();
        obj->data = resolve<uint8_t*>(stream);
    }

    if (callback) {
        callback({.object = obj, .level = level});
    }

    auto* fields = resolve<GR2TypeNode*>(node->fields);
    if (!isValid(fields)) {
        return obj;
    }

    for (auto i = 0u; i < obj->size; ++i) {
        traverseFields(fields, obj, level + 1, callback);
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeReferenceVarArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                                const GR2Callback& callback)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_REFERENCE_TO_VARIANT_ARRAY);

    auto obj = std::make_shared<GR2ReferenceVarArray>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto stream = getStream(parent);
    if (!stream.isNull()) {
        if (is64Bit()) {
            obj->offset = stream.read<uint64_t>();
        } else {
            obj->offset = stream.read<uint32_t>();
        }

        obj->size = stream.read<uint32_t>();
        obj->data = resolve<uint8_t*>(stream) + obj->offset;
    }

    if (callback) {
        callback({.object = obj, .level = level});
    }

    auto* fields = resolve<GR2TypeNode*>(node->fields);
    if (!isValid(fields)) {
        return obj;
    }

    for (auto i = 0u; i < obj->size; ++i) {
        traverseFields(fields, obj, level + 1, callback);
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeString(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_STRING);

    auto stream = getStream(parent);

    auto obj = std::make_shared<GR2String>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->data = resolve<uint8_t*>(stream);
    if (obj->data == nullptr) {
        return obj;
    }

    obj->value = reinterpret_cast<const char*>(obj->data);

    return obj;
}

GR2Object::Ptr GR2Reader::makeTransform(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_TRANSFORM);

    auto obj = std::make_shared<GRTransform>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto stream = getStream(parent);
    if (stream.isNull()) {
        return obj;
    }

    obj->transform = stream.read<GR2TransformData>();
    obj->data = reinterpret_cast<uint8_t*>(&obj->transform);

    return obj;
}

GR2Object::Ptr GR2Reader::makeVarReference(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_VARIANT_REFERENCE);

    auto obj = std::make_shared<GR2VarReference>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto stream = getStream(parent);
    if (stream.isNull()) {
        return obj;
    }

    if (is64Bit()) {
        obj->offset = stream.read<uint64_t>();
    } else {
        obj->offset = stream.read<uint32_t>();
    }

    obj->data = resolve<uint8_t*>(stream) + obj->offset;

    return obj;
}

GR2Object::Ptr GR2Reader::makeArrayReference(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level,
                                             const GR2Callback& callback)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_ARRAY_OF_REFERENCES);

    auto obj = std::make_shared<GR2ArrayReference>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto stream = getStream(parent);
    if (stream.isNull()) {
        return obj;
    }

    auto size = stream.read<uint32_t>();

    obj->data = resolve<uint8_t*>(stream);

    if (callback) {
        callback({.object = obj, .level = level});
    }

    if (obj->data == nullptr) {
        return obj;
    }

    auto* pdata = reinterpret_cast<uint64_t*>(obj->data);
    auto* fields = resolve<GR2TypeNode*>(node->fields);
    if (!isValid(fields)) {
        return obj;
    }

    for (auto i = 0u; i < size; ++i) {
        auto ptr = pdata[i];
        obj->data = resolve<uint8_t*>(ptr);
        traverseFields(fields, obj, level + 1, callback);
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeInt16(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_INT16);

    auto obj = std::make_shared<GRInt16>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto count = node->arraySize != 0 ? node->arraySize : 1;
    if (count == 0) {
        return obj;
    }

    auto stream = getStream(parent);
    if (stream.isNull()) {
        return obj;
    }

    obj->values.resize(count);

    for (auto i = 0; i < count; ++i) {
        obj->values[i] = stream.read<int16_t>();
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeInt32(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_INT32);

    auto obj = std::make_shared<GRInt32>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto count = node->arraySize != 0 ? node->arraySize : 1;
    if (count == 0) {
        return obj;
    }

    auto stream = getStream(parent);
    if (stream.isNull()) {
        return obj;
    }

    obj->values.resize(count);

    for (auto i = 0; i < count; ++i) {
        obj->values[i] = stream.read<int32_t>();
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeUInt8(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_UINT8);

    auto obj = std::make_shared<GRUInt8>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto count = node->arraySize != 0 ? node->arraySize : 1;
    if (count == 0) {
        return obj;
    }

    auto stream = getStream(parent);
    if (stream.isNull()) {
        return obj;
    }

    obj->values.resize(count);

    for (auto i = 0; i < count; ++i) {
        obj->values[i] = stream.read<uint8_t>();
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeFloat(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_REAL32);

    auto obj = std::make_shared<GRFloat>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto count = node->arraySize != 0 ? node->arraySize : 1;
    if (count == 0) {
        return obj;
    }

    auto stream = getStream(parent);
    if (stream.isNull()) {
        return obj;
    }

    obj->values.resize(count);

    for (auto i = 0; i < count; ++i) {
        obj->values[i] = stream.read<float>();
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeInline(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_INLINE);

    auto obj = std::make_shared<GR2Object>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);

    auto stream = getStream(parent);
    obj->data = get<uint8_t*>(stream);

    return obj;
}

bool GR2Reader::is64Bit() const
{
    return m_flags & FLAG_64_BIT;
}

bool GR2Reader::isExtra16() const
{
    return m_flags & FLAG_EXTRA_16;
}

void GR2Reader::readHeader()
{
    memcpy(&m_header, m_data.first.get(), sizeof(GR2Header));

    bool found = false;
    for (const auto& magicInfo : MAGIC) {
        if (std::equal(std::begin(magicInfo.magic), std::end(magicInfo.magic), std::begin(m_header.magic))) {
            // Valid magic found
            m_flags = magicInfo.flags;
            found = true;
            break;
        }
    }

    if (!found) {
        throw Exception("Invalid GR2 file magic.");
    }

    if (m_flags & FLAG_BIG_ENDIAN) {
        throw Exception("Big-endian GR2 files are not supported.");
    }

    if (m_header.format != 0) {
        throw Exception("Unsupported GR2 file format version.");
    }
}

void GR2Reader::readFileInfo()
{
    auto* pdata = m_data.first.get() + sizeof(GR2Header);
    memcpy(&m_fileInfo, pdata, sizeof(GR2FileInfo));

    if (m_fileInfo.format != 6 && m_fileInfo.format != 7) {
        throw Exception("Unsupported GR2 file format.");
    }

    if (m_fileInfo.fileInfoSize != sizeof(GR2FileInfo)) {
        throw Exception("GR2 file info size does not match expected size.");
    }
}

void GR2Reader::readSections()
{
    m_sectionHeaders = std::vector<GR2SectionHeader>(m_fileInfo.sectionCount);

    auto* pdata = m_data.first.get() + sizeof(GR2Header) + sizeof(GR2FileInfo);

    // read all section headers
    for (auto& section : m_sectionHeaders) {
        memcpy(&section, pdata, sizeof(GR2SectionHeader));

        if (section.compressType != COMPRESSION_NONE) {
            throw Exception("Compressed GR2 sections are not supported.");
        }
        pdata += sizeof(GR2SectionHeader);
    }

    // TODO: marshalling 

    // make fixup table and apply fixups
    pdata = m_data.first.get();

    for (auto i = 0u; i < m_sectionHeaders.size(); ++i) {
        const auto& section = m_sectionHeaders[i];
        for (auto j = 0u; j < section.fixupSize; ++j) {
            auto* fixup = reinterpret_cast<GR2FixUp*>(pdata + section.fixupOffset + j * sizeof(GR2FixUp));
            addFixup(i, *fixup);
        }
    }
}

void GR2Reader::addFixup(uint32_t srcSection, GR2FixUp& fixup)
{
    auto pdata = m_data.first.get();
    auto* srcPtr = pdata + m_sectionHeaders[srcSection].dataOffset + fixup.srcOffset;
    auto* dstPtr = pdata + m_sectionHeaders[fixup.dstSection].dataOffset + fixup.dstOffset;

    m_fixups.emplace_back(dstPtr);
    auto index = static_cast<uint32_t>(m_fixups.size()); // 1-based index

    if (is64Bit()) {
        uint64_t index64 = index;
        memcpy(srcPtr, &index64, sizeof(uint64_t));
    } else {
        memcpy(srcPtr, &index, sizeof(uint32_t));
    }
}
