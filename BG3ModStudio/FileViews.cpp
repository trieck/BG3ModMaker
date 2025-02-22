#include "stdafx.h"
#include "FileViews.h"

#include "FileViewFactory.h"

bool FilesView::CreateTabControl()
{
     m_tab.Create(this->m_hWnd, rcDefault, nullptr, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
         TCS_TOOLTIPS | TCS_OWNERDRAWFIXED | TCS_FIXEDWIDTH, 0, m_nTabID);
    ATLASSERT(m_tab.m_hWnd != NULL);
    if (m_tab.m_hWnd == nullptr) {
        ATLTRACE("Failed to create tab control.\n");
        return false;
    }

    auto font = AtlCreateControlFont();

    m_tab.SetFont(font);

    CDC dc = m_tab.GetDC();
    auto oldFont = dc.SelectFont(font);

    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);

    int fontHeight = tm.tmHeight;
    int padding = 16;  // Extra space for better spacing
    int newTabHeight = fontHeight + padding;

    dc.SelectFont(oldFont);

    m_bInternalFont = true;

    m_tab.SetItemExtra(sizeof(TABVIEWPAGE));

    m_tab.SendMessage(TCM_SETITEMSIZE, 0, MAKELPARAM(150, newTabHeight));

    m_cyTabHeight = CalcTabHeight();    

    return true;
}

void FilesView::UpdateLayout()
{
    RECT rc;
    GetClientRect(&rc);

    if (m_tab.IsWindow()) {
        m_tab.SetWindowPos(nullptr, 0, 0, rc.right - rc.left + 1, m_cyTabHeight,
            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);

        m_tab.SetWindowPos(nullptr, 0, 0, rc.right - rc.left, m_cyTabHeight,
            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
    }

    if (m_nActivePage != -1) {
        ::SetWindowPos(GetPageHWND(m_nActivePage), nullptr, 0, m_cyTabHeight, rc.right - rc.left,
                       (rc.bottom - rc.top) - m_cyTabHeight, SWP_NOZORDER);
    }
}

LRESULT FilesView::OnDrawItem(int /*nID*/, LPDRAWITEMSTRUCT pdis) const
{
    if (pdis->CtlType != ODT_TAB) {
        return 0;
    }
    
    auto tabIndex = static_cast<int>(pdis->itemID);
    if (tabIndex < 0) {
        return 0;
    }

    auto isActive = tabIndex == m_nActivePage;

    auto rc = pdis->rcItem;
    auto hdc = pdis->hDC;

    auto hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    auto hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hBrush));
    auto hPen = CreatePen(PS_SOLID, 1, isActive ? GetSysColor(COLOR_BTNTEXT) : GetSysColor(COLOR_3DSHADOW));
    auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));

    auto cornerRadius = 8;
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, cornerRadius, cornerRadius);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);

    DeleteObject(hPen);
    DeleteObject(hBrush);

    auto hPenHighlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHIGHLIGHT));
    auto hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
    auto hPenDarkShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));

    hOldPen = static_cast<HPEN>(SelectObject(hdc, hPenHighlight));

    MoveToEx(hdc, rc.left, rc.bottom - 1, nullptr);
    LineTo(hdc, rc.left, rc.top);
    LineTo(hdc, rc.right - 1, rc.top);

    SelectObject(hdc, hPenShadow);

    MoveToEx(hdc, rc.left, rc.bottom - 1, nullptr);
    LineTo(hdc, rc.right - 1, rc.bottom - 1);
    LineTo(hdc, rc.right - 1, rc.top);

    SelectObject(hdc, hPenDarkShadow);

    MoveToEx(hdc, rc.left + 1, rc.bottom - 2, nullptr);
    LineTo(hdc, rc.right - 2, rc.bottom - 2);
    LineTo(hdc, rc.right - 2, rc.top + 1);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPenHighlight);
    DeleteObject(hPenShadow);
    DeleteObject(hPenDarkShadow);

    TCHAR szText[256];
    TCITEM item;
    item.mask = TCIF_TEXT;
    item.pszText = szText;
    item.cchTextMax = sizeof(szText);
    m_tab.GetItem(tabIndex, &item);

    SetBkMode(hdc, TRANSPARENT);

    SetTextColor(hdc, GetSysColor(COLOR_3DSHADOW));
    RECT rcShadow = rc;
    OffsetRect(&rcShadow, 1, 1);

    auto dtFlags = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
    DrawText(hdc, szText, -1, &rcShadow, dtFlags);

    SetTextColor(hdc, isActive ? GetSysColor(COLOR_BTNTEXT) : GetSysColor(COLOR_GRAYTEXT));

    DrawText(hdc, szText, -1, &rc, dtFlags);

    return 1;
}

