#include "pch.h"
#include "OsiReader.h"
#include "Story.h"

#include "Exception.h"

Story::Story()
{
}

Story::Story(Story&& rhs) noexcept
{
    *this = std::move(rhs);
}

Story& Story::operator=(Story&& rhs) noexcept
{
    if (this != &rhs) {
        header = std::move(rhs.header);
        types = std::move(rhs.types);
        typeAliases = std::move(rhs.typeAliases);
        stringTable = std::move(rhs.stringTable);
        enums = std::move(rhs.enums);
        divObjects = std::move(rhs.divObjects);
        functions = std::move(rhs.functions);
        nodes = std::move(rhs.nodes);
    }

    return *this;
}

OsiVersion Story::version() const
{
    return static_cast<OsiVersion>(
        (static_cast<uint16_t>(header.majorVersion) << 8) |
        static_cast<uint16_t>(header.minorVersion));
}

void OsiNode::read(OsiReader& reader)
{
    type = static_cast<OsiNodeType>(reader.read<uint8_t>());
    index = reader.read<uint32_t>();
    dbRef = reader.read<uint32_t>();
    name = reader.readString();
    if (!name.empty()) {
        std::cout << "Reading node (" << typeName() << "): " << name << "\n";
        numParams = reader.read<uint8_t>();
    } else {
        std::cout << "Reading node (" << typeName() << ")\n";
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

void OsiRelNode::read(OsiReader& reader)
{
    OsiTreeNode::read(reader);
    parentRef = reader.read<uint32_t>();
    adapterRef = reader.read<uint32_t>();
    relDbNodeRef = reader.read<uint32_t>();
    relJoin.read(reader);
    relDBIndirect = reader.read<uint8_t>();
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

    // TODO: LOTS more TODO HERE
    auto unknown = reader.read<uint8_t>(); // possible isRef?
    if (unknown == 1) {
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
        
        // TODO: resolve alias type
        switch (type) {
        case OVT_INT:
            value = reader.read<int32_t>();
            break;
        default:
            throw Exception("Unsupported value type {}.", static_cast<int>(type));
        }

    } else if (unknown == 'e') {
        type = static_cast<OsiValueType>(reader.read<uint16_t>());
        ATLASSERT(0); // TODO
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
        adapted = reader.read<uint8_t>() != 0;
    }
}

void OsiCall::read(OsiReader& reader)
{
    name = reader.readString();
    if (!name.empty()) {
        std::cout << "   Reading call: " << name << "\n";

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
        if (var.adapted) {
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

void OsiAndNode::read(OsiReader& reader)
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
