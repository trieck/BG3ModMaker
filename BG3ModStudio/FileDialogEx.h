#pragma once

class FileDialogEx
{
public:
    enum DialogType {
        Open = 0,
        Save = 1,
        Folder = 2
    };

    explicit FileDialogEx(DialogType type,
        HWND hWndParent = nullptr,
        LPCTSTR lpszDefExt = nullptr,
        LPCTSTR lpszFileName = nullptr,
        DWORD dwFlags = 0,
        LPCTSTR lpszFilter = nullptr);

    INT_PTR DoModal();
    const std::vector<CString>& paths() const
    {
        return m_strPaths;
    }

private:
    DialogType m_type;
    CString m_strDefExt, m_strFilename;
    std::vector<CString> m_strPaths;
    std::vector<COMDLG_FILTERSPEC> m_filters;
    DWORD m_dwFlags;
    HWND m_hWndParent;
};

