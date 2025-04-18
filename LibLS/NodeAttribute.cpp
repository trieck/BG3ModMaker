#include "pch.h"

#include "Exception.h"
#include "NodeAttribute.h"
#include "StringHelper.h"

static AttributeValue parseString(const std::string& str, AttributeType type);

std::string TranslatedStringT::str() const
{
    if (!value.empty()) {
        return value;
    }

    return std::format("{};{}", handle, version);
}

std::string TranslatedFSStringT::str() const
{
    if (!value.empty()) {
        return value;
    }

    std::string args;
    for (const auto& arg : arguments) {
        args += std::format("{}={};", arg.key, arg.value);
    }

    return std::format("{};{};{}", handle, version, args);
}

std::string FixedStringT::str() const
{
    return content;
}

std::string LSWStringT::str() const
{
    return StringHelper::toUTF8(content.c_str()).GetString();
}

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

void NodeAttribute::fromString(const std::string& str)
{
    m_value = parseString(str, m_type);
}

bool NodeAttribute::isValid() const
{
    return m_type != None;
}

template <typename T>
struct is_std_array : std::false_type
{
};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_std_array_v = is_std_array<T>::value;

std::string NodeAttribute::str() const
{
    return std::visit([]<typename T>(const T& val) -> std::string {
        if constexpr (std::is_same_v<T, std::string>) {
            return val;
        } else if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(val);
        } else if constexpr (requires { val.str(); }) {
            return val.str();
        } else if constexpr (is_std_array_v<T>) {
            std::string result;
            for (const auto& item : val) {
                if (!result.empty()) {
                    result += " ";
                }
                result += std::to_string(item);
            }
            return result;
        }

        return "";
    }, m_value);
}

AttributeValue NodeAttribute::value() const
{
    if (std::holds_alternative<std::monostate>(m_value)) {
        return {};
    }

    return m_value;
}

void NodeAttribute::setValue(AttributeValue value)
{
    m_value = std::move(value);
}

AttributeValue parseString(const std::string& str, AttributeType type)
{
    switch (type) {
    case Byte:
        return static_cast<uint8_t>(std::stoi(str));
    case Short:
        return static_cast<int16_t>(std::stoi(str));
    case UShort:
        return static_cast<uint16_t>(std::stoul(str));
    case Int:
        return std::stoi(str);
    case UInt:
        return static_cast<uint32_t>(std::stoul(str));
    case Float:
        return std::stof(str);
    case Double:
        return std::stod(str);
    case IVec2:
    case IVec3:
    case IVec4:
    case Vec2:
    case Vec3:
    case Vec4:
    case Mat2:
    case Mat3:
    case Mat3x4:
    case Mat4x3:
    case Mat4:
        ASSERT(0);
        return {};
    case Bool:
        if (_stricmp(str.c_str(), "true") == 0 || str == "1") {
            return true;
        }
        return false;
    case String:
    case Path:
    case FixedString:
    case LSString:
    case WString:
    case LSWString:
        return str;
    case ULongLong:
        return std::stoull(str);
    case ScratchBuffer:
        // TODO: convert from Base64
        return {};
    case Long:
        return std::stoll(str);
    case Int8:
        return static_cast<int8_t>(std::stoi(str));
    case TranslatedString:
        ASSERT(0);
        return {};
    case TranslatedFSString:
        ASSERT(0);
        return {};
    case Uuid:
        return UUIDT::fromString(str);
    case Int64:
        return std::stoll(str);
    default:
        throw Exception("Unsupported attribute type.");
    }
}
