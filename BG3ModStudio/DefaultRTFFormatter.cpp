#include "stdafx.h"
#include "DefaultRTFFormatter.h"

CStringA DefaultRTFFormatter::GetDefaultFormat() const
{
    CStringA output("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}\n\\cf1\n");
    return output;
}

CStringA DefaultRTFFormatter::Format(UTF8Stream& stream)
{
    CStringA output = GetDefaultFormat();

    auto strText = stream.ReadString();

    output += EscapeRTF(strText);

    output += "\\par}";

    return output;
}
