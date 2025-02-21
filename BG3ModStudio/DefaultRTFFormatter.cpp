#include "stdafx.h"
#include "DefaultRTFFormatter.h"

CStringW DefaultRTFFormatter::Format(TextStream& stream)
{
    CStringW output(L"{\\rtf\\ansi\\deff0{\\fonttbl{\\f0 Cascadia Mono;\\f1 Courier New;}}\n\\cf1\n");

    auto strText = stream.ReadString();

    output += EscapeRTF(strText);

    output += L'}';

    return output;
}
