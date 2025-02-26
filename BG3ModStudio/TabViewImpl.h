#pragma once

#include "TabViewCtrl.h"

template<class T, class TBase = TabViewCtrl, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE TabViewImpl : public CWindowImpl<T, TBase, TWinTraits>
{
public:
    DECLARE_WND_CLASS_EX2(NULL, T, 0, COLOR_APPWORKSPACE)

    CContainedWindowT<CTabCtrl> m_tabCtrl;

    static constexpr auto m_nTabID = 0x1001;
    static constexpr auto ALT_MSG_MAP_TABCTRL = 1;

    BEGIN_MSG_MAP(TabViewImpl)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        //MSG_WM_SIZE(OnSize)
    ALT_MSG_MAP(ALT_MSG_MAP_TABCTRL)
    END_MSG_MAP()

    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct)
   {
        ATLASSERT(this->IsWindow());
        ATLASSERT(!m_tabCtrl.IsWindow());

        CreateTabControl();
        ATLASSERT(m_tabCtrl.m_hWnd != nullptr);
        if (m_tabCtrl.m_hWnd == nullptr) {
            ATLTRACE("Failed to create tab control.\n");
            return -1;
        }

        return 0;
    }

    void OnDestroy()
    {
        RemoveAllPages();
    }

    void OnSize(UINT nType, CSize size)
    {
        /*T* pT = static_cast<T*>(this);
        pT->UpdateLayout();*/
    }

    void RemovePage(int index)
    {
        ATLASSERT(this->IsWindow());
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

    TabViewImpl() : m_tabCtrl(this, ALT_MSG_MAP_TABCTRL)
    {    
    }

    BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (!this->IsWindow()) {
            return FALSE;
        }

        return FALSE;
    }

    int GetPageCount() const
    {
        ATLASSERT(this->IsWindow());
        return this->GetItemCount();
    }

    int GetActivePage() const
    {
        ATLASSERT(this->IsWindow());
        return this->GetActiveTab();
    }

    void SetActivePage(int index)
    {
        ATLASSERT(this->IsWindow());
        this->SetActiveTab(index);
    }

    void ClosePage(int index)
    {
        ATLASSERT(this->IsWindow());
        this->RemoveTab(index);
    }

    BOOL InsertPage(int nPage, HWND hWndView, LPCTSTR lpszTitle, int nImage = -1, LPVOID pData = nullptr)
    {
        ATLASSERT(this->IsWindow());

        TVWITEM item{};
        item.tci.mask = TCIF_TEXT | TCIF_PARAM;
        item.tci.pszText = const_cast<LPTSTR>(lpszTitle);
        item.tci.lParam = reinterpret_cast<LPARAM>(pData);
        item.hWndView = hWndView;

        return this->AddTab(&item);
    }

    // FIXME: this seems wrong
    BOOL CreateTabViewControl()
    {
        this->Create(this->m_hWnd, this->rcDefault, nullptr, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 1);
        ATLASSERT(this->m_hWnd != NULL);
        if (this->m_hWnd == nullptr) {
            ATLTRACE("Failed to create tab view control.\n");
            return FALSE;
        }

        return TRUE;
    }

    HWND CreateTabControl(_U_RECT rect = nullptr, DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, DWORD dwStyleEx = 0)
    {
        ATLASSERT(this->IsWindow());
        ATLASSERT(!m_tabCtrl.IsWindow());

        m_tabCtrl.Create(*this, rect, nullptr, dwStyle, dwStyleEx, m_nTabID);

        ATLASSERT(m_tabCtrl.m_hWnd != nullptr);
        if (m_tabCtrl.m_hWnd == nullptr) {
            ATLTRACE("Failed to create tab control.\n");
            return nullptr;
        }

        LOGFONT lf;
        memset(&lf, 0, sizeof(LOGFONT));
        lf.lfHeight = 18;
        lf.lfWeight = FW_SEMIBOLD;
        _tcscpy_s(lf.lfFaceName, _T("Arial"));

        auto hFont = ::CreateFontIndirect(&lf);
        m_tabCtrl.SetFont(hFont);
        //m_tabCtrl.SetFont(AtlCreateControlFont());

        this->SetTabCtrl(m_tabCtrl);

        return m_tabCtrl;
    }

    void OnPageActivated(int nPage)
    {
        NMHDR nmhdr;
        nmhdr.hwndFrom = this->m_hWnd;
        nmhdr.idFrom = nPage;
        nmhdr.code = TBVN_PAGEACTIVATED;
        this->GetParent().SendMessage(WM_NOTIFY, this->GetDlgCtrlID(), reinterpret_cast<LPARAM>(&nmhdr));
    }

    bool AddPage(HWND hWndView, LPCTSTR lpstrTitle, int nImage = -1, LPVOID pData = nullptr)
    {
        return InsertPage(GetPageCount(), hWndView, lpstrTitle, nImage, pData);
    }

    LPVOID GetPageData(int index) const
    {
        ATLASSERT(this->IsWindow());
        TCITEM item{};
        item.mask = TCIF_PARAM;
        this->GetTabItem(index, &item);
        return reinterpret_cast<LPVOID>(item.lParam);
    }

    void SetPageData(int index, LPVOID pData)
    {
        ATLASSERT(this->IsWindow());
        TCITEM item{};
        item.mask = TCIF_PARAM;
        item.lParam = reinterpret_cast<LPARAM>(pData);
        this->SetTabItem(index, &item);
    }
};
