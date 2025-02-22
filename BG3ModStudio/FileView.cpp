#include "stdafx.h"
#include "FileView.h"
#include "Exception.h"
#include "FileStream.h"
#include "RTFFormatterRegistry.h"
#include "UTF8Stream.h"

namespace { // anonymous

DWORD CALLBACK StreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
    auto** pptext = reinterpret_cast<LPCSTR*>(dwCookie);

    if (pptext == nullptr || *pptext == nullptr) {
        return E_POINTER;
    }

    auto strSize = strlen(*pptext);

    if (strSize == 0) {
        *pcb = 0;
        return 0;
    }

    auto sz = std::min<LONG>(static_cast<LONG>(strSize), cb);

    memcpy(pbBuff, *pptext, sz);

    *pptext += sz;

    *pcb = sz;

    return 0;
}

DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
    auto** ppStream = reinterpret_cast<IStream**>(dwCookie);

    if (ppStream == nullptr || *ppStream == nullptr) {
        return E_POINTER;
    }

    if (cb == 0) {
        *pcb = 0;
        return 0;
    }

    auto hr = (*ppStream)->Write(pbBuff, cb, reinterpret_cast<ULONG*>(pcb));
    if (FAILED(hr)) {
        return hr;
    }

    if (*pcb == 0) {
        return E_FAIL;
    }

    return 0;
}

auto constexpr BUFFER_SIZE = 4096;

}

LRESULT FileView::OnCreate(LPCREATESTRUCT pcs)
{
    auto lRet = DefWindowProc();

    SetModify(FALSE);

    return lRet;
}

void FileView::LoadFile(const CString& path)
{
    m_stream.Reset();

    m_path = path;
    m_encoding = UNKNOWN;

    FileStream file;

    CStringA strPath(path);

    try {
        file.open(strPath, "rb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return;
    }

    char buf[BUFFER_SIZE];
    auto read = file.read(buf, sizeof(buf));
    if (read == 0) {
        return;
    }

    auto isText = true;
    for (auto i = 0u; i < read; ++i) {
        if (buf[i] == '\0') {
            isText = false;
            break;
        }
    }

    if (!isText) {
        Write("*** Binary file ***");
        Flush();
        return;
    }

    LPSTR pb = buf;
    if (SkipBOM(pb, read)) {
        read -= 3;
        m_encoding = UTF8BOM;
    } else {
        m_encoding = UTF8;    // nothing else supported for now
    }

    Write(pb, read);

    for (;;) {
        read = file.read(buf, sizeof(buf));
        if (read == 0) {
            break;
        }

        Write(buf, read);
    }

    Flush();
}

void FileView::SaveFile(const CString& path)
{
    CComObjectStack<UTF8Stream> stream;
    IStream* pStream = &stream;

    EDITSTREAM es{};
    es.dwCookie = reinterpret_cast<DWORD_PTR>(&pStream);
    es.pfnCallback = StreamOutCallback;

    StreamOut((CP_UTF8 << 16) | SF_USECODEPAGE | SF_TEXT, es);

    if (es.dwError != NOERROR) {
        ATLTRACE("Failed to stream in text.\n");
        return;
    }

    LARGE_INTEGER li{};
    auto hr = pStream->Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        ATLTRACE("Failed to seek stream.\n");
        return;
    }

    CStringA strPath(path);

    FileStream file;
    file.open(strPath, "wb");

    WriteBOM(file);

    for (;;) {
        char buf[BUFFER_SIZE];
        ULONG read;

        hr = pStream->Read(buf, sizeof(buf), &read);
        if (FAILED(hr)) {
            ATLTRACE("Failed to read stream.\n");
            return;
        }

        if (read == 0) {
            break;
        }

        file.write(buf, read);
    }

    file.close();
}

LPCTSTR FileView::GetPath() const
{
    return m_path.GetString();
}

FileView::FileEncoding FileView::GetEncoding() const
{
    return m_encoding;
}

BOOL FileView::Write(LPCWSTR text) const
{
    auto hr = m_stream.Write(text);

    return SUCCEEDED(hr);
}

BOOL FileView::Write(LPCWSTR text, size_t length) const
{
    CStringW str(text, static_cast<int>(length));

    return Write(str);
}

BOOL FileView::Write(LPCSTR text) const
{
    auto hr = m_stream.Write(text);
    return SUCCEEDED(hr);
}

BOOL FileView::Write(LPCSTR text, size_t length) const
{
    auto hr = m_stream.Write(text, length);
    return SUCCEEDED(hr);
}

BOOL FileView::SkipBOM(LPSTR& str, size_t size)
{
    if (size < 3) {
        return FALSE;
    }

    if (str[0] == '\xEF' && str[1] == '\xBB' && str[2] == '\xBF') {
        str += 3;
        return TRUE;
    }

    return FALSE;
}

BOOL FileView::WriteBOM(const IStreamBase& stream) const
{
    switch (m_encoding) {
    case UTF8BOM:
        stream.write("\xEF\xBB\xBF", 3);
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

BOOL FileView::Flush()
{
    auto formatter = RTFFormatterRegistry::GetInstance().GetFormatter(m_path);

    auto str = formatter->Format(m_stream);

    LPCSTR pStr = str.GetString();

    EDITSTREAM es{};
    es.dwCookie = reinterpret_cast<DWORD_PTR>(&pStr);
    es.pfnCallback = StreamInCallback;

    StreamIn((CP_UTF8 << 16) | SF_USECODEPAGE | SF_RTF, es);
    if (es.dwError != NOERROR) {
        ATLTRACE("Failed to stream in text.\n");
        return FALSE;
    }

    CPoint origin;
    SetScrollPos(&origin);

    auto hr = m_stream.Reset();

    return SUCCEEDED(hr);
}

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

    auto style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
        ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE
        | ES_NOOLEDRAGDROP | ES_READONLY;

    auto fileView = std::make_shared<FileView>();
    if (!fileView->Create(m_hWnd, rcDefault, nullptr, style)) {
        ATLTRACE("Failed to create file view.\n");
        return FALSE;
    }

    fileView->LoadFile(path);

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

FileView::Ptr FilesView::ActiveFile() const
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
    fileView->DestroyWindow();

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

FileView::FileEncoding FilesView::FileEncoding(int index) const
{
    if (index < 0 || index >= GetPageCount()) {
        return FileView::UNKNOWN;
    }

    ATLASSERT(index < static_cast<int>(m_views.size()));

    return m_views[index]->GetEncoding();
}
