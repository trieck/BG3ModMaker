#include "stdafx.h"
#include <nlohmann/json.hpp>
#include "SearchDlg.h"

#include "AttributeDlg.h"
#include "Searcher.h"
#include "StringHelper.h"

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

    m_listResults.ModifyStyle(0, LVS_REPORT);
    m_listResults.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_listResults.InsertColumn(0, _T("Source File"), LVCFMT_LEFT, 150);
    m_listResults.InsertColumn(1, _T("Type"), LVCFMT_LEFT, 80);
    m_listResults.InsertColumn(2, _T("Atrributes"), LVCFMT_LEFT);

    DlgResize_Init();
    AutoAdjustColumns();

    CenterWindow(GetParent());

    return FALSE; // Let the system set the focus
}

void SearchDlg::OnClose()
{
    EndDialog(0);
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

    auto results = Searcher::search(R"(C:\Users\trieck\Desktop\IDX)", utf8Query);
    for (auto it = results.begin(); it != results.end(); ++it) {
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
