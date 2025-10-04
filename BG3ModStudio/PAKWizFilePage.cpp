#include "stdafx.h"
#include "FileDialogEx.h"
#include "PAKWizFilePage.h"

#include <filesystem>
namespace fs = std::filesystem;

PAKWizFilePage::PAKWizFilePage(PAKWizard* pWiz, _U_STRINGorID title) : BasePage(title), m_pWiz(pWiz)
{
    SetHeaderTitle(_T("BG3 PAK Builder"));
    SetHeaderSubTitle(_T(""));
}

BOOL PAKWizFilePage::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    m_filePath = GetDlgItem(IDC_E_PAK_BUILDER);
    ATLASSERT(m_filePath.IsWindow());

    const auto& root = m_pWiz->GetRoot();
    auto rootSuffix = fs::path(root.GetString()).filename();

    auto gameModPath = GetModsPath();
    auto fullPath = fs::path(gameModPath.GetString()) / rootSuffix;
    fullPath.replace_extension(".pak");

    m_filePath.SetWindowText(fullPath.wstring().c_str());

    return TRUE; // let the system set the focus
}

void PAKWizFilePage::OnBrowse()
{
    auto filter = L"PAK Files (*.pak)\0*.pak\0"
        L"All Files(*.*)\0*.*\0\0";

    TCHAR szPath[MAX_PATH];
    m_filePath.GetWindowText(szPath, MAX_PATH);

    FileDialogEx dlg(FileDialogEx::Save, *this, nullptr, szPath, 0, filter);
    auto hr = dlg.Construct();
    if (FAILED(hr)) {
        return;
    }

    auto modsPath = GetModsPath();
    if (modsPath.IsEmpty() || !PathFileExists(modsPath)) {
        modsPath = fs::current_path().wstring().c_str();
    }

    hr = dlg.SetFolder(modsPath);
    if (FAILED(hr)) {
        return;
    }

    if (dlg.DoModal() != IDOK) {
        return;
    }

    m_filePath.SetWindowText(dlg.paths().front());
}

CString PAKWizFilePage::GetModsPath() const
{
    WCHAR localAppDataPath[MAX_PATH]{};

    // Get %LOCALAPPDATA%
    auto hr = SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppDataPath);
    if (FAILED(hr)) {
        return "";
    }

    std::filesystem::path modsPath(localAppDataPath);

    modsPath /= L"Larian Studios";
    modsPath /= L"Baldur's Gate 3";
    modsPath /= L"Mods";

    return modsPath.wstring().c_str();
}

int PAKWizFilePage::OnSetActive()
{
    SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    return 0;
}

int PAKWizFilePage::OnWizardBack()
{
    return 0;
}

int PAKWizFilePage::OnWizardNext()
{
    TCHAR szPath[MAX_PATH];
    m_filePath.GetWindowText(szPath, MAX_PATH);

    CString pakPath(szPath);

    if (pakPath.IsEmpty()) {
        AtlMessageBox(*this, _T("Please specify a PAK file path."), _T("Error"), MB_ICONERROR);
        return -1; // stay on this page
    }

    TCHAR drive[_MAX_DRIVE];
    TCHAR dir[_MAX_DIR];
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];
    _tsplitpath_s(pakPath, drive, dir, fname, ext);

    if (GetDriveType(drive) == DRIVE_NO_ROOT_DIR) {
        AtlMessageBox(*this, _T("The specified drive is not available."), _T("Error"), MB_ICONERROR);
        return -1;
    }

    CString dirPath;
    dirPath.Format(_T("%s%s"), drive, dir);
    if (!PathFileExists(dirPath)) {
        AtlMessageBox(*this, _T("The specified folder does not exist."), _T("Error"), MB_ICONERROR);
        return -1;
    }

    HANDLE hFile = CreateFile(pakPath,
                              GENERIC_WRITE,
                              0,
                              nullptr,
                              OPEN_ALWAYS, // create if not exists
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        AtlMessageBox(*this, _T("You do not have permission to write to this location."),
                      _T("Error"), MB_ICONERROR);
        return -1;
    }

    CloseHandle(hFile);
    DeleteFile(pakPath);

    m_pWiz->SetPAKFile(pakPath);

    return 0;
}

BOOL PAKWizFilePage::OnQueryCancel()
{
    return FALSE;
}
