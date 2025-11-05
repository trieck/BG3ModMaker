#include "stdafx.h"
#include "Exception.h"
#include "GameObjectDlg.h"

#include "IconDlg.h"
#include "Settings.h"
#include "StringHelper.h"
#include "Util.h"

static constexpr auto COLUMN_PADDING = 12;
static constexpr auto PAGE_SIZE = 25;

BOOL GameObjectDlg::OnIdle()
{
    auto pageCount = GetPageCount();

    bool enableNext = m_nPage + 1 < static_cast<int>(pageCount) && pageCount > 0;
    if (m_iterator && m_iterator->hasPrefix()) {
        enableNext = true; // We can't know the total pages with a prefix, so always enable
    }

    bool enablePrev = m_nPage > 0 && pageCount > 0;
    if (m_iterator && m_iterator->hasPrefix()) {
        enablePrev = true; // We can't know the total pages with a prefix, so always enable
    }

    UIEnable(IDC_B_FIRST_PAGE, enablePrev);
    UIEnable(IDC_B_PREV_PAGE, enablePrev);
    UIEnable(IDC_B_NEXT_PAGE, enableNext);
    UIEnable(IDC_B_LAST_PAGE, enableNext);

    UpdatePageInfo();
    UIUpdateChildWindows(TRUE);

    return FALSE;
}

BOOL GameObjectDlg::OnInitDialog(HWND, LPARAM)
{
    Settings settings;
    m_dbPath = settings.GetString("Settings", "GameObjectPath");

    auto wndFrame = GetDlgItem(IDC_ST_GAMEOBJECT);
    ATLASSERT(wndFrame.IsWindow());

    CRect rcDlg;
    GetClientRect(&rcDlg);

    CRect rcFrame;
    wndFrame.GetWindowRect(&rcFrame);
    ScreenToClient(&rcFrame);

    m_marginLeft = rcFrame.left;
    m_marginTop = rcFrame.top;
    m_marginRight = rcDlg.right - rcFrame.right;
    m_marginBottom = rcDlg.bottom - rcFrame.bottom;

    wndFrame.DestroyWindow(); // Needed only for margin calculation

    m_pageInfo = GetDlgItem(IDC_GAMEOBJECT_PAGEINFO);
    ATLASSERT(m_pageInfo.IsWindow());

    m_splitter.Create(m_hWnd, rcFrame, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    ATLASSERT(m_splitter.IsWindow());

    m_list.Create(m_splitter, rcDefault, nullptr,
                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LBS_NOINTEGRALHEIGHT |
                  LBS_NOTIFY, WS_EX_CLIENTEDGE, ID_UUID_LIST);
    ATLASSERT(m_list.IsWindow());

    m_font = AtlCreateControlFont();
    m_list.SetFont(m_font);

    m_attributes.Create(m_splitter, rcDefault, nullptr,
                        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                        LVS_REPORT | LVS_SINGLESEL, WS_EX_CLIENTEDGE, ID_ATTRIBUTE_LIST);

    m_attributes.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_attributes.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100);
    m_attributes.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 150);
    m_attributes.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 150);

    m_splitter.SetSplitterPane(0, m_list);
    m_splitter.SetSplitterPane(1, m_attributes);
    m_splitter.SetSplitterPosPct(40);

    Populate();

    UIAddChildWindowContainer(m_hWnd);

    DlgResize_Init();

    CenterWindow(GetParent());

    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddIdleHandler(this);

    auto icon = Util::LoadBitmapAsIcon(ID_TOOL_GAMEOBJECT, 32, 32);
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    return TRUE; // Let the system set the focus
}

LRESULT GameObjectDlg::OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto pia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
    if (!pia || pia->iItem < 0) {
        return 0;
    }

    CString id, value;
    m_attributes.GetItemText(pia->iItem, 1, value);
    if (value.IsEmpty()) {
        return 0;
    }

    CWaitCursor cursor;
    auto pDlg = std::make_unique<IconDlg>(value);
    if (!pDlg->HasImage()) {
        return 0;
    }

    pDlg->Run(*this);
    pDlg.release();

    return 0;
}

