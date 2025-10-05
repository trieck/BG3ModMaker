#include "stdafx.h"
#include "IndexDlg.h"
#include "FileDialogEx.h"
#include "Indexer.h"
#include "Settings.h"
#include "StringHelper.h"

#include <filesystem>
namespace fs = std::filesystem;

BOOL IndexDlg::OnInitDialog(HWND, LPARAM)
{
    m_pakFile = GetDlgItem(IDC_E_PAKFILE);
    ATLASSERT(m_pakFile.IsWindow());

    m_indexPath = GetDlgItem(IDC_E_INDEXPATH);
    ATLASSERT(m_indexPath.IsWindow());

    Settings settings;
    auto indexPath = settings.GetString(_T("Settings"), _T("IndexPath"), _T(""));
    m_indexPath.SetWindowText(indexPath);

    m_gamePath = settings.GetString(_T("Settings"), _T("GamePath"), _T(""));

    m_progress = GetDlgItem(IDC_PROGRESS_INDEX);
    ATLASSERT(m_progress.IsWindow());

    (void)SetWindowTheme(m_progress, L"", L"");

    m_progress.SetBarColor(RGB(252, 129, 2)); // Fanta orange
    m_progress.SetBkColor(GetSysColor(COLOR_APPWORKSPACE));

    m_indexButton = GetDlgItem(IDOK);
    ATLASSERT(m_indexButton.IsWindow());

    m_overwriteCheckbox = GetDlgItem(IDC_CHK_OVERWRITE);
    ATLASSERT(m_overwriteCheckbox.IsWindow());

    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}

void IndexDlg::OnPakFile()
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

void IndexDlg::OnGetPakPath(WPARAM wParam, LPARAM lParam)
{
    ::GetWindowText(m_pakFile.m_hWnd, reinterpret_cast<LPTSTR>(lParam), static_cast<int>(wParam));
}

void IndexDlg::OnGetIndexPath(WPARAM wParam, LPARAM lParam)
{
    ::GetWindowText(m_indexPath.m_hWnd, reinterpret_cast<LPTSTR>(lParam), static_cast<int>(wParam));
}

void IndexDlg::OnSetState(WPARAM state, LPARAM)
{
    m_state = static_cast<State>(state);
    switch (m_state) {
    case IDLE:
        m_progress.SetPos(0);
        m_progress.SetState(PBST_NORMAL);
        m_indexButton.EnableWindow(TRUE);
        break;
    case INDEXING:
        m_progress.SetPos(0);
        m_progress.SetState(PBST_NORMAL);
        m_indexButton.EnableWindow(FALSE);
        break;
    case CANCELING:
        m_progress.SetState(PBST_PAUSED);
        m_indexButton.EnableWindow(FALSE);
        break;
    case CANCELLED:
    case FAILED:
        m_progress.SetPos(std::numeric_limits<int>::max());
        m_progress.SetState(PBST_ERROR);
        m_indexButton.EnableWindow(TRUE);
        OnIndexingFinished();
        break;
    }
}

LRESULT IndexDlg::OnGetOverwriteCheck()
{
    return m_overwriteCheckbox.GetCheck();
}

void IndexDlg::OnIndexingStarted(WPARAM wParam, LPARAM)
{
    m_progress.SetRange32(0, static_cast<int>(wParam));
    m_progress.SetPos(0);
    m_progress.SetStep(1);
    m_progress.SetState(PBST_NORMAL);
}

void IndexDlg::OnIndexingFinished()
{
    if (m_state == CANCELLED) {
        AtlMessageBox(*this, _T("Indexing cancelled."), nullptr, MB_ICONWARNING);
    } else if (m_state == FAILED) {
        if (m_lastError.IsEmpty()) {
            m_lastError = _T("Indexing failed due to an unknown error.");
        }
        AtlMessageBox(*this, m_lastError.GetString(), _T("Error"), MB_ICONERROR);
        m_lastError.Empty();
    } else {
        AtlMessageBox(*this, _T("Indexing completed successfully."), nullptr, MB_ICONINFORMATION);
        OnSetState(IDLE, 0);
    }
}

void IndexDlg::OnSetProgress(WPARAM wParam, LPARAM lParam)
{
    m_progress.SetPos(static_cast<int>(wParam));
    m_progress.SetState(static_cast<int>(lParam));
}

