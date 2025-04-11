#include "pch.h"
#include "Resource.h"

AttributeType AttributeTypeMaps::typeToId(const std::string& type)
{
    static const std::unordered_map<std::string, AttributeType> m = {
        {"Bool", Bool},
        {"Byte", Byte},
        {"Double", Double},
        {"FixedString", FixedString},
        {"Float", Float},
        {"Int", Int},
        {"Int64", Int64},
        {"Int8", Int8},
        {"IVec2", IVec2},
        {"IVec3", IVec3},
        {"IVec4", IVec4},
        {"Long", Long},
        {"LSString", LSString},
        {"LSWString", LSWString},
        {"Mat2", Mat2},
        {"Mat3", Mat3},
        {"Mat3x4", Mat3x4},
        {"Mat4", Mat4},
        {"Mat4x3", Mat4x3},
        {"None", None},
        {"Path", Path},
        {"ScratchBuffer", ScratchBuffer},
        {"Short", Short},
        {"String", String},
        {"TranslatedFSString", TranslatedFSString},
        {"TranslatedString", TranslatedString},
        {"UInt", UInt},
        {"ULongLong", ULongLong},
        {"UShort", UShort},
        {"Uuid", Uuid},
        {"Vec2", Vec2},
        {"Vec3", Vec3},
        {"Vec4", Vec4},
        {"WString", WString}
    };

    if (auto it = m.find(type); it != m.end()) {
        return it->second;
    }

    return None;
}

std::string AttributeTypeMaps::idToType(AttributeType type)
{
    static const std::unordered_map<AttributeType, std::string> m = {
        {Bool, "Bool"},
        {Byte, "Byte"},
        {Double, "Double"},
        {FixedString, "FixedString"},
        {Float, "Float"},
        {Int, "Int"},
        {Int64, "Int64"},
        {Int8, "Int8"},
        {IVec2, "IVec2"},
        {IVec3, "IVec3"},
        {IVec4, "IVec4"},
        {Long, "Long"},
        {LSString, "LSString"},
        {LSWString, "LSWString"},
        {Mat2, "Mat2"},
        {Mat3, "Mat3"},
        {Mat3x4, "Mat3x4"},
        {Mat4, "Mat4"},
        {Mat4x3, "Mat4x3"},
        {None, "None"},
        {Path, "Path"},
        {ScratchBuffer, "ScratchBuffer"},
        {Short, "Short"},
        {String, "String"},
        {TranslatedFSString, "TranslatedFSString"},
        {TranslatedString, "TranslatedString"},
        {UInt, "UInt"},
        {ULongLong, "ULongLong"},
        {UShort, "UShort"},
        {Uuid, "Uuid"},
        {Vec2, "Vec2"},
        {Vec3, "Vec3"},
        {Vec4, "Vec4"},
        {WString, "WString"}
    };

    if (auto it = m.find(type); it != m.end()) {
        return it->second;
    }

    return "";
}