void GameObjectDlg::OnContextMenu(const CWindow& wnd, const CPoint& point)
{
    CRect rc;
    m_attributes.GetWindowRect(&rc);

    if (rc.PtInRect(point)) {
        OnContextAttributes(point);
        return;
    }

    m_list.GetWindowRect(&rc);
    if (!rc.PtInRect(point)) {
        return; // Click was outside the list
    }

    CMenu menu;
    menu.LoadMenuW(IDR_VALUE_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);
    auto cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, *this);
    if (cmd == 0) {
        return; // No command selected
    }

    auto curSel = m_list.GetCurSel();
    if (curSel == LB_ERR) {
        return; // No selection
    }

    CString text;
    m_list.GetText(curSel, text);

    if (text.IsEmpty()) {
        return; // Nothing to copy
    }

    Util::CopyToClipboard(*this, text);
}

void GameObjectDlg::OnContextAttributes(const CPoint& point)
{
    CMenu menu;
    menu.LoadMenuW(IDR_ATTRIBUTE_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);
    auto cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, *this);
    if (cmd == 0) {
        return; // No command selected
    }

    auto selectedRow = m_attributes.GetSelectedIndex();
    if (selectedRow < 0) {
        return; // No item selected
    }

    CString text;
    switch (cmd) {
    case ID_ATTRIBUTE_COPYNAME: // Copy Name
        m_attributes.GetItemText(selectedRow, 0, text);
        break;
    case ID_ATTRIBUTE_COPYVALUE: // Copy Value
        m_attributes.GetItemText(selectedRow, 1, text);
        break;
    case ID_ATTRIBUTE_COPYTYPE: // Copy Type
        m_attributes.GetItemText(selectedRow, 2, text);
        break;
    default:
        return; // Unknown command
    }

    if (text.IsEmpty()) {
        return; // Nothing to copy
    }

    Util::CopyToClipboard(*this, text);
}

void GameObjectDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    if (m_splitter.IsWindow()) {
        CRect rc;
        rc.left = m_marginLeft;
        rc.top = m_marginTop;
        rc.right = size.cx - m_marginRight;
        rc.bottom = size.cy - m_marginBottom;

        m_splitter.MoveWindow(&rc);
    }
}

void GameObjectDlg::PopulateKeys()
{
    m_list.ResetContent();
    m_attributes.DeleteAllItems();

    if (!m_iterator) {
        return;
    }

    for (const auto& key : m_iterator->keys()) {
        auto wideKey = StringHelper::fromUTF8(key.c_str());
        m_list.AddString(wideKey);
    }
}

void GameObjectDlg::AutoAdjustAttributes()
{
    CClientDC dc(m_attributes);
    auto hFont = m_attributes.GetFont();
    auto hOldFont = dc.SelectFont(hFont);

    auto header = m_attributes.GetHeader();

    for (auto col = 0; col < header.GetItemCount(); ++col) {
        LVCOLUMN lvc{};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        TCHAR textBuf[256]{};
        lvc.pszText = textBuf;
        lvc.cchTextMax = _countof(textBuf);
        m_attributes.GetColumn(col, &lvc);

        CString headerText = lvc.pszText;

        CSize sz;
        dc.GetTextExtent(headerText, headerText.GetLength(), &sz);
        auto maxWidth = sz.cx;

        auto rowCount = m_attributes.GetItemCount();
        for (auto row = 0; row < rowCount; ++row) {
            CString cellText;
            m_attributes.GetItemText(row, col, cellText);

            dc.GetTextExtent(cellText, cellText.GetLength(), &sz);
            maxWidth = std::max(sz.cx, maxWidth);
        }

        maxWidth += COLUMN_PADDING;
        m_attributes.SetColumnWidth(col, maxWidth);
    }

    dc.SelectFont(hOldFont);
}

void GameObjectDlg::OnUuidSelChange()
{
    auto sel = m_list.GetCurSel();
    if (sel == LB_ERR) {
        return;
    }

    CString uuid;
    m_list.GetText(sel, uuid);

    m_attributes.DeleteAllItems();

    auto attributes = GetAttributes(uuid);
    if (!attributes.is_object()) {
        return;
    }

    for (const auto& attr : attributes["attributes"]) {
        CString name = attr.value("id", "").c_str();
        CString value = attr.value("value", "").c_str();
        CString type = attr.value("type", "").c_str();
        auto row = m_attributes.InsertItem(m_attributes.GetItemCount(), name);
        m_attributes.SetItemText(row, 1, value);
        m_attributes.SetItemText(row, 2, type);
    }

    AutoAdjustAttributes();
}

