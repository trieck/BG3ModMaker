#pragma once

#include "NewProjectWizard.h"
#include "resources/resource.h"

class NewProjectWelcomePage : public CWizard97ExteriorPageImpl<NewProjectWelcomePage>
{
public:
    using BasePage = CWizard97ExteriorPageImpl;

    explicit NewProjectWelcomePage(NewProjectWizard* pWiz, _U_STRINGorID title = nullptr);

    enum { IDD = IDD_NEW_PROJECT };

    // Overrides
    int OnSetActive();
    int OnWizardNext();

    BEGIN_MSG_MAP(NewProjectWelcomePage)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(BasePage)
    END_MSG_MAP()

private:
    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);

    NewProjectWizard* m_pWiz;
};
