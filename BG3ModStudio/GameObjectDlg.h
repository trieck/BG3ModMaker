#pragma once

#include "Cataloger.h"
#include "ModelessDialog.h"
#include "resources/resource.h"

#include <nlohmann/json.hpp>

class GameObjectDlg : public ModelessDialog<GameObjectDlg>,
                      public CDialogResize<GameObjectDlg>,
                      public CUpdateUI<GameObjectDlg>,
                      public CIdleHandler
{
public:
    enum { IDD = IDD_GAMEOBJECT };

    BEGIN_MSG_MAP(GameObjectDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER3(IDC_B_FIRST_PAGE, OnFirstPage)
        COMMAND_ID_HANDLER3(IDC_B_NEXT_PAGE, OnNextPage)
        COMMAND_ID_HANDLER3(IDC_B_PREV_PAGE, OnPrevPage)
        COMMAND_ID_HANDLER3(IDC_B_LAST_PAGE, OnLastPage)
        COMMAND_ID_HANDLER3(IDC_B_SEARCH_GAMEOBJECT, OnSearch)
        COMMAND_HANDLER3(ID_UUID_LIST, LBN_SELCHANGE, OnUuidSelChange)
        COMMAND_HANDLER3(IDC_E_QUERY_GAMEOBJECT, EN_CHANGE, OnQueryChange)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
        NOTIFY_HANDLER(ID_ATTRIBUTE_LIST, NM_DBLCLK, OnDoubleClick)
        MSG_WM_CONTEXTMENU(OnContextMenu)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(GameObjectDlg)
    END_DLGRESIZE_MAP()

    BEGIN_UPDATE_UI_MAP(GameObjectDlg)
        UPDATE_ELEMENT(IDC_B_FIRST_PAGE, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_PREV_PAGE, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_NEXT_PAGE, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_LAST_PAGE, UPDUI_CHILDWINDOW)
    END_UPDATE_UI_MAP()

    BOOL OnIdle() override;

private:
    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    LRESULT OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    void OnContextMenu(const CWindow& wnd, const CPoint& point);
    void OnContextAttributes(const CPoint& point);    
    void OnUuidSelChange();
    void AutoAdjustAttributes();
    void OnClose();
    void OnDestroy();
    void OnFirstPage();
    void OnNextPage();
    void OnPrevPage();
    void OnLastPage();
    void OnQueryChange();
    void OnSearch();
    void OnSize(UINT /*uMsg*/, const CSize& size);
    void PopulateKeys();
    void Populate();
    void UpdatePageInfo();
    size_t GetPageCount() const;

    nlohmann::json GetAttributes(const CString& uuid);

    Cataloger m_cataloger;
    PageableIterator::Ptr m_iterator;

    CSplitterWindow m_splitter;
    CListBox m_list;
    CListViewCtrl m_attributes;
    CStatic m_pageInfo;
    CFont m_font;
    CString m_dbPath;

    int m_marginLeft = 0;
    int m_marginTop = 0;
    int m_marginRight = 0;
    int m_marginBottom = 0;
    int m_nPage = 0;
};
