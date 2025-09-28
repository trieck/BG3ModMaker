#pragma once

#include "Settings.h"
#include "resources/resource.h"

class SettingsDlg : public CDialogImpl<SettingsDlg>
{
public:
    enum { IDD = IDD_SETTINGS };

    BEGIN_MSG_MAP(SettingsDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER3(IDC_B_BROWSE_INDEX, OnBrowseIndex)
        COMMAND_ID_HANDLER3(IDC_B_BROWSE_GAMEOBJECT, OnBrowseGameObject)
        COMMAND_ID_HANDLER3(IDC_B_BROWSE_GAMEDATA, OnBrowseGameData)
        COMMAND_ID_HANDLER3(IDC_B_BROWSE_ICON, OnBrowseIcon)
        COMMAND_ID_HANDLER3(IDOK, OnOK)
        COMMAND_ID_HANDLER3(IDCANCEL, OnCancel)
    END_MSG_MAP()

private:
    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    void OnBrowseIndex();
    void OnBrowseGameObject();
    void OnBrowseGameData();
    void OnBrowseIcon();
    void OnCancel();
    void OnOK();

    Settings m_settings;
    CEdit m_indexPath;
    CEdit m_gameObjectPath;
    CEdit m_gameDataPath;
    CEdit m_iconPath;
};

