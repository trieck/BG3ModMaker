#include "stdafx.h"
#include "JSONRTFFormatter.h"

namespace { //

enum JsonColor
{
    COLOR_TEXT = 1, // Default text
    COLOR_BRACE, // { and }
    COLOR_BRACKET, // [ and ]
    COLOR_COLON, // :
    COLOR_COMMA, // ,
    COLOR_STRING, // "string"
    COLOR_NUMBER, // 123, -45.67, 0.12e3
    COLOR_BOOLEAN, // true, false
    COLOR_NULL, // null
    COLOR_ERROR // Invalid tokens
};

JsonColor GetColor(JsonTokenType type)
{
    switch (type) {
    case JsonTokenType::TT_OBJECT_START: // {
    case JsonTokenType::TT_OBJECT_END: // }
        return COLOR_BRACE;

    case JsonTokenType::TT_ARRAY_START: // [
    case JsonTokenType::TT_ARRAY_END: // ]
        return COLOR_BRACKET;

    case JsonTokenType::TT_COLON: // :
        return COLOR_COLON;

    case JsonTokenType::TT_COMMA: // ,
        return COLOR_COMMA;

    case JsonTokenType::TT_STRING: // "string"
        return COLOR_STRING;

    case JsonTokenType::TT_NUMBER: // 123, -45.67, 2e10
        return COLOR_NUMBER;

    case JsonTokenType::TT_BOOLEAN: // true, false
        return COLOR_BOOLEAN;

    case JsonTokenType::TT_NULL: // null
        return COLOR_NULL;

    case JsonTokenType::TT_ERROR: // Invalid characters
        return COLOR_ERROR;

    default:
        return COLOR_TEXT; // Whitespace and unknown tokens
    }
}

} // anonymous

CStringA JSONRTFFormatter::GetDefaultFormat() const
{
    CStringA output("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}"
        "{\\colortbl;"
        "\\red0\\green0\\blue0;" // COLOR_TEXT (Black)
        "\\red0\\green80\\blue160;" // COLOR_BRACE (Deep Blue)
        "\\red0\\green128\\blue128;" // COLOR_BRACKET (Dark Teal)
        "\\red196\\green98\\blue0;" // COLOR_COLON (Dark Orange)
        "\\red96\\green96\\blue96;" // COLOR_COMMA (Dark Gray)
        "\\red0\\green112\\blue0;" // COLOR_STRING (Dark Green)
        "\\red160\\green0\\blue0;" // COLOR_NUMBER (Dark Red)
        "\\red128\\green0\\blue128;" // COLOR_BOOLEAN (Dark Purple)
        "\\red70\\green130\\blue180;" // COLOR_NULL (Steel Blue)
        "\\red215\\green0\\blue0;" // COLOR_ERROR (Bold Red)
        ";}\r\n"
        "\\cf1\r\n");

    return output;
}

CStringA JSONRTFFormatter::Format(UTF8Stream& stream)
{
    CStringA output = GetDefaultFormat();

    auto strText = stream.ReadString();

    m_tokenizer.SetInput(strText);

    COLORREF lastColor = COLOR_TEXT;

    while (auto token = m_tokenizer.GetNextToken()) {
        auto color = GetColor(token->GetType());

        if (lastColor != color) {
            CStringW strColor;
            strColor.Format(L"\\cf%d\r\n", color);
            output += strColor;
        }

        output += EscapeRTF(token->GetValue());

        lastColor = color;
    }

    output += "\\par}";

    return output;
}
