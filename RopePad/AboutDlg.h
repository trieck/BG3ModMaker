#pragma once
#include "Resource.h"

class AboutDlg : public CDialogImpl<AboutDlg>
{
public:
    enum { IDD = IDD_ABOUT_DLG };

    BEGIN_MSG_MAP(AboutDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER3(IDOK, OnClose)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
    END_MSG_MAP()

    BOOL OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    void OnClose();

private:
    CFont m_font;
};

