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

void IndexDlg::onStart(std::size_t totalEntries)
{
    m_progress.SetRange32(0, static_cast<int>(totalEntries));
    m_progress.SetPos(0);
    m_progress.SetStep(1);
    m_progress.SetState(PBST_NORMAL);
}

void IndexDlg::onFinished(std::size_t entries)
{
    m_state = IDLE;

    int min, max;
    m_progress.GetRange(min, max);
    m_progress.SetPos(max);
    m_progress.SetState(PBST_NORMAL);
}

void IndexDlg::onCancel()
{
    m_state = CANCELLED;
    m_progress.SetPos(std::numeric_limits<int>::max());
    m_progress.SetState(PBST_ERROR);
    m_indexButton.EnableWindow(TRUE);
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

void IndexDlg::onFileIndexing(std::size_t index, const std::string& filename)
{
    m_progress.SetPos(static_cast<int>(index));
    m_progress.SetState(PBST_NORMAL);
    CString message;

    constexpr auto COMPACT_LEN = 40;

    char szShort[MAX_PATH];
    PathCompactPathExA(szShort, filename.c_str(), COMPACT_LEN, 0);

    message.Format(_T("Indexing %s..."), StringHelper::fromUTF8(szShort).GetString());

    SetDlgItemText(IDC_INDEX_STATUS, message);

    PumpMessages();
}

bool IndexDlg::isCancelled()
{
    return m_state == CANCELING;
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

    Index(pakFile, indexPath);
}

void IndexDlg::Index(const CString& pakFile, const CString& indexPath)
{
    m_state = INDEXING;

    m_progress.SetState(PBST_NORMAL);

    auto utf8PakFile = StringHelper::toUTF8(pakFile);
    auto utf8IndexPath = StringHelper::toUTF8(indexPath);

    auto overwrite = m_overwriteCheckbox.GetCheck() == BST_CHECKED;

    Indexer indexer;
    indexer.setProgressListener(this);

    m_indexButton.EnableWindow(FALSE);

    try {
        indexer.index(utf8PakFile.GetString(), utf8IndexPath.GetString(), overwrite);
        indexer.compact();

        if (m_state == CANCELLED) {
            AtlMessageBox(*this, _T("Indexing cancelled."), nullptr, MB_ICONWARNING);
        } else {
            AtlMessageBox(*this, _T("Indexing completed successfully."), nullptr, MB_ICONINFORMATION);
        }
    } catch (const Xapian::Error& e) {
        m_state = IDLE;
        CString errorMessage;
        errorMessage.Format(_T("Error: %s\nContext: %s\nType: %s\nError String: %s"),
                            StringHelper::fromUTF8(e.get_msg().c_str()).GetString(),
                            StringHelper::fromUTF8(e.get_context().c_str()).GetString(),
                            StringHelper::fromUTF8(e.get_type()).GetString(),
                            StringHelper::fromUTF8(e.get_error_string()).GetString());
        MessageBox(errorMessage, _T("Index Error"), MB_OK | MB_ICONERROR);
    } catch (const std::exception& e) {
        m_state = IDLE;
        CString errorMessage;
        errorMessage.Format(_T("Error: %s"), StringHelper::fromUTF8(e.what()).GetString());
        MessageBox(errorMessage, _T("Index Error"), MB_OK | MB_ICONERROR);
    } catch (...) {
        m_state = IDLE;
        MessageBox(_T("An unknown error occurred."), _T("Index Error"), MB_OK | MB_ICONERROR);
    }

    m_indexButton.EnableWindow(TRUE);
}
