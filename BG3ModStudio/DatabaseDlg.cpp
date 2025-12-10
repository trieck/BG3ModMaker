#include "stdafx.h"
#include "Cataloger.h"
#include "DatabaseDlg.h"
#include "FileDialogEx.h"
#include "StringHelper.h"
#include "Util.h"

#include <filesystem>

#include "Iconizer.h"
namespace fs = std::filesystem;

BOOL DatabaseDlg::OnInitDialog(HWND, LPARAM)
{
    m_pakFile = GetDlgItem(IDC_E_PAKFILE);
    ATLASSERT(m_pakFile.IsWindow());

    m_dbPath = GetDlgItem(IDC_E_DBPATH);
    ATLASSERT(m_dbPath.IsWindow());

    m_gamePath = m_settings.GetString(_T("Settings"), _T("GamePath"), _T(""));

    m_progress = GetDlgItem(IDC_PROGRESS_DB);
    ATLASSERT(m_progress.IsWindow());

    (void)SetWindowTheme(m_progress, L"", L"");

    m_progress.SetBarColor(RGB(0, 51, 128)); // Deep ocean blue
    m_progress.SetBkColor(GetSysColor(COLOR_APPWORKSPACE));

    m_generate = GetDlgItem(IDOK);
    ATLASSERT(m_generate.IsWindow());

    m_overwrite = GetDlgItem(IDC_CHK_OVERWRITE);
    ATLASSERT(m_overwrite.IsWindow());

    CheckRadioButton(IDC_RADIO_GAMEOBJECT, IDC_RADIO_ICON, IDC_RADIO_GAMEOBJECT);

    OnGameObject();

    auto icon = Util::LoadBitmapAsIcon(ID_TOOL_DB, 32, 32);
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}

LRESULT DatabaseDlg::OnGetOverwriteCheck()
{
    return m_overwrite.GetCheck();
}

LRESULT DatabaseDlg::OnIsGameObject()
{
    return IsDlgButtonChecked(IDC_RADIO_GAMEOBJECT) == BST_CHECKED;
}

void DatabaseDlg::OnCancelRequested()
{
    if (m_state == IDLE || m_state == FAILED || m_state == CANCELLED) {
        Destroy();
        return;
    }

    m_state = CANCELING;

    AtlMessageBox(*this, _T("Database generation is in progress. Please wait for it to be cancelled."), nullptr,
                  MB_ICONWARNING);
}

void DatabaseDlg::OnDBFinished()
{
    KillTimer(1);

    if (m_state == CANCELLED) {
        AtlMessageBox(*this, _T("Cancelled."), nullptr, MB_ICONWARNING);
    } else if (m_state == FAILED) {
        if (m_lastError.IsEmpty()) {
            m_lastError = _T("Failed due to an unknown error.");
        }
        AtlMessageBox(*this, m_lastError.GetString(), _T("Error"), MB_ICONERROR);
        m_lastError.Empty();
    } else {
        m_progress.SetPos(std::numeric_limits<int>::max());
        CString time = StringHelper::fromUTF8(m_timer.str().c_str());
        CString msg;
        msg.Format(_T("Completed successfully in %s."), time);
        AtlMessageBox(*this, msg.GetString(), _T("Completed"), MB_ICONINFORMATION);
        OnSetState(IDLE, 0);
    }
}

void DatabaseDlg::OnDBPath()
{
    FileDialogEx dlg(FileDialogEx::Folder, *this);
    auto hr = dlg.Construct();
    if (FAILED(hr)) {
        return;
    }

    if (dlg.DoModal() != IDOK) {
        return;
    }

    const auto& paths = dlg.paths();
    if (paths.empty()) {
        return;
    }

    auto path = paths.front();

    m_dbPath.SetWindowText(path);
}

