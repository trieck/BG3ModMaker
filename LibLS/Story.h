#pragma once

class OsiReader;

struct SaveFileHeader
{
    std::string version;
    uint8_t majorVersion;
    uint8_t minorVersion;
    bool bigEndian;
    std::string versionString;
    uint32_t debugFlags;
};

enum class OsiVersion : uint16_t
{
    INITIAL = 0x0100,
    ADD_INIT_EXIT_CALLS = 0x0101,
    ADD_VERSION_STRING = 0x0102,
    ADD_DEBUG_FLAGS = 0x0103,
    SCRAMBLED = 0x0104, /* scramble strings by xor-ing with 0xAD */
    ADD_TYPE_MAP = 0x0105, /* custom string types */
    ADD_QUERY = 0x0106, /* add query nodes */
    TYPE_ALIASES = 0x0109, /* types can be aliases of any builtin type, not just strings */
    ENHANCED_TYPES = 0x010A, /* added INT64, GUIDSTRING types*/
    EXTERNAL_STRING_TABLE = 0x010B, /* string table is stored externally */
    REMOVE_EXTERNAL_STRING_TABLE = 0x010C, /* string table is stored internally */
    ENUMS = 0x010D, /* added enum type */
    VALUE_FLAGS = 0x010E, /* changed values to store flags/indices in a more compact way */
    PATCH8_HOTFIX_2 = 0x010F, /* hotfix for patch 8 */
    LAST_SUPPORTED = PATCH8_HOTFIX_2
};

struct OsiReadable
{
    OsiReadable() = default;
    OsiReadable(const OsiReadable& rhs) = default;
    virtual ~OsiReadable() = default;

    virtual void read(OsiReader& reader) = 0;
};

struct OsiType : OsiReadable
{
    std::string name;
    uint8_t index;
    bool builtIn = false;
    uint8_t alias;

    void read(OsiReader& reader) override;
};

struct OsiEnum : OsiReadable
{
    uint16_t type;
    std::unordered_map<std::string, uint64_t> elements;

    void read(OsiReader& reader) override;
};

struct OsiDivObject : OsiReadable
{
    std::string name;
    uint8_t type;
    uint32_t keys[4];

    void read(OsiReader& reader) override;
};

enum OsiFunctionType : uint8_t
{
    FT_UNDEFINED = 0,
    FT_EVENT = 1,
    FT_QUERY = 2,
    FT_CALL = 3,
    FT_DATABASE = 4,
    FT_PROC = 5,
    FT_SYS_QUERY = 6,
    FT_SYS_CALL = 7,
    FT_USER_QUERY = 8,
};

struct OsiParameterList : OsiReadable
{
    std::vector<uint32_t> types;

    void read(OsiReader& reader) override;
};

struct OsiFunctionSig : OsiReadable
{
    std::string name;
    std::vector<uint8_t> outParamMask;
    OsiParameterList parameters;

    void read(OsiReader& reader) override;
};

struct OsiFunction : OsiReadable
{
    uint32_t line;
    uint32_t conditionRef;
    uint32_t actionRef;
    uint32_t nodeRef; // FIXME: NodeReference<Node>
    OsiFunctionType type;
    uint32_t meta[4];
    OsiFunctionSig name;

    void read(OsiReader& reader) override;
};

enum OsiNodeType : uint8_t
{
    NT_UNDEFINED = 0,
    NT_DATABASE = 1,
    NT_PROC = 2,
    NT_DIV_QUERY = 3,
    NT_AND = 4,
    NT_NAND = 5,
    NT_REL_OP = 6,
    NT_RULE = 7,
    NT_INTERNAL_QUERY = 8,
    NT_USER_QUERY = 9,
    NT_TOTAL_TYPES = 10,
};

struct OsiNode : OsiReadable
{
    OsiNodeType type;
    uint32_t index;
    uint32_t dbRef;
    std::string name;
    uint8_t numParams;

    using Ptr = std::unique_ptr<OsiNode>;

    void read(OsiReader& reader) override;
    virtual std::string typeName() const = 0;
};

