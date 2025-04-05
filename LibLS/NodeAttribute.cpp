#include "pch.h"

#include "Exception.h"
#include "NodeAttribute.h"

NodeAttribute::NodeAttribute() : m_type(None)
{
}

NodeAttribute::NodeAttribute(AttributeType type) : m_type(type)
{
}

AttributeType NodeAttribute::type() const
{
    return m_type;
}

std::string NodeAttribute::typeStr() const
{
    switch (m_type) {
    case None:
        return "None";
    case Byte:
        return "Byte";
    case Short:
        return "Short";
    case UShort:
        return "UShort";
    case Int:
        return "Int";
    case UInt:
        return "UInt";
    case Float:
        return "Float";
    case Double:
        return "Double";
    case IVec2:
        return "IVec2";
    case IVec3:
        return "IVec3";
    case IVec4:
        return "IVec4";
    case Vec2:
        return "Vec2";
    case Vec3:
        return "Vec3";
    case Vec4:
        return "Vec4";
    case Mat2:
        return "Mat2";
    case Mat3:
        return "Mat3";
    case Mat3x4:
        return "Mat3x4";
    case Mat4x3:
        return "Mat4x3";
    case Mat4:
        return "Mat4";
    case Bool:
        return "Bool";
    case String:
        return "String";
    case Path:
        return "Path";
    case FixedString:
        return "FixedString";
    case LSString:
        return "LSString";
    case ULongLong:
        return "ULongLong";
    case ScratchBuffer:
        return "ScratchBuffer";
    case Long:
        return "Long";
    case Int8:
        return "Int8";
    case TranslatedString:
        return "TranslatedString";
    case WString:
        return "WString";
    case LSWString:
        return "LSWString";
    case Uuid:
        return "UUID";
    case Int64:
        return "Int64";
    case TranslatedFSString:
        return "TranslatedFSString";
    }

    throw Exception("Unsupported attribute type.");
}

std::string TranslatedString_T::str() const
{
    if (!value.empty()) {
        return value;
    }

    return std::format("{};{}", handle, version);
}
