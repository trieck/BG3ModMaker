#pragma once
#include <atlstr.h>

class StringHelper
{
public:
    static CStringW fromUTF8(LPCSTR input);
    static CStringW fromUTF8(const CStringA& input);
    static CStringW fromUTF8(const CHAR* input, size_t len);

    static CStringA toUTF8(LPCWSTR input);
    static CStringA toUTF8(const CStringW& input);
    static CStringA toUTF8(const WCHAR* input, size_t len);
};
