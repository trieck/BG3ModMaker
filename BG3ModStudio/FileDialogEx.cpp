#include "stdafx.h"
#include "FileDialogEx.h"

namespace { // anonymous namespace

    std::vector<COMDLG_FILTERSPEC> parseFilters(LPCTSTR lpszFilter)
    {
        std::vector<COMDLG_FILTERSPEC> filters;
        if (lpszFilter == nullptr) {
            return filters;
        }

        auto len = _tcslen(lpszFilter);
        if (len == 0) {
            return filters;
        }

        auto p = lpszFilter;
        while (*p) {
            COMDLG_FILTERSPEC filter;
            filter.pszName = p;
            p += _tcslen(p) + 1;
            if (*p == _T('\0')) {
                break;
            }

            filter.pszSpec = p;
            filters.push_back(filter);

            p += _tcslen(p) + 1;
        }

        return filters;
    }
} // anonymous namespace

FileDialogEx::FileDialogEx(DialogType type, HWND hWndParent, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
                           LPCTSTR lpszFilter) : m_type(type), m_strDefExt(lpszDefExt), m_strFilename(lpszFileName),
                                                 m_filters(parseFilters(lpszFilter)),
                                                 m_dwFlags(dwFlags), m_hWndParent(hWndParent)
{
}

INT_PTR FileDialogEx::DoModal()
{
    CComPtr<IFileDialog> pfd;

    HRESULT hr;
    DWORD dwOptions;

    if (m_type == Open || m_type == Folder) {
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    } else if (m_type == Save) {
        hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    } else {
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    }

    if (FAILED(hr)) {
        ATLTRACE("Unable to create file dialog.\n");
        return -1;
    }

    hr = pfd->GetOptions(&dwOptions);
    if (FAILED(hr)) {
        ATLTRACE("Unable to get file dialog options.\n");
        return -1;
    }

    dwOptions |= m_dwFlags;

    if (m_type == Folder) {
        dwOptions |= FOS_PICKFOLDERS;
    } else {
        dwOptions |= FOS_FORCEFILESYSTEM;
        dwOptions &= ~FOS_PICKFOLDERS;
    }

    if (m_type == Save) {
        dwOptions |= FOS_OVERWRITEPROMPT;
    }

    hr = pfd->SetOptions(dwOptions);
    if (FAILED(hr)) {
        ATLTRACE("Unable to set file dialog options.\n");
        return -1;
    }

    if (!m_strDefExt.IsEmpty()) {
        hr = pfd->SetDefaultExtension(m_strDefExt);
        if (FAILED(hr)) {
            ATLTRACE("Unable to set file dialog default extension.\n");
            return -1;
        }
    }

    if (!m_strFilename.IsEmpty() && m_type == Save) {
        hr = pfd->SetFileName(m_strFilename);
        if (FAILED(hr)) {
            ATLTRACE("Unable to set file dialog filename.\n");
            return -1;
        }
    }

    if (!m_filters.empty()) {
        hr = pfd->SetFileTypes(static_cast<UINT>(m_filters.size()), m_filters.data());
        if (FAILED(hr)) {
            ATLTRACE("Unable to set file dialog filters.\n");
            return -1;
        }

        hr = pfd->SetFileTypeIndex(1); // Set the default file type index to the first (one-based) filter.
        if (FAILED(hr)) {
            ATLTRACE("Unable to set file dialog file type index.\n");
            return -1;
        }
    }

    hr = pfd->Show(m_hWndParent);
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        return IDCANCEL;
    }

    if (FAILED(hr)) {
        ATLTRACE("Unable to show file dialog.\n");
        return -1;
    }

    m_strPaths.clear();

    if (m_dwFlags & FOS_ALLOWMULTISELECT) {
        CComPtr<IFileOpenDialog> pfo;
        hr = pfd->QueryInterface(IID_PPV_ARGS(&pfo));
        if (FAILED(hr)) {
            ATLTRACE("Unable to get IFileOpenDialog interface.\n");
            return -1;
        }

        CComPtr<IShellItemArray> psia;
        hr = pfo->GetResults(&psia);
        if (FAILED(hr)) {
            ATLTRACE("Unable to get file dialog results.\n");
            return -1;
        }

        DWORD count;
        hr = psia->GetCount(&count);
        if (FAILED(hr)) {
            ATLTRACE("Unable to get file dialog result count.\n");
            return -1;
        }

        for (auto i = 0u; i < count; ++i) {
            CComPtr<IShellItem> psi;
            hr = psia->GetItemAt(i, &psi);
            if (FAILED(hr)) {
                ATLTRACE("Unable to get file item.\n");
                return -1;
            }

            CComHeapPtr<wchar_t> pszPath;
            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
            if (FAILED(hr)) {
                ATLTRACE("Unable to get file dialog path.\n");
                return -1;
            }

            m_strPaths.emplace_back(pszPath);
        }

        return IDOK;
    }

    CComPtr<IShellItem> psi;
    hr = pfd->GetResult(&psi);
    if (FAILED(hr)) {
        ATLTRACE("Unable to get file dialog result.\n");
        return -1;
    }

    CComHeapPtr<wchar_t> pszPath;
    hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
    if (FAILED(hr)) {
        ATLTRACE("Unable to get file dialog path.\n");
        return -1;
    }

    m_strPaths.emplace_back(pszPath);

    return IDOK;
}
