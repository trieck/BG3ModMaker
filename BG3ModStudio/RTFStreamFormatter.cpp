#include "stdafx.h"
#include "RTFStreamFormatter.h"

CStringA RTFStreamFormatter::EscapeRTF(const CStringA& input)
{
    CStringA output(input);

    output.Replace("\\", "\\\\");
    output.Replace("\n", "\\line\n");
    output.Replace("\t", "\\tab");
    output.Replace("{", "\\{");
    output.Replace("}", "\\}");

    return output;
}
