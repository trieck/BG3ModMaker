#pragma once

#include "UUIDT.h"

enum AttributeType
{
    None = 0,
    Byte = 1,
    Short = 2,
    UShort = 3,
    Int = 4,
    UInt = 5,
    Float = 6,
    Double = 7,
    IVec2 = 8,
    IVec3 = 9,
    IVec4 = 10,
    Vec2 = 11,
    Vec3 = 12,
    Vec4 = 13,
    Mat2 = 14,
    Mat3 = 15,
    Mat3x4 = 16,
    Mat4x3 = 17,
    Mat4 = 18,
    Bool = 19,
    String = 20,
    Path = 21,
    FixedString = 22,
    LSString = 23,
    ULongLong = 24,
    ScratchBuffer = 25, // Seems to be unused?
    Long = 26,
    Int8 = 27,
    TranslatedString = 28,
    WString = 29,
    LSWString = 30,
    Uuid = 31,
    Int64 = 32,
    TranslatedFSString = 33,
    Max = TranslatedFSString // Last supported datatype, always keep this one at the end
};

struct TranslatedStringT
{
    TranslatedStringT() = default;
    TranslatedStringT(const TranslatedStringT&) = default;
    TranslatedStringT(TranslatedStringT&&) = default;

    TranslatedStringT& operator=(const TranslatedStringT&) = default;
    TranslatedStringT& operator=(TranslatedStringT&&) = default;

    virtual ~TranslatedStringT() = default;

    uint16_t version = 0;
    std::string value;
    std::string handle;

    virtual std::string str() const;
};

struct TranslatedFSStringT;

struct TranslatedFSStringArgument
{
    std::string key;
    std::string value;
    std::shared_ptr<TranslatedFSStringT> string;
};

struct TranslatedFSStringT : TranslatedStringT
{
    std::vector<TranslatedFSStringArgument> arguments;

    std::string str() const override;
};

struct FixedStringT
{
    std::string content;
    std::string str() const;
};

struct LSWStringT
{
    std::wstring content;
    std::string str() const;
};

using AttributeValue = std::variant <
    std::monostate, // None
    int8_t, // Int8
    uint8_t, // Byte
    int16_t, // Short
    uint16_t, // UShort
    int32_t, // Int
    uint32_t, // UInt
    int64_t, // Int64, Long
    uint64_t, // ULongLong
    float, // Float
    double, // Double
    bool, // Bool
    std::array<int32_t, 2>, // IVec2
    std::array<int32_t, 3>, // IVec3
    std::array<int32_t, 4>, // IVec4
    std::array<float, 2>, // Vec2
    std::array<float, 3>, // Vec3
    std::array<float, 4>, // Vec4, Mat2(2x2)
    std::array<float, 9>, // Mat3 (3x3)
    std::array<float, 12>, // Mat3x4, Mat4x3 (3x4)
    std::array<float, 16>, // Mat4 (4x4)
    std::string, // String, Path, LSString, WString (narrowed)
    FixedStringT, // FixedString
    TranslatedStringT, // TranslatedString
    TranslatedFSStringT, // TranslatedFSString
    LSWStringT, // LSWString
    UUIDT // Uuid
>;

class NodeAttribute
{
public:
    NodeAttribute();
    explicit NodeAttribute(AttributeType type);

    AttributeType type() const;
    AttributeValue value() const;
    bool isValid() const;
    std::string str() const;
    std::string typeStr() const;
    void fromString(const std::string& str);
    void setValue(AttributeValue value);

private:
    AttributeType m_type;
    AttributeValue m_value;
};
