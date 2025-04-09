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
        COMMAND_ID_HANDLER3(IDOK, OnOK)
        COMMAND_ID_HANDLER3(IDCANCEL, OnCancel)
    END_MSG_MAP()

private:
    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    void OnBrowseIndex();
    void OnCancel();
    void OnOK();

    Settings m_settings;
    CEdit m_indexPath;
};

