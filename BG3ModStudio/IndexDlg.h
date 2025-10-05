#pragma once

#include "Indexer.h"
#include "ModelessDialog.h"
#include "resources/resource.h"

#define WM_SET_STATE            (WM_APP + 1)
#define WM_GET_OVERWRITE_CHECK  (WM_APP + 2)
#define WM_GET_PAK_PATH         (WM_APP + 3)
#define WM_GET_INDEX_PATH       (WM_APP + 4)
#define WM_SET_PROGRESS         (WM_APP + 5)
#define WM_INDEXING_STARTED     (WM_APP + 6)
#define WM_INDEXING_FINISHED    (WM_APP + 7)
#define WM_SET_STATUS_MESSAGE   (WM_APP + 8)

class IndexDlg : public ModelessDialog<IndexDlg>
{
public:
    enum { IDD = IDD_INDEX };

    BEGIN_MSG_MAP(IndexDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnCancelRequested)
        COMMAND_ID_HANDLER3(IDC_B_PAKFILE, OnPakFile)
        COMMAND_ID_HANDLER3(IDC_B_INDEXPATH, OnIndexPath)
        COMMAND_ID_HANDLER3(IDOK, OnIndex)
        COMMAND_ID_HANDLER3(IDCANCEL, OnCancelRequested)
        MESSAGE_HANDLER2(WM_SET_STATE, OnSetState)
        MESSAGE_HANDLER4(WM_GET_OVERWRITE_CHECK, OnGetOverwriteCheck)
        MESSAGE_HANDLER2(WM_GET_PAK_PATH, OnGetPakPath)
        MESSAGE_HANDLER2(WM_GET_INDEX_PATH, OnGetIndexPath)
        MESSAGE_HANDLER2(WM_INDEXING_STARTED, OnIndexingStarted)
        MESSAGE_HANDLER3(WM_INDEXING_FINISHED, OnIndexingFinished)
        MESSAGE_HANDLER2(WM_SET_PROGRESS, OnSetProgress)
        MESSAGE_HANDLER2(WM_SET_STATUS_MESSAGE, OnSetStatusMessage)
    END_MSG_MAP()

private:
    enum State
    {
        IDLE,
        INDEXING,
        CANCELING,
        CANCELLED,
        FAILED
    };

    BOOL OnInitDialog(HWND, LPARAM);
    void OnCancelRequested();
    void OnIndex();
    void OnIndexPath();
    void OnPakFile();
    void OnGetPakPath(WPARAM, LPARAM);
    void OnGetIndexPath(WPARAM, LPARAM);
    void OnSetState(WPARAM, LPARAM);
    LRESULT OnGetOverwriteCheck();
    void OnIndexingStarted(WPARAM, LPARAM);
    void OnIndexingFinished();
    void OnSetProgress(WPARAM, LPARAM);
    void OnSetStatusMessage(WPARAM, LPARAM);

    static DWORD IndexProc(LPVOID pv);

    std::atomic<State> m_state = IDLE;
    CEdit m_pakFile;
    CEdit m_indexPath;
    CButton m_indexButton;
    CButton m_overwriteCheckbox;
    CProgressBarCtrl m_progress;
    CString m_gamePath;
    CString m_lastError;
};
