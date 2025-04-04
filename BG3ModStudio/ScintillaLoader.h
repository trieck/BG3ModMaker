#pragma once

class ScintillaLoader
{
public:
    ScintillaLoader()        
    {
        Load();
    }

    ~ScintillaLoader()
    {
        Unload();
    }

private:
    void Load()
    {
        m_scintilla = LoadLibrary(L"Scintilla.dll");
        ATLASSERT(m_scintilla && L"Failed to load Scintilla.dll");
    }

    void Unload()
    {
        if (m_scintilla) {
            FreeLibrary(m_scintilla);
            m_scintilla = nullptr;
        }
    }

    HMODULE m_scintilla = nullptr;
};
