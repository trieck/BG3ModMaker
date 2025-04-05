#pragma once
#include "resources/resource.h"

class SearchResultsDlg : public CDialogImpl<SearchResultsDlg>,
                         public CDialogResize<SearchResultsDlg>
{
public:
    enum { IDD = IDD_SEARCH_RESULTS };

    BEGIN_MSG_MAP(IndexDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(SearchResultsDlg)
        DLGRESIZE_CONTROL(IDC_LST_RESULTS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

private:
    void AutoAdjustColumns();

    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    void OnClose();
    void OnSize(UINT /*uMsg*/, const CSize &size);

    CListViewCtrl m_listResults;
};
