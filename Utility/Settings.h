#pragma once

class Settings
{
public:
    Settings();
    ~Settings();

    CStringW GetString(const CStringW& section, const CStringW& key, const CStringW& defaultValue = L"");
    BOOL SetString(const CStringW& section, const CStringW& key, const CStringW& value);
    BOOL GetBool(const CStringW& section, const CStringW& key, BOOL defaultValue = FALSE);
    BOOL SetBool(const CStringW& section, const CStringW& key, BOOL value);
    int GetInt(const CStringW& section, const CStringW& key, int defaultValue = 0);
    BOOL SetInt(const CStringW& section, const CStringW& key, int value);
    float GetFloat(const CStringW& section, const CStringW& key, float defaultValue = 0.0f);
    BOOL SetFloat(const CStringW& section, const CStringW& key, float value);

private:
    CStringW GetPath();
};
