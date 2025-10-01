#pragma once

#include "PAKWizard.h"
#include "resources/resource.h"

class PAKWizFilePage : public CWizard97InteriorPageImpl<PAKWizFilePage>
{
public:
    using BasePage = CWizard97InteriorPageImpl;

    explicit PAKWizFilePage(PAKWizard* pWiz, _U_STRINGorID title = nullptr);

    enum { IDD = IDD_PAKWIZ_FILE };

    int OnSetActive();
    int OnWizardBack();
    int OnWizardNext();
    BOOL OnQueryCancel();

private:
    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);

    BEGIN_MSG_MAP(PAKWizFilePage)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_ID_HANDLER3(IDC_B_PAK_BUILDER_BROWSE, OnBrowse)
        CHAIN_MSG_MAP(BasePage)
    END_MSG_MAP()

    void OnBrowse();
    CString GetModsPath() const;

    PAKWizard* m_pWiz;
    CEdit m_filePath;
};
