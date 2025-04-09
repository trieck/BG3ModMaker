#include "stdafx.h"
#include "Settings.h"

static constexpr auto BUFFER_SIZE = 1024;

Settings::Settings()
{
}

Settings::~Settings()
{
}

CString Settings::GetString(const CString& section, const CString& key, const CString& defaultValue)
{
    CString value;
    auto path = GetPath();

    auto* pbuf = value.GetBuffer(BUFFER_SIZE);
    auto result = GetPrivateProfileString(section, key, defaultValue, pbuf, BUFFER_SIZE, path);

    value.ReleaseBuffer();

    if (result == 0) {
        return defaultValue;
    }

    return value;
}

BOOL Settings::SetString(const CString& section, const CString& key, const CString& value)
{
    auto path = GetPath();
    
    return WritePrivateProfileString(section, key, value, path);
}

BOOL Settings::GetBool(const CString& section, const CString& key, BOOL defaultValue)
{
    auto value = GetString(section, key, defaultValue ? _T("true") : _T("false"));
    if (value.CompareNoCase(_T("true")) == 0) {
        return TRUE;
    }
    if (value.CompareNoCase(_T("false")) == 0) {
        return FALSE;
    }
    return defaultValue;
}

BOOL Settings::SetBool(const CString& section, const CString& key, BOOL value)
{
    return SetString(section, key, value ? _T("true") : _T("false"));
}

int Settings::GetInt(const CString& section, const CString& key, int defaultValue)
{
    CString strDefault;
    strDefault.Format(_T("%d"), defaultValue);

    auto value = GetString(section, key, strDefault);

    return _ttoi(value);
}

BOOL Settings::SetInt(const CString& section, const CString& key, int value)
{
    CString strValue;
    strValue.Format(_T("%d"), value);

    return SetString(section, key, strValue);
}

float Settings::GetFloat(const CString& section, const CString& key, float defaultValue)
{
    CString strDefault;
    strDefault.Format(_T("%f"), defaultValue);

    auto value = GetString(section, key, strDefault);
    return static_cast<float>(_ttof(value));
}

BOOL Settings::SetFloat(const CString& section, const CString& key, float value)
{
    CString strValue;
    strValue.Format(_T("%f"), value);

    return SetString(section, key, strValue);
}

CString Settings::GetPath()
{
    TCHAR exePath[MAX_PATH]{};

    GetModuleFileName(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpec(exePath);
    PathAppend(exePath, _T("settings.ini"));

    return exePath;
}
