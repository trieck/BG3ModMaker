#include "stdafx.h"
#include "XmlToken.h"

CStringA XmlToken::GetValue() const
{
    return value;
}

XmlTokenType XmlToken::GetType() const
{
    return type;
}

CStringA XmlToken::GetTypeAsString() const
{
    switch (type) {
    case XmlTokenType::TT_EMPTY:
        return "Empty";
    case XmlTokenType::TT_TAG_START:
        return "Tag Start";
    case XmlTokenType::TT_TAG_END:
        return "Tag End";
    case XmlTokenType::TT_TAG_SELF_CLOSE:
        return "Tag Self Close";
    case XmlTokenType::TT_TAG_NAME:
        return "Tag Name";
    case XmlTokenType::TT_ATTRIBUTE_NAME:
        return "Attribute Name";
    case XmlTokenType::TT_ATTRIBUTE_VALUE:
        return "Attribute Value";
    case XmlTokenType::TT_TEXT:
        return "Text";
    case XmlTokenType::TT_COMMENT:
        return "Comment";
    case XmlTokenType::TT_CDATA:
        return "CDATA";
    case XmlTokenType::TT_DOCTYPE:
        return "DOCTYPE";
    case XmlTokenType::TT_INSTRUCTION_START:
        return "Instruction Start";
    case XmlTokenType::TT_INSTRUCTION_END:
        return "Instruction End";
    case XmlTokenType::TT_WHITESPACE:
        return "Whitespace";
    case XmlTokenType::TT_QUOTE:
        return "Quote";
    case XmlTokenType::TT_EQUAL:
        return "Equal";
    case XmlTokenType::TT_ERROR:
        return "Error";
    default:
        return "Unknown";
    }
}
