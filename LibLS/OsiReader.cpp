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

bool OsiReader::isOsiFile(StreamBase& stream)
{
    stream.seek(0, SeekMode::Begin);

    uint8_t sig[17]{};
    stream.read(sig, 17);

    return std::memcmp(sig, "\0Osiris save file", 17) == 0;
}

bool OsiReader::isOsiFile(const ByteBuffer& contents)
{
    if (contents.second < 17) {
        return false;
    }

    return std::memcmp(contents.first.get(), "\0Osiris save file", 17) == 0;
}

bool OsiReader::getEnum(uint16_t type, OsiEnum& osiEnum)
{
    auto it = m_story.enums.find(static_cast<uint16_t>(type));
    if (it != m_story.enums.end()) {
        osiEnum = it->second;
        return true;
    }

    return false;
}

bool OsiReader::isAlias(uint32_t type) const
{
    return m_story.isAlias(type);
}

OsiValueType OsiReader::resolveAlias(OsiValueType type) const
{
    return m_story.resolveAlias(type);
}

OsiReader::OsiReader(OsiReader&& rhs) noexcept
{
    *this = std::move(rhs);
}

OsiReader& OsiReader::operator=(OsiReader&& rhs) noexcept
{
    if (this != &rhs) {
        m_stream = std::move(rhs.m_stream);
        m_story = std::move(rhs.m_story);
    }

    return *this;
}

void OsiReader::read(StreamBase& stream)
{
    m_stream = Stream::makeStream(stream);
    read();
}

void OsiReader::read(const ByteBuffer& buffer)
{
    m_stream = Stream::makeStream(buffer);
    read();
}

size_t OsiReader::read(char* buf, size_t size)
{
    return m_stream.read(buf, size);
}

size_t OsiReader::write(const char* buf, size_t size)
{
    return m_stream.write(buf, size);
}

void OsiReader::seek(int64_t offset, SeekMode mode)
{
    m_stream.seek(offset, mode);
}

size_t OsiReader::tell() const
{
    return m_stream.tell();
}

size_t OsiReader::size() const
{
    return m_stream.size();
}

