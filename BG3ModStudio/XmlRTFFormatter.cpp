#include "stdafx.h"
#include "XmlRTFFormatter.h"

namespace { // anonymous

    enum Color {
        COLOR_TEXT = 1,
        COLOR_KEYWORD,
        COLOR_PROPERTY,
        COLOR_COMMENT,
        COLOR_LITERAL,
        COLOR_TYPE,
    };

    Color GetColor(XmlTokenType type)
    {
        switch (type) {
        case XmlTokenType::TT_TAG_NAME:
            return COLOR_KEYWORD;
        case XmlTokenType::TT_ATTRIBUTE_NAME:
        case XmlTokenType::TT_ATTRIBUTE_VALUE:
            return COLOR_PROPERTY;
        case XmlTokenType::TT_COMMENT:
            return COLOR_COMMENT;
        case XmlTokenType::TT_CDATA:
            return COLOR_LITERAL;
        case XmlTokenType::TT_DOCTYPE:
            return COLOR_TYPE;
        default:
            return COLOR_TEXT;
        }
    };

}  // anonymous

CStringW XmlRTFFormatter::Format(TextStream& stream)
{
    CString output(_T("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}"
        "{\\colortbl;"
        "\\red0\\green0\\blue0;" // COLOR_TEXT
        "\\red0\\green0\\blue200;" // COLOR_KEYWORD
        "\\red200\\green0\\blue0;" // COLOR_PROPERTY
        "\\red0\\green128\\blue0;" // COLOR_COMMENT
        "\\red200\\green0\\blue200;" // COLOR_LITERAL
        "\\red128\\green0\\blue0;" // COLOR_TYPE
        ";}\n"
        "\\cf1\n"));

    auto strText = stream.ReadString();

    m_tokenizer.SetInput(strText);

    COLORREF lastColor = COLOR_TEXT;

    while (auto token = m_tokenizer.GetNextToken()) {
        auto color = GetColor(token->GetType());

        if (lastColor != color) {
            CStringW strColor;
            strColor.Format(L"\\cf%d\n", color);
            output += strColor;
        }

        output += EscapeRTF(token->GetValue());

        lastColor = color;
    }

    output += L'}';

    return output;
}
