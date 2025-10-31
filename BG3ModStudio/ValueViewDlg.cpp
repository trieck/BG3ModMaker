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

BOOL ValueViewDlg::OnInitDialog(HWND, LPARAM)
{
    if (!m_title.IsEmpty()) {
        SetWindowText(m_title);
    }

    m_font.CreatePointFont(90, L"Cascadia Mono");

    m_edit.Attach(GetDlgItem(IDC_E_VALUE));
    ATLASSERT(m_edit.IsWindow());

    m_edit.SetFont(m_font);
    m_edit.SetWindowText(m_value);
    m_edit.SetSel(0, 0);
    m_edit.SetFocus();

    CenterWindow();

    DlgResize_Init();

    return TRUE; // let the system set the focus
}

void ValueViewDlg::OnClose()
{
    EndDialog(0);
}

void ValueViewDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);
}
