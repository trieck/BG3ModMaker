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
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MESSAGE_HANDLER5(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        MESSAGE_HANDLER3(WM_SHOWWINDOW, OnShowWindow)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(ValueViewDlg)
        DLGRESIZE_CONTROL(IDC_E_VALUE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    void SetTitle(const CString& title);
    void SetValue(const CString& value);

private:
    BOOL CreateFont();
    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    LRESULT OnCtlColorStatic(WPARAM, LPARAM);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, const CPoint& /*pt*/);
    void OnClose();
    void OnSize(UINT /*uMsg*/, const CSize& size);
    void OnShowWindow();

    static constexpr COLORREF m_clrBkgnd = RGB(0xFF, 0xF8, 0xDC);
    CBrush m_brBackground;
    CEdit m_edit;
    CFont m_font;
    CString m_title, m_value;
    int m_fontSize = 9;
};
