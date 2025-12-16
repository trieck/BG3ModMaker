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
    auto icon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    SendDlgItemMessage(IDC_APPICON, STM_SETICON, reinterpret_cast<WPARAM>(icon), 0);

    ModifyStyle(0, WS_CLIPCHILDREN);

    // Bold font for app name
    LOGFONT lf{};
    CFontHandle sysFont = GetFont();
    sysFont.GetLogFont(&lf);
    Checked::tcsncpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Tahoma"), _TRUNCATE);
    lf.lfWeight = FW_BOLD;
    lf.lfHeight = MulDiv(lf.lfHeight, 5, 4); // 1.25x larger

    // Store as member so it outlives the dialog
    m_boldFont.CreateFontIndirect(&lf);
    SetDlgItemFont(m_hWnd, IDC_APPNAME, m_boldFont);

    CString ver;
    ver.Format(_T("Version %s"), _T(APP_VERSION_STR));
    SetDlgItemText(IDC_VERSION, ver);

    RECT rcWindow{}, rcClient{};
    GetWindowRect(&rcWindow);
    GetClientRect(&rcClient);

    auto cyNonClient = (rcWindow.bottom - rcWindow.top) -
        (rcClient.bottom - rcClient.top);

    auto wndLicense = GetDlgItem(IDC_LICENSE_TEXT);
    ATLASSERT(wndLicense.IsWindow());

    RECT rcLicense{}, rcClientLicense{};
    wndLicense.GetWindowRect(&rcLicense);
    wndLicense.GetClientRect(&rcClientLicense);
    ScreenToClient(&rcLicense);

    constexpr auto kCollapsedBottomPaddingPx = 6;

    m_cyExpanded = rcWindow.bottom - rcWindow.top;
    m_cyCollapsed = rcLicense.top + cyNonClient - kCollapsedBottomPaddingPx;

    // Create the license view
    m_view.Create(
        wndLicense,
        rcClientLicense,
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        WS_EX_CLIENTEDGE
    );
    if (!m_view.IsWindow()) {
        ATLTRACE("Failed to create license view\n");
        EndDialog(IDCANCEL);
        return FALSE;
    }

    m_view.ShowWindow(SW_HIDE);

    SetWindowPos(
        nullptr,
        0, 0,
        rcWindow.right - rcWindow.left,
        m_cyCollapsed,
        SWP_NOMOVE | SWP_NOZORDER
    );

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}

LRESULT AboutDlg::OnCtlColorStatic(HDC hDC, HWND hWndStatic)
{
    if (hWndStatic == GetDlgItem(IDC_VERSION)) {
        SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
    }

    SetBkColor(hDC, GetSysColor(COLOR_3DFACE));

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

void AboutDlg::OnLicense()
{
    m_bExpanded = !m_bExpanded;

    RECT rc{};
    GetWindowRect(&rc);

    SetWindowPos(
        nullptr,
        0, 0,
        rc.right - rc.left,
        m_bExpanded ? m_cyExpanded : m_cyCollapsed,
        SWP_NOMOVE | SWP_NOZORDER
    );

    m_view.ShowWindow(m_bExpanded ? SW_SHOW : SW_HIDE);

    SetDlgItemText(IDC_LICENSE, m_bExpanded ? _T("License <<") : _T("License >>"));
}
