#pragma once

class OutputWindow : public CWindowImpl<OutputWindow>
{
public:
    DECLARE_WND_SUPERCLASS(L"BG3_OutputWnd", CListViewCtrl::GetWndClassName())

    BEGIN_MSG_MAP_EX(FolderView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_NCPAINT(OnNcPaint)
        MSG_WM_NCCALCSIZE2(OnNcCalcSize)
        MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
        MSG_WM_NCHITTEST(OnNcHitTest)
        MSG_WM_NCMOUSEMOVE(OnNcMouseMove)
        MSG_WM_NCMOUSELEAVE(OnNcMouseLeave)
        ALT_MSG_MAP(1)
    END_MSG_MAP()

    OutputWindow() : m_bMsgHandled(FALSE), m_listView(this, 1) {
    }

    LRESULT OnCreate(LPCREATESTRUCT /*pcs*/);
    void OnClose();
    void OnNcPaint(HRGN hRgn);
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPNCCALCSIZE_PARAMS lpncsp);
    LRESULT OnNcLButtonDown(UINT nHitTest, const CPoint& point);
    LRESULT OnNcMouseMove(UINT nHitTest, CPoint point);
    LRESULT OnNcHitTest(CPoint point);
    void OnSize(UINT nType, CSize size);
    void OnNcMouseLeave();

    void AddLog(const CString& message, BOOL ensureVisible = FALSE);

private:
    void CalcSizes();
    void Redraw();

    CContainedWindowT<CListViewCtrl> m_listView;

    BOOL m_bCloseHover{};
    CFont m_font;
    int m_cxChar{}, m_cyChar{}, m_titleBarHeight{};
};
