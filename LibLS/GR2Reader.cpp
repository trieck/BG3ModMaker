#include "pch.h"
#include "Exception.h"
#include "FileStream.h"
#include "GR2Reader.h"

void GR2Reader::read(const char* filename)
{
    m_sectionHeaders = std::vector<GR2SectionHeader>(8);

    FileStream stream;
    stream.open(filename, "rb");
    m_data = stream.read();

    readHeader();
    readFileInfo();
    readSections();
}

bool GR2Reader::isValid(const GR2TypeNode* node) const
{
    return node != nullptr && node->type > TYPE_NONE && node->type <= TYPE_MAX;
}

void GR2Reader::traverse()
{
    m_visitedNodes.clear();

    auto* node = get<GR2TypeNode*>(m_fileInfo.type);
    while (isValid(node)) {
        traverse(node, nullptr, 0);
        node++;
    }
}

std::string GR2Reader::typeToString(GR2NodeType type)
{
    switch (type) {
    case TYPE_NONE:
        return "NONE";
    case TYPE_INLINE:
        return "INLINE";
    case TYPE_REFERENCE:
        return "REFERENCE";
    case TYPE_REFERENCE_TO_ARRAY:
        return "REFERENCE_TO_ARRAY";
    case TYPE_ARRAY_OF_REFERENCES:
        return "ARRAY_OF_REFERENCES";
    case TYPE_VARIANT_REFERENCE:
        return "VARIANT_REFERENCE";
    case TYPE_REMOVED:
        return "REMOVED";
    case TYPE_REFERENCE_TO_VARIANT_ARRAY:
        return "REFERENCE_TO_VARIANT_ARRAY";
    case TYPE_STRING:
        return "STRING";
    case TYPE_TRANSFORM:
        return "TRANSFORM";
    case TYPE_REAL32:
        return "REAL32";
    case TYPE_INT8:
        return "INT8";
    case TYPE_UINT8:
        return "UINT8";
    case TYPE_BINORMAL_INT8:
        return "BINORMAL_INT8";
    case TYPE_NORMAL_UINT8:
        return "NORMAL_UINT8";
    case TYPE_INT16:
        return "INT16";
    case TYPE_UINT16:
        return "UINT16";
    case TYPE_BINORMAL_INT16:
        return "BINORMAL_INT16";
    case TYPE_NORMAL_UINT16:
        return "NORMAL_UINT16";
    case TYPE_INT32:
        return "INT32";
    case TYPE_UINT32:
        return "UINT32";
    case TYPE_REAL16:
        return "REAL16";
    case TYPE_EMPTY_REFERENCE:
        return "EMPTY_REFERENCE";
    }

    return "UNKNOWN";
}

GR2Object::Ptr GR2Reader::traverse(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level)
{
    if (!isValid(node)) {
        return nullptr;
    }

    auto* name = resolve<char*>(node->name);

    std::cout << std::string(static_cast<size_t>(level) * 3, ' ');
    std::cout << "Name: " << name
        << ", type: " << typeToString(node->type);
    if (node->arraySize != 0) {
        std::cout << ", arraySize: " << node->arraySize;
    }

    auto obj = makeObject(node, parent, level);

    if (node->type != TYPE_ARRAY_OF_REFERENCES && node->type != TYPE_REFERENCE_TO_ARRAY) {
        auto* fields = resolve<GR2TypeNode*>(node->fields);
        traverseFields(fields, obj, level + 1);
    }

    return obj;
}

void GR2Reader::traverseFields(const GR2TypeNode* fields, const GR2Object::Ptr& parent, uint32_t level)
{
    while (isValid(fields)) {
        traverse(fields, parent, level);
        fields++;
    }
}

