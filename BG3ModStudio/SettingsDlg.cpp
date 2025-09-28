#include "stdafx.h"
#include "FileDialogEx.h"
#include "SettingsDlg.h"

LRESULT SettingsDlg::OnInitDialog(HWND, LPARAM)
{
    m_indexPath = GetDlgItem(IDC_E_INDEX_FOLDER);
    ATLASSERT(m_indexPath.IsWindow());

    m_gameObjectPath = GetDlgItem(IDC_E_GAMEOBJECT_FOLDER);
    ATLASSERT(m_gameObjectPath.IsWindow());

    m_gameDataPath = GetDlgItem(IDC_E_GAMEDATA_FOLDER);
    ATLASSERT(m_gameDataPath.IsWindow());

    m_iconPath = GetDlgItem(IDC_E_ICON_FOLDER);
    ATLASSERT(m_iconPath.IsWindow());

    auto path = m_settings.GetString(_T("Settings"), _T("IndexPath"));
    m_indexPath.SetWindowText(path);

    path = m_settings.GetString(_T("Settings"), _T("GameObjectPath"));
    m_gameObjectPath.SetWindowText(path);

    path = m_settings.GetString(_T("Settings"), _T("GameDataPath"));
    m_gameDataPath.SetWindowText(path);

    path = m_settings.GetString(_T("Settings"), _T("IconPath"));
    m_iconPath.SetWindowText(path);

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

void SettingsDlg::OnBrowseGameData()
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
    m_gameDataPath.SetWindowText(path);
}

void SettingsDlg::OnBrowseIcon()
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
    m_iconPath.SetWindowText(path);
}

void SettingsDlg::OnOK()
{
    CString path;
    m_indexPath.GetWindowText(path);
    m_settings.SetString(_T("Settings"), _T("IndexPath"), path);

    m_gameObjectPath.GetWindowText(path);
    m_settings.SetString(_T("Settings"), _T("GameObjectPath"), path);

    m_gameDataPath.GetWindowText(path);
    m_settings.SetString(_T("Settings"), _T("GameDataPath"), path);

    m_iconPath.GetWindowText(path);
    m_settings.SetString(_T("Settings"), _T("IconPath"), path);

    EndDialog(IDOK);
}

void SettingsDlg::OnCancel()
{
    EndDialog(IDCANCEL);
}
