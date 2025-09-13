#pragma once

#include <xapian.h>

#include "ModelessDialog.h"
#include "resources/resource.h"

class SearchDlg : public ModelessDialog<SearchDlg>,
                  public CDialogResize<SearchDlg>,
                  public CUpdateUI<SearchDlg>,
                  public CIdleHandler
{
public:
    enum { IDD = IDD_SEARCH };

    BEGIN_MSG_MAP(SearchDlg)
        NOTIFY_HANDLER(IDC_LST_RESULTS, NM_DBLCLK, OnDoubleClick)
        MSG_WM_SIZE(OnSize)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
        COMMAND_ID_HANDLER3(IDC_B_SEARCH, OnSearch)
        COMMAND_ID_HANDLER3(IDC_B_FIRST, OnFirst)
        COMMAND_ID_HANDLER3(IDC_B_PREV, OnPrev)
        COMMAND_ID_HANDLER3(IDC_B_NEXT, OnNext)
        COMMAND_ID_HANDLER3(IDC_B_LAST, OnLast)
        COMMAND_ID_HANDLER3(IDC_B_BROWSE, OnBrowse)
        COMMAND_HANDLER3(IDC_E_QUERY, EN_CHANGE, OnQueryChange)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(SearchDlg)
        DLGRESIZE_CONTROL(IDC_LST_RESULTS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    BEGIN_UPDATE_UI_MAP(SearchDlg)
        UPDATE_ELEMENT(IDC_B_FIRST, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_PREV, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_NEXT, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_LAST, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_PAGEINFO, UPDUI_CHILDWINDOW)
    END_UPDATE_UI_MAP()

    BOOL OnIdle() override;

private:
    uint32_t GetPageCount() const;
    void AutoAdjustColumns();
    void Search();
    void Search(uint32_t offset);
    void UpdatePageInfo();

    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    LRESULT OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    void OnQueryChange();
    void OnBrowse();
    void OnClose();
    void OnFirst();
    void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    void OnLast();
    void OnNext();
    void OnPrev();
    void OnSearch();
    void OnSize(UINT /*uMsg*/, const CSize& size);

    CListViewCtrl m_listResults;
    CEdit m_indexPath;
    CStatic m_pageInfo;

    Xapian::MSet m_results;
    int m_nPage = 0;
};
