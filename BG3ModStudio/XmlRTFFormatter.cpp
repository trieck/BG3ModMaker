#include "stdafx.h"
#include "XmlRTFFormatter.h"

namespace { // anonymous

    enum XmlColor {
        COLOR_TEXT = 1,       // Default text
        COLOR_TAG,            // <tag>
        COLOR_ATTRIBUTE,      // attribute_name=
        COLOR_VALUE,          // "attribute_value"
        COLOR_COMMENT,        // <!-- comment -->
        COLOR_CDATA,          // <![CDATA[...]]>
        COLOR_DOCTYPE         // <!DOCTYPE ...>
    };


    XmlColor GetColor(XmlTokenType type)
    {
        switch (type) {
        case XmlTokenType::TT_TAG_NAME:
            return COLOR_TAG;
        case XmlTokenType::TT_ATTRIBUTE_NAME:
            return COLOR_ATTRIBUTE;
        case XmlTokenType::TT_ATTRIBUTE_VALUE:
            return COLOR_VALUE;
        case XmlTokenType::TT_COMMENT:
            return COLOR_COMMENT;
        case XmlTokenType::TT_CDATA:
            return COLOR_CDATA;
        case XmlTokenType::TT_DOCTYPE:
            return COLOR_DOCTYPE;
        default:
            return COLOR_TEXT;
        }
    };

}  // anonymous

CStringA XmlRTFFormatter::Format(UTF8Stream& stream)
{
    CStringA output("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}"
        "{\\colortbl;"
        "\\red0\\green0\\blue0;"      // COLOR_TEXT (black)
        "\\red0\\green0\\blue200;"    // COLOR_TAG (blue)
        "\\red200\\green0\\blue0;"    // COLOR_ATTRIBUTE (red)
        "\\red0\\green128\\blue0;"    // COLOR_VALUE (green)
        "\\red128\\green128\\blue128;"// COLOR_COMMENT (gray)
        "\\red200\\green0\\blue200;"  // COLOR_CDATA (purple)
        "\\red128\\green0\\blue0;"    // COLOR_DOCTYPE (dark red)
        ";}\r\n"
        "\\cf1\r\n");

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
