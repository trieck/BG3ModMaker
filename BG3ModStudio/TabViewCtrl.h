#pragma once

#include "TabView.h"

template<class TBase>
class TabViewCtrlT : public TBase
{
public:

    CTabCtrl m_tabCtrl;
    static constexpr auto m_nTabID = 0x1001;

    explicit TabViewCtrlT(HWND hWnd = nullptr) : TBase(hWnd)
    {
    }

    TabViewCtrlT& operator=(HWND hWnd)
    {
        this->m_hWnd = hWnd;
        return *this;
    }

    HWND Create(HWND hWndParent, _U_RECT rect = nullptr, LPCTSTR szWindowName = nullptr,
        DWORD dwStyle = CControlWinTraits::GetWndStyle(0), DWORD dwExStyle = CControlWinTraits::GetWndExStyle(0), _U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = nullptr)
    {
        ATLASSERT(hWndParent != nullptr);
        ATLASSERT(this->m_hWnd == nullptr);
        ATLASSERT(!m_tabCtrl.IsWindow());

        if (!TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam)) {
            ATLTRACE("Failed to create tab view control.\n");
            return nullptr;
        }

        if (!m_tabCtrl.Create(*this, this->rcDefault, nullptr, CControlWinTraits::GetWndStyle(0) | TCS_TOOLTIPS, 0, m_nTabID)) {
            ATLTRACE("Failed to create tab control.\n");
            return nullptr;
        }
        
        m_tabCtrl.SetFont(AtlCreateControlFont());

        this->SetTabCtrl(m_tabCtrl);

        return this->m_hWnd;
    }

    static LPCTSTR GetWndClassName()
    {
        return WC_TABVIEW;
    }

    int GetItemCount() const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<int>(::SendMessage(this->m_hWnd, TVWM_GETTABCOUNT, 0, 0));
    }

    BOOL GetTabItem(int nItem, LPTCITEM pItem) const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<BOOL>(::SendMessage(this->m_hWnd, TVWM_GETTABITEM, nItem, reinterpret_cast<LPARAM>(pItem)));
    }

    void SetTabItem(int nItem, LPTCITEM pItem)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        ::SendMessage(this->m_hWnd, TVWM_SETTABITEM, nItem, reinterpret_cast<LPARAM>(pItem));
    }

    void SetTabCtrl(HWND hWndTabCtrl)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        ::SendMessage(this->m_hWnd, TVWM_SETTABCTRL, 0, reinterpret_cast<LPARAM>(hWndTabCtrl));
    }

    BOOL AddTab(LPTVWITEM pItem)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<BOOL>(::SendMessage(this->m_hWnd, TVWM_ADDTAB, 0, reinterpret_cast<LPARAM>(pItem)));
    }

    BOOL RemoveTab(int nItem)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<BOOL>(::SendMessage(this->m_hWnd, TVWM_REMOVETAB, nItem, 0));
    }

    BOOL ShowTabCtrl(BOOL bShow)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<BOOL>(::SendMessage(this->m_hWnd, TVWM_SHOWTABCTRL, bShow, 0));
    }

    BOOL SetActiveTab(int nItem)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<BOOL>(::SendMessage(this->m_hWnd, TVWM_SETACTIVETAB, nItem, 0));
    }

    void SetViewBorder(int nBorder)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        ::SendMessage(this->m_hWnd, TVWM_SETVIEWBORDER, nBorder, 0);
    }

    int GetViewBorder() const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<int>(::SendMessage(this->m_hWnd, TVWM_GETVIEWBORDER, 0, 0));
    }

    void SetTabHeight(int nHeight)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        ::SendMessage(this->m_hWnd, TVWM_SETTABHEIGHT, nHeight, 0);
    }

    int GetTabHeight() const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<int>(::SendMessage(this->m_hWnd, TVWM_GETTABHEIGHT, 0, 0));
    }

    HWND GetView(int nItem) const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return reinterpret_cast<HWND>(::SendMessage(this->m_hWnd, TVWM_GETVIEW, nItem, 0));
    }

    BOOL SetView(int nItem, HWND hWndView)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<BOOL>(::SendMessage(this->m_hWnd, TVWM_SETVIEW, nItem, reinterpret_cast<LPARAM>(hWndView)));
    }

    int GetActiveTab() const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<int>(::SendMessage(this->m_hWnd, TVWM_GETACTIVETAB, 0, 0));
    }

    int HitTest(int x, int y) const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<int>(::SendMessage(this->m_hWnd, TVWM_HITTEST, 0, MAKELPARAM(x, y)));
    }

    void ContextMenu(HWND hWndTabCtrl, POINT pt) const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));

        ::SendMessage(this->m_hWnd, WM_CONTEXTMENU, reinterpret_cast<WPARAM>(hWndTabCtrl), MAKELPARAM(pt.x, pt.y));
    }

    void SetBkColor(COLORREF color)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        ::SendMessage(this->m_hWnd, TVWM_SETBKCOLOR, color, 0);
    }

    COLORREF GetBkColor() const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<COLORREF>(::SendMessage(this->m_hWnd, TVWM_GETBKCOLOR, 0, 0));
    }

    void SetTopMargin(int nBorder)
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        ::SendMessage(this->m_hWnd, TVWM_SETTOPMARGIN, nBorder, 0);
    }

    int GetTopMargin() const
    {
        ATLASSERT(::IsWindow(this->m_hWnd));
        return static_cast<int>(::SendMessage(this->m_hWnd, TVWM_GETTOPMARGIN, 0, 0));
    }
};

using TabViewCtrl = TabViewCtrlT<CWindow>;
