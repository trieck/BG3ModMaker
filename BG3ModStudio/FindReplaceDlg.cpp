#include "stdafx.h"
#include "FindReplaceDlg.h"
#include "Util.h"

BOOL FindReplaceDlg::OnIdle()
{
    auto bHasEditableView = HasEditableView();

    UIEnable(IDC_B_FIND_NEXT, bHasEditableView);
    UIEnable(IDC_B_REPLACE, bHasEditableView);
    UIEnable(IDC_B_REPLACE_ALL, bHasEditableView);

    UIUpdateChildWindows(TRUE);

    return FALSE;
}

BOOL FindReplaceDlg::HasEditableView()
{
    auto result = static_cast<BOOL>(GetTopLevelParent().SendMessage(WM_HAS_EDITABLE_VIEW));

    return result;
}

LRESULT FindReplaceDlg::OnInitDialog(HWND, LPARAM)
{
    UIAddChildWindowContainer(m_hWnd);

    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddIdleHandler(this);

    auto icon = Util::LoadBitmapAsIcon(ID_TOOL_FIND_REPLACE, 32, 32);
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    return TRUE; // Let the system set the focus
}

void FindReplaceDlg::OnFindNext()
{
    FINDREPLACE_PARAMS frp{};
    frp.cmd = FRC_FIND_NEXT;
    GetDlgItemText(IDC_E_FIND, frp.findText);
    frp.matchCase = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_MATCHCASE));
    frp.wholeWord = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_WHOLEWORD));
    frp.regex = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_REGEX));

    GetTopLevelParent().SendMessage(WM_FIND_REPLACE_COMMAND, 0, reinterpret_cast<LPARAM>(&frp));
}

void FindReplaceDlg::OnReplace()
{
    FINDREPLACE_PARAMS frp{};
    frp.cmd = FRC_REPLACE;
    GetDlgItemText(IDC_E_FIND, frp.findText);
    GetDlgItemText(IDC_E_REPLACE, frp.replaceText);
    frp.matchCase = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_MATCHCASE));
    frp.wholeWord = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_WHOLEWORD));
    frp.regex = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_REGEX));

    GetTopLevelParent().SendMessage(WM_FIND_REPLACE_COMMAND, 0, reinterpret_cast<LPARAM>(&frp));
}

void FindReplaceDlg::OnReplaceAll()
{
    FINDREPLACE_PARAMS frp{};
    frp.cmd = FRC_REPLACE_ALL;
    GetDlgItemText(IDC_E_FIND, frp.findText);
    GetDlgItemText(IDC_E_REPLACE, frp.replaceText);
    frp.matchCase = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_MATCHCASE));
    frp.wholeWord = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_WHOLEWORD));
    frp.regex = static_cast<BOOL>(IsDlgButtonChecked(IDC_B_REGEX));

    CWaitCursor cursor;
    GetTopLevelParent().SendMessage(WM_FIND_REPLACE_COMMAND, 0, reinterpret_cast<LPARAM>(&frp));
}

void FindReplaceDlg::OnClose()
{
    Destroy();
}

void FindReplaceDlg::OnDestroy()
{
    auto* pLoop = _Module.GetMessageLoop();
    if (pLoop != nullptr) {
        pLoop->RemoveIdleHandler(this);
    }
}
