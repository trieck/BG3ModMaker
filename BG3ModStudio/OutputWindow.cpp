#include "stdafx.h"
#include "OutputWindow.h"
#include "resources/resource.h"
#include "SelectObject.h"
#include "Util.h"

static constexpr auto CLOSE_BUTTON_WIDTH = 20;
static constexpr auto TEXT_LPADDING = 8;
static constexpr auto TITLE_BAR_PADDING = 8;

LRESULT OutputWindow::OnCreate(LPCREATESTRUCT)
{
    if (!m_listView.Create(*this, rcDefault, nullptr,
                           WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT |
                           LVS_SINGLESEL | LVS_SORTASCENDING, WS_EX_CLIENTEDGE | LVS_EX_FULLROWSELECT)) {
        ATLTRACE("Unable to create list view.\n");
        return -1;
    }

    auto index = m_listView.InsertColumn(0, L"Time", LVCFMT_LEFT, 100);
    if (index == -1) {
        ATLTRACE("Unable to insert column into list view.\n");
        return -1;
    }

    index = m_listView.InsertColumn(1, L"Message", LVCFMT_LEFT, 500);
    if (index == -1) {
        ATLTRACE("Unable to insert column into list view.\n");
        return -1;
    }

    m_font = AtlCreateControlFont();

    CalcSizes();

    return 0;
}

void OutputWindow::OnSize(UINT nType, CSize size)
{
    if (m_listView.IsWindow()) {
        m_listView.MoveWindow(0, 0, size.cx, size.cy, TRUE);
    }
}

void OutputWindow::OnClose()
{
    ::SendMessage(GetParent(), WM_COMMAND, ID_VIEW_OUTPUT, 0);
}

void OutputWindow::OnNcPaint(HRGN hRgn)
{
    CRgn region(hRgn);

    CWindowDC dc(*this);

    CRect rcWindow;
    GetWindowRect(&rcWindow);
    rcWindow.OffsetRect(-rcWindow.TopLeft()); // normalize to (0, 0)

    dc.SelectClipRgn(region);
    dc.FillRect(&rcWindow, reinterpret_cast<HBRUSH>(COLOR_APPWORKSPACE + 1));
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(0, 0, 0));

    CRect rcTitle(rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.top + m_titleBarHeight);
    CRect rcText(rcTitle);
    rcText.left += TEXT_LPADDING; // Left padding for title text

    CSelectObject<HFONT> selectFont(dc, m_font);

    dc.DrawText(_T("Output"), -1, &rcText, DT_VCENTER | DT_SINGLELINE);

    CRect rcClose(rcTitle.right - CLOSE_BUTTON_WIDTH,
                  rcTitle.top, rcTitle.right, rcTitle.top + m_titleBarHeight);

    if (m_bCloseHover) {
        dc.FillSolidRect(&rcClose, RGB(240, 240, 240)); // gray background
        dc.DrawEdge(&rcClose, EDGE_RAISED, BF_RECT); // 3D effect
        dc.SetTextColor(0); // Black text           // black text
    } else {
        dc.SetTextColor(RGB(255, 255, 255)); // white text   
        dc.FillSolidRect(&rcClose, RGB(220, 80, 80)); // red background
    }

    dc.DrawText(_T("x"), -1, &rcClose, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    dc.SelectClipRgn(nullptr);
}

LRESULT OutputWindow::OnNcCalcSize(BOOL bCalcValidRects, LPNCCALCSIZE_PARAMS lpncsp)
{
    if (bCalcValidRects) {
        lpncsp->rgrc[0].top += m_titleBarHeight;
    }

    return 0;
}

LRESULT OutputWindow::OnNcHitTest(CPoint point)
{
    CRect rcWindow;
    GetWindowRect(&rcWindow);

    point.Offset(-rcWindow.left, -rcWindow.top);
    rcWindow.OffsetRect(-rcWindow.TopLeft());

    CRect rcTitle(rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.top + m_titleBarHeight);

    CRect rcClose(rcTitle.right - CLOSE_BUTTON_WIDTH, rcTitle.top, rcTitle.right, rcTitle.top + m_titleBarHeight);

    if (rcClose.PtInRect(point)) {
        return HTCLOSE;
    }

    return HTCAPTION;
}

void OutputWindow::OnNcMouseLeave()
{
    if (m_bCloseHover) {
        m_bCloseHover = FALSE;
        Redraw();
    }
}

void OutputWindow::AddLog(const CString& message, BOOL ensureVisible)
{
    auto time = Util::GetCurrentTimeString();

    auto index = m_listView.InsertItem(0, time);
    if (index == -1) {
        ATLTRACE("Unable to insert item into list view.\n");
        return;
    }

    m_listView.SetItemText(index, 1, message);
    m_listView.EnsureVisible(index, ensureVisible);
}

void OutputWindow::CalcSizes()
{
    CClientDC dc(*this);
    CSelectObject<HFONT> selectFont(dc, m_font);

    TEXTMETRIC tm{};
    dc.GetTextMetrics(&tm);

    GetCharWidth32(dc, '0', '0', &m_cxChar);

    m_cyChar = tm.tmHeight + tm.tmExternalLeading;

    m_titleBarHeight = m_cyChar + TITLE_BAR_PADDING;
}

void OutputWindow::Redraw()
{
    if (!IsWindow()) {
        return;
    }

    RedrawWindow(nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

    if (!m_listView.IsWindow()) {
        return;
    }

    m_listView.RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);

    auto header = m_listView.GetHeader();
    if (header) {
        header.RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

LRESULT OutputWindow::OnNcLButtonDown(UINT nHitTest, const CPoint& point)
{
    if (nHitTest == HTCLOSE) {
        ::SendMessage(GetParent(), WM_COMMAND, ID_VIEW_OUTPUT, 0);
    }

    return 0;
}

LRESULT OutputWindow::OnNcMouseMove(UINT nHitTest, CPoint point)
{
    CRect rcWindow;
    GetWindowRect(&rcWindow);

    point.Offset(-rcWindow.left, -rcWindow.top);
    rcWindow.OffsetRect(-rcWindow.TopLeft());

    CRect rcTitle(rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.top + m_titleBarHeight);

    CRect rcClose(rcTitle.right - CLOSE_BUTTON_WIDTH,
                  rcTitle.top, rcTitle.right, rcTitle.top + m_titleBarHeight);

    BOOL bCloseHover = rcClose.PtInRect(point);

    if (m_bCloseHover != bCloseHover) {
        m_bCloseHover = bCloseHover;
        Redraw();
    }

    TRACKMOUSEEVENT tme{};
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
    tme.hwndTrack = m_hWnd;
    TrackMouseEvent(&tme);

    return 0;
}
