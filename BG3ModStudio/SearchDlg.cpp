#include "stdafx.h"
#include "AttributeDlg.h"
#include "FileDialogEx.h"
#include "SearchDlg.h"
#include "Searcher.h"
#include "Settings.h"
#include "StringHelper.h"
#include "Util.h"

#include <nlohmann/json.hpp>

static constexpr auto PAGE_SIZE = 25;

BOOL SearchDlg::OnIdle()
{
    auto pageCount = GetPageCount();

    UIEnable(IDC_B_FIRST, m_nPage > 0 && pageCount);
    UIEnable(IDC_B_PREV, m_nPage > 0 && pageCount);
    UIEnable(IDC_B_NEXT, m_nPage + 1 < static_cast<int>(pageCount) && pageCount);
    UIEnable(IDC_B_LAST, m_nPage + 1 < static_cast<int>(pageCount) && pageCount);

    UpdatePageInfo();
    UIUpdateChildWindows(TRUE);

    return FALSE;
}

void SearchDlg::AutoAdjustColumns()
{
    CRect rcClient;
    m_listResults.GetClientRect(&rcClient);

    int totalWidth = 0;
    auto columnCount = m_listResults.GetHeader().GetItemCount();

    for (auto i = 0; i < columnCount - 1; ++i) {
        totalWidth += m_listResults.GetColumnWidth(i);
    }

    auto remaining = rcClient.Width() - totalWidth;
    if (remaining > 0) {
        m_listResults.SetColumnWidth(columnCount - 1, remaining);
    }
}

BOOL SearchDlg::OnInitDialog(HWND, LPARAM)
{
    m_listResults = GetDlgItem(IDC_LST_RESULTS);
    ATLASSERT(m_listResults.IsWindow());

    m_indexPath = GetDlgItem(IDC_E_FOLDER);
    ATLASSERT(m_indexPath.IsWindow());

    m_pageInfo = GetDlgItem(IDC_PAGEINFO);
    ATLASSERT(m_pageInfo.IsWindow());

    Settings settings;
    auto indexPath = settings.GetString(_T("Settings"), _T("IndexPath"), _T(""));
    m_indexPath.SetWindowText(indexPath);

    m_listResults.ModifyStyle(0, LVS_REPORT | LVS_SINGLESEL);
    m_listResults.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_listResults.InsertColumn(0, _T("Source File"), LVCFMT_LEFT, 150);
    m_listResults.InsertColumn(1, _T("Type"), LVCFMT_LEFT, 80);
    m_listResults.InsertColumn(2, _T("Entry"), LVCFMT_LEFT, 150);
    m_listResults.InsertColumn(3, _T("Attributes"), LVCFMT_LEFT);

    UIAddChildWindowContainer(m_hWnd);

    DlgResize_Init();
    AutoAdjustColumns();

    CenterWindow(GetParent());

    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddIdleHandler(this);

    auto icon = Util::LoadBitmapAsIcon(ID_TOOL_SEARCH, 32, 32);
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    return TRUE; // Let the system set the focus
}

void SearchDlg::OnContextMenu(const CWindow& wnd, const CPoint& point)
{
    if (wnd != m_listResults) {
        return;
    }

    CMenu menu;
    menu.LoadMenuW(IDR_ENTRY_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);
    auto cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, *this);
    if (cmd == 0) {
        return; // No command selected
    }

    auto selectedRow = m_listResults.GetSelectedIndex();
    if (selectedRow < 0) {
        return; // No item selected
    }

    CString text;
    m_listResults.GetItemText(selectedRow, 2, text);
    if (text.IsEmpty()) {
        return; // Nothing to copy
    }

    Util::CopyToClipboard(*this, text);
}

void SearchDlg::OnClose()
{
    Destroy();
}

void SearchDlg::OnDestroy()
{
    auto* pLoop = _Module.GetMessageLoop();
    if (pLoop != nullptr) {
        pLoop->RemoveIdleHandler(this);
    }
}

void SearchDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    AutoAdjustColumns();
}

void SearchDlg::OnQueryChange()
{
    m_results = Xapian::MSet();
    m_listResults.DeleteAllItems();
    m_nPage = 0;
}

void SearchDlg::OnBrowse()
{
    FileDialogEx dlg(FileDialogEx::Folder, *this);
    auto hr = dlg.Construct();
    if (FAILED(hr)) {
        return;
    }

    if (dlg.DoModal() != IDOK) {
        return;
    }

    const auto& paths = dlg.paths();
    if (paths.empty()) {
        return;
    }

    auto path = paths.front();
    m_indexPath.SetWindowText(path);
}

void SearchDlg::OnSearch()
{
    m_nPage = 0;
    Search();
}

void SearchDlg::Search()
{
    auto offset = m_nPage * PAGE_SIZE;
    Search(offset);
}

