#pragma once
#include "PAKWizard.h"
#include "resources/resource.h"

#define WM_PAK_RANGE (WM_APP + 2)
#define WM_PAK_PROGRESS (WM_APP + 3)
#define WM_PAK_COMPLETE (WM_APP + 4)

class PAKWizBuildPage : public CWizard97InteriorPageImpl<PAKWizBuildPage>
{
public:
    using BasePage = CWizard97InteriorPageImpl;

    explicit PAKWizBuildPage(PAKWizard* pWiz, _U_STRINGorID title = nullptr);

    enum { IDD = IDD_PAKWIZ_BUILD };

    BEGIN_MSG_MAP(RegCleanRun)
        MSG_WM_INITDIALOG(OnInitDialog)
        MESSAGE_HANDLER(WM_PAK_RANGE, OnPAKRange)
        MESSAGE_HANDLER(WM_PAK_PROGRESS, OnPAKProgress)
        MESSAGE_HANDLER(WM_PAK_COMPLETE, OnPAKComplete)
        CHAIN_MSG_MAP(BasePage)
    END_MSG_MAP()

    int OnSetActive();
    int OnWizardBack();
    int OnWizardNext();
    BOOL OnQueryCancel();

private:
    LRESULT OnPAKRange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPAKProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPAKComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
    static DWORD WINAPI BuildProc(LPVOID pv);

    PAKWizard* m_pWiz;
    CProgressBarCtrl m_progress;
    CString m_lastError;
};
