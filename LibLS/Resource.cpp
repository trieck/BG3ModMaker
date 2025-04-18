#include "pch.h"
#include "Resource.h"

struct CaseInsensitiveHash
{
    std::size_t operator()(const std::string& key) const
    {
        std::size_t h = 0;

        for (char c : key) {
            h = h * 31 + std::tolower(static_cast<unsigned char>(c));
        }

        return h;
    }
};

struct CaseInsensitiveEqual
{
    bool operator()(const std::string& a, const std::string& b) const
    {
        return a.size() == b.size() &&
            std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
                return std::tolower(static_cast<unsigned char>(ac)) ==
                    std::tolower(static_cast<unsigned char>(bc));
            });
    }
};

AttributeType AttributeTypeMaps::typeToId(const std::string& type)
{
    static const std::unordered_map<std::string, AttributeType, CaseInsensitiveHash, CaseInsensitiveEqual> m = {
        {"None", None},
        {"uint8", Byte},
        {"int8", Int8},
        {"int16", Short},
        {"uint16", UShort},
        {"int32", Int},
        {"uint32", UInt},
        {"int64", Long},
        {"uint64", ULongLong},
        {"float", Float},
        {"double", Double},
        {"bool", Bool},
        {"ivec2", IVec2},
        {"ivec3", IVec3},
        {"ivec4", IVec4},
        {"fvec2", Vec2},
        {"fvec3", Vec3},
        {"fvec4", Vec4},
        {"mat2x2", Mat2},
        {"mat3x3", Mat3},
        {"mat3x4", Mat3x4},
        {"mat4x3", Mat4x3},
        {"mat4x4", Mat4},
        {"string", String},
        {"path", Path},
        {"FixedString", FixedString},
        {"LSString", LSString},
        {"guid", Uuid},
        {"TranslatedString", TranslatedString},
        {"TranslatedFSString", TranslatedFSString},
        {"WString", WString},
        {"LSWString", LSWString},
        {"ScratchBuffer", ScratchBuffer}
    };

    if (auto it = m.find(type); it != m.end()) {
        return it->second;
    }

    return None;
}

std::string AttributeTypeMaps::idToType(AttributeType type)
{
    static const std::unordered_map<AttributeType, std::string> m = {
        {None, "None"},
        {Byte, "uint8"},
        {Int8, "int8"},
        {Short, "int16"},
        {UShort, "uint16"},
        {Int, "int32"},
        {UInt, "uint32"},
        {Long, "int64"},
        {ULongLong, "uint64"},
        {Float, "float"},
        {Double, "double"},
        {Bool, "bool"},
        {IVec2, "ivec2"},
        {IVec3, "ivec3"},
        {IVec4, "ivec4"},
        {Vec2, "fvec2"},
        {Vec3, "fvec3"},
        {Vec4, "fvec4"},
        {Mat2, "mat2x2"},
        {Mat3, "mat3x3"},
        {Mat3x4, "mat3x4"},
        {Mat4x3, "mat4x3"},
        {Mat4, "mat4x4"},
        {String, "string"},
        {Path, "path"},
        {FixedString, "FixedString"},
        {LSString, "LSString"},
        {Uuid, "guid"},
        {TranslatedString, "TranslatedString"},
        {TranslatedFSString, "TranslatedFSString"},
        {WString, "WString"},
        {LSWString, "LSWString"}
    };

    if (auto it = m.find(type); it != m.end()) {
        return it->second;
    }

    return "";
}
