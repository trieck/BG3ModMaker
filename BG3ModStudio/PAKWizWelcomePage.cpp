#include "stdafx.h"
#include "PAKWizWelcomePage.h"

PAKWizWelcomePage::PAKWizWelcomePage(PAKWizard* pWiz, _U_STRINGorID title) : BasePage(title), m_pWiz(pWiz)
{
}

int PAKWizWelcomePage::OnSetActive()
{
    SetWizardButtons(PSWIZB_NEXT);

    return 0;
}

int PAKWizWelcomePage::OnWizardNext()
{
    m_pWiz->SetGenerateLoca(m_generateLoca.GetCheck() == BST_CHECKED);
    m_pWiz->SetGeneateLSF(m_generateLSF.GetCheck() == BST_CHECKED);

    return 0;
}

BOOL PAKWizWelcomePage::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    CFontHandle titleFont(GetExteriorPageTitleFont());
    ATLASSERT(titleFont);

    auto title = GetDlgItem(IDC_PAKWIZ_TITLE);
    ATLASSERT(title);

    title.SetFont(titleFont);

    m_generateLoca = GetDlgItem(IDC_CHK_GENERATE_LOCA);
    ATLASSERT(m_generateLoca.IsWindow());
    m_generateLoca.SetCheck(TRUE);

    m_generateLSF = GetDlgItem(IDC_CHK_GENERATE_LSF);
    ATLASSERT(m_generateLSF.IsWindow());
    m_generateLSF.SetCheck(TRUE);

    return TRUE; // let the system set the focus
}
