#pragma once
#include "resources/resource.h"

class AboutDlg : public CDialogImpl<AboutDlg>
{
public:
    enum { IDD = IDD_ABOUT };

    BEGIN_MSG_MAP(AboutDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
        COMMAND_ID_HANDLER3(IDOK, OnOK)
        COMMAND_ID_HANDLER3(IDCANCEL, OnCancel)
    END_MSG_MAP()

private:
    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    LRESULT OnCtlColorStatic(HDC hDC, HWND hWndStatic);
    void OnCancel();
    void OnOK();
    CFont m_boldFont;
};
