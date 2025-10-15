#pragma once

#include "TabViewCtrl.h"

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE TabViewImpl : public CWindowImpl<T, TBase, TWinTraits>
{
public:
    DECLARE_WND_CLASS_EX2(NULL, T, 0, COLOR_APPWORKSPACE)

    CContainedWindowT<TabViewCtrl> m_tabViewCtrl;

    BEGIN_MSG_MAP(TabViewImpl)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_CONTEXTMENU(OnContextMenu)
    END_MSG_MAP()

    TabViewImpl() : m_tabViewCtrl(this, 1)
    {
    }

    LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        auto lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
        if (wParam != TabViewCtrl::m_nTabID) {
            return 0;
        }

        auto pT = static_cast<T*>(this);
        pT->OnDrawTabItem(lpDrawItemStruct);

        bHandled = TRUE;

        return 0;
    }

    LRESULT OnTabSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
    {
        ATLASSERT(idCtrl == TabViewCtrl::m_nTabID);
        ATLASSERT(pnmh->hwndFrom == m_tabViewCtrl.m_tabCtrl);
        ATLASSERT(pnmh->code == TCN_SELCHANGE);

        auto index = m_tabViewCtrl.GetActiveTab();
        if (index == -1) {
            return 0;
        }

        SetActivePage(index);

        auto pT = static_cast<T*>(this);
        pT->OnPageActivated(index);

        UpdateLayout();

        bHandled = TRUE;

        return 0;
    }

    LRESULT OnContextMenu(HWND hWnd, const CPoint& pt)
    {
        if (hWnd != m_tabViewCtrl) {
            return 0;
        }

        TCHITTESTINFO hti{};
        hti.pt = pt;

        auto& tabCtrl = m_tabViewCtrl.m_tabCtrl;

        auto result = tabCtrl.ScreenToClient(&hti.pt);
        if (!result) {
            return 0;
        }

        auto nCurSel = tabCtrl.HitTest(&hti);
        if (nCurSel == -1) {
            return 0;
        }

        auto pT = static_cast<T*>(this);
        pT->OnPageContextMenu(nCurSel, pt);

        return 0;
    }

    void UpdateLayout()
    {
        CRect rc;
        this->GetClientRect(&rc);

        if (m_tabViewCtrl.IsWindow()) {
            m_tabViewCtrl.SetWindowPos(nullptr, rc.left, rc.top, rc.Width() + 1, rc.Height() + 1, SWP_NOZORDER);
            m_tabViewCtrl.SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
        }
    }

    void OnSize(UINT nType, CSize size)
    {
        this->DefWindowProc();

        if (m_tabViewCtrl.IsWindow()) {
            m_tabViewCtrl.SetWindowPos(nullptr, 0, 0, size.cx, size.cy, SWP_NOZORDER);
        }
    }

    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        ATLASSERT(!m_tabViewCtrl.IsWindow());

        if (!m_tabViewCtrl.Create(*this, this->rcDefault)) {
            ATLTRACE("Failed to create tab view control.\n");
            return -1;
        }

        auto pT = static_cast<T*>(this);
        auto dwStyle = pT->GetTabCtrlStyle();

        if (!m_tabViewCtrl.CreateTabControl(this->rcDefault, nullptr, dwStyle)) {
            ATLTRACE("Failed to create tab control.\n");
            return -1;
        }

        ATLASSERT(m_tabViewCtrl.IsWindow());
        ATLASSERT(m_tabViewCtrl.m_tabCtrl.IsWindow());

        m_tabViewCtrl.ShowTabCtrl(FALSE);

        return 0;
    }

    void OnDestroy()
    {
        RemoveAllPages();
    }

    void RemovePage(int index)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());

        if (index < 0 || index >= GetPageCount()) {
            return;
        }

        ClosePage(index);

        int nPage = GetActivePage();

        auto pT = static_cast<T*>(this);
        pT->OnPageActivated(nPage);

        UpdateLayout();
    }

    void RemoveAllPages()
    {
        for (int i = 0; i < GetPageCount(); i++) {
            ClosePage(i);
        }
    }

    int GetPageCount() const
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());
        return m_tabViewCtrl.GetItemCount();
    }

    CString GetPageTitle(int index) const
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());

        CString title;
        auto* ptitle = title.GetBuffer(MAX_PATH);

        TCITEM item{};
        item.mask = TCIF_TEXT;
        item.cchTextMax = MAX_PATH;
        item.pszText = ptitle;

        m_tabViewCtrl.GetTabItem(index, &item);
        title.ReleaseBuffer();

        return title;
    }

    void SetPageTitle(int index, LPCTSTR lpstrTitle)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());

        TCITEM item{};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<LPTSTR>(lpstrTitle);

        m_tabViewCtrl.SetTabItem(index, &item);
    }

    int GetActivePage() const
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());
        return m_tabViewCtrl.GetActiveTab();
    }

    HWND GetActiveView() const
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());
        return m_tabViewCtrl.GetView(m_tabViewCtrl.GetActiveTab());
    }

    void SetActivePage(int index)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());
        m_tabViewCtrl.SetActiveTab(index);
    }

    void ClosePage(int index)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());

        m_tabViewCtrl.RemoveTab(index);

        if (GetPageCount() == 0) {
            m_tabViewCtrl.ShowTabCtrl(FALSE);
        }
    }

    BOOL InsertPage(int nPage, HWND hWndView, LPCTSTR lpszTitle, int nImage = -1, LPVOID pData = nullptr)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());

        TVWITEM item{};
        item.tci.mask = TCIF_TEXT | TCIF_PARAM;
        item.tci.pszText = const_cast<LPTSTR>(lpszTitle);
        item.tci.iImage = nImage;
        item.tci.lParam = reinterpret_cast<LPARAM>(pData);
        item.hWndView = hWndView;

        if (!m_tabViewCtrl.AddTab(&item)) {
            return FALSE;
        }

        SetActivePage(nPage);

        auto pT = static_cast<T*>(this);
        pT->OnPageActivated(nPage);

        UpdateLayout();

        return TRUE;
    }

    BOOL AddPage(HWND hWndView, LPCTSTR lpstrTitle, int nImage = -1, LPVOID pData = nullptr)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());

        int nCount = GetPageCount();

        auto result = InsertPage(nCount, hWndView, lpstrTitle, nImage, pData);
        if (!result) {
            return FALSE;
        }

        if (nCount == 0) {
            m_tabViewCtrl.ShowTabCtrl(TRUE);
        }

        return TRUE;
    }

    LPVOID GetPageData(int index) const
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());
        TCITEM item{};
        item.mask = TCIF_PARAM;

        if (!m_tabViewCtrl.GetTabItem(index, &item)) {
            return nullptr;
        }

        return reinterpret_cast<LPVOID>(item.lParam);
    }

    void SetPageData(int index, LPVOID pData)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());
        TCITEM item{};
        item.mask = TCIF_PARAM;
        item.lParam = reinterpret_cast<LPARAM>(pData);
        m_tabViewCtrl.SetTabItem(index, &item);
    }
};
