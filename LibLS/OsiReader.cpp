#include "pch.h"
#include "Exception.h"
#include "OsiReader.h"

OsiReader::OsiReader()
{
}

OsiReader::~OsiReader()
{
}

bool OsiReader::shortTypeIds() const
{
    return m_shortTypeIds;
}

OsiReader::OsiReader(OsiReader&& rhs) noexcept
{
    *this = std::move(rhs);
}

OsiReader& OsiReader::operator=(OsiReader&& rhs) noexcept
{
    if (this != &rhs) {
        m_file = std::move(rhs.m_file);
        m_story = std::move(rhs.m_story);
    }

    return *this;
}

bool OsiReader::readFile(const char* filename)
{
    m_file.open(filename, "rb");
    m_file.read<uint8_t>(); // unused byte

    m_scramble = 0x00;
    m_story.header.version = readString();
    m_story.header.majorVersion = m_file.read<uint8_t>();
    m_story.header.minorVersion = m_file.read<uint8_t>();
    m_story.header.bigEndian = m_file.read<uint8_t>() != 0;
    m_file.read<uint8_t>(); // unused byte

    auto version = m_story.version();
    if (version > OsiVersion::LAST_SUPPORTED) {
        throw std::runtime_error("Unsupported Osi version");
    }

    if (version >= OsiVersion::ADD_VERSION_STRING) {
        m_story.header.versionString = m_file.read(0x80).str(); // version string buffer
    } else {
        m_story.header.versionString.clear();
    }

    if (version >= OsiVersion::ADD_DEBUG_FLAGS) {
        m_story.header.debugFlags = m_file.read<uint32_t>();
    } else {
        m_story.header.debugFlags = 0;
    }

    if (version < OsiVersion::REMOVE_EXTERNAL_STRING_TABLE) {
        m_shortTypeIds = false;
    } else if (version >= OsiVersion::ENUMS) {
        m_shortTypeIds = true;
    }

    if (version >= OsiVersion::SCRAMBLED) {
        m_scramble = 0xAD;
    } else {
        m_scramble = 0x00;
    }

    readTypes();
    readStringTable();
    makeBuiltins();
    readEnums();
    readDivObjects();
    readFunctions();
    readNodes();

    return true;
}

size_t OsiReader::read(char* buf, size_t size)
{
    return m_file.read(buf, size);
}

size_t OsiReader::write(const char* buf, size_t size)
{
    return m_file.write(buf, size);
}

void OsiReader::seek(int64_t offset, SeekMode mode)
{
    m_file.seek(offset, mode);
}

size_t OsiReader::tell() const
{
    return m_file.tell();
}

size_t OsiReader::size() const
{
    return m_file.size();
}

OsiDivObject OsiReader::readDivObject()
{
    OsiDivObject obj{};
    obj.name = readString();
    obj.type = m_file.read<uint8_t>();
    m_file.read(obj.keys, 4);

    return obj;
}

void OsiReader::readTypes()
{
    auto count = m_file.read<uint32_t>();

    m_story.types.clear();
    m_story.typeAliases.clear();

    // Read types
    for (auto i = 0u; i < count; ++i) {
        auto type = readType();
        m_story.types[type.index] = type;
    }

    if (m_story.version() < OsiVersion::TYPE_ALIASES) {
        return;
    }

    // Resolve alias chains
    for (const auto& [index, type] : m_story.types) {
        if (type.alias == 0) {
            continue;
        }

        auto alias = type.alias;
        while (true) {
            const auto& aliasType = m_story.types.at(alias);
            if (aliasType.alias == 0) {
                break;
            }
            alias = aliasType.alias;
        }
        m_story.typeAliases[index] = alias;
    }
}

void OsiReader::readStringTable()
{
    m_story.stringTable.clear();

    if (m_story.version() >= OsiVersion::EXTERNAL_STRING_TABLE &&
        m_story.version() < OsiVersion::REMOVE_EXTERNAL_STRING_TABLE) {
        m_story.stringTable = readStrings();
    }
}

void OsiReader::makeBuiltins()
{
    m_story.types[0] = OsirisType{.name = "UNKNOWN", .index = 0, .builtIn = true, .alias = 0};
    m_story.types[1] = OsirisType{.name = "INTEGER", .index = 1, .builtIn = true, .alias = 0};

    if (m_story.version() >= OsiVersion::ENHANCED_TYPES) {
        m_story.types[2] = OsirisType{.name = "INT64", .index = 2, .builtIn = true, .alias = 0};
        m_story.types[3] = OsirisType{.name = "REAL", .index = 3, .builtIn = true, .alias = 0};
        m_story.types[4] = OsirisType{.name = "STRING", .index = 4, .builtIn = true, .alias = 0};

        if (!m_story.types.contains(5)) {
            m_story.types[5] = OsirisType{.name = "GUIDSTRING", .index = 5, .builtIn = true, .alias = 0};
        }
    } else {
        m_story.types[2] = OsirisType{.name = "FLOAT", .index = 2, .builtIn = true, .alias = 0};
        m_story.types[3] = OsirisType{.name = "STRING", .index = 3, .builtIn = true, .alias = 0};
    }

    // TODO: populate custom type ids for versions without type aliases
}

