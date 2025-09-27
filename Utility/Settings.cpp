#include "pch.h"
#include "Settings.h"

#include "StringHelper.h"

static constexpr auto BUFFER_SIZE = 1024;

Settings::Settings()
{
}

Settings::~Settings()
{
}

CStringW Settings::GetString(const CStringW& section, const CStringW& key, const CStringW& defaultValue)
{
    CStringW value;
    auto path = GetPath();

    auto* pbuf = value.GetBuffer(BUFFER_SIZE);
    auto result = GetPrivateProfileStringW(section, key, defaultValue, pbuf, BUFFER_SIZE, path);

    value.ReleaseBuffer();

    if (result == 0) {
        return defaultValue;
    }

    return value;
}

BOOL Settings::SetString(const CStringW& section, const CStringW& key, const CStringW& value)
{
    auto path = GetPath();

    return WritePrivateProfileStringW(section, key, value, path);
}

BOOL Settings::GetBool(const CStringW& section, const CStringW& key, BOOL defaultValue)
{
    auto value = GetString(section, key, defaultValue ? L"true" : L"false");
    if (value.CompareNoCase(L"true") == 0) {
        return TRUE;
    }
    if (value.CompareNoCase(L"false") == 0) {
        return FALSE;
    }
    return defaultValue;
}

BOOL Settings::SetBool(const CStringW& section, const CStringW& key, BOOL value)
{
    return SetString(section, key, value ? _T("true") : _T("false"));
}

int Settings::GetInt(const CStringW& section, const CStringW& key, int defaultValue)
{
    CStringW strDefault;
    strDefault.Format(L"%d", defaultValue);

    auto value = GetString(section, key, strDefault);

    return _wtoi(value);
}

BOOL Settings::SetInt(const CStringW& section, const CStringW& key, int value)
{
    CStringW strValue;
    strValue.Format(L"%d", value);

    return SetString(section, key, strValue);
}

float Settings::GetFloat(const CStringW& section, const CStringW& key, float defaultValue)
{
    CStringW strDefault;
    strDefault.Format(L"%f", defaultValue);

    auto value = GetString(section, key, strDefault);
    return static_cast<float>(_wtof(value));
}

BOOL Settings::SetFloat(const CStringW& section, const CStringW& key, float value)
{
    CStringW strValue;
    strValue.Format(L"%f", value);

    return SetString(section, key, strValue);
}

CStringW Settings::GetPath()
{
    WCHAR exePath[MAX_PATH]{};

    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);
    PathAppendW(exePath, L"settings.ini");

    return exePath;
}
