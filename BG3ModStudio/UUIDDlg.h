#pragma once
#include "resources/resource.h"

class UUIDDlg : public CDialogImpl<UUIDDlg>
{
public:
    enum { IDD = IDD_UUID };

    BEGIN_MSG_MAP(UUIDDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER3(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER3(IDC_GENERATE_UUID, OnGenerateUUID)
        COMMAND_HANDLER3(IDC_ST_UUID, STN_CLICKED, OnUUIDClicked)
        MSG_WM_SETCURSOR(OnSetCursor)
    END_MSG_MAP()

private:
    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    void OnCancel();
    void OnGenerateUUID();
    BOOL OnSetCursor(CWindow /*wnd*/, UINT /*nHitTest*/, UINT /*message*/);
    void OnUUIDClicked();

    CFont m_font;
    CStatic m_uuid;
    CButton m_handle;
};
