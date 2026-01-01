#include "pch.h"
#include "Exception.h"
#include "OsiReader.h"
#include "OsiStory.h"

OsiStory::OsiStory()
{
}

OsiStory::OsiStory(OsiStory&& rhs) noexcept
{
    *this = std::move(rhs);
}

OsiStory& OsiStory::operator=(OsiStory&& rhs) noexcept
{
    if (this != &rhs) {
        adapters = std::move(rhs.adapters);
        databases = std::move(rhs.databases);
        divObjects = std::move(rhs.divObjects);
        enums = std::move(rhs.enums);
        functionSigs = std::move(rhs.functionSigs);
        functions = std::move(rhs.functions);
        globalActions = std::move(rhs.globalActions);
        goals = std::move(rhs.goals);
        header = std::move(rhs.header);
        nodes = std::move(rhs.nodes);
        stringTable = std::move(rhs.stringTable);
        typeAliases = std::move(rhs.typeAliases);
        types = std::move(rhs.types);
    }

    return *this;
}

OsiVersion OsiStory::version() const
{
    return static_cast<OsiVersion>(
        (static_cast<uint16_t>(header.majorVersion) << 8) |
        static_cast<uint16_t>(header.minorVersion));
}

bool OsiStory::isAlias(uint32_t type) const
{
    return typeAliases.contains(static_cast<uint8_t>(type));
}

OsiValueType OsiStory::resolveAlias(OsiValueType type) const
{
    auto it = typeAliases.find(static_cast<uint8_t>(type));
    if (it != typeAliases.end()) {
        return static_cast<OsiValueType>(it->second);
    }

    return type;
}

std::string OsiStory::typeName(OsiValueType typeId) const
{
    auto typeId8 = static_cast<uint8_t>(typeId);

    auto it = types.find(typeId8);
    if (it != types.end()) {
        return it->second.name;
    }

    return std::format("Type{}", typeId8);
}

std::string OsiStory::typeName(uint32_t typeId) const
{
    return typeName(static_cast<OsiValueType>(typeId));
}

StreamBase& OsiStory::decompile(const OsiGoal& goal, StreamBase& stream)
{
    return goal.decompile(*this, stream);
}

bool OsiStory::getEnum(uint16_t type, OsiEnum& osiEnum) const
{
    auto it = enums.find(type);
    if (it != enums.end()) {
        osiEnum = it->second;
        return true;
    }

    return false;
}

std::string OsiStory::nodeTypeName(OsiNodeType type) const
{
    switch (type) {
    case NT_UNDEFINED:
        return "Undefined";
    case NT_DATABASE:
        return "Database";
    case NT_PROC:
        return "Proc";
    case NT_DIV_QUERY:
        return "Div Query";
    case NT_AND:
        return "And";
    case NT_AND_NOT:
        return "And Not";
    case NT_REL_OP:
        return "Rel Op";
    case NT_RULE:
        return "Rule";
    case NT_INTERNAL_QUERY:
        return "Internal Query";
    case NT_USER_QUERY:
        return "User Query";
    default:
        return std::format("NodeType{}", static_cast<uint32_t>(type));
    }
}

void OsiType::read(OsiReader& reader)
{
    name = reader.readString();
    index = reader.read<uint8_t>();
    alias = 0;

    if (reader.version() >= OsiVersion::TYPE_ALIASES) {
        alias = reader.read<uint8_t>();
    }
}

void OsiEnum::read(OsiReader& reader)
{
    type = reader.read<uint16_t>();
    auto count = reader.read<uint32_t>();

    for (auto i = 0u; i < count; ++i) {
        auto name = reader.readString();
        auto value = reader.read<uint64_t>();
        elements[name] = value;
    }
}

void OsiDivObject::read(OsiReader& reader)
{
    name = reader.readString();
    type = reader.read<uint8_t>();
    reader.read(keys, std::size(keys));
}

void OsiParameterList::read(OsiReader& reader)
{
    auto paramCount = reader.read<uint8_t>();
    types.resize(paramCount);

    for (auto i = 0u; i < paramCount; ++i) {
        if (reader.shortTypeIds()) {
            types[i] = reader.read<uint16_t>();
        } else {
            types[i] = reader.read<uint32_t>();
        }
    }
}

