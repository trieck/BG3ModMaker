#pragma once

#include "Iconizer.h"
#include "ImageView.h"
#include "ModelessDialog.h"
#include "resources/resource.h"

class IconDlg : public ModelessDialog<IconDlg>,
                public CDialogResize<IconDlg>,
                public CUpdateUI<IconDlg>,
                public CIdleHandler
{
public:
    enum { IDD = IDD_ICON_EXPLORER };

    BEGIN_MSG_MAP(IconDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER3(IDC_B_ICON_FIRST_PAGE, OnFirstPage)
        COMMAND_ID_HANDLER3(IDC_B_ICON_NEXT_PAGE, OnNextPage)
        COMMAND_ID_HANDLER3(IDC_B_ICON_PREV_PAGE, OnPrevPage)
        COMMAND_ID_HANDLER3(IDC_B_ICON_LAST_PAGE, OnLastPage)
        COMMAND_ID_HANDLER3(IDC_B_SEARCH_ICON, OnSearch)
        COMMAND_HANDLER3(ID_ICON_LIST, LBN_SELCHANGE, OnIconSelChange)
        COMMAND_HANDLER3(IDC_E_QUERY_ICON, EN_CHANGE, OnQueryChange)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(IconDlg)
    END_DLGRESIZE_MAP()

    BEGIN_UPDATE_UI_MAP(IconDlg)
        UPDATE_ELEMENT(IDC_B_ICON_FIRST_PAGE, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_ICON_PREV_PAGE, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_ICON_NEXT_PAGE, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_ICON_LAST_PAGE, UPDUI_CHILDWINDOW)
    END_UPDATE_UI_MAP()

    BOOL OnIdle() override;

private:
    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    void OnIconSelChange();
    void OnClose();
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
    void RenderIcon(const DirectX::ScratchImage& icon);

    Iconizer m_iconizer;
    PageableIterator::Ptr m_iterator;

    CSplitterWindow m_splitter;
    CListBox m_list;
    ImageView m_iconView;
    CStatic m_pageInfo;
    CFont m_font;
    CString m_dbPath;

    int m_marginLeft = 0;
    int m_marginTop = 0;
    int m_marginRight = 0;
    int m_marginBottom = 0;
    int m_nPage = 0;
};
