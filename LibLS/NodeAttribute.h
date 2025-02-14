#pragma once

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
    UUID = 31,
    Int64 = 32,
    TranslatedFSString = 33,
    Max = TranslatedFSString // Last supported datatype, always keep this one at the end
};

class NodeAttribute
{
public:
    NodeAttribute();
    explicit NodeAttribute(AttributeType type);

    AttributeType type() const;
    std::string typeStr() const;

    std::string value() const { return m_value; }
    void setValue(const std::string& value) { m_value = value; }
private:
    AttributeType m_type;
    std::string m_value;
};

struct TranslatedString_T
{
    uint16_t version = 0;
    std::string value;
    std::string handle;

    std::string str() const;
};