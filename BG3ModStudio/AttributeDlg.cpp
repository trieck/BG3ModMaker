#include "stdafx.h"
#include "AttributeDlg.h"

static constexpr auto COLUMN_PADDING = 16;

void AttributeDlg::SetAttributeJson(const std::string& json)
{
    m_attributes = nlohmann::json::parse(json);
    ATLASSERT(m_attributes.is_array());
}

void AttributeDlg::AutoAdjustColumns()
{
    CClientDC dc(m_list);
    CFontHandle hFont = m_list.GetFont();
    HGDIOBJ hOldFont = dc.SelectFont(hFont);

    CHeaderCtrl header = m_list.GetHeader();

    for (auto col = 0; col < header.GetItemCount(); ++col) {
        CString headerText;
        LVCOLUMN lvc{};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        TCHAR textBuf[256]{};
        lvc.pszText = textBuf;
        lvc.cchTextMax = _countof(textBuf);
        m_list.GetColumn(col, &lvc);
        headerText = lvc.pszText;

        CSize sz;
        dc.GetTextExtent(headerText, headerText.GetLength(), &sz);
        auto maxWidth = sz.cx;

        int rowCount = m_list.GetItemCount();
        for (int row = 0; row < rowCount; ++row) {
            CString cellText;
            m_list.GetItemText(row, col, cellText);

            dc.GetTextExtent(cellText, cellText.GetLength(), &sz);
            maxWidth = std::max(sz.cx, maxWidth);
        }

        maxWidth += COLUMN_PADDING;
        m_list.SetColumnWidth(col, maxWidth);
    }

    dc.SelectFont(static_cast<HFONT>(hOldFont));
}

void AttributeDlg::OnClose()
{
    EndDialog(0);
}

void AttributeDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    AutoAdjustColumns();
}

BOOL AttributeDlg::OnInitDialog(HWND, LPARAM)
{
    m_list = GetDlgItem(IDC_LST_ATTRIBUTES);
    ATLASSERT(m_list.IsWindow());

    m_list.ModifyStyle(0, LVS_REPORT);
    m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 150);
    m_list.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 80);
    m_list.InsertColumn(2, _T("Type"), LVCFMT_LEFT);

    for (auto i = 0u; i < m_attributes.size(); ++i) {
        const auto& attr = m_attributes[i];

        CString name = attr.value("id", "").c_str();
        CString value = attr.value("value", "").c_str();
        CString type = attr.value("type", "").c_str();

        int row = m_list.InsertItem(static_cast<int>(i), name);
        m_list.SetItemText(row, 1, value);
        m_list.SetItemText(row, 2, type);
    }


    DlgResize_Init();
    AutoAdjustColumns();

    CenterWindow(GetParent());

    return FALSE; // Let the system set the focus
}
