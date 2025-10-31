#pragma once
#include "resources/resource.h"

class ValueViewDlg : public CDialogImpl<ValueViewDlg>,
                     public CDialogResize<ValueViewDlg>
{
public:
    enum { IDD = IDD_VALUE_VIEW };

    BEGIN_MSG_MAP(ValueViewDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(ValueViewDlg)
        DLGRESIZE_CONTROL(IDC_E_VALUE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    void SetTitle(const CString& title);
    void SetValue(const CString& value);

private:
    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    void OnClose();
    void OnSize(UINT /*uMsg*/, const CSize& size);

    CString m_title, m_value;
    CEdit m_edit;
    CFont m_font;
};
