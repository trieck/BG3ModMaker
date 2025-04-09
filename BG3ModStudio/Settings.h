#pragma once

class Settings
{
public:
    Settings();
    ~Settings();

    CString GetString(const CString& section, const CString& key, const CString& defaultValue = _T(""));
    BOOL SetString(const CString& section, const CString& key, const CString& value);
    BOOL GetBool(const CString& section, const CString& key, BOOL defaultValue = FALSE);
    BOOL SetBool(const CString& section, const CString& key, BOOL value);
    int GetInt(const CString& section, const CString& key, int defaultValue = 0);
    BOOL SetInt(const CString& section, const CString& key, int value);
    float GetFloat(const CString& section, const CString& key, float defaultValue = 0.0f);
    BOOL SetFloat(const CString& section, const CString& key, float value);

private:
    CString GetPath();
};

