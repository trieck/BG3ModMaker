#include "stdafx.h"
#include "AttributeDlg.h"
#include "IconDlg.h"
#include "Util.h"

static constexpr auto COLUMN_PADDING = 12;

static int CALLBACK AttributeListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    auto* pList = reinterpret_cast<CListViewCtrl*>(lParamSort);
    ATLASSERT(pList && pList->IsWindow());

    CString text1, text2;
    pList->GetItemText(static_cast<int>(lParam1), 0, text1);
    pList->GetItemText(static_cast<int>(lParam2), 0, text2);

    return text1.CompareNoCase(text2);
}

void AttributeDlg::SetAttributeJson(const std::string& json)
{
    m_attributes = nlohmann::json::parse(json);
    ATLASSERT(m_attributes.is_array());
}

void AttributeDlg::AutoAdjustColumns()
{
    CClientDC dc(m_list);
    auto hFont = m_list.GetFont();
    auto hOldFont = dc.SelectFont(hFont);

    auto header = m_list.GetHeader();

    for (auto col = 0; col < header.GetItemCount(); ++col) {
        LVCOLUMN lvc{};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        TCHAR textBuf[256]{};
        lvc.pszText = textBuf;
        lvc.cchTextMax = _countof(textBuf);
        m_list.GetColumn(col, &lvc);

        CString headerText = lvc.pszText;

        CSize sz;
        dc.GetTextExtent(headerText, headerText.GetLength(), &sz);
        auto maxWidth = sz.cx;

        auto rowCount = m_list.GetItemCount();
        for (auto row = 0; row < rowCount; ++row) {
            CString cellText;
            m_list.GetItemText(row, col, cellText);

            dc.GetTextExtent(cellText, cellText.GetLength(), &sz);
            maxWidth = std::max(sz.cx, maxWidth);
        }

        maxWidth += COLUMN_PADDING;
        m_list.SetColumnWidth(col, maxWidth);
    }

    dc.SelectFont(hOldFont);
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

LRESULT AttributeDlg::OnMouseWheel(UINT nFlags, short zDelta, const CPoint&)
{
    if ((GetKeyState(VK_CONTROL) & 0x8000) == 0) {
        return 1;
    }

    if (zDelta > 0 && m_fontSize < 48) {
        m_fontSize++;
    } else if (zDelta < 0 && m_fontSize > 6) {
        m_fontSize--;
    }

    LOGFONT lf = {};
    lf.lfHeight = -MulDiv(m_fontSize, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
    _tcscpy_s(lf.lfFaceName, _T("Tahoma"));
    lf.lfWeight = FW_NORMAL;

    m_font.DeleteObject();
    m_font.CreateFontIndirect(&lf);

    m_list.SetFont(m_font);

    AutoAdjustColumns();

    m_list.Invalidate();

    return 1;
}

void AttributeDlg::OnContextMenu(const CWindow& wnd, const CPoint& point)
{
    if (wnd != m_list) {
        return;
    }

    CMenu menu;
    menu.LoadMenuW(IDR_ATTRIBUTE_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);
    auto cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, *this);
    if (cmd == 0) {
        return; // No command selected
    }

    auto selectedRow = m_list.GetSelectedIndex();
    if (selectedRow < 0) {
        return; // No item selected
    }

    CString text;
    switch (cmd) {
    case ID_ATTRIBUTE_COPYNAME: // Copy Name
        m_list.GetItemText(selectedRow, 0, text);
        break;
    case ID_ATTRIBUTE_COPYVALUE: // Copy Value
        m_list.GetItemText(selectedRow, 1, text);
        break;
    case ID_ATTRIBUTE_COPYTYPE: // Copy Type
        m_list.GetItemText(selectedRow, 2, text);
        break;
    default:
        return; // Unknown command
    }

    if (text.IsEmpty()) {
        return; // Nothing to copy
    }

    Util::CopyToClipboard(*this, text);
}

LRESULT AttributeDlg::OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto pia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
    if (!pia || pia->iItem < 0) {
        return 0;
    }

    CString id, value;
    m_list.GetItemText(pia->iItem, 1, value);
    if (value.IsEmpty()) {
        return 0;
    }

    CWaitCursor cursor;

    IconDlg dlg(value);
    if (!dlg.HasImage()) {
        return 0;
    }

    dlg.DoModal(*this);

    return 0;
}

BOOL AttributeDlg::OnInitDialog(HWND, LPARAM)
{
    m_list = GetDlgItem(IDC_LST_ATTRIBUTES);
    ATLASSERT(m_list.IsWindow());

    m_list.ModifyStyle(0, LVS_REPORT | LVS_SINGLESEL);
    m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LOGFONT lf = {};
    lf.lfHeight = -MulDiv(m_fontSize, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
    _tcscpy_s(lf.lfFaceName, _T("Tahoma"));
    lf.lfWeight = FW_NORMAL;

    m_font.CreateFontIndirect(&lf);
    m_list.SetFont(m_font);
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

    m_list.SortItemsEx(AttributeListCompare, reinterpret_cast<LPARAM>(&m_list));

    DlgResize_Init();
    AutoAdjustColumns();

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}
