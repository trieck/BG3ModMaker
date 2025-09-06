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
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER3(IDC_B_FIRST_PAGE, OnFirstPage)
        COMMAND_ID_HANDLER3(IDC_B_NEXT_PAGE, OnNextPage)
        COMMAND_ID_HANDLER3(IDC_B_PREV_PAGE, OnPrevPage)
        COMMAND_ID_HANDLER3(IDC_B_LAST_PAGE, OnLastPage)
        COMMAND_HANDLER(ID_UUID_LIST, LBN_SELCHANGE, OnUuidSelChange)
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
    LRESULT OnUuidSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void AutoAdjustAttributes();
    void OnClose();
    void OnFirstPage();
    void OnNextPage();
    void OnPrevPage();
    void OnLastPage();
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

    int m_marginLeft = 0;
    int m_marginTop = 0;
    int m_marginRight = 0;
    int m_marginBottom = 0;
    int m_nPage = 0;
};
