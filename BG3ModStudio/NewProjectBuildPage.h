#pragma once

#include "NewProjectWizard.h"
#include "ResourceHelper.h"
#include "resources/resource.h"

#define WM_PROJ_RANGE (WM_APP + 1)
#define WM_PROJ_PROGRESS (WM_APP + 2)
#define WM_PROJ_COMPLETE (WM_APP + 3)

class NewProjectBuildPage : public CWizard97InteriorPageImpl<NewProjectBuildPage>
{
public:
    using BasePage = CWizard97InteriorPageImpl;

    explicit NewProjectBuildPage(NewProjectWizard* pWiz, _U_STRINGorID title = nullptr);

    enum { IDD = IDD_NEW_PROJECT_BUILD };

    BEGIN_MSG_MAP(NewProjectBuildPage)
        MSG_WM_INITDIALOG(OnInitDialog)
        MESSAGE_HANDLER(WM_PROJ_RANGE, OnProjRange)
        MESSAGE_HANDLER(WM_PROJ_PROGRESS, OnProjProgress)
        MESSAGE_HANDLER(WM_PROJ_COMPLETE, OnProjComplete)
        CHAIN_MSG_MAP(BasePage)
    END_MSG_MAP()

    // Overrides
    int OnSetActive();
    int OnWizardBack();
    int OnWizardNext();
    int OnWizardFinish();
    BOOL OnQueryCancel();

private:
    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
    LRESULT OnProjRange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnProjProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnProjComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BOOL CreateFolder(const CString& folder);
    BOOL CreateFileEx(const CString& path);
    BOOL WriteFileEx(const CString& path, const CStringA& contents);
    BOOL WriteTemplate(UINT nResID, const ResourceHelper::ResourceMap& map, const CString& path);
    static DWORD WINAPI BuildProc(LPVOID pv);

    NewProjectWizard* m_pWiz;
    CProgressBarCtrl m_progress;
    CButton m_openNewProject;
    CString m_lastError;
};
