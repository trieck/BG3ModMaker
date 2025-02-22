#include "stdafx.h"
#include "DefaultRTFFormatter.h"

CStringA DefaultRTFFormatter::Format(UTF8Stream& stream)
{
    CStringA output("{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}\n\\cf1\n");

    auto strText = stream.ReadString();

    output += EscapeRTF(strText);

    output += "\\par}";

    return output;
}