GR2Object::Ptr GR2Reader::makeObject(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level)
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
        obj = makeArrayReference(node, parent, level);
        break;
    case TYPE_REFERENCE_TO_ARRAY:
        obj = makeReferenceArray(node, parent, level);
        break;
    case TYPE_REFERENCE_TO_VARIANT_ARRAY:
        obj = makeReferenceVarArray(node, parent, level);
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
        obj = makeTransform(node, parent, level);
        break;
    default:
        abort(); // not yet supported
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

    // Create a new object for the reference
    auto obj = std::make_shared<GR2Object>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->rootOffset = m_rootOffset;

    if (parent == nullptr) {
        obj->data = rootResolve(); // root reference
        m_rootOffset += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    } else if (parent->data) {
        obj->data = parent->data;
        parent->data += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        std::cout << "0x"
            << std::setw(16) << std::setfill('0') << std::hex
            << reinterpret_cast<uint64_t>(obj->data)
            << std::dec
            << std::setfill(' ');
    }

    std::cout << "\n";

    return obj;
}

GR2Object::Ptr GR2Reader::makeReferenceArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_REFERENCE_TO_ARRAY);

    // Create a new object for the reference to array
    auto obj = std::make_shared<GR2ReferenceArray>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->rootOffset = m_rootOffset;

    if (parent == nullptr) {
        obj->size = *rootGet<uint32_t*>();
        m_rootOffset += sizeof(uint32_t); // size
        obj->data = rootResolve<uint8_t*>(); // data pointer
        m_rootOffset += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    } else if (parent->data) {
        obj->size = *reinterpret_cast<uint32_t*>(parent->data);
        parent->data += sizeof(uint32_t); // advance parent data pointer
        obj->data = resolve<uint8_t*>(*reinterpret_cast<uint64_t*>(parent->data));
        parent->data += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        std::cout << "0x"
            << std::setw(16) << std::setfill('0') << std::hex
            << reinterpret_cast<uint64_t>(obj->data)
            << std::dec
            << std::setfill(' ');
    }

    std::cout << "\n";

    if (obj->data == nullptr) {
        return obj;
    }

    auto* fields = resolve<GR2TypeNode*>(node->fields);
    for (auto i = 0u; i < obj->size; ++i) {
        traverseFields(fields, obj, level + 1);
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeReferenceVarArray(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_REFERENCE_TO_VARIANT_ARRAY);

    // Create a new object for the reference to variant array
    auto obj = std::make_shared<GR2ReferenceVarArray>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->rootOffset = m_rootOffset;

    if (parent == nullptr) {
        obj->offset = *rootGet<uint64_t*>();
        m_rootOffset += sizeof(uint64_t); // offset
        obj->size = *rootGet<uint32_t*>();
        m_rootOffset += sizeof(uint32_t); // size
        obj->data = rootResolve(); // FIXME: handle offset
        m_rootOffset += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    } else if (parent->data) {
        obj->offset = *reinterpret_cast<uint64_t*>(parent->data);
        parent->data += sizeof(uint64_t); // advance parent data pointer
        obj->size = *reinterpret_cast<uint32_t*>(parent->data);
        parent->data += sizeof(uint32_t); // advance parent data pointer
        obj->data = resolve<uint8_t*>(*reinterpret_cast<uint64_t*>(parent->data + +obj->offset));
        // FIXME: no idea if this is correct
        parent->data += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    }

    std::cout << ", Value: ";
    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        std::cout << "0x"
            << std::setw(16) << std::setfill('0') << std::hex
            << reinterpret_cast<uint64_t>(obj->data)
            << std::dec
            << std::setfill(' ');
    }

    std::cout << "\n";

    if (obj->data == nullptr) {
        return obj;
    }

    auto* fields = resolve<GR2TypeNode*>(node->fields);
    for (auto i = 0u; i < obj->size && isValid(fields); ++i) {
        traverseFields(fields, obj, level + 1);
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeString(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_STRING);

    // Create a new object for the string
    auto obj = std::make_shared<GR2String>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->rootOffset = m_rootOffset;

    if (parent == nullptr) {
        obj->data = rootResolve(); // root string
        auto* pvalue = reinterpret_cast<const char*>(obj->data);
        obj->value = pvalue;
        m_rootOffset += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    } else if (parent->data) {
        obj->data = parent->data;
        auto* pvalue = resolve<const char*>(*reinterpret_cast<uint64_t*>(obj->data));
        obj->value = pvalue;
        parent->data += sizeof(uint64_t);
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        std::cout << "\"" << obj->value << "\"";
    }

    std::cout << "\n";

    return obj;
}

