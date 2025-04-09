#include "stdafx.h"
#include <nlohmann/json.hpp>
#include "SearchDlg.h"

#include "AttributeDlg.h"
#include "Searcher.h"
#include "Settings.h"
#include "StringHelper.h"

static constexpr auto PAGE_SIZE = 25;

BOOL SearchDlg::OnIdle()
{
    int totalResults = static_cast<int>(m_results.get_matches_upper_bound());
    int pageCount = (totalResults + PAGE_SIZE - 1) / PAGE_SIZE;

    UIEnable(IDC_B_FIRST, m_nPage > 0 && totalResults);
    UIEnable(IDC_B_PREV, m_nPage > 0 && totalResults);
    UIEnable(IDC_B_NEXT, m_nPage + 1 < pageCount && totalResults);
    UIEnable(IDC_B_LAST, m_nPage + 1 < pageCount && totalResults);

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

    Settings settings;
    auto indexPath = settings.GetString(_T("Indexing"), _T("IndexPath"), _T(""));
    m_indexPath.SetWindowText(indexPath);

    m_listResults.ModifyStyle(0, LVS_REPORT);
    m_listResults.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_listResults.InsertColumn(0, _T("Source File"), LVCFMT_LEFT, 150);
    m_listResults.InsertColumn(1, _T("Type"), LVCFMT_LEFT, 80);
    m_listResults.InsertColumn(2, _T("Atrributes"), LVCFMT_LEFT);

    UIAddChildWindowContainer(m_hWnd);

    DlgResize_Init();
    AutoAdjustColumns();

    CenterWindow(GetParent());

    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddIdleHandler(this);

    return FALSE; // Let the system set the focus
}

void SearchDlg::OnClose()
{
    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveIdleHandler(this);

    Destroy();
}

void SearchDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    AutoAdjustColumns();
}

void SearchDlg::OnSearch()
{
    CString query;
    GetDlgItemText(IDC_E_QUERY, query);

    auto utf8Query = StringHelper::toUTF8(query);

    m_listResults.DeleteAllItems();

    CWaitCursor cursor;

    auto offset = m_nPage * PAGE_SIZE;

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
        auto attributes = doc["attributes"].dump();

        auto wSourceFile = StringHelper::fromUTF8(sourceFile.c_str());
        auto wType = StringHelper::fromUTF8(type.c_str());
        auto wAttributes = StringHelper::fromUTF8(attributes.c_str());

        auto index = m_listResults.InsertItem(0, wSourceFile.GetString());
        m_listResults.SetItemText(index, 1, wType.GetString());
        m_listResults.SetItemText(index, 2, wAttributes.GetString());
    }
}

void SearchDlg::OnFirst()
{
    m_nPage = 0;
    OnSearch();
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

void SearchDlg::OnPrev()
{
    m_nPage = std::max(0, m_nPage - 1);
    OnSearch();
}

void SearchDlg::OnNext()
{
    auto lastPage = (m_results.get_matches_upper_bound() + PAGE_SIZE - 1) / PAGE_SIZE;
    m_nPage = std::min<int>(m_nPage + 1, static_cast<int>(lastPage) - 1);
    OnSearch();
}

void SearchDlg::OnLast()
{
    m_nPage = static_cast<int>(m_results.get_matches_upper_bound() + PAGE_SIZE - 1) / PAGE_SIZE - 1;
    OnSearch();
}

LRESULT SearchDlg::OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto pia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
    if (!pia || pia->iItem < 0) {
        return 0;
    }

    CString text;
    m_listResults.GetItemText(pia->iItem, 2, text);

    AttributeDlg dlg;
    dlg.SetAttributeJson(StringHelper::toUTF8(text).GetString());

    dlg.DoModal();

    return 0;
}
