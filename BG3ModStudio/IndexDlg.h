#pragma once

#include "ModelessDialog.h"
#include "resources/resource.h"
#include "ThreadSafeLatest.h"
#include "Timer.h"

class IndexDlg : public ModelessDialog<IndexDlg>
{
public:
    enum { IDD = IDD_INDEX };

    static constexpr UINT WM_SET_STATE = WM_APP + 1;
    static constexpr UINT WM_GET_OVERWRITE_CHECK = WM_APP + 2;
    static constexpr UINT WM_GET_PAK_PATH = WM_APP + 3;
    static constexpr UINT WM_GET_INDEX_PATH = WM_APP + 4;
    static constexpr UINT WM_INDEXING_FINISHED = WM_APP + 5;

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
        MESSAGE_HANDLER3(WM_INDEXING_FINISHED, OnIndexingFinished)
        MSG_WM_TIMER(OnTimer)
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
    LRESULT OnGetOverwriteCheck();
    void OnCancelRequested();
    void OnGetIndexPath(WPARAM, LPARAM);
    void OnGetPakPath(WPARAM, LPARAM);
    void OnIndex();
    void OnIndexingFinished();
    void OnIndexPath();
    void OnPakFile();
    void OnSetState(WPARAM, LPARAM);
    void OnTimer(UINT_PTR);

    static DWORD IndexProc(LPVOID pv);

    std::atomic<State> m_state = IDLE;
    std::atomic<size_t> m_progressCur{0};
    std::atomic<size_t> m_progressTotal{0};
    ThreadSafeLatest<CString> m_status;

    CEdit m_pakFile;
    CEdit m_indexPath;
    CButton m_indexButton;
    CButton m_overwriteCheckbox;
    CProgressBarCtrl m_progress;
    CString m_gamePath;
    CString m_lastError;
    Timer m_timer;
};
