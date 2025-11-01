#pragma once
#include "ModelessDialog.h"
#include "resources/resource.h"
#include "Settings.h"
#include "ThreadSafeLatest.h"
#include "Timer.h"

class DatabaseDlg : public ModelessDialog<DatabaseDlg>
{
public:
    enum { IDD = IDD_DB };

    static constexpr UINT WM_SET_STATE = WM_APP + 1;
    static constexpr UINT WM_GET_OVERWRITE_CHECK = WM_APP + 2;
    static constexpr UINT WM_GET_PAK_PATH = WM_APP + 3;
    static constexpr UINT WM_GET_DB_PATH = WM_APP + 4;
    static constexpr UINT WM_DB_FINISHED = WM_APP + 5;
    static constexpr UINT WM_IS_GAME_OBJECT = WM_APP + 6;

    BEGIN_MSG_MAP(DatabaseDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnCancelRequested)
        COMMAND_ID_HANDLER3(IDC_B_PAKFILE, OnPakFile)
        COMMAND_ID_HANDLER3(IDC_B_DBPATH, OnDBPath)
        COMMAND_ID_HANDLER3(IDOK, OnGenerate)
        COMMAND_ID_HANDLER3(IDCANCEL, OnCancelRequested)
        COMMAND_ID_HANDLER3(IDC_RADIO_GAMEOBJECT, OnGameObject)
        COMMAND_ID_HANDLER3(IDC_RADIO_ICON, OnIcon)
        MESSAGE_HANDLER2(WM_SET_STATE, OnSetState)
        MESSAGE_HANDLER4(WM_GET_OVERWRITE_CHECK, OnGetOverwriteCheck)
        MESSAGE_HANDLER2(WM_GET_PAK_PATH, OnGetPakPath)
        MESSAGE_HANDLER2(WM_GET_DB_PATH, OnGetDBPath)
        MESSAGE_HANDLER3(WM_DB_FINISHED, OnDBFinished)
        MESSAGE_HANDLER4(WM_IS_GAME_OBJECT, OnIsGameObject)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()

private:
    enum State
    {
        IDLE,
        GENERATING,
        CANCELING,
        CANCELLED,
        FAILED
    };

    BOOL OnInitDialog(HWND, LPARAM);
    LRESULT OnGetOverwriteCheck();
    LRESULT OnIsGameObject();
    void OnCancelRequested();
    void OnDBFinished();
    void OnDBPath();
    void OnGameObject();
    void OnGenerate();
    void OnGetDBPath(WPARAM, LPARAM);
    void OnGetPakPath(WPARAM, LPARAM);
    void OnIcon();
    void OnPakFile();
    void OnSetState(WPARAM, LPARAM);
    void OnTimer(UINT_PTR);

    static DWORD DBProc(LPVOID pv);

    std::atomic<State> m_state = IDLE;
    std::atomic<size_t> m_progressCur{0};
    std::atomic<size_t> m_progressTotal{0};
    ThreadSafeLatest<CString> m_status;

    CButton m_generate;
    CButton m_overwrite;
    CEdit m_dbPath;
    CEdit m_pakFile;
    CProgressBarCtrl m_progress;
    CString m_gamePath;
    CString m_lastError;
    Settings m_settings;
    Timer m_timer;
};