enum OsiEntryPoint : uint32_t
{
    EP_NONE = 0, /* the next node is not an AND/NAND expression */
    EP_LEFT = 1, /* the next node is the left child of an AND/NAND expression */
    EP_RIGHT = 2, /* the next node is the right child of an AND/NAND expression */
};

struct OsiNodeEntry : OsiReadable
{
    uint32_t nodeRef;
    OsiEntryPoint entryPoint;
    uint32_t goalRef;

    void read(OsiReader& reader) override;
};

struct OsiDataNode : OsiNode
{
    std::vector<OsiNodeEntry> refBy;

    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Data";
    }
};

struct OsiDBNode : OsiDataNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "DB";
    }
};

struct OsiProcNode : OsiDataNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Proc";
    }
};

struct OsiTreeNode : OsiNode
{
    OsiNodeEntry nextNode;

    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Tree";
    }
};

struct OsiRelNode : OsiTreeNode
{
    uint32_t parentRef;
    uint32_t adapterRef;
    uint32_t relDbNodeRef;
    OsiNodeEntry relJoin;
    uint8_t relDBIndirect;

    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Relational";
    }
};

enum OsiValueType : uint32_t
{
    OVT_NONE = 0,
    OVT_INT = 1,
    OVT_INT64 = 2,
    OVT_FLOAT = 3,
    OVT_STRING = 4,
    OVT_GUIDSTRING = 5,
    OVT_TOTAL_TYPES = 6,
};

enum OsiValueFlags : uint8_t
{
    OVF_NONE = 0x00,
    OVF_SIMPLE = 0x01,
    OVF_TYPED = 0x02,
    OVF_VARIABLE = 0x03,
    OVF_VALID = 0x08,
    OFV_OUT_PARAM = 0x10,
    OVF_IS_TYPE = 0x20,
    OVF_UNUSED = 0x40,
    OVF_ADAPTED = 0x80,
};

struct OsiValue : OsiReadable
{
    OsiValueType type;
    using Value = std::variant<
        int32_t,
        int64_t,
        float,
        std::string
    >;
    Value value;
    uint8_t flags;
    int8_t index;
    bool adapted = false;

    bool isValid() const
    {
        return (flags & OVF_VALID) != 0;
    }

    void setValid(bool valid)
    {
        if (valid) {
            flags = static_cast<OsiValueFlags>(flags | OVF_VALID);
        } else {
            flags = static_cast<OsiValueFlags>(flags & ~OVF_VALID);
        }
    }

    bool isOutParam() const
    {
        return (flags & OFV_OUT_PARAM) != 0;
    }

    void setOutParam(bool outParam)
    {
        if (outParam) {
            flags = static_cast<OsiValueFlags>(flags | OFV_OUT_PARAM);
        } else {
            flags = static_cast<OsiValueFlags>(flags & ~OFV_OUT_PARAM);
        }
    }

    bool isType() const
    {
        return (flags & OVF_IS_TYPE) != 0;
    }

    void setIsType(bool isType)
    {
        if (isType) {
            flags = static_cast<OsiValueFlags>(flags | OVF_IS_TYPE);
        } else {
            flags = static_cast<OsiValueFlags>(flags & ~OVF_IS_TYPE);
        }
    }

    bool isUnused() const
    {
        return (flags & OVF_UNUSED) != 0;
    }

    bool isAdapted() const
    {
        return (flags & OVF_ADAPTED) != 0;
    }

    void read(OsiReader& reader) override;
};

struct OsiTypedValue : OsiValue
{
    using Ptr = std::unique_ptr<OsiTypedValue>;

    void read(OsiReader& reader) override;
};

struct OsiVariable : OsiTypedValue
{
    std::string variableName;

    void read(OsiReader& reader) override;
};

struct OsiCall : OsiReadable
{
    std::string name;

    std::vector<OsiTypedValue::Ptr> parameters;
    bool negate;
    int32_t goalIdOrDebugHook;

    void read(OsiReader& reader) override;
};

