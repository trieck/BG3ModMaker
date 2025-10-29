#include "stdafx.h"
#include "LocaFileView.h"
#include "StringHelper.h"

LocaFileView::LocaFileView()
{
}

BOOL LocaFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE;

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);
    if (!hWnd) {
        return FALSE;
    }

    return TRUE;
}

LRESULT LocaFileView::OnCreate(LPCREATESTRUCT pcs)
{
    m_list.Create(*this, rcDefault, nullptr,
                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
                  | LVS_REPORT | LVS_SINGLESEL | LVS_OWNERDATA, WS_EX_CLIENTEDGE);
    ATLASSERT(m_list.IsWindow());

    m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    auto font = AtlCreateControlFont();
    m_list.SetFont(font);

    m_list.InsertColumn(0, _T("Key"), LVCFMT_LEFT, 100);
    m_list.InsertColumn(1, _T("Version"), LVCFMT_LEFT, 150);
    m_list.InsertColumn(2, _T("Text"), LVCFMT_LEFT, 150);

    return 0;
}

void LocaFileView::OnSize(UINT nType, CSize size)
{
    if (m_list.IsWindow()) {
        m_list.MoveWindow(0, 0, size.cx, size.cy);
    }
}

LRESULT LocaFileView::OnGetDispInfo(NMHDR* pNMHDR)
{
    auto* plvdi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

    if (plvdi->hdr.hwndFrom != m_list) {
        return 0;
    }

    if ((plvdi->item.mask & LVIF_TEXT) == 0) {
        return 0;
    }

    auto index = plvdi->item.iItem;
    auto subItem = plvdi->item.iSubItem;
    if (index < 0 || static_cast<size_t>(index) >= m_resource.entries.size()) {
        return 0;
    }

    const auto& entry = m_resource.entries[index];
    CString text;
    switch (subItem) {
    case 0:
        text = StringHelper::fromUTF8(entry.key.c_str());
        break;
    case 1:
        text = CString(std::to_string(entry.version).c_str());
        break;
    case 2:
        text = StringHelper::fromUTF8(entry.text.c_str());
        break;
    default:
        break;
    }
    if (!text.IsEmpty()) {
        _tcsncpy_s(plvdi->item.pszText, plvdi->item.cchTextMax, text, _TRUNCATE);
    } else {
        plvdi->item.pszText[0] = L'\0';
    }

    return 0;
}

void LocaFileView::Populate()
{
    m_list.DeleteAllItems();

    m_list.SetItemCountEx(static_cast<int>(m_resource.entries.size()),
                          LVSICF_NOINVALIDATEALL);
}

BOOL LocaFileView::LoadFile(const CString& path)
{
    auto utf8Path = StringHelper::toUTF8(path);

    try {
        m_resource = LocaReader::Read(utf8Path.GetString());
        Populate();
    } catch (const std::exception& e) {
        ATLTRACE("Failed to read file: %s\n", e.what());
        return FALSE;
    }

    return TRUE;
}

BOOL LocaFileView::LoadBuffer(const CString& path, const ByteBuffer& buffer)
{
    try {
        m_resource = LocaReader::Read(buffer);
        Populate();
    } catch (const std::exception& e) {
        ATLTRACE("Failed to read buffer: %s\n", e.what());
        return FALSE;
    }

    return TRUE;
}

BOOL LocaFileView::SaveFile()
{
    return TRUE;
}

BOOL LocaFileView::SaveFileAs(const CString& path)
{
    return TRUE;
}

BOOL LocaFileView::Destroy()
{
    if (m_hWnd == nullptr) {
        return FALSE;
    }

    return DestroyWindow();
}

BOOL LocaFileView::IsDirty() const
{
    return FALSE;
}

BOOL LocaFileView::IsEditable() const
{
    return FALSE;
}

BOOL LocaFileView::IsText() const
{
    return FALSE;
}

const CString& LocaFileView::GetPath() const
{
    return m_path;
}

void LocaFileView::SetPath(const CString& path)
{
    m_path = path;
}

FileEncoding LocaFileView::GetEncoding() const
{
    return UNKNOWN;
}

LocaFileView::operator HWND() const
{
    return m_hWnd;
}
