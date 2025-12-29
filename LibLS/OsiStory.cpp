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
        functionNames = std::move(rhs.functionNames);
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
    case NT_NAND:
        return "Nand";
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
    } else if (unknown == 'e') {
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

void OsiVariable::read(OsiReader& reader)
{
    OsiTypedValue::read(reader);

    if (reader.version() < OsiVersion::VALUE_FLAGS) {
        index = reader.read<int8_t>();
        reader.read<uint8_t>(); // unused
        setIsAdapted(reader.read<uint8_t>() != 0);
    }
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
            ruleRoot->name.compare(ruleRoot->name.size() - 7, 7, "__DEF__") == 0) {
            ruleRoot->name.resize(ruleRoot->name.size() - 7);
        }
    }
}

OsiNode* OsiRuleNode::getRoot(OsiStory& story)
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

void OsiAndNode::read(OsiReader& reader)
{
    OsiJoinNode::read(reader);
}

void OsiNandNode::read(OsiReader& reader)
{
    OsiJoinNode::read(reader);
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
    return node.type == NT_AND || node.type == NT_NAND;
}

bool isRelNode(const OsiNode& node) noexcept
{
    return node.type == NT_REL_OP || node.type == NT_RULE;
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

void Tuple::read(OsiReader& reader)
{
    physical.clear();
    logical.clear();

    auto count = reader.read<uint8_t>();
    physical.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        OsiValue val{};
        int8_t index;
        if (reader.version() >= OsiVersion::VALUE_FLAGS) {
            val.read(reader);
            index = val.index;
        } else {
            index = reader.read<int8_t>();
            val.read(reader);
        }
        physical.emplace_back(val);
        logical[index] = val;
    }
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