struct OsiRuleNode : OsiRelNode
{
    std::vector<OsiCall> calls;
    std::vector<OsiVariable> variables;
    uint32_t line;
    uint32_t derivedGoalRef;
    bool isQuery = false;

    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Rule";
    }
};

struct OsiJoinNode : OsiTreeNode
{
    uint32_t leftParentRef;
    uint32_t rightParentRef;
    uint32_t leftAdapterRef;
    uint32_t rightAdapterRef;
    uint32_t leftDBNodeRef;
    uint8_t leftDBIndirect;
    OsiNodeEntry leftDBJoin;
    uint32_t rightDBNodeRef;
    uint8_t rightDBIndirect;
    OsiNodeEntry rightDBJoin;

    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Join";
    }
};

struct OsiAndNode : OsiJoinNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "And";
    }
};

struct OsiNandNode : OsiJoinNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Nand";
    }
};

struct OsiQueryNode : OsiNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Query";
    }
};

struct OsiDivQueryNode : OsiQueryNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Div Query";
    }
};

struct OsiUserQueryNode : OsiQueryNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "User Query";
    }
};

struct OsiInternalQueryNode : OsiQueryNode
{
    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "Internal Query";
    }
};

enum RelOpType : uint32_t
{
    ROT_LESS = 0,
    ROT_LESS_EQUAL = 1,
    ROT_GREATER = 2,
    ROT_GREATER_EQUAL = 3,
    ROT_EQUAL = 4,
    ROT_NOT_EQUAL = 5
};

std::string relOpString(RelOpType type);

struct OsiRelOpNode : OsiRelNode
{
    int8_t leftValueIndex;
    int8_t rightValueIndex;
    OsiValue leftValue;
    OsiValue rightValue;
    RelOpType relOp;

    void read(OsiReader& reader) override;

    std::string typeName() const override
    {
        return "RelOp";
    }
};

struct Tuple : OsiReadable
{
    std::vector<OsiValue> physical;
    std::unordered_map<int32_t, OsiValue> logical;

    void read(OsiReader& reader) override;
};

struct OsiAdapter : OsiReadable
{
    uint32_t index;
    Tuple constants;
    std::vector<int8_t> logicalIndices;
    std::unordered_map<int8_t, int8_t> localToPhysicalMap;

    void read(OsiReader& reader) override;
};

struct OsiFact : OsiReadable
{
    std::vector<OsiValue> columns;

    void read(OsiReader& reader) override;
};

struct OsiDatabase : OsiReadable
{
    uint32_t index;
    OsiParameterList parameters;
    std::vector<OsiFact> facts;

    void read(OsiReader& reader) override;
};

struct OsiGoal : OsiReadable
{
    uint32_t index;
    std::string name;
    int8_t subGoalCombination;
    std::vector<uint32_t> parentGoals;
    std::vector<uint32_t> subGoals;
    int8_t flags; // 0x02 = Child goal
    std::vector<OsiCall> initCalls;
    std::vector<OsiCall> exitCalls;

    void read(OsiReader& reader) override;
};

struct Story
{
    Story();
    ~Story() = default;
    Story(Story&&) noexcept;
    Story& operator=(Story&&) noexcept;
    Story(const Story&) = delete;
    Story& operator=(const Story&) = delete;

    OsiVersion version() const;
    bool isAlias(uint32_t type) const;
    OsiValueType resolveAlias(OsiValueType type) const;

    SaveFileHeader header{};
    std::unordered_map<uint16_t, OsiEnum> enums;
    std::unordered_map<uint32_t, OsiAdapter> adapters;
    std::unordered_map<uint32_t, OsiDatabase> databases;
    std::unordered_map<uint32_t, OsiGoal> goals;
    std::unordered_map<uint32_t, OsiNode::Ptr> nodes;
    std::unordered_map<uint8_t, OsiType> types;
    std::unordered_map<uint8_t, uint8_t> typeAliases;

    std::vector<OsiDivObject> divObjects;
    std::vector<OsiFunction> functions;
    std::vector<std::string> stringTable;
    std::vector<OsiCall> globalActions;
};
