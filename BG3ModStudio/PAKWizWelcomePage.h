#pragma once

#include "PAKWizard.h"
#include "resources/resource.h"

class PAKWizWelcomePage : public CWizard97ExteriorPageImpl<PAKWizWelcomePage>
{
public:
    using BasePage = CWizard97ExteriorPageImpl;

    explicit PAKWizWelcomePage(PAKWizard* pWiz, _U_STRINGorID title = nullptr);

    enum { IDD = IDD_PAKWIZ_WELCOME };

    // Overrides
    int OnSetActive();
    int OnWizardNext();

    BEGIN_MSG_MAP(PAKWizWelcomePage)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(BasePage)
    END_MSG_MAP()

private:
    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);

    PAKWizard* m_pWiz;
    CButton m_generateLoca;
    CButton m_generateLSF;
};
