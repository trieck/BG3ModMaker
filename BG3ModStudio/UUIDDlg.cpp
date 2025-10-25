#include "stdafx.h"
#include "UUIDDlg.h"
#include "Util.h"

void UUIDDlg::OnCancel()
{
    EndDialog(IDCANCEL);
}

LRESULT UUIDDlg::OnInitDialog(HWND, LPARAM)
{
    m_font = AtlCreateControlFont();
    m_uuid = GetDlgItem(IDC_ST_UUID);

    ATLASSERT(m_uuid.IsWindow());
    m_uuid.SetFont(m_font);
    m_uuid.ModifyStyle(0, SS_NOTIFY);

    m_handle = GetDlgItem(IDC_CHK_UUID_HANDLE);
    ATLASSERT(m_handle.IsWindow());

    CenterWindow(GetParent());

    auto icon = Util::LoadBitmapAsIcon(ID_TOOL_UUID, 32, 32);
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    return TRUE; // Let the system set the focus
}

void UUIDDlg::OnGenerateUUID()
{
    auto uuid = Util::MakeUUID(m_handle.GetCheck() == BST_CHECKED);

    m_uuid.SetWindowText(uuid);
}

BOOL UUIDDlg::OnSetCursor(CWindow hWnd, UINT, UINT)
{
    if (hWnd == m_uuid) { // hand cursor for UUID static control
        SetCursor(LoadCursor(nullptr, IDC_HAND));
        return TRUE;
    }
    return FALSE;
}

void UUIDDlg::OnUUIDClicked()
{
    CString s;
    m_uuid.GetWindowText(s);
    if (!s.IsEmpty()) {
        Util::CopyToClipboard(*this, s);
    }
}
