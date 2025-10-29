#pragma once

#include "ModelessDialog.h"
#include "resources/resource.h"

class FindReplaceDlg : public ModelessDialog<FindReplaceDlg>,
                       public CUpdateUI<FindReplaceDlg>,
                       public CIdleHandler
{
public:
    enum { IDD = IDD_FIND_REPLACE };

    BEGIN_MSG_MAP(FindReplaceDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_DESTROY(OnDestroy)
        COMMAND_ID_HANDLER3(IDC_B_FIND_NEXT, OnFindNext)
        COMMAND_ID_HANDLER3(IDC_B_REPLACE, OnReplace)
        COMMAND_ID_HANDLER3(IDC_B_REPLACE_ALL, OnReplaceAll)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
    END_MSG_MAP()

    BEGIN_UPDATE_UI_MAP(FindReplaceDlg)
        UPDATE_ELEMENT(IDC_B_FIND_NEXT, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_REPLACE, UPDUI_CHILDWINDOW)
        UPDATE_ELEMENT(IDC_B_REPLACE_ALL, UPDUI_CHILDWINDOW)
    END_UPDATE_UI_MAP()

    BOOL OnIdle() override;

private:
    BOOL HasEditableView();
    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    void OnFindNext();
    void OnReplace();
    void OnReplaceAll();
    void OnClose();
    void OnDestroy();
};