bool OsiFunctionSig::isOutParam(uint32_t index) const
{
    auto byteIndex = index >> 3;
    auto bitIndex = index & 7;

    if (byteIndex >= outParamMask.size()) {
        return false;
    }

    return (outParamMask[byteIndex] & (0x80 >> bitIndex)) != 0;
}

void OsiFunctionSig::read(OsiReader& reader)
{
    name = reader.readString();

    auto outParamBytes = reader.read<uint32_t>();
    outParamMask.resize(outParamBytes);

    reader.read(outParamMask.data(), outParamBytes);

    parameters.read(reader);
}

std::string OsiFunction::functionType() const
{
    switch (type) {
    case FT_UNDEFINED:
        return "Undefined";
    case FT_EVENT:
        return "Event";
    case FT_QUERY:
        return "Query";
    case FT_CALL:
        return "Call";
    case FT_DATABASE:
        return "Database";
    case FT_PROC:
        return "Proc";
    case FT_SYS_QUERY:
        return "System Query";
    case FT_SYS_CALL:
        return "System Call";
    case FT_USER_QUERY:
        return "User Query";
    default:
        return std::format("FunctionType{}", static_cast<uint32_t>(type));
    }
}

void OsiFunction::read(OsiReader& reader)
{
    line = reader.read<uint32_t>();
    conditionRef = reader.read<uint32_t>();
    actionRef = reader.read<uint32_t>();
    nodeRef = reader.read<uint32_t>();
    type = static_cast<OsiFunctionType>(reader.read<uint8_t>());

    reader.read(meta, std::size(meta));

    name.read(reader);
}

void OsiNode::read(OsiReader& reader)
{
    type = static_cast<OsiNodeType>(reader.read<uint8_t>());
    index = reader.read<uint32_t>();
    dbRef = reader.read<uint32_t>();
    name = reader.readString();
    if (!name.empty()) {
        numParams = reader.read<uint8_t>();
    }
}

void OsiNode::resolve(OsiStory& story)
{
    if (dbRef != INVALID_REF) {
        auto& node = story.databases[dbRef];

        if (node.ownerNode != 0) {
            throw Exception("Database {} is already owned by node {}.", dbRef, node.ownerNode);
        }

        node.ownerNode = index;
    }
}

void OsiNodeEntry::read(OsiReader& reader)
{
    nodeRef = reader.read<uint32_t>();
    entryPoint = static_cast<OsiEntryPoint>(reader.read<uint32_t>());
    goalRef = reader.read<uint32_t>();
}

void OsiDataNode::read(OsiReader& reader)
{
    OsiNode::read(reader);

    refBy.clear();

    auto count = reader.read<uint32_t>();
    refBy.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        OsiNodeEntry entry{};
        entry.read(reader);
        refBy.emplace_back(entry);
    }
}

void OsiDataNode::resolve(OsiStory& story)
{
    OsiNode::resolve(story);

    for (auto& entry : refBy) {
        if (entry.nodeRef == INVALID_REF) {
            continue;
        }

        auto& node = story.nodes[entry.nodeRef];

        if (entry.goalRef != INVALID_REF && node->type == NT_RULE) {
            auto* ruleNode = static_cast<OsiRuleNode*>(node.get());
            ruleNode->derivedGoalRef = entry.goalRef;
        }
    }
}

void OsiDBNode::read(OsiReader& reader)
{
    OsiDataNode::read(reader);
}

StreamBase& OsiDBNode::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                 bool printTypes) const
{
    stream.write(std::format("{}(", name));
    tuple.decompile(story, stream, printTypes);
    stream.write(")\n");

    return stream;
}

StreamBase& OsiProcNode::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                   bool printTypes) const
{
    stream.write(std::format("{}(", name));
    tuple.decompile(story, stream, true);
    stream.write(")\n");

    return stream;
}

void OsiProcNode::read(OsiReader& reader)
{
    OsiDataNode::read(reader);
}

void OsiTreeNode::read(OsiReader& reader)
{
    OsiNode::read(reader);
    nextNode.read(reader);
}

