#include "stdafx.h"
#include "NewProjectWelcomePage.h"

NewProjectWelcomePage::NewProjectWelcomePage(NewProjectWizard* pWiz, _U_STRINGorID title) : BasePage(title),
    m_pWiz(pWiz)
{
}

int NewProjectWelcomePage::OnSetActive()
{
    SetWizardButtons(PSWIZB_NEXT);
    return 0;
}

int NewProjectWelcomePage::OnWizardNext()
{
    return 0;
}

BOOL NewProjectWelcomePage::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    CFontHandle titleFont(GetExteriorPageTitleFont());
    ATLASSERT(titleFont);

    auto title = GetDlgItem(IDC_NEW_PROJECT_TITLE);
    ATLASSERT(title);

    title.SetFont(titleFont);

    return TRUE; // let the system set the focus
}
