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

    return TRUE; // Let the system set the focus
}

void UUIDDlg::OnGenerateUUID()
{
    GUID guid;
    if (FAILED(CoCreateGuid(&guid))) {
        AtlMessageBox(*this, L"Failed to generate UUID", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    wchar_t buffer[40];
    StringFromGUID2(guid, buffer, _countof(buffer));

    // Strip curly braces
    auto* pbuf = buffer;
    size_t len = wcslen(buffer);
    if (buffer[0] == L'{') {
        buffer[len - 1] = L'\0';
        pbuf++;
    }

    // Lowercase
    std::ranges::transform(pbuf, pbuf + wcslen(pbuf), pbuf, ::towlower);

    CString s(pbuf);
    if (m_handle.GetCheck() == BST_CHECKED) { // Larian handle format
        s.Replace(L"-", L"g");
        s.Insert(0, L"h");
    }

    m_uuid.SetWindowText(s);
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
