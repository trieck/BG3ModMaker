#include "stdafx.h"
#include "AboutDlg.h"

extern CAppModule _Module;

BOOL AboutDlg::OnInitDialog(HWND, LPARAM)
{
    m_tabCtrl = GetDlgItem(IDC_TAB_ABOUT);
    //m_static = GetDlgItem(IDC_ABOUT_TEXT);

    if (m_tabCtrl.IsWindow()) {
        // Set the tab control to have a single tab
        TCITEM tie;
        tie.mask = TCIF_TEXT;
        tie.pszText = const_cast<LPSTR>("RopePad");
        tie.cchTextMax = 256;
        m_tabCtrl.InsertItem(0, &tie);
    }

    auto hWnd = GetDlgItem(IDC_ABOUT_IMG);
    if (hWnd) {
        auto hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
        if (hIcon) {
            SendMessage(hWnd, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(hIcon));
        }
    }

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}

void AboutDlg::OnClose()
{
    EndDialog(IDOK);
}
