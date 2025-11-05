#include "stdafx.h"
#include "ValueViewDlg.h"

void ValueViewDlg::SetTitle(const CString& title)
{
    m_title = title;
    if (IsWindow()) {
        SetWindowText(m_title);
    }
}

void ValueViewDlg::SetValue(const CString& value)
{
    m_value = value;
    if (m_edit.IsWindow()) {
        m_edit.SetWindowText(m_value);
    }
}

BOOL ValueViewDlg::CreateFont()
{
    LOGFONT lf{};
    lf.lfHeight = -MulDiv(m_fontSize, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
    _tcscpy_s(lf.lfFaceName, _T("Cascadia Mono"));
    lf.lfWeight = FW_NORMAL;

    if (m_font) {
        m_font.DeleteObject();
    }

    return m_font.CreateFontIndirect(&lf) != nullptr;
}

LRESULT ValueViewDlg::OnCtlColorStatic(WPARAM wParam, LPARAM lParam)
{
    auto hDC = reinterpret_cast<HDC>(wParam);
    auto hWndCtrl = reinterpret_cast<HWND>(lParam);

    if (hWndCtrl == m_edit.m_hWnd) {
        SetBkColor(hDC, m_clrBkgnd);
        SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
        return reinterpret_cast<LRESULT>(m_brBackground.m_hBrush);
    }

    return 0;
}

BOOL ValueViewDlg::OnInitDialog(HWND, LPARAM)
{
    if (!m_title.IsEmpty()) {
        SetWindowText(m_title);
    }

    if (!m_brBackground.CreateSolidBrush(m_clrBkgnd)) {
        ATLTRACE(_T("Failed to create background brush\n"));
        return FALSE;
    }

    if (!CreateFont()) {
        ATLTRACE(_T("Failed to create font\n"));
        return FALSE;
    }

    m_edit.Attach(GetDlgItem(IDC_E_VALUE));
    ATLASSERT(m_edit.IsWindow());

    m_edit.SetFont(m_font);
    m_edit.SetWindowText(m_value);
    m_edit.ModifyStyle(WS_TABSTOP, 0);

    CenterWindow();

    DlgResize_Init();

    return TRUE; // let the system set the focus
}

LRESULT ValueViewDlg::OnMouseWheel(UINT nFlags, short zDelta, const CPoint&)
{
    if ((GetKeyState(VK_CONTROL) & 0x8000) == 0) {
        return 1;
    }

    if (zDelta > 0 && m_fontSize < 48) {
        m_fontSize++;
    } else if (zDelta < 0 && m_fontSize > 6) {
        m_fontSize--;
    }

    CreateFont();

    m_edit.SetFont(m_font);
    m_edit.Invalidate();

    return 1;
}

void ValueViewDlg::OnClose()
{
    Destroy();
}

void ValueViewDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);
}

void ValueViewDlg::OnShowWindow()
{
    if (m_edit.IsWindow()) {
        m_edit.SetSelNone();
    }
}