void OsiTreeNode::resolve(OsiStory& story)
{
    OsiNode::resolve(story);

    if (nextNode.nodeRef != INVALID_REF) {
        auto& node = story.nodes[nextNode.nodeRef];
        if (node->type == NT_RULE) {
            auto* ruleNode = static_cast<OsiRuleNode*>(node.get());
            ruleNode->derivedGoalRef = nextNode.goalRef;
        }
    }
}

void OsiRelNode::read(OsiReader& reader)
{
    OsiTreeNode::read(reader);
    parentRef = reader.read<uint32_t>();
    adapterRef = reader.read<uint32_t>();
    relDbNodeRef = reader.read<uint32_t>();
    relJoin.read(reader);
    relDBIndirect = reader.read<uint8_t>();
}

void OsiRelNode::resolve(OsiStory& story)
{
    OsiTreeNode::resolve(story);

    if (adapterRef != INVALID_REF) {
        auto& adapter = story.adapters[adapterRef];
        if (adapter.ownerNode != INVALID_REF) {
            throw Exception("Adapter {} is already owned by node {}.", adapterRef, adapter.ownerNode);
        }
        adapter.ownerNode = index;
    }
}

std::string OsiValue::toString() const
{
    if (!isValid()) {
        return "<invalid>";
    }

    std::string strValue;
    std::visit([&]<typename T>(const T& arg) {
        if constexpr (std::is_arithmetic_v<T>) {
            strValue = std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            strValue = arg;
        }
    }, value);

    return strValue;
}

StreamBase& OsiValue::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple, bool printTypes) const
{
    OsiEnum osiEnum;
    if (story.getEnum(static_cast<uint16_t>(type), osiEnum)) {
        stream.write(std::get<std::string>(value));
        return stream;
    }

    auto resolvedType = story.resolveAlias(type);
    switch (resolvedType) {
    case OVT_INT:
        stream.writeText(std::get<int32_t>(value));
        break;
    case OVT_INT64:
        stream.writeText(std::get<int64_t>(value));
        break;
    case OVT_FLOAT:
        stream.writeText(std::get<float>(value));
        break;
    case OVT_STRING:
        stream.write(std::format("\"{}\"", std::get<std::string>(value)));
        break;
    case OVT_GUIDSTRING:
        stream.write(std::get<std::string>(value));
        break;
    default:
        throw Exception("Decompilation of value type {} not implemented.", static_cast<int>(resolvedType));
    }

    return stream;
}

void OsiValue::read(OsiReader& reader)
{
    if (reader.version() >= OsiVersion::VALUE_FLAGS) {
        index = reader.read<int8_t>();
        flags = static_cast<OsiValueFlags>(reader.read<uint8_t>());
        if (!isValid()) {
            return;
        }
    }

    auto unknown = reader.read<uint8_t>(); // possible isRef?
    if (unknown == '1') {
        if (reader.shortTypeIds()) {
            type = static_cast<OsiValueType>(reader.read<uint16_t>());
        } else {
            type = static_cast<OsiValueType>(reader.read<uint32_t>());
        }
        value = reader.read<int32_t>();
    } else if (unknown == '0') {
        if (reader.shortTypeIds()) {
            type = static_cast<OsiValueType>(reader.read<uint16_t>());
        } else {
            type = static_cast<OsiValueType>(reader.read<uint32_t>());
        }

        auto resolvedType(type);
        if (type >= OVT_TOTAL_TYPES) { // alias type
            resolvedType = reader.resolveAlias(type);
        }

        switch (resolvedType) {
        case OVT_NONE:
            break;
        case OVT_INT:
            value = reader.read<int32_t>();
            break;
        case OVT_INT64:
            value = reader.read<int64_t>();
            break;
        case OVT_FLOAT:
            value = reader.read<float>();
            break;

        case OVT_STRING:
        case OVT_GUIDSTRING:
        default:
            if (reader.read<uint8_t>() > 0) {
                value = reader.readString();
            }
            break;
        }
    } else if (unknown == 'e') { // enum
        type = static_cast<OsiValueType>(reader.read<uint16_t>());

        OsiEnum e;
        if (!reader.getEnum(static_cast<uint16_t>(type), e)) {
            throw Exception("Enum label serialized for a non-enum type: {}", static_cast<int>(type));
        }

        value = reader.readString();
        auto it = e.elements.find(std::get<std::string>(value));
        if (it == e.elements.end()) {
            throw Exception("Enum value \"{}\" not found in enum type {}.", std::get<std::string>(value),
                            static_cast<int>(type));
        }
    } else {
        throw Exception("Unsupported value format {}.", static_cast<int>(unknown));
    }
}