void GameObjectDlg::OnClose()
{
    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveIdleHandler(this);

    m_iterator = nullptr;
    m_cataloger.close();

    Destroy();
}

void GameObjectDlg::OnDestroy()
{
    auto* pLoop = _Module.GetMessageLoop();
    if (pLoop != nullptr) {
        pLoop->RemoveIdleHandler(this);
    }
}

void GameObjectDlg::OnFirstPage()
{
    m_nPage = 0;

    if (!m_iterator) {
        return;
    }

    m_iterator->first();

    PopulateKeys();

    m_attributes.DeleteAllItems();
}

void GameObjectDlg::OnNextPage()
{
    if (!m_iterator) {
        return;
    }

    if (!m_iterator->next()) {
        return;
    }

    PopulateKeys();

    m_attributes.DeleteAllItems();

    auto pageCount = GetPageCount();

    m_nPage = std::min(m_nPage + 1, static_cast<int>(pageCount) - 1);
}

void GameObjectDlg::OnPrevPage()
{
    if (!m_iterator) {
        return;
    }

    if (!m_iterator->prev()) {
        return;
    }

    PopulateKeys();

    m_attributes.DeleteAllItems();

    m_nPage = std::max(0, m_nPage - 1);
}

void GameObjectDlg::OnLastPage()
{
    if (!m_iterator) {
        return;
    }

    if (!m_iterator->last()) {
        return;
    }

    PopulateKeys();

    m_attributes.DeleteAllItems();

    auto pageCount = GetPageCount();

    m_nPage = static_cast<int>(pageCount) - 1;
}

void GameObjectDlg::OnQueryChange()
{
    m_list.ResetContent();
    m_attributes.DeleteAllItems();
    m_iterator = nullptr;

    CString uuid;
    GetDlgItemText(IDC_E_QUERY_GAMEOBJECT, uuid);

    if (uuid.IsEmpty()) {
        Populate();
    }
}

void GameObjectDlg::OnSearch()
{
    m_list.ResetContent();
    m_attributes.DeleteAllItems();

    CString uuid;
    GetDlgItemText(IDC_E_QUERY_GAMEOBJECT, uuid);

    m_nPage = 0;

    if (uuid.IsEmpty()) {
        Populate();
        return;
    }

    auto utf8Uuid = StringHelper::toUTF8(uuid);

    try {
        m_iterator = m_cataloger.newIterator(utf8Uuid.GetString(), PAGE_SIZE);
        PopulateKeys();
    } catch (const Exception& ex) {
        CString msg;
        msg.Format(_T("Failed to open game object database: %s"), CString(ex.what()));
        AtlMessageBox(*this, msg.GetString(), nullptr, MB_ICONERROR);
    }
}

void GameObjectDlg::Populate()
{
    CWaitCursor cursor;
    Settings settings;

    m_list.ResetContent();
    m_attributes.DeleteAllItems();

    m_iterator = nullptr;

    try {
        if (!m_cataloger.isOpen()) {
            m_cataloger.openReadOnly(StringHelper::toUTF8(m_dbPath).GetString());
        }
        m_iterator = m_cataloger.newIterator(PAGE_SIZE);
        PopulateKeys();
    } catch (const Exception& ex) {
        CString msg;
        msg.Format(_T("Failed to open game object database: %s"), CString(ex.what()));
        AtlMessageBox(*this, msg.GetString(), nullptr, MB_ICONERROR);
    }
}

void GameObjectDlg::UpdatePageInfo()
{
    CString pageInfo;

    auto totalPages = GetPageCount();

    if (totalPages > 0) {
        pageInfo.Format(_T("Page %d of about %llu"), m_nPage + 1, totalPages);
    }

    m_pageInfo.SetWindowText(pageInfo);
}

size_t GameObjectDlg::GetPageCount() const
{
    return m_iterator ? m_iterator->totalPages() : 0;
}

nlohmann::json GameObjectDlg::GetAttributes(const CString& uuid)
{
    if (!m_cataloger.isOpen()) {
        return {};
    }

    auto key = StringHelper::toUTF8(uuid);

    try {
        return m_cataloger.get(key.GetString());
    } catch (const std::exception& e) {
        CString msg;
        msg.Format(_T("Failed to read attributes: %s"), CString(e.what()));
        AtlMessageBox(*this, msg.GetString(), nullptr, MB_ICONERROR);
    }

    return {};
}
