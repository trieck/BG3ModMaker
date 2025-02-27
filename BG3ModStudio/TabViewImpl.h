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
        MSG_WM_SETFOCUS(OnSetFocus)
        NOTIFY_HANDLER(TabViewCtrl::m_nTabID, TCN_SELCHANGE, OnTabChanged)
    END_MSG_MAP()

    void OnSetFocus(HWND /*hWndOld*/)
    {
        if (m_tabViewCtrl.IsWindow()) {
            HWND hWndActivePage = GetActiveView();
            if (hWndActivePage != nullptr) {
                ::SetFocus(hWndActivePage);
            }
        }
    }

    LRESULT OnTabChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
    {
        ATLASSERT(m_tabViewCtrl.IsWindow());
        auto index = m_tabViewCtrl.GetActiveTab();
        if (index == -1) {
            return 0;
        }

        SetActivePage(index);
                        
        T* pT = static_cast<T*>(this);
        pT->OnPageActivated(index);

        return 0;
    }
        
    void UpdateLayout()
    {
        CRect rc;
        this->GetClientRect(&rc);

        if (m_tabViewCtrl.IsWindow()) {
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