void OsiTypedValue::read(OsiReader& reader)
{
    OsiValue::read(reader);

    if (reader.version() < OsiVersion::VALUE_FLAGS) {
        setValid(reader.read<uint8_t>() != 0);
        setOutParam(reader.read<uint8_t>() != 0);
        setIsType(reader.read<uint8_t>() != 0);
    }
}

StreamBase& OsiVariable::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                   bool printTypes) const
{
    if (isUnused()) {
        if (printTypes && type != OVT_NONE) {
            stream.write(std::format("({})", story.typeName(type)));
        }
        stream.write("_");
    } else if (isAdapted()) {
        if (!variableName.empty()) {
            if (printTypes && type != OVT_NONE) {
                stream.write(std::format("({})", story.typeName(type)));
            }
            stream.write(variableName);
        } else {
            const auto& var = tuple.logical.at(index);
            var->decompile(story, stream, {});
        }
    } else {
        OsiValue::decompile(story, stream, tuple);
    }

    return stream;
}

void OsiVariable::read(OsiReader& reader)
{
    OsiTypedValue::read(reader);

    if (reader.version() < OsiVersion::VALUE_FLAGS) {
        index = reader.read<int8_t>();
        reader.read<uint8_t>(); // unused
        setIsAdapted(reader.read<uint8_t>() != 0);
    }
}

StreamBase& OsiCall::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple, bool printTypes) const
{
    if (name.empty()) {
        return stream;
    }

    if (negate) {
        stream.write("NOT ");
    }

    stream.write(std::format("{}(", name));

    for (auto i = 0u; i < parameters.size(); ++i) {
        if (i > 0) {
            stream.write(", ");
        }
        parameters[i]->decompile(story, stream, tuple, printTypes);
    }

    stream.write(")");

    return stream;
}

void OsiCall::read(OsiReader& reader)
{
    name = reader.readString();
    if (!name.empty()) {
        auto hasParams = reader.read<uint8_t>();
        if (hasParams) {
            auto paramCount = reader.read<uint8_t>();

            parameters.clear();
            parameters.reserve(paramCount);

            for (auto i = 0u; i < paramCount; ++i) {
                OsiTypedValue::Ptr param;

                if (reader.version() >= OsiVersion::VALUE_FLAGS) {
                    param = std::make_unique<OsiVariable>();
                } else {
                    auto type = reader.read<uint8_t>();
                    if (type == 1) {
                        param = std::make_unique<OsiVariable>();
                    } else {
                        param = std::make_unique<OsiTypedValue>();
                    }
                }

                param->read(reader);

                parameters.emplace_back(std::move(param));
            }
        }

        negate = reader.read<uint8_t>() != 0;
    }
    goalIdOrDebugHook = reader.read<int32_t>();
}

StreamBase& OsiRuleNode::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                   bool printTypes) const
{
    auto type = ruleType(story);

    switch (type) {
    case RT_PROC:
        stream.write("PROC\n");
        break;
    case RT_QUERY:
        stream.write("QRY\n");
        break;
    case RT_RULE:
        stream.write("IF\n");
        break;
    default:
        throw Exception("Decompilation of rule node type {} not implemented.", static_cast<int>(type));
    }

    auto initialTuple = makeInitialTuple();
    if (adapterRef != INVALID_REF) {
        const auto& adapter = story.adapters[adapterRef];
        initialTuple = adapter.adapt(initialTuple);
    }

    printTypes = printTypes || type == RT_PROC || type == RT_QUERY;

    assert(parentRef != INVALID_REF);

    const auto& parentNode = story.nodes[parentRef];
    parentNode->decompile(story, stream, initialTuple, printTypes);

    stream.write("THEN\n");
    for (const auto& call : calls) {
        call.decompile(story, stream, initialTuple, false);
        stream.write(";\n");
    }

    return stream;
}