void SearchDlg::Search(uint32_t offset)
{
    CString query;
    GetDlgItemText(IDC_E_QUERY, query);

    auto utf8Query = StringHelper::toUTF8(query);

    m_listResults.DeleteAllItems();

    CWaitCursor cursor;

    CString indexPath;
    m_indexPath.GetWindowText(indexPath);

    auto utf8IndexPath = StringHelper::toUTF8(indexPath);

    try {
        m_results = Searcher::search(utf8IndexPath, utf8Query, offset, PAGE_SIZE);
    } catch (const Xapian::Error& e) {
        CString errorMessage;
        errorMessage.Format(_T("Error: %s\nContext: %s\nType: %s\nError String: %s"),
                            StringHelper::fromUTF8(e.get_msg().c_str()).GetString(),
                            StringHelper::fromUTF8(e.get_context().c_str()).GetString(),
                            StringHelper::fromUTF8(e.get_type()).GetString(),
                            StringHelper::fromUTF8(e.get_error_string()).GetString());
        MessageBox(errorMessage, _T("Search Error"), MB_OK | MB_ICONERROR);
    }

    if (m_results.empty()) {
        m_nPage = 0;
    }

    for (auto it = m_results.begin(); it != m_results.end(); ++it) {
        auto doc = nlohmann::json::parse(it.get_document().get_data());
        auto sourceFile = doc["source_file"].get<std::string>();
        auto type = doc["type"].get<std::string>();
        auto entry = doc.value("entry", "");
        auto attributes = doc["attributes"].dump();

        if (entry.empty()) {
            auto attrDoc = doc["attributes"];
            ATLASSERT(attrDoc.is_array());
            for (const auto& attr : attrDoc) {
                auto name = attr.value("id", "");
                auto value = attr.value("value", "");
                if (name == "Name" && !value.empty()) {
                    entry = value;
                    break;
                }
            }
        }

        auto wSourceFile = StringHelper::fromUTF8(sourceFile.c_str());
        auto wType = StringHelper::fromUTF8(type.c_str());
        auto wEntry = StringHelper::fromUTF8(entry.c_str());
        auto wAttributes = StringHelper::fromUTF8(attributes.c_str());

        auto index = m_listResults.InsertItem(0, wSourceFile.GetString());
        m_listResults.SetItemText(index, 1, wType.GetString());
        m_listResults.SetItemText(index, 2, wEntry.GetString());
        m_listResults.SetItemText(index, 3, wAttributes.GetString());
    }
}

void SearchDlg::UpdatePageInfo()
{
    CString pageInfo;

    auto totalPages = GetPageCount();

    if (totalPages > 0) {
        pageInfo.Format(_T("Page %d of about %d"), m_nPage + 1, totalPages);
    }

    m_pageInfo.SetWindowText(pageInfo);
}

void SearchDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    ATLASSERT(lpMMI != nullptr);

    // Define size in dialog units
    constexpr SIZE dlgSizeInDLUs = {500, 100};

    CRect rcPixels(0, 0, dlgSizeInDLUs.cx, dlgSizeInDLUs.cy);
    MapDialogRect(&rcPixels); // Convert DLUs to pixels based on dialog font

    lpMMI->ptMinTrackSize.x = rcPixels.Width();
    lpMMI->ptMinTrackSize.y = rcPixels.Height();
}

void SearchDlg::OnFirst()
{
    m_nPage = 0;
    Search();
}

void SearchDlg::OnPrev()
{
    m_nPage = std::max(0, m_nPage - 1);
    Search();
}

void SearchDlg::OnNext()
{
    auto totalPages = GetPageCount();
    m_nPage = std::min<int>(m_nPage + 1, static_cast<int>(totalPages) - 1);
    Search();
}

void SearchDlg::OnLast()
{
    auto offset = std::numeric_limits<uint32_t>::max();
    Search(offset);

    auto total = static_cast<int>(m_results.get_matches_lower_bound());

    m_nPage = std::max(0, (total + PAGE_SIZE - 1) / PAGE_SIZE - 1);

    Search();
}

uint32_t SearchDlg::GetPageCount() const
{
    return (m_results.get_matches_estimated() + PAGE_SIZE - 1) / PAGE_SIZE;
}

LRESULT SearchDlg::OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto pia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
    if (!pia || pia->iItem < 0) {
        return 0;
    }

    CString entry, text;
    m_listResults.GetItemText(pia->iItem, 2, entry);
    m_listResults.GetItemText(pia->iItem, 3, text);

    auto* pDlg = new AttributeDlg();
    pDlg->SetEntry(entry);
    pDlg->SetAttributeJson(StringHelper::toUTF8(text).GetString());
    pDlg->Run(*this);

    return 0;
}
