#include "stdafx.h"
#include "NewProjectSheet.h"
#include "NewProjectWizard.h"

INT_PTR NewProjectWizard::DoModal(HWND hWndParent)
{
    NewProjectSheet sheet(this);
    return sheet.DoModal(hWndParent);
}

void NewProjectWizard::SetProjectFolder(const CString& folder)
{
    m_projectFolder = folder;
}

const CString& NewProjectWizard::GetProjectFolder() const
{
    return m_projectFolder;
}

void NewProjectWizard::SetModName(const CString& modName)
{
    m_modName = modName;
}

const CString& NewProjectWizard::GetModName() const
{
    return m_modName;
}

void NewProjectWizard::SetAuthor(const CString& author)
{
    m_author = author;
}

const CString& NewProjectWizard::GetAuthor() const
{
    return m_author;
}

void NewProjectWizard::SetOpenNewProject(BOOL bOpenNewProject)
{
    m_openNewProject = bOpenNewProject;
}

BOOL NewProjectWizard::GetOpenNewProject() const
{
    return m_openNewProject;
}
