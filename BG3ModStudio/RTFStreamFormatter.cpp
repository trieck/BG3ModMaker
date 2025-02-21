#include "stdafx.h"
#include "RTFStreamFormatter.h"

CStringW RTFStreamFormatter::EscapeRTF(const CStringW& input)
{
    CStringW output(input);

    output.Replace(L"\\", L"\\\\");
    output.Replace(L"\n", L"\\line\n");
    output.Replace(L"\t", L"\\tab");
    output.Replace(L"{", L"\\{");
    output.Replace(L"}", L"\\}");

    return output;
}
