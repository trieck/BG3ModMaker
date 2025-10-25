#pragma once

#include "PAKWizard.h"
#include "PAKWizBuildPage.h"
#include "PAKWizFilePage.h"
#include "PAKWizWelcomePage.h"

class PAKWizSheet : public CWizard97SheetImpl<PAKWizSheet>
{
public:
    using BaseSheet = CWizard97SheetImpl;

    explicit PAKWizSheet(PAKWizard* pWiz);

private:
    PAKWizWelcomePage m_welcomePage;
    PAKWizFilePage m_filePage;
    PAKWizBuildPage m_buildPage;
    PAKWizard* m_pWiz;
};
