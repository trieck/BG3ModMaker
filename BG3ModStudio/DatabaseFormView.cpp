#include "stdafx.h"
#include "DatabaseFormView.h"
#include "OsiData.h"
#include "StringHelper.h"
#include "Util.h"

static constexpr auto MIN_LAST_COLUMN_WIDTH = 150;

HWND DatabaseFormView::Create(HWND hWndParent, LPARAM dwInitParam)
{
    if (!Base::Create(hWndParent, dwInitParam)) {
        return nullptr;
    }

    auto* pData = reinterpret_cast<OsiData*>(dwInitParam);
    ATLASSERT(pData != nullptr);

    m_pStory = pData->pStory;
    ATLASSERT(m_pStory != nullptr);

    m_pDatabase = static_cast<const OsiDatabase*>(pData->pdata);
    ATLASSERT(m_pDatabase != nullptr);

    m_pOwnerNode = m_pStory->nodes[m_pDatabase->ownerNode].get();
    ATLASSERT(m_pOwnerNode != nullptr);

    m_font = Util::CreateDialogFont(80, TRUE);

    m_view = GetDlgItem(IDC_LST_DATABASE);
    ATLASSERT(m_view.IsWindow());
    (void)SetWindowTheme(m_view, L"", L"");
    (void)SetWindowTheme(m_view.GetHeader(), L"", L"");

    m_view.SetFont(m_font);
    m_view.ModifyStyle(0, LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
    m_view.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    InsertColumns();
    InsertFacts();

    DlgResize_Init(FALSE);

    CRect rc;
    m_view.GetClientRect(&rc);
    m_view.SetWindowPos(
        nullptr,
        0, 0,
        rc.Width(),
        rc.Height(),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );

    return this->m_hWnd;
}

void DatabaseFormView::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    AutoAdjustColumns();
}

void DatabaseFormView::InsertColumns()
{
    auto i = 0;
    for (const auto& param : m_pDatabase->parameters.types) {
        auto typeName = m_pStory->typeName(param);
        auto wideType = StringHelper::fromUTF8(typeName.c_str());
        m_view.InsertColumn(i++, wideType, LVCFMT_LEFT, MIN_LAST_COLUMN_WIDTH);
    }

    AutoAdjustColumns();
}

void DatabaseFormView::InsertFacts()
{
    for (const auto& fact : m_pDatabase->facts) {
        auto i = 0;
        auto row = 0;

        for (const auto& param : fact.columns) {
            CString paramValue = StringHelper::fromUTF8(param.toString().c_str());
            if (i == 0) {
                row = m_view.InsertItem(m_view.GetItemCount(), paramValue);
            } else {
                m_view.SetItemText(row, i, paramValue);
            }
            ++i;
        }
    }
}

void DatabaseFormView::AutoAdjustColumns()
{
    auto columnCount = m_view.GetHeader().GetItemCount();
    if (columnCount == 0) {
        return;
    }

    CRect rcClient;
    m_view.GetClientRect(&rcClient);

    // Calculate total width of all columns except the last
    int totalWidth = 0;
    for (auto i = 0; i < columnCount - 1; ++i) {
        totalWidth += m_view.GetColumnWidth(i);
    }

    auto remaining = std::max(MIN_LAST_COLUMN_WIDTH, rcClient.Width() - totalWidth);

    m_view.SetColumnWidth(columnCount - 1, remaining);
}