void OsiRuleNode::read(OsiReader& reader)
{
    OsiRelNode::read(reader);

    // read calls
    auto count = reader.read<uint32_t>();

    calls.clear();
    calls.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        OsiCall call{};
        call.read(reader);
        calls.emplace_back(std::move(call));
    }

    // read variables
    count = reader.read<uint8_t>();

    variables.clear();
    variables.reserve(count);
    for (auto i = 0u; i < count; ++i) {
        if (reader.version() < OsiVersion::VALUE_FLAGS) {
            auto type = reader.read<uint8_t>();
            if (type != 1) {
                throw Exception("Unsupported variable type {} in rule node.", static_cast<int>(type));
            }
        }

        OsiVariable var{};
        var.read(reader);
        if (var.isAdapted()) {
            var.variableName = std::format("_Var{}", variables.size() + 1);
        }

        variables.emplace_back(std::move(var));
    }

    line = reader.read<uint32_t>();

    if (reader.version() >= OsiVersion::ADD_QUERY) {
        isQuery = reader.read<uint8_t>() != 0;
    } else {
        isQuery = false;
    }
}

void OsiRuleNode::resolve(OsiStory& story)
{
    OsiRelNode::resolve(story);

    // Remove the __DEF__ postfix that is added to the end of Query nodes
    if (isQuery) {
        auto* ruleRoot = getRoot(story);
        if (ruleRoot->name.size() > 7 &&
            ruleRoot->name.ends_with("__DEF__")) {
            ruleRoot->name.resize(ruleRoot->name.size() - 7);
        }
    }
}

OsiTuple OsiRuleNode::makeInitialTuple() const
{
    OsiTuple tuple{};

    for (auto i = 0u; i < variables.size(); ++i) {
        auto var = std::make_shared<OsiVariable>(variables[i]);
        tuple.physical.emplace_back(var);
        std::pair p{static_cast<int32_t>(i), var};

        tuple.logical.emplace(p);
    }

    return tuple;
}

OsiNode* OsiRuleNode::getRoot(const OsiStory& story)
{
    OsiNode* parent = this;

    for (;;) {
        if (isRelNode(*parent)) {
            auto* relNode = static_cast<OsiRelNode*>(parent);
            parent = story.nodes[relNode->parentRef].get();
        } else if (isJoinNode(*parent)) {
            auto* joinNode = static_cast<OsiJoinNode*>(parent);
            parent = story.nodes[joinNode->leftParentRef].get();
        } else {
            break;
        }
    }

    return parent;
}

const OsiNode* OsiRuleNode::getRoot(const OsiStory& story) const
{
    const OsiNode* parent = this;

    for (;;) {
        if (isRelNode(*parent)) {
            auto* relNode = static_cast<const OsiRelNode*>(parent);
            parent = story.nodes[relNode->parentRef].get();
        } else if (isJoinNode(*parent)) {
            auto* joinNode = static_cast<const OsiJoinNode*>(parent);
            parent = story.nodes[joinNode->leftParentRef].get();
        } else {
            break;
        }
    }

    return parent;
}

OsiRuleType OsiRuleNode::ruleType(const OsiStory& story) const
{
    const auto* pRoot = getRoot(story);
    if (pRoot->type == NT_DATABASE) {
        return RT_RULE;
    }
    if (pRoot->type == NT_PROC) {
        auto querySig = std::format("{}__DEF__/{}", pRoot->name, pRoot->numParams);
        auto sig = std::format("{}/{}", pRoot->name, pRoot->numParams);

        if (!story.functionSigs.contains(querySig) &&
            !story.functionSigs.contains(sig)) {
            return RT_UNKNOWN;
        }

        const auto& func = story.functionSigs.at(sig);
        switch (func->type) {
        case FT_EVENT:
            return RT_RULE;
        case FT_PROC:
            return RT_PROC;
        case FT_USER_QUERY:
            return RT_QUERY;
        default:
            return RT_UNKNOWN;
        }
    }

    return RT_UNKNOWN;
}

