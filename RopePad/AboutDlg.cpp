#include "stdafx.h"
#include "AboutDlg.h"

extern CAppModule _Module;

BOOL AboutDlg::OnInitDialog(HWND, LPARAM)
{
    CTabCtrl tabCtrl;
    tabCtrl.Attach(GetDlgItem(IDC_TAB_ABOUT));
    if (tabCtrl.IsWindow()) {
        // Set the tab control to have a single tab
        TCITEM tie;
        tie.mask = TCIF_TEXT;
        tie.pszText = const_cast<LPSTR>("RopePad");
        tie.cchTextMax = 256;
        tabCtrl.InsertItem(0, &tie);
    }

    CStatic aboutText;
    aboutText.Attach(GetDlgItem(IDC_ABOUT_TEXT));

    LOGFONT lf{};
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = 16;
    lf.lfWeight = FW_SEMIBOLD;
    Checked::tcsncpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Segoe UI"), _TRUNCATE);

    m_font.CreateFontIndirect(&lf);
    aboutText.SetFont(m_font);

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
