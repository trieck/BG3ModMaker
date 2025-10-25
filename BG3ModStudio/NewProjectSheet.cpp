#include "stdafx.h"
#include "NewProjectSheet.h"
#include "resources/resource.h"

NewProjectSheet::NewProjectSheet(NewProjectWizard* pWiz)
    : BaseSheet(_T("New Project Wizard"), IDB_PAK_HEADER, IDB_NEW_PROJECT_LARGE, 0, nullptr),
      m_welcomePage(pWiz), m_folderPage(pWiz), m_buildPage(pWiz), m_pWiz(pWiz)
{
    AddPage(m_welcomePage);
    AddPage(m_folderPage);
    AddPage(m_buildPage);
}