void DatabaseDlg::OnGenerate()
{
    CString pakFile;
    m_pakFile.GetWindowText(pakFile);
    if (pakFile.IsEmpty()) {
        MessageBox(_T("Please select a pak file."), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    if (!PathFileExists(pakFile)) {
        MessageBox(_T("The selected pak file does not exist."), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    CString dbPath;
    m_dbPath.GetWindowText(dbPath);
    if (dbPath.IsEmpty()) {
        MessageBox(_T("Please select a database path."), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    auto hThread = CreateThread(nullptr, 0, DBProc, this, 0, nullptr);
    if (hThread == nullptr) {
        ATLTRACE(_T("Unable to create worker thread.\n"));
        return;
    }

    CloseHandle(hThread);
}

void DatabaseDlg::OnGetPakPath(WPARAM wParam, LPARAM lParam)
{
    m_pakFile.GetWindowText(reinterpret_cast<LPTSTR>(lParam), static_cast<int>(wParam));
}

void DatabaseDlg::OnGetDBPath(WPARAM wParam, LPARAM lParam)
{
    m_dbPath.GetWindowText(reinterpret_cast<LPTSTR>(lParam), static_cast<int>(wParam));
}

void DatabaseDlg::OnPakFile()
{
    FileDialogEx dlg(FileDialogEx::Open, m_hWnd, _T("pak"), nullptr, OFN_HIDEREADONLY,
                     _T("Pak Files (*.pak)\0*.pak\0All Files (*.*)\0*.*\0"));
    auto hr = dlg.Construct();
    if (FAILED(hr)) {
        return;
    }

    auto gameDataPath = fs::path(m_gamePath.GetString()) / "Data";
    if (!exists(gameDataPath)) {
        gameDataPath = fs::current_path();
    }

    hr = dlg.SetFolder(gameDataPath.c_str());
    if (FAILED(hr)) {
        return;
    }

    if (dlg.DoModal() != IDOK) {
        return;
    }

    const auto& paths = dlg.paths();
    if (paths.empty()) {
        return;
    }

    auto filename = paths.front();

    m_pakFile.SetWindowText(filename);
}

void DatabaseDlg::OnSetState(WPARAM state, LPARAM)
{
    m_state = static_cast<State>(state);

    switch (m_state) {
    case IDLE:
        m_progressCur = 0;
        m_progress.SetState(PBST_NORMAL);
        m_generate.EnableWindow(TRUE);
        SetDlgItemText(IDC_INDEX_STATUS, _T(""));
        break;
    case GENERATING:
        m_progressCur = 0;
        m_progress.SetState(PBST_NORMAL);
        m_generate.EnableWindow(FALSE);
        break;
    case CANCELING:
        SetDlgItemText(IDC_INDEX_STATUS, _T("Canceling..."));
        m_progress.SetState(PBST_PAUSED);
        m_generate.EnableWindow(FALSE);
        break;
    case CANCELLED:
        SetDlgItemText(IDC_INDEX_STATUS, _T("Cancelled"));
        m_progressCur = std::numeric_limits<size_t>::max();
        m_progress.SetPos(std::numeric_limits<int>::max());
        m_progress.SetState(PBST_ERROR);
        m_generate.EnableWindow(TRUE);
        OnDBFinished();
        break;
    case FAILED:
        SetDlgItemText(IDC_INDEX_STATUS, _T("Complete"));
        m_progressCur = std::numeric_limits<size_t>::max();
        m_progress.SetPos(std::numeric_limits<int>::max());
        m_progress.SetState(PBST_ERROR);
        m_generate.EnableWindow(TRUE);
        OnDBFinished();
        break;
    }
}

void DatabaseDlg::OnTimer(UINT_PTR)
{
    auto cur = m_progressCur.load();
    auto total = m_progressTotal.load();

    if (total > 0) {
        m_progress.SetRange32(0, static_cast<int>(total));
        m_progress.SetPos(static_cast<int>(cur));
    }

    if (auto msg = m_status.get()) {
        SetDlgItemText(IDC_DB_STATUS, msg->GetString());
    }
}

DWORD DatabaseDlg::DBProc(LPVOID pv)
{
    auto* pThis = static_cast<DatabaseDlg*>(pv);
    ATLASSERT(pThis);

    TCHAR pakPath[MAX_PATH]{};
    TCHAR dbPath[MAX_PATH]{};

    pThis->SendMessage(WM_GET_PAK_PATH, _countof(pakPath), reinterpret_cast<LPARAM>(pakPath));
    pThis->SendMessage(WM_GET_DB_PATH, _countof(dbPath), reinterpret_cast<LPARAM>(dbPath));

    pThis->PostMessage(WM_SET_STATE, GENERATING, 0);

    auto overwrite = pThis->SendMessage(WM_GET_OVERWRITE_CHECK, 0, 0) == BST_CHECKED;
    auto isGameObject = pThis->SendMessage(WM_IS_GAME_OBJECT, 0, 0) != 0;

    struct DBListener : IFileProgressListener
    {
        DatabaseDlg* m_pDlg;

        explicit DBListener(DatabaseDlg* pDlg) : m_pDlg(pDlg)
        {
        }

        void onStart(std::size_t totalEntries) override
        {
            m_pDlg->m_progressTotal = totalEntries;
            m_pDlg->KillTimer(1);
            m_pDlg->SetTimer(1, 100, nullptr); // 10Hz UI update
        }

        void onFinished(std::size_t entries) override
        {
            m_pDlg->PostMessage(WM_DB_FINISHED, entries, 0);
        }

        void onFile(std::size_t index, const std::string& filename) override
        {
            m_pDlg->m_progressCur.store(index, std::memory_order_relaxed);

            constexpr auto COMPACT_LEN = 40;

            char szShort[MAX_PATH];
            PathCompactPathExA(szShort, filename.c_str(), COMPACT_LEN, 0);

            CString msg;
            msg.Format(_T("Processing %s..."), StringHelper::fromUTF8(szShort).GetString());

            m_pDlg->m_status.set(msg);
        }

        bool isCancelled() override
        {
            return m_pDlg->m_state == CANCELING;
        }

        void onCancel() override
        {
            m_pDlg->PostMessage(WM_SET_STATE, CANCELLED);
        }
    };

    DBListener listener(pThis);

    auto utf8PakPath = StringHelper::toUTF8(pakPath);
    auto utf8dbPath = StringHelper::toUTF8(dbPath);

    pThis->m_timer.restart();

    try {
        if (isGameObject) {
            Cataloger cataloger;
            cataloger.setProgressListener(&listener);
            cataloger.catalog(utf8PakPath, utf8dbPath, overwrite);
        } else {
            auto iconizer = Iconizer::create();
            iconizer->setProgressListener(&listener);
            iconizer->iconize(utf8PakPath, utf8dbPath, overwrite);
        }
    } catch (const std::exception& e) {
        pThis->m_lastError.Format(_T("Error: %s"), StringHelper::fromUTF8(e.what()).GetString());
        pThis->PostMessage(WM_SET_STATE, FAILED);
    } catch (...) {
        pThis->m_lastError = _T("An unknown error occurred.");
        pThis->PostMessage(WM_SET_STATE, FAILED);
    }

    return 0;
}

void DatabaseDlg::OnGameObject()
{
    auto path = m_settings.GetString(_T("Settings"), _T("GameObjectPath"));
    m_dbPath.SetWindowText(path);
}

void DatabaseDlg::OnIcon()
{
    auto path = m_settings.GetString(_T("Settings"), _T("IconPath"));
    m_dbPath.SetWindowText(path);
}