void OsiReader::readEnums()
{
    m_story.enums.clear();

    if (m_story.version() < OsiVersion::ENUMS) {
        return;
    }

    auto count = m_file.read<uint32_t>();
    for (auto i = 0u; i < count; ++i) {
        auto e = readEnum();
        m_story.enums[e.type] = e;
    }
}

OsiEnum OsiReader::readEnum()
{
    OsiEnum e{};

    e.type = m_file.read<uint16_t>();

    auto elements = m_file.read<uint32_t>();
    for (auto i = 0u; i < elements; ++i) {
        auto name = readString();
        auto value = m_file.read<uint64_t>();
        e.elements[name] = value;
    }

    return e;
}

OsiFunction OsiReader::readFunction()
{
    OsiFunction func{};
    func.line = m_file.read<uint32_t>();
    func.conditionRef = m_file.read<uint32_t>();
    func.actionRef = m_file.read<uint32_t>();
    func.nodeRef = m_file.read<uint32_t>();
    func.type = static_cast<OsiFunctionType>(m_file.read<uint8_t>());

    m_file.read(func.meta, 4);

    func.name = readFunctionSig();

    return func;
}

OsiFunctionSig OsiReader::readFunctionSig()
{
    OsiFunctionSig sig{};
    sig.name = readString();

    auto outParamBytes = m_file.read<uint32_t>();
    sig.outParamMask.resize(outParamBytes);
    m_file.read(sig.outParamMask.data(), outParamBytes);

    auto count = m_file.read<uint8_t>();
    sig.parameter.types.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        if (m_shortTypeIds) {
            sig.parameter.types.emplace_back(m_file.read<uint16_t>());
        } else {
            sig.parameter.types.emplace_back(m_file.read<uint32_t>());
        }
    }

    return sig;
}

void OsiReader::readDivObjects()
{
    m_story.divObjects.clear();

    auto count = m_file.read<uint32_t>();
    m_story.divObjects.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        m_story.divObjects.emplace_back(readDivObject());
    }
}

void OsiReader::readFunctions()
{
    m_story.functions.clear();

    auto count = m_file.read<uint32_t>();
    m_story.functions.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        m_story.functions.emplace_back(readFunction());
    }
}

void OsiReader::readNodes()
{
    m_story.nodes.clear();

    auto count = m_file.read<uint32_t>();
    m_story.nodes.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        auto node = readNode();
        m_story.nodes[node->index] = std::move(node);
    }
}

OsiNode::Ptr OsiReader::readNode()
{
    OsiNode::Ptr node;

    auto type = static_cast<OsiNodeType>(m_file.peek<uint8_t>());
    switch (type) {
    case NT_DATABASE:
        node = std::make_unique<OsiDBNode>();
        break;
    case NT_PROC:
        node = std::make_unique<OsiProcNode>();
        break;
    case NT_RULE:
        node = std::make_unique<OsiRuleNode>();
        break;
    case NT_AND:
        node = std::make_unique<OsiAndNode>();
        break;
    case NT_DIV_QUERY:
        node = std::make_unique<OsiDivQueryNode>();
        break;
    default:
        throw Exception("Unsupported node type {}.", static_cast<int>(type));
    }

    node->read(*this);

    return node;
}

OsirisType OsiReader::readType()
{
    OsirisType type{};
    type.name = readString();
    type.index = m_file.read<uint8_t>();

    if (m_story.version() >= OsiVersion::TYPE_ALIASES) {
        type.alias = m_file.read<uint8_t>();
    } else {
        type.alias = 0; // TODO:
    }

    return type;
}

std::string OsiReader::readString()
{
    std::string s;

    while (true) {
        auto c = m_file.read<uint8_t>() ^ m_scramble;
        if (c == '\0') {
            break;
        }
        s += static_cast<char>(c);
    }

    return s;
}

OsiVersion OsiReader::version() const
{
    return m_story.version();
}

std::vector<std::string> OsiReader::readStrings()
{
    auto count = m_file.read<uint32_t>();

    std::vector<std::string> strings;
    strings.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        strings.emplace_back(readString());
    }

    return strings;
}
