#include "stdafx.h"
#include "AboutDlg.h"

BOOL AboutDlg::OnInitDialog(HWND, LPARAM)
{
    m_tabCtrl = GetDlgItem(IDC_TAB_ABOUT);
    m_static = GetDlgItem(IDC_ABOUT_TEXT);

    if (m_tabCtrl.IsWindow() && m_static.IsWindow()) {
        // Set the tab control to have a single tab
        TCITEM tie; 
        tie.mask = TCIF_TEXT;
        tie.pszText = const_cast<LPSTR>("RopePad");
        tie.cchTextMax = 256;
        m_tabCtrl.InsertItem(0, &tie);
    }

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}

void AboutDlg::OnClose()
{
    EndDialog(IDOK);
}
