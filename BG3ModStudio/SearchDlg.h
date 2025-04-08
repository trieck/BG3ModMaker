#pragma once
#include "resources/resource.h"

class SearchDlg : public CDialogImpl<SearchDlg>,
                  public CDialogResize<SearchDlg>
{
public:
    enum { IDD = IDD_SEARCH };

    BEGIN_MSG_MAP(IndexDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER3(IDC_B_SEARCH, OnSearch)
        NOTIFY_HANDLER(IDC_LST_RESULTS, NM_DBLCLK, OnDoubleClick)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(SearchResultsDlg)
        DLGRESIZE_CONTROL(IDC_LST_RESULTS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

private:
    void AutoAdjustColumns();

    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    LRESULT OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    void OnClose();
    void OnSearch();
    void OnSize(UINT /*uMsg*/, const CSize& size);

    CListViewCtrl m_listResults;
};
