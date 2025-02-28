#pragma once

#include "TabViewCtrl.h"

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE TabViewImpl : public CWindowImpl<T, TBase, TWinTraits>
{
public:
    DECLARE_WND_CLASS_EX2(NULL, T, 0, COLOR_APPWORKSPACE)

    TabViewCtrl m_tabViewCtrl;

    BEGIN_MSG_MAP(TabViewImpl)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        NOTIFY_CODE_HANDLER(TVWN_CONTEXTMENU, OnTabContextMenu)
    END_MSG_MAP()

    LRESULT OnTabContextMenu(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
    {
        auto pcmi = reinterpret_cast<LPTBVCONTEXTMENUINFO>(pnmh);
        pcmi->hdr.code = TBVN_CONTEXTMENU;

        SendMessage(GetParent(this->m_hWnd), WM_NOTIFY, idCtrl, reinterpret_cast<LPARAM>(pcmi));

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

        if (!m_tabViewCtrl.Create(*this)) {
            ATLTRACE("Failed to create tab view control.\n");
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

    TabViewImpl() = default;

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

    void OnPageActivated(int nPage)
    {
        NMHDR nmhdr;
        nmhdr.hwndFrom = this->m_hWnd;
        nmhdr.idFrom = nPage;
        nmhdr.code = TBVN_PAGEACTIVATED;
        this->GetParent().SendMessage(WM_NOTIFY, this->GetDlgCtrlID(), reinterpret_cast<LPARAM>(&nmhdr));
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
