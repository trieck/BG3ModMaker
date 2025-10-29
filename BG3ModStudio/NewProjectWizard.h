#pragma once

class NewProjectWizard
{
public:
    NewProjectWizard() = default;
    ~NewProjectWizard() = default;

    INT_PTR DoModal(HWND hWndParent = GetActiveWindow());

    void SetProjectFolder(const CString& folder);
    const CString& GetProjectFolder() const;

    void SetModName(const CString& modName);
    const CString& GetModName() const;

    void SetAuthor(const CString& author);
    const CString& GetAuthor() const;

    void SetOpenNewProject(BOOL bOpenNewProject);
    BOOL GetOpenNewProject() const;

private:
    CString m_projectFolder;
    CString m_modName;
    CString m_author;
    BOOL m_openNewProject = TRUE;
};