void OsiJoinNode::read(OsiReader& reader)
{
    OsiTreeNode::read(reader);

    leftParentRef = reader.read<uint32_t>();
    rightParentRef = reader.read<uint32_t>();
    leftAdapterRef = reader.read<uint32_t>();
    rightAdapterRef = reader.read<uint32_t>();

    leftDBNodeRef = reader.read<uint32_t>();
    leftDBJoin.read(reader);
    leftDBIndirect = reader.read<uint8_t>();

    rightDBNodeRef = reader.read<uint32_t>();
    rightDBJoin.read(reader);
    rightDBIndirect = reader.read<uint8_t>();
}

void OsiJoinNode::resolve(OsiStory& story)
{
    OsiTreeNode::resolve(story);

    if (leftAdapterRef != INVALID_REF) {
        auto& adapter = story.adapters[leftAdapterRef];
        if (adapter.ownerNode != INVALID_REF) {
            throw Exception("Adapter {} is already owned by node {}.", leftAdapterRef, adapter.ownerNode);
        }
        adapter.ownerNode = index;
    }

    if (rightAdapterRef != INVALID_REF) {
        auto& adapter = story.adapters[rightAdapterRef];
        if (adapter.ownerNode != INVALID_REF) {
            throw Exception("Adapter {} is already owned by node {}.", rightAdapterRef, adapter.ownerNode);
        }
        adapter.ownerNode = index;
    }
}

StreamBase& OsiAndNode::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                  bool printTypes) const
{
    assert(leftAdapterRef != INVALID_REF);
    assert(rightAdapterRef != INVALID_REF);
    assert(leftParentRef != INVALID_REF);
    assert(rightParentRef != INVALID_REF);

    const auto& leftAdapter = story.adapters[leftAdapterRef];
    const auto& rightAdapter = story.adapters[rightAdapterRef];

    auto leftTuple = leftAdapter.adapt(tuple);
    const auto& leftParentNode = story.nodes[leftParentRef];
    leftParentNode->decompile(story, stream, leftTuple, printTypes);

    stream.write("AND\n");

    auto rightTuple = rightAdapter.adapt(tuple);
    const auto& rightParentNode = story.nodes[rightParentRef];
    rightParentNode->decompile(story, stream, rightTuple, false);

    return stream;
}

void OsiAndNode::read(OsiReader& reader)
{
    OsiJoinNode::read(reader);
}

StreamBase& OsiAndNotNode::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                   bool printTypes) const
{
    assert(leftAdapterRef != INVALID_REF);
    assert(rightAdapterRef != INVALID_REF);
    assert(leftParentRef != INVALID_REF);
    assert(rightParentRef != INVALID_REF);

    const auto& leftAdapter = story.adapters[leftAdapterRef];
    const auto& rightAdapter = story.adapters[rightAdapterRef];

    auto leftTuple = leftAdapter.adapt(tuple);
    const auto& leftParentNode = story.nodes[leftParentRef];
    leftParentNode->decompile(story, stream, leftTuple, printTypes);

    stream.write("AND NOT\n");

    auto rightTuple = rightAdapter.adapt(tuple);
    const auto& rightParentNode = story.nodes[rightParentRef];
    rightParentNode->decompile(story, stream, rightTuple, false);

    return stream;
}

void OsiAndNotNode::read(OsiReader& reader)
{
    OsiJoinNode::read(reader);
}

StreamBase& OsiQueryNode::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                    bool printTypes) const
{
    stream.write(std::format("{}(", name));
    tuple.decompile(story, stream, printTypes);
    stream.write(")\n");

    return stream;
}

void OsiQueryNode::read(OsiReader& reader)
{
    OsiNode::read(reader);
}

void OsiDivQueryNode::read(OsiReader& reader)
{
    OsiQueryNode::read(reader);
}

void OsiUserQueryNode::read(OsiReader& reader)
{
    OsiQueryNode::read(reader);
}

