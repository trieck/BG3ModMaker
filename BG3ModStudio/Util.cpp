#include "stdafx.h"
#include "Util.h"

CString Util::GetModulePath()
{
    TCHAR path[MAX_PATH];
    GetModuleFileName(_Module.GetModuleInstance(), path, MAX_PATH);

    return path;
}

CString Util::GetModuleDirectory()
{
    auto path = GetModulePath();
    auto pos = path.ReverseFind('\\');
    if (pos == -1) {
        return CString();
    }
    return path.Left(pos);
}

CString Util::GetModuleFileNameW()
{
    auto path = GetModulePath();
    auto pos = path.ReverseFind('\\');
    if (pos == -1) {
        return CString();
    }
    return path.Mid(pos + 1);
}

CString Util::GetModuleFileNameWithoutExtension()
{
    auto path = GetModuleFileName();
    auto pos = path.ReverseFind('.');
    if (pos == -1) {
        return path;
    }
    return path.Left(pos);
}

CString Util::GetCurrentTimeString()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    CString strTime;
    strTime.Format(L"%02d:%02d:%02d.%03d",
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    return strTime;
}
