#pragma once
#include "LicenseView.h"
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
        COMMAND_ID_HANDLER3(IDC_B_LICENSE, OnLicense)
    END_MSG_MAP()

private:
    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    LRESULT OnCtlColorStatic(HDC hDC, HWND hWndStatic);
    void OnCancel();
    void OnOK();
    void OnLicense();

    CFont m_boldFont;
    LONG m_cyExpanded = 0;
    LONG m_cyCollapsed = 0;
    BOOL m_bExpanded = FALSE;
    LicenseView m_view;
};