std::string relOpString(RelOpType type)
{
    switch (type) {
    case ROT_LESS:
        return "LESS";
    case ROT_LESS_EQUAL:
        return "LESS_EQUAL";
    case ROT_GREATER:
        return "GREATER";
    case ROT_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case ROT_EQUAL:
        return "EQUAL";
    case ROT_NOT_EQUAL:
        return "NOT_EQUAL";
    }

    return "UNKNOWN";
}

bool isJoinNode(const OsiNode& node) noexcept
{
    return node.type == NT_AND || node.type == NT_AND_NOT;
}

bool isRelNode(const OsiNode& node) noexcept
{
    return node.type == NT_REL_OP || node.type == NT_RULE;
}

StreamBase& OsiRelOpNode::decompile(const OsiStory& story, StreamBase& stream, const OsiTuple& tuple,
                                    bool printTypes) const
{
    assert(adapterRef != INVALID_REF);
    assert(parentRef != INVALID_REF);

    const auto& adapter = story.adapters[adapterRef];
    auto adaptedTuple = adapter.adapt(tuple);

    const auto& parentNode = story.nodes[parentRef];
    parentNode->decompile(story, stream, adaptedTuple, printTypes);

    stream.write("AND\n");

    if (leftValueIndex != -1) {
        adaptedTuple.logical[leftValueIndex]->decompile(story, stream, adaptedTuple);
    } else {
        leftValue.decompile(story, stream, tuple);
    }

    switch (relOp) {
    case ROT_LESS:
        stream.write(" < ");
        break;
    case ROT_LESS_EQUAL:
        stream.write(" <= ");
        break;
    case ROT_GREATER:
        stream.write(" > ");
        break;
    case ROT_GREATER_EQUAL:
        stream.write(" >= ");
        break;
    case ROT_EQUAL:
        stream.write(" == ");
        break;
    case ROT_NOT_EQUAL:
        stream.write(" != ");
        break;
    }

    if (rightValueIndex != -1) {
        adaptedTuple.logical[rightValueIndex]->decompile(story, stream, adaptedTuple);
    } else {
        rightValue.decompile(story, stream, tuple);
    }

    stream.write("\n");

    return stream;
}

void OsiRelOpNode::read(OsiReader& reader)
{
    OsiRelNode::read(reader);

    leftValueIndex = reader.read<int8_t>();
    rightValueIndex = reader.read<int8_t>();

    leftValue.read(reader);
    rightValue.read(reader);

    relOp = static_cast<RelOpType>(reader.read<int32_t>());
}

void OsiInternalQueryNode::read(OsiReader& reader)
{
    OsiQueryNode::read(reader);
}

StreamBase& OsiTuple::decompile(const OsiStory& story, StreamBase& stream, bool printTypes) const
{
    for (auto i = 0u; i < physical.size(); ++i) {
        if (i > 0) {
            stream.write(", ");
        }
        physical[i]->decompile(story, stream, {}, printTypes);
    }
    return stream;
}

void OsiTuple::read(OsiReader& reader)
{
    physical.clear();
    logical.clear();

    auto count = reader.read<uint8_t>();
    physical.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        auto val = std::make_shared<OsiValue>();

        int8_t index;
        if (reader.version() >= OsiVersion::VALUE_FLAGS) {
            val->read(reader);
            index = val->index;
        } else {
            index = reader.read<int8_t>();
            val->read(reader);
        }
        physical.emplace_back(val);
        logical[index] = val;
    }
}

OsiTuple OsiAdapter::adapt(const OsiTuple& columns) const
{
    OsiTuple result{};

    for (auto i = 0u; i < logicalIndices.size(); ++i) {
        auto logicalIndex = logicalIndices[i];
        if (logicalIndex != -1) {
            if (columns.logical.contains(logicalIndex)) {
                const auto& value = columns.logical.at(logicalIndex);
                result.physical.emplace_back(value);
            } else if (logicalIndex == 0) {
                // special case for savegames where adapaters are padded with 0 logical indices
                auto value = std::make_shared<OsiValue>();
                value->type = OVT_NONE;
                value->setIsUnused(true);
                result.physical.emplace_back(std::move(value));
            } else {
                throw Exception("Logical index {} not found in tuple.", static_cast<int>(logicalIndex));
            }
        } else if (constants.logical.contains(static_cast<int32_t>(i))) {
            const auto& value = constants.logical.at(static_cast<int32_t>(i));
            result.physical.emplace_back(value);
        } else { // emit a null value
            auto var = std::make_shared<OsiVariable>();
            var->type = OVT_NONE;
            var->setIsUnused(true);
            result.physical.emplace_back(std::move(var));
        }
    }

    // Generate logical -> physical map
    for (const auto& map : logicalToPhysicalMap) {
        result.logical[map.first] = result.physical[map.second];
    }

    return result;
}

