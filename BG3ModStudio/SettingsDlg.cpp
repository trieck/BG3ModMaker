#include "stdafx.h"
#include "FileDialogEx.h"
#include "SettingsDlg.h"

LRESULT SettingsDlg::OnInitDialog(HWND, LPARAM)
{
    m_indexPath = GetDlgItem(IDC_E_INDEX_FOLDER);
    ATLASSERT(m_indexPath.IsWindow());

    m_gameObjectPath = GetDlgItem(IDC_E_GAMEOBJECT_FOLDER);
    ATLASSERT(m_gameObjectPath.IsWindow());

    auto path = m_settings.GetString(_T("Settings"), _T("IndexPath"), _T(""));
    m_indexPath.SetWindowText(path);

    path = m_settings.GetString(_T("Settings"), _T("GameObjectPath"), _T(""));
    m_gameObjectPath.SetWindowText(path);

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

void SettingsDlg::OnBrowseGameObject()
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
    m_gameObjectPath.SetWindowText(path);
}

void SettingsDlg::OnOK()
{
    CString path;
    m_indexPath.GetWindowText(path);
    m_settings.SetString(_T("Settings"), _T("IndexPath"), path);

    m_gameObjectPath.GetWindowText(path);
    m_settings.SetString(_T("Settings"), _T("GameObjectPath"), path);

    EndDialog(IDOK);
}

void SettingsDlg::OnCancel()
{   
    EndDialog(IDCANCEL);
}