void OsiReader::readTypes()
{
    auto count = m_stream.read<uint32_t>();

    m_story.types.clear();
    m_story.typeAliases.clear();

    // Read types
    for (auto i = 0u; i < count; ++i) {
        OsiType type{};
        type.read(*this);
        m_story.types[type.index] = std::move(type);
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

void OsiReader::resolve()
{
    for (auto& node : m_story.nodes) {
        node->resolve(m_story);
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
    m_story.types[0] = makeBuiltin("UNKNOWN", 0);
    m_story.types[1] = makeBuiltin("INTEGER", 1);
    if (m_story.version() >= OsiVersion::ENHANCED_TYPES) {
        m_story.types[2] = makeBuiltin("INT64", 2);
        m_story.types[3] = makeBuiltin("REAL", 3);
        m_story.types[4] = makeBuiltin("STRING", 4);
        if (!m_story.types.contains(5)) {
            m_story.types[5] = makeBuiltin("GUIDSTRING", 5);
        }
    } else {
        m_story.types[2] = makeBuiltin("FLOAT", 2);
        m_story.types[3] = makeBuiltin("STRING", 3);
    }

    // TODO: populate custom type ids for versions without type aliases
}

void OsiReader::read()
{
    m_stream.read<uint8_t>(); // unused byte
    m_scramble = 0x00;
    m_story.header.version = readString();
    m_story.header.majorVersion = m_stream.read<uint8_t>();
    m_story.header.minorVersion = m_stream.read<uint8_t>();
    m_story.header.bigEndian = m_stream.read<uint8_t>() != 0;
    m_stream.read<uint8_t>(); // unused byte

    auto version = m_story.version();
    if (version > OsiVersion::LAST_SUPPORTED) {
        throw Exception("Unsupported Osi version");
    }

    if (version >= OsiVersion::ADD_VERSION_STRING) {
        m_story.header.versionString = m_stream.read(0x80).str(); // version string buffer
    } else {
        m_story.header.versionString.clear();
    }

    if (version >= OsiVersion::ADD_DEBUG_FLAGS) {
        m_story.header.debugFlags = m_stream.read<uint32_t>();
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
    readAdapters();
    readDatabases();
    readGoals();
    readGlobalActions();
    resolve();
}

void OsiReader::readAdapters()
{
    m_story.adapters.clear();

    auto count = m_stream.read<uint32_t>();

    m_story.adapters.resize(count);
    for (auto i = 0u; i < count; ++i) {
        OsiAdapter adapter{};
        adapter.read(*this);
        m_story.adapters[adapter.index] = std::move(adapter);
    }
}

void OsiReader::readDatabases()
{
    m_story.databases.clear();

    auto count = m_stream.read<uint32_t>();

    m_story.databases.resize(count);
    for (auto i = 0u; i < count; ++i) {
        OsiDatabase db{};
        db.read(*this);
        m_story.databases[db.index] = std::move(db);
    }
}

void OsiReader::readEnums()
{
    m_story.enums.clear();

    if (m_story.version() < OsiVersion::ENUMS) {
        return;
    }

    auto count = m_stream.read<uint32_t>();
    for (auto i = 0u; i < count; ++i) {
        OsiEnum e{};
        e.read(*this);
        m_story.enums[e.type] = std::move(e);
    }
}

void OsiReader::readDivObjects()
{
    m_story.divObjects.clear();

    auto count = m_stream.read<uint32_t>();
    m_story.divObjects.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        OsiDivObject object{};
        object.read(*this);
        m_story.divObjects.emplace_back(std::move(object));
    }
}

void OsiReader::readFunctions()
{
    m_story.functions.clear();
    m_story.functionNames.clear();

    auto count = m_stream.read<uint32_t>();
    m_story.functions.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        OsiFunction func{};
        func.read(*this);
        auto& f = m_story.functions.emplace_back(std::move(func));
        m_story.functionNames[f.name.name] = &f;
    }
}

void OsiReader::readGlobalActions()
{
    m_story.globalActions.clear();

    auto count = m_stream.read<uint32_t>();
    m_story.globalActions.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        OsiCall action{};
        action.read(*this);
        m_story.globalActions.emplace_back(std::move(action));
    }
}

void OsiReader::readGoals()
{
    m_story.goals.clear();
    auto count = m_stream.read<uint32_t>();

    m_story.goals.resize(count);
    for (auto i = 0u; i < count; ++i) {
        OsiGoal goal{};
        goal.read(*this);
        m_story.goals[goal.index] = std::move(goal);
    }
}

void OsiReader::readNodes()
{
    m_story.nodes.clear();

    auto count = m_stream.read<uint32_t>();
    m_story.nodes.resize(count);

    for (auto i = 0u; i < count; ++i) {
        auto node = readNode();
        m_story.nodes[node->index] = std::move(node);
    }
}

OsiNode::Ptr OsiReader::readNode()
{
    OsiNode::Ptr node;

    auto type = static_cast<OsiNodeType>(m_stream.peek<uint8_t>());
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
    case NT_NAND:
        node = std::make_unique<OsiNandNode>();
        break;
    case NT_USER_QUERY:
        node = std::make_unique<OsiUserQueryNode>();
        break;
    case NT_REL_OP:
        node = std::make_unique<OsiRelOpNode>();
        break;
    case NT_INTERNAL_QUERY:
        node = std::make_unique<OsiInternalQueryNode>();
        break;
    default:
        throw Exception("Unsupported node type {}.", static_cast<int>(type));
    }

    node->read(*this);

    return node;
}

OsiType OsiReader::makeBuiltin(const std::string& name, uint8_t index) const
{
    OsiType type{};
    type.name = name;
    type.index = index;
    type.builtIn = true;
    type.alias = 0;
    return type;
}

std::string OsiReader::readString()
{
    std::string s;

    while (true) {
        auto c = m_stream.read<uint8_t>() ^ m_scramble;
        if (c == '\0') {
            break;
        }
        s += static_cast<char>(c);
    }

    return s;
}

OsiStory OsiReader::takeStory() &&
{
    return std::move(m_story);
}

const OsiStory& OsiReader::story() const
{
    return m_story;
}

OsiVersion OsiReader::version() const
{
    return m_story.version();
}

std::vector<std::string> OsiReader::readStrings()
{
    auto count = m_stream.read<uint32_t>();

    std::vector<std::string> strings;
    strings.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        strings.emplace_back(readString());
    }

    return strings;
}