BOOL FilesView::ActivateFile(const CString& path, void* data)
{
    auto it = m_data.find(data);
    if (it != m_data.end()) {
        SetActivePage(it->second);
        OnPageActivated(it->second);
        UpdateLayout();
        return TRUE;
    }

    auto fileView = FileViewFactory::CreateFileView(path, *this, rcDefault);
    if (fileView == nullptr) {
        CString message;
        message.Format(L"Unable to create a file view for \"%s\".\nThis file type may be unsupported.", path.GetString());
        AtlMessageBox(*this, static_cast<LPCWSTR>(message), nullptr, MB_ICONERROR);
        return FALSE;
    }

    if (!fileView->LoadFile(path)) {
        CString message;
        message.Format(L"Unable to load file \"%s\".", path.GetString());
        AtlMessageBox(*this, static_cast<LPCWSTR>(message), nullptr, MB_ICONERROR);
        return FALSE;
    }

    auto nPages = GetPageCount();
    m_data[data] = nPages;

    CString title = PathFindFileName(path);

    m_views.emplace_back(fileView);

    AddPage(*fileView, title , 0, data);

    TOOLINFO ti{};
    ti.cbSize = sizeof(TOOLINFO);
    ti.hwnd = m_tab;
    ti.uId = nPages;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = const_cast<LPWSTR>(path.GetString());

    m_tab.GetToolTips().UpdateTipText(&ti);

    SetActivePage(nPages);

    return TRUE;
}

IFileView::Ptr FilesView::ActiveFile() const
{
    if (m_nActivePage < 0) {
        return nullptr;
    }

    return m_views[m_nActivePage];
}

PVOID FilesView::GetData(int index) const
{
    if (index < 0 || index >= GetPageCount()) {
        return nullptr;
    }

    auto hItem = GetPageData(index);

    return hItem;
}

PVOID FilesView::CloseFile(int index)
{
    if (index < 0 || index >= GetPageCount()) {
        return nullptr;
    }

    auto hItem = GetData(index);

    auto it = m_data.find(hItem);
    if (it != m_data.end()) {
        m_data.erase(it);

        // adjust indexes
        for (auto& idx : m_data | std::views::values) {
            if (idx > index) {
                --idx;
            }
        }
    }

    const auto& vit = m_views.begin() + index;

    auto& fileView = *vit;
    fileView->Destroy();

    m_views.erase(vit);

    RemovePage(index);

    // Update tooltips
    TOOLINFO ti{};
    ti.cbSize = sizeof(TOOLINFO);
    ti.hwnd = m_tab;
    ti.uId = index;
    ti.hinst = _Module.GetResourceInstance();

    auto toolTips = m_tab.GetToolTips();

    auto toolCount = toolTips.GetToolCount();
    for (auto i = 0; i < toolCount; ++i) {
        ti.uId = i;
        toolTips.DelTool(&ti);
    }

    for (auto i = 0; i < GetPageCount(); ++i) {
        ti.uId = i;
        ti.lpszText = const_cast<LPWSTR>(m_views[i]->GetPath());
        toolTips.AddTool(&ti);
    }

    UpdateLayout();

    return hItem;
}

PVOID FilesView::CloseOtherFiles(int index)
{
    auto hItem = GetData(index);

    for (auto i = GetPageCount() - 1; i >= 0; --i) {
        if (i == index) {
            continue;
        }
        CloseFile(i);
    }

    return hItem;
}

void FilesView::CloseAllFiles()
{
    for (auto i = GetPageCount() - 1; i >= 0; --i) {
        CloseFile(i);
    }
}

PVOID FilesView::CloseActiveFile()
{
    return CloseFile(GetActivePage());
}

FileEncoding FilesView::FileEncoding(int index) const
{
    if (index < 0 || index >= GetPageCount()) {
        return UNKNOWN;
    }

    ATLASSERT(index < static_cast<int>(m_views.size()));

    return m_views[index]->GetEncoding();
}