GR2Object::Ptr GR2Reader::makeTransform(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_TRANSFORM);

    // Create a new object for the transform
    auto obj = std::make_shared<GRTransform>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->rootOffset = m_rootOffset;

    if (parent == nullptr) {
        obj->data = rootResolve(); // root transform
        obj->transform = *reinterpret_cast<GR2TransformData*>(obj->data);
        m_rootOffset += sizeof(GR2TransformData);
    } else if (parent->data) {
        obj->data = parent->data;
        obj->transform = *reinterpret_cast<GR2TransformData*>(parent->data);
        parent->data += sizeof(GR2TransformData);
    }

    const auto& tr = obj->transform;

    std::cout << '\n';
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ');
    std::cout << std::fixed
        << "Flags: " << tr.flags << ",\n";
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ')
        << "Translation: (" << tr.translation[0] << ", " << tr.translation[1] << ", " << tr.translation[2] << "),\n";
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ')
        << "Rotation: (" << tr.rotation[0] << ", " << tr.rotation[1] << ", " << tr.rotation[2]
        << "),\n";
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ')
        << "Scale/Shear: (" << tr.scaleShear[0][0] << ", " << tr.scaleShear[0][1] << ", " << tr.scaleShear[0][2] << ", "
        << tr.scaleShear[1][0] << ", " << tr.scaleShear[1][1] << ", " << tr.scaleShear[1][2] << ", "
        << tr.scaleShear[2][0] << ", " << tr.scaleShear[2][1] << ", " << tr.scaleShear[2][2] << ")"
        << std::dec << '\n';


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
    obj->rootOffset = m_rootOffset;

    if (parent == nullptr) {
        obj->offset = *rootGet<uint64_t*>();
        m_rootOffset += sizeof(uint64_t);
        obj->data = rootResolve<uint8_t*>();
        m_rootOffset += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    } else if (parent->data) {
        if (is64Bit()) {
            obj->offset = *reinterpret_cast<uint64_t*>(parent->data);
            parent->data += sizeof(uint64_t);
            obj->data = resolve<uint8_t*>(*reinterpret_cast<uint64_t*>(parent->data));
            parent->data += sizeof(uint64_t);
        } else {
            obj->offset = *reinterpret_cast<uint32_t*>(parent->data);
            parent->data += sizeof(uint32_t);
            obj->data = resolve<uint8_t*>(*reinterpret_cast<uint32_t*>(parent->data));
            parent->data += sizeof(uint32_t);
        }
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        std::cout << "0x"
            << std::setw(16) << std::setfill('0') << std::hex
            << reinterpret_cast<uint64_t>(obj->data)
            << std::dec
            << std::setfill(' ');
    }

    std::cout << "\n";

    return obj;
}

GR2Object::Ptr GR2Reader::makeArrayReference(const GR2TypeNode* node, const GR2Object::Ptr& parent, uint32_t level)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_ARRAY_OF_REFERENCES);

    // Create a new object for the array of references
    auto obj = std::make_shared<GR2ArrayReference>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->rootOffset = m_rootOffset;

    uint32_t size = 0;
    if (parent == nullptr) {
        size = *rootGet<uint32_t*>();
        m_rootOffset += sizeof(uint32_t);
        obj->data = rootResolve<>();
        m_rootOffset += is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t);
    } else if (parent->data) {
        size = *reinterpret_cast<uint32_t*>(parent->data);
        parent->data += sizeof(uint32_t);
        obj->data = parent->data;
        parent->data += size * (is64Bit() ? sizeof(uint64_t) : sizeof(uint32_t));
    }

    if (obj->data != nullptr) {
        // resolve each reference in the array
        obj->data = resolve<uint8_t*>(*reinterpret_cast<uint64_t*>(obj->data));
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        std::cout << "0x"
            << std::setw(16) << std::setfill('0') << std::hex
            << reinterpret_cast<uint64_t>(obj->data)
            << std::dec
            << std::setfill(' ');
    }

    std::cout << "\n";

    if (obj->data == nullptr) {
        return obj;
    }

    auto* fields = resolve<GR2TypeNode*>(node->fields);
    for (auto i = 0u; i < size; ++i) {
        traverseFields(&fields[i], obj, level + 1);
    }

    if (size == 0) {
        std::cout << "\n";
    }

    return obj;
}

