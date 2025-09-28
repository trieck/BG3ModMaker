#include "stdafx.h"
#include "Exception.h"
#include "IconDlg.h"
#include "StringHelper.h"

static constexpr auto PAGE_SIZE = 25;

BOOL IconDlg::OnIdle()
{
    return 0;
}

BOOL IconDlg::OnInitDialog(HWND, LPARAM)
{
    Settings settings;
    m_dbPath = settings.GetString("Settings", "IconPath");

    auto wndFrame = GetDlgItem(IDC_ST_ICON_EXPLORER);
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

    m_pageInfo = GetDlgItem(IDC_ICON_PAGEINFO);
    ATLASSERT(m_pageInfo.IsWindow());

    m_splitter.Create(m_hWnd, rcFrame, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    ATLASSERT(m_splitter.IsWindow());

    m_list.Create(m_splitter, rcDefault, nullptr,
                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LBS_NOINTEGRALHEIGHT |
                  LBS_NOTIFY, WS_EX_CLIENTEDGE, ID_ICON_LIST);
    ATLASSERT(m_list.IsWindow());

    m_font = AtlCreateControlFont();
    m_list.SetFont(m_font);

    m_iconView.Create(m_splitter, rcDefault, nullptr,
                      WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE, ID_ICON_VIEW);
    ATLASSERT(m_iconView.IsWindow());

    m_splitter.SetSplitterPane(0, m_list);
    m_splitter.SetSplitterPane(1, m_iconView);
    m_splitter.SetSplitterPosPct(40);

    Populate();

    UIAddChildWindowContainer(m_hWnd);

    DlgResize_Init();

    CenterWindow(GetParent());

    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddIdleHandler(this);

    return FALSE; // Let the system set the focus
}

void IconDlg::OnIconSelChange()
{
    auto sel = m_list.GetCurSel();
    if (sel == LB_ERR) {
        return;
    }

    CWaitCursor cursor;

    CString iconID;
    m_list.GetText(sel, iconID);

    auto key = StringHelper::toUTF8(iconID);

    auto icon = m_iconizer.getIcon(key.GetString());

    RenderIcon(icon);
}

void IconDlg::OnClose()
{
    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveIdleHandler(this);

    m_iterator = nullptr;
    m_iconizer.close();

    Destroy();
}

void IconDlg::OnFirstPage()
{
}

void IconDlg::OnNextPage()
{
}

void IconDlg::OnPrevPage()
{
}

void IconDlg::OnLastPage()
{
}

void IconDlg::OnQueryChange()
{
}

void IconDlg::OnSearch()
{
}

void IconDlg::OnSize(UINT, const CSize& size)
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

void IconDlg::PopulateKeys()
{
    m_list.ResetContent();

    if (!m_iterator) {
        return;
    }

    for (const auto& key : m_iterator->keys()) {
        auto wideKey = StringHelper::fromUTF8(key.c_str());
        m_list.AddString(wideKey);
    }
}

void IconDlg::Populate()
{
    CWaitCursor cursor;
    Settings settings;

    m_list.ResetContent();

    m_iterator = nullptr;

    try {
        if (!m_iconizer.isOpen()) {
            m_iconizer.open(StringHelper::toUTF8(m_dbPath).GetString());
        }

        m_iterator = m_iconizer.newIterator(PAGE_SIZE);

        PopulateKeys();
    } catch (const Exception& ex) {
        CString msg;
        msg.Format(_T("Failed to open game object database: %s"), CString(ex.what()));
        AtlMessageBox(*this, msg.GetString(), nullptr, MB_ICONERROR);
    }
}

void IconDlg::UpdatePageInfo()
{
}

size_t IconDlg::GetPageCount() const
{
    return 0;
}

void IconDlg::RenderIcon(const DirectX::ScratchImage& icon)
{
}
