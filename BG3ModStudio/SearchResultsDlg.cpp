#include "stdafx.h"
#include "SearchResultsDlg.h"

void SearchResultsDlg::AutoAdjustColumns()
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

BOOL SearchResultsDlg::OnInitDialog(HWND, LPARAM)
{
    m_listResults = GetDlgItem(IDC_LST_RESULTS);
    ATLASSERT(m_listResults.IsWindow());

    m_listResults.ModifyStyle(0, LVS_REPORT);
    m_listResults.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_listResults.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 150);
    m_listResults.InsertColumn(1, _T("Size"), LVCFMT_CENTER, 80);
    m_listResults.InsertColumn(2, _T("Date Modified"), LVCFMT_CENTER);

    DlgResize_Init();
    AutoAdjustColumns();

    CenterWindow(GetParent());

    return FALSE; // Let the system set the focus
}

void SearchResultsDlg::OnClose()
{
    EndDialog(0);
}

void SearchResultsDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    AutoAdjustColumns();
}

