#include "pch.h"
#include "StringHelper.h"

CStringW StringHelper::fromUTF8(LPCSTR input)
{
    return fromUTF8(input, -1);
}

CStringW StringHelper::fromUTF8(const CStringA& input)
{
    return fromUTF8(input, -1);
}

CStringW StringHelper::fromUTF8(const CHAR* input, size_t len)
{
    auto destLen = MultiByteToWideChar(CP_UTF8, 0, input, static_cast<int>(len), nullptr, 0);

    CStringW dest;
    dest.GetBufferSetLength(destLen);

    MultiByteToWideChar(CP_UTF8, 0, input, static_cast<int>(len), dest.GetBuffer(), destLen);

    dest.ReleaseBuffer();

    return dest;
}

CStringA StringHelper::toUTF8(LPCWSTR input)
{
    return toUTF8(input, -1);
}

CStringA StringHelper::toUTF8(const CStringW& input)
{
    return toUTF8(input, -1);
}

CStringA StringHelper::toUTF8(const WCHAR* input, size_t len)
{
    auto destLen = WideCharToMultiByte(CP_UTF8, 0, input, static_cast<int>(len), nullptr, 0, nullptr, nullptr);

    CStringA dest;
    dest.GetBufferSetLength(destLen);

    WideCharToMultiByte(CP_UTF8, 0, input, static_cast<int>(len), dest.GetBuffer(), destLen, nullptr, nullptr);

    dest.ReleaseBuffer();

    return dest;
}
