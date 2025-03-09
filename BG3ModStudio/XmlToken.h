#pragma once

enum class XmlTokenType
{
    TT_UNKNOWN = -1,
    TT_EMPTY,
    TT_TAG_START,
    TT_TAG_END,
    TT_TAG_SELF_CLOSE,
    TT_TAG_NAME,
    TT_ATTRIBUTE_NAME,
    TT_ATTRIBUTE_VALUE,
    TT_TEXT,
    TT_COMMENT,
    TT_CDATA,
    TT_DOCTYPE,
    TT_INSTRUCTION_START,
    TT_INSTRUCTION_END,
    TT_WHITESPACE,
    TT_QUOTE,
    TT_EQUAL,
    TT_ERROR
};

struct XmlToken
{
    XmlToken() = default;
    XmlToken(const XmlToken&) = default;
    XmlToken(XmlToken&&) = default;

    XmlToken& operator=(const XmlToken& rhs) = default;
    XmlToken& operator=(XmlToken&& rhs) = default;

    CStringA GetValue() const;
    XmlTokenType GetType() const;
    CStringA GetTypeAsString() const;

    XmlTokenType type{ XmlTokenType::TT_EMPTY };
    CStringA value;
};
