#pragma once
#include "NewProjectWizard.h"
#include "resources/resource.h"

class NewProjectFolderPage : public CWizard97InteriorPageImpl<NewProjectFolderPage>
{
public:
    using BasePage = CWizard97InteriorPageImpl;

    explicit NewProjectFolderPage(NewProjectWizard* pWiz, _U_STRINGorID title = nullptr);

    enum { IDD = IDD_NEW_PROJECT_FOLDER };

    BEGIN_MSG_MAP(NewProjectFolderPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER3(IDC_B_NEW_PROJECT_BROWSE, OnBrowse)

        CHAIN_MSG_MAP(BasePage)
    END_MSG_MAP()

    // Overrides
    int OnSetActive();
    int OnWizardBack();
    int OnWizardNext();

private:
    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
    void OnBrowse();

    CEdit m_folderPath;
    CEdit m_modName;
    CEdit m_author;
    NewProjectWizard* m_pWiz;
};
