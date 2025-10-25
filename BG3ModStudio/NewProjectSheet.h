#pragma once

#include "NewProjectBuildPage.h"
#include "NewProjectFolderPage.h"
#include "NewProjectWelcomePage.h"
#include "NewProjectWizard.h"

class NewProjectSheet : public CWizard97SheetImpl<NewProjectSheet>
{
public:
    using BaseSheet = CWizard97SheetImpl;

    explicit NewProjectSheet(NewProjectWizard* pWiz);

private:
    NewProjectWelcomePage m_welcomePage;
    NewProjectFolderPage m_folderPage;
    NewProjectBuildPage m_buildPage;
    NewProjectWizard* m_pWiz;
};
