#include "stdafx.h"
#include "IRTFStreamFormatter.h"

CStringW IRTFStreamFormatter::EscapeRTF(const CStringW& input)
{
    CStringW output(input);

    output.Replace(L"\n", L"\\line\n");
    output.Replace(L"{", L"\\{");
    output.Replace(L"}", L"\\}");

    return output;
}
