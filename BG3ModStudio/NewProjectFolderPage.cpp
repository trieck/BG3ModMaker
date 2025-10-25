#include "stdafx.h"
#include "FileDialogEx.h"
#include "NewProjectFolderPage.h"

#define SECURITY_WIN32
#include <Security.h>
#include <Lmcons.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace { // anonymous

BOOL NonEmptyFolderExists(const CString& folder)
{
    if (folder.IsEmpty()) {
        return FALSE;
    }

    fs::path folderPath(folder.GetString());
    if (!exists(folderPath) || !is_directory(folderPath)) {
        return FALSE;
    }

    return fs::directory_iterator(folderPath) != fs::directory_iterator{};
}

BOOL CanonicalizeFolder(CString& folder)
{
    if (PathIsRelative(folder)) { // must be absolute
        return FALSE;
    }

    WCHAR canon[MAX_PATH];
    auto result = PathCanonicalize(canon, folder.GetString());
    if (!result) {
        return FALSE;
    }

    if (PathIsRoot(canon)) { // cannot be root
        return FALSE;
    }

    folder = canon;

    return TRUE;
}
} // anonymous namespace

NewProjectFolderPage::NewProjectFolderPage(NewProjectWizard* pWiz, _U_STRINGorID title) : BasePage(title), m_pWiz(pWiz)
{
    SetHeaderTitle(_T("New Project"));
    SetHeaderSubTitle(_T(""));
}

BOOL NewProjectFolderPage::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    m_folderPath = GetDlgItem(IDC_E_NEW_FOLDER);
    ATLASSERT(m_folderPath.IsWindow());

    m_modName = GetDlgItem(IDC_E_MOD_NAME);
    ATLASSERT(m_modName.IsWindow());

    m_author = GetDlgItem(IDC_E_AUTHOR_NAME);
    ATLASSERT(m_author.IsWindow());

    WCHAR user[UNLEN + 1]{};
    DWORD size = _countof(user);

    if (!GetUserNameEx(NameDisplay, user, &size)) {
        GetUserName(user, &size);
    }

    m_author.SetWindowText(user);

    return TRUE; // let the system set the focus
}

int NewProjectFolderPage::OnSetActive()
{
    SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    return 0;
}

int NewProjectFolderPage::OnWizardBack()
{
    return 0;
}

int NewProjectFolderPage::OnWizardNext()
{
    CString folder;
    m_folderPath.GetWindowText(folder);

    if (folder.IsEmpty()) {
        AtlMessageBox(*this, _T("Please specify a project directory."), _T("Error"), MB_ICONERROR);
        m_folderPath.SetFocus();
        return -1; // stay on this page
    }

    if (!CanonicalizeFolder(folder)) {
        AtlMessageBox(*this, _T("The specified directory path is invalid."), _T("Error"), MB_ICONERROR);
        return -1;
    }

    if (NonEmptyFolderExists(folder)) {
        CString msg;
        msg.Format(
            _T("The folder \"%s\" already exists and contains files.\n\n")
            _T("Continuing may overwrite existing data.\n\n")
            _T("Do you want to continue?"),
            folder.GetString());

        auto result = AtlMessageBox(*this, msg.GetString(), _T("Warning"), MB_ICONWARNING | MB_YESNO);
        if (result != IDYES) {
            m_folderPath.SetFocus();
            return -1; // stay on this page
        }
    }

    CString modName;
    m_modName.GetWindowText(modName);

    if (modName.IsEmpty()) {
        m_modName.SetFocus();
        AtlMessageBox(*this, _T("Please specify a mod name."), _T("Error"), MB_ICONERROR);
        return -1; // stay on this page
    }

    CString author;
    m_author.GetWindowText(author);

    m_pWiz->SetProjectFolder(folder);
    m_pWiz->SetModName(modName);
    m_pWiz->SetAuthor(author);

    return 0;
}

void NewProjectFolderPage::OnBrowse()
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

    auto folder = paths.front();
    m_folderPath.SetWindowText(folder);

    // Only set mod name if it's currently empty
    CString currentName;
    m_modName.GetWindowText(currentName);
    currentName.Trim();
    if (!currentName.IsEmpty()) {
        return;
    }

    std::filesystem::path p(folder.GetString());
    CString modName(p.filename().wstring().c_str());

    modName.Replace(L" ", L"_");

    m_modName.SetWindowText(modName);
}
