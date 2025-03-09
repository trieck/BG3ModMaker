#include "stdafx.h"
#include "XmlFormat.h"
#include "XmlRTFFormatter.h"

namespace { // anonymous

CStringA GetXmlRTFColor(XmlColor type)
{
    auto color = XmlToColorRef(type);
    return RTFStreamFormatter::GetRTFColor(color);
}

}  // anonymous

CStringA XmlRTFFormatter::GetDefaultFormat() const
{
    CStringA output;

    output.Format("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}"
        "{\\colortbl;"
        "%s;%s;%s;%s;%s;%s;%s"
        ";}\r\n"
        "\\cf1\r\n", 
        GetXmlRTFColor(COLOR_TEXT),
        GetXmlRTFColor(COLOR_TAG),
        GetXmlRTFColor(COLOR_ATTRIBUTE),
        GetXmlRTFColor(COLOR_VALUE),
        GetXmlRTFColor(COLOR_COMMENT),
        GetXmlRTFColor(COLOR_CDATA),
        GetXmlRTFColor(COLOR_DOCTYPE));

    return output;
}

CStringA XmlRTFFormatter::Format(UTF8Stream& stream)
{
    CStringA output = GetDefaultFormat();

    auto strText = stream.ReadString();

    m_tokenizer.SetInput(strText);

    COLORREF lastColor = COLOR_TEXT;

    for (;;) {
        auto token = m_tokenizer.GetNextToken();
        if (token.GetType() == XmlTokenType::TT_EMPTY) {
            break;
        }

        auto color = XmlGetColor(token.GetType());

        if (lastColor != color) {
            CStringW strColor;
            strColor.Format(L"\\cf%d\r\n", color);
            output += strColor;
        }

        output += EscapeRTF(token.GetValue());

        lastColor = color;
    }

    output += "\\par}";

    return output;
}
