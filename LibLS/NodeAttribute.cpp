#include "pch.h"
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

std::string TranslatedString_T::str() const
{
    if (!value.empty()) {
        return value;
    }

    return std::format("{};{}", handle, version);
}