void OsiAdapter::read(OsiReader& reader)
{
    index = reader.read<uint32_t>();
    constants.read(reader);

    auto count = reader.read<uint8_t>();

    logicalIndices.clear();
    logicalIndices.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        logicalIndices.emplace_back(reader.read<int8_t>());
    }

    logicalToPhysicalMap.clear();

    count = reader.read<uint8_t>();
    for (auto i = 0u; i < count; ++i) {
        auto key = reader.read<int8_t>();
        auto value = reader.read<int8_t>();
        logicalToPhysicalMap[key] = value;
    }
}

void OsiFact::read(OsiReader& reader)
{
    auto count = reader.read<uint8_t>();

    columns.resize(count);
    for (auto i = 0u; i < count; ++i) {
        columns[i].read(reader);
    }
}

void OsiDatabase::read(OsiReader& reader)
{
    index = reader.read<uint32_t>();
    parameters.read(reader);

    auto count = reader.read<uint32_t>();

    facts.clear();
    facts.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        OsiFact fact{};
        fact.read(reader);
        facts.emplace_back(std::move(fact));
    }
}

void OsiGoal::read(OsiReader& reader)
{
    index = reader.read<uint32_t>();
    name = reader.readString();
    subGoalCombination = reader.read<int8_t>();

    auto count = reader.read<uint32_t>();
    parentGoals.clear();
    parentGoals.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        parentGoals.emplace_back(reader.read<uint32_t>());
    }

    count = reader.read<uint32_t>();
    subGoals.clear();
    subGoals.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        subGoals.emplace_back(reader.read<uint32_t>());
    }

    flags = reader.read<int8_t>();

    initCalls.clear();
    exitCalls.clear();

    if (reader.version() < OsiVersion::ADD_INIT_EXIT_CALLS) {
        return;
    }

    auto initCount = reader.read<uint32_t>();

    initCalls.reserve(initCount);
    for (auto i = 0u; i < initCount; ++i) {
        OsiCall call{};
        call.read(reader);
        initCalls.emplace_back(std::move(call));
    }
    auto exitCount = reader.read<uint32_t>();

    exitCalls.reserve(exitCount);
    for (auto i = 0u; i < exitCount; ++i) {
        OsiCall call{};
        call.read(reader);
        exitCalls.emplace_back(std::move(call));
    }
}

StreamBase& OsiGoal::decompile(const OsiStory& story, StreamBase& stream) const
{
    stream.write("Version 1\n");
    stream.write("SubGoalCombiner SGC_AND\n\n");
    stream.write("INITSECTION\n");

    OsiTuple nullTuple{};

    for (const auto& call : initCalls) {
        call.decompile(story, stream, nullTuple, false);
        stream.write(";\n");
    }

    stream.write("\nKBSECTION\n");

    for (const auto& node : story.nodes) {
        if (node->type == NT_RULE) {
            const auto& ruleNode = static_cast<const OsiRuleNode&>(*node);
            if (ruleNode.derivedGoalRef == index) {
                ruleNode.decompile(story, stream, nullTuple, false);
                stream.write("\n");
            }
        }
    }

    stream.write("\nEXITSECTION\n");

    for (const auto& call : exitCalls) {
        call.decompile(story, stream, nullTuple, false);
        stream.write(";\n");
    }

    stream.write("ENDEXITSECTION\n");

    for (const auto& goalId : parentGoals) {
        const auto& goal = story.goals[goalId];
        stream.write(std::format("ParentTargetEdge \"{}\"\n", goal.name));
    }

    return stream;
}
