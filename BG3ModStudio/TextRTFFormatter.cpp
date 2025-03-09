#include "stdafx.h"
#include "TextRTFFormatter.h"

namespace { //

enum TextColor
{
    COLOR_TEXT = 1,
    COLOR_WORD,
    COLOR_NUMBER,
    COLOR_QUOTED_STRING,
};

TextColor GetColor(TextTokenType type)
{
    switch (type)
    {
    case TextTokenType::TT_WORD:
        return COLOR_WORD;
    case TextTokenType::TT_NUMBER:
        return COLOR_NUMBER;
    case TextTokenType::TT_QUOTED_STRING:
        return COLOR_QUOTED_STRING;
    default:
        return COLOR_TEXT;
    }
}

} // anonymous

CStringA TextRTFFormatter::GetDefaultFormat() const
{
    CStringA output("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}"
        "{\\colortbl;"
        "\\red0\\green0\\blue0;"    // COLOR_TEXT (Black)
        "\\red0\\green0\\blue255;"  // COLOR_WORD (Blue)
        "\\red0\\green128\\blue0;"  // COLOR_NUMBER (Green)
        "\\red255\\green0\\blue0;"  // COLOR_QUOTED_STRING (Red)
        ";}\r\n"
        "\\cf1\r\n");

    return output;
}

CStringA TextRTFFormatter::Format(UTF8Stream& stream)
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
