#include "stdafx.h"
#include "FileDialogEx.h"
#include "SettingsDlg.h"

LRESULT SettingsDlg::OnInitDialog(HWND, LPARAM)
{
    m_indexPath = GetDlgItem(IDC_E_INDEX_FOLDER);
    ATLASSERT(m_indexPath.IsWindow());

    auto path = m_settings.GetString(_T("Indexing"), _T("IndexPath"), _T(""));
    m_indexPath.SetWindowText(path);

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus to the first control
}

void SettingsDlg::OnBrowseIndex()
{
    FileDialogEx dlg(FileDialogEx::Folder, *this);
    auto hr = dlg.Construct();
    if (FAILED(hr)) {
        return;
    }

    if (dlg.DoModal() != IDOK) {
        return;
    }

    const auto& paths = dlg.paths();
    if (paths.empty()) {
        return;
    }

    auto path = paths.front();
    m_indexPath.SetWindowText(path);
}

void SettingsDlg::OnOK()
{
    CString indexPath;
    m_indexPath.GetWindowText(indexPath);

    Settings settings;
    settings.SetString(_T("Indexing"), _T("IndexPath"), indexPath);

    EndDialog(IDOK);
}

void SettingsDlg::OnCancel()
{   
    EndDialog(IDCANCEL);
}