GR2Object::Ptr GR2Reader::makeInt32(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_INT32);
    ATLASSERT(parent != nullptr); // root integer not supported

    auto obj = std::make_shared<GRInt32>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->data = parent->data;
    obj->rootOffset = m_rootOffset;

    if (obj->data) {
        auto count = node->arraySize != 0 ? node->arraySize : 1;
        obj->values.resize(count);
        for (auto i = 0; i < count; ++i) {
            obj->values[i] = *reinterpret_cast<int32_t*>(parent->data);
            parent->data += sizeof(int32_t);
        }
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        printValues<int32_t>(obj->values);
    }

    std::cout << "\n";

    return obj;
}

GR2Object::Ptr GR2Reader::makeUInt8(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_UINT8);
    ATLASSERT(parent != nullptr); // root uint8 not supported

    auto obj = std::make_shared<GRUInt8>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->data = parent->data;
    obj->rootOffset = m_rootOffset;

    if (obj->data) {
        auto count = node->arraySize != 0 ? node->arraySize : 1;
        obj->values.resize(count);
        for (auto i = 0; i < count; ++i) {
            obj->values[i] = *reinterpret_cast<uint8_t*>(parent->data);
            parent->data += sizeof(uint8_t);
        }
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        printValues<uint8_t>(obj->values);
    }

    std::cout << "\n";

    return obj;
}

GR2Object::Ptr GR2Reader::makeFloat(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_REAL32);
    ATLASSERT(parent != nullptr); // root float not supported

    auto obj = std::make_shared<GRFloat>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->data = parent->data;
    obj->rootOffset = m_rootOffset;

    if (obj->data) {
        auto count = node->arraySize != 0 ? node->arraySize : 1;
        obj->values.resize(count);
        for (auto i = 0; i < count; ++i) {
            obj->values[i] = *reinterpret_cast<float*>(parent->data);
            parent->data += sizeof(float);
        }
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        printValues<float>(obj->values);
    }

    std::cout << "\n";

    return obj;
}

GR2Object::Ptr GR2Reader::makeInline(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_INLINE);

    // Create a new object for the inline node
    auto obj = std::make_shared<GR2Object>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->rootOffset = m_rootOffset;

    if (parent == nullptr) {
        obj->data = rootResolve();
    } else {
        obj->data = parent->data;
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        std::cout << "0x"
            << std::setw(16) << std::setfill('0') << std::hex
            << reinterpret_cast<uint64_t>(obj->data)
            << std::dec
            << std::setfill(' ');
    }

    std::cout << "\n";

    return obj;
}

GR2Object::Ptr GR2Reader::makeInt16(const GR2TypeNode* node, const GR2Object::Ptr& parent)
{
    ATLASSERT(isValid(node));
    ATLASSERT(node->type == TYPE_INT16);
    ATLASSERT(parent != nullptr); // root integer not supported

    auto obj = std::make_shared<GRInt16>();
    obj->typeNode = node;
    obj->parent = parent;
    obj->name = resolve<char*>(node->name);
    obj->data = parent->data;
    obj->rootOffset = m_rootOffset;

    if (obj->data) {
        auto count = node->arraySize != 0 ? node->arraySize : 1;
        obj->values.resize(count);
        for (auto i = 0; i < count; ++i) {
            obj->values[i] = *reinterpret_cast<int16_t*>(parent->data);
            parent->data += sizeof(int16_t);
        }
    }

    std::cout << ", Value: ";

    if (obj->data == nullptr) {
        std::cout << "NULL";
    } else {
        printValues<int16_t>(obj->values);
    }

    std::cout << "\n";

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
        auto& section = m_sectionHeaders[i];

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
