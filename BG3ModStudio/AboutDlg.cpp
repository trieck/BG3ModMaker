#include "stdafx.h"
#include "AboutDlg.h"
#include "Version.h"

namespace { // anonymous

void SetDlgItemFont(HWND hDlg, int nIDDlgItem, HFONT hFont)
{
    auto hControl = GetDlgItem(hDlg, nIDDlgItem);
    if (hControl) {
        SendMessage(hControl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }
}
}

LRESULT AboutDlg::OnInitDialog(HWND, LPARAM)
{
    auto icon = LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    SendDlgItemMessage(IDC_APPICON, STM_SETICON, reinterpret_cast<WPARAM>(icon), 0);

    // Bold font for app name
    LOGFONT lf{};
    CFontHandle sysFont = GetFont();
    sysFont.GetLogFont(&lf);
    _tcscpy_s(lf.lfFaceName, _T("Tahoma"));
    lf.lfWeight = FW_BOLD;

    // Store as member so it outlives the dialog
    m_boldFont.CreateFontIndirect(&lf);
    SetDlgItemFont(m_hWnd, IDC_APPNAME, m_boldFont);

    CString ver;
    ver.Format(_T("Version %s"), _T(APP_VERSION_STR));
    SetDlgItemText(IDC_VERSION, ver);

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}

LRESULT AboutDlg::OnCtlColorStatic(HDC hDC, HWND hWndStatic)
{
    if (hWndStatic == GetDlgItem(IDC_VERSION)) {
        ::SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
    }

    ::SetBkColor(hDC, GetSysColor(COLOR_3DFACE));

    return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_3DFACE));
}

void AboutDlg::OnCancel()
{
    EndDialog(IDCANCEL);
}

void AboutDlg::OnOK()
{
    EndDialog(IDOK);
}