void IndexDlg::OnSetStatusMessage(WPARAM, LPARAM lParam)
{
    std::unique_ptr<CString> text(reinterpret_cast<CString*>(lParam));
    SetDlgItemText(IDC_INDEX_STATUS, *text);
}

void IndexDlg::OnCancelRequested()
{
    if (m_state == IDLE || m_state == CANCELLED) {
        Destroy();
        return;
    }

    m_state = CANCELING;
    AtlMessageBox(*this, _T("Indexing is in progress. Please wait for it to be cancelled."), nullptr, MB_ICONWARNING);
}

void IndexDlg::OnIndexPath()
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

    m_indexPath.SetWindowText(path);
}

void IndexDlg::OnIndex()
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

    CString indexPath;
    m_indexPath.GetWindowText(indexPath);
    if (indexPath.IsEmpty()) {
        MessageBox(_T("Please select an index path."), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    auto hThread = CreateThread(nullptr, 0, IndexProc, this, 0, nullptr);
    if (hThread == nullptr) {
        ATLTRACE(_T("Unable to create worker thread.\n"));
        return;
    }

    CloseHandle(hThread);
}

DWORD IndexDlg::IndexProc(LPVOID pv)
{
    auto* pThis = static_cast<IndexDlg*>(pv);
    ATLASSERT(pThis);

    TCHAR pakPath[MAX_PATH]{};
    TCHAR indexPath[MAX_PATH]{};

    pThis->SendMessage(WM_GET_PAK_PATH, _countof(pakPath), reinterpret_cast<LPARAM>(pakPath));
    pThis->SendMessage(WM_GET_INDEX_PATH, _countof(indexPath), reinterpret_cast<LPARAM>(indexPath));

    pThis->PostMessage(WM_SET_STATE, INDEXING, 0);

    auto overwrite = pThis->SendMessage(WM_GET_OVERWRITE_CHECK, 0, 0) == BST_CHECKED;

    struct IndexListener : IIndexProgressListener
    {
        IndexDlg* m_pDlg;

        explicit IndexListener(IndexDlg* pDlg) : m_pDlg(pDlg)
        {
        }

        void onStart(std::size_t totalEntries) override
        {
            m_pDlg->PostMessage(WM_INDEXING_STARTED, totalEntries, 0);
        }

        void onFinished(std::size_t entries) override
        {
            m_pDlg->PostMessage(WM_INDEXING_FINISHED, entries, 0);
        }

        void onFileIndexing(std::size_t index, const std::string& filename) override
        {
            m_pDlg->PostMessage(WM_SET_PROGRESS, index, PBST_NORMAL);

            constexpr auto COMPACT_LEN = 40;

            char szShort[MAX_PATH];
            PathCompactPathExA(szShort, filename.c_str(), COMPACT_LEN, 0);

            auto* pMessage = new CString;
            pMessage->Format(_T("Indexing %s..."), StringHelper::fromUTF8(szShort).GetString());

            m_pDlg->PostMessage(WM_SET_STATUS_MESSAGE, 0, reinterpret_cast<LPARAM>(pMessage));
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

    IndexListener listener(pThis);

    Indexer indexer;
    indexer.setProgressListener(&listener);

    auto utf8PakPath = StringHelper::toUTF8(pakPath);
    auto utf8IndexPath = StringHelper::toUTF8(indexPath);

    try {
        indexer.index(utf8PakPath, utf8IndexPath, overwrite);
        indexer.compact();
    } catch (const Xapian::Error& e) {
        pThis->m_lastError.Format(_T("Error: %s\nContext: %s\nType: %s\nError String: %s"),
                                  StringHelper::fromUTF8(e.get_msg().c_str()).GetString(),
                                  StringHelper::fromUTF8(e.get_context().c_str()).GetString(),
                                  StringHelper::fromUTF8(e.get_type()).GetString(),
                                  StringHelper::fromUTF8(e.get_error_string()).GetString());
        pThis->PostMessage(WM_SET_STATE, FAILED);
    } catch (const std::exception& e) {
        pThis->m_lastError.Format(_T("Error: %s"), StringHelper::fromUTF8(e.what()).GetString());
        pThis->PostMessage(WM_SET_STATE, FAILED);
    } catch (...) {
        pThis->m_lastError = _T("An unknown error occurred.");
        pThis->PostMessage(WM_SET_STATE, FAILED);
    }

    return 0;
}
