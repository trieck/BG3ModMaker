#pragma once

#include "TextFileView.h"
#include "TabViewImpl.h"

class FilesView : public TabViewImpl<FilesView>
{
public:
    BEGIN_MSG_MAP(FilesView)
        REFLECT_NOTIFY_CODE(TBVN_PAGEACTIVATED)
        NOTIFY_CODE_HANDLER(TCN_SELCHANGE, OnTabSelChange)
        CHAIN_MSG_MAP(TabViewImpl)
    END_MSG_MAP()

    DECLARE_WND_CLASS_EX(_T("FilesView"), 0, COLOR_APPWORKSPACE)

    LRESULT OnTabSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
    {
        ATLASSERT(idCtrl == TabViewCtrl::m_nTabID);
        ATLASSERT(pnmh->hwndFrom == m_tabViewCtrl.m_tabCtrl);
        ATLASSERT(pnmh->code == TCN_SELCHANGE);

        auto index = m_tabViewCtrl.GetActiveTab();
        if (index == -1) {
            return 0;
        }

        SetActivePage(index);
        OnPageActivated(index);
        UpdateLayout();

        bHandled = TRUE;

        return 0;
    }

    BOOL ActivateFile(const CString& path, void* data);
    IFileView::Ptr ActiveFile() const;
    int ActivePage() const;
    const std::vector<IFileView::Ptr>& Files() const;

    PVOID GetData(int index) const;
    PVOID CloseFile(int index);
    PVOID CloseOtherFiles(int index);
    void CloseAllFiles();
    PVOID CloseActiveFile();
    FileEncoding FileEncoding(int index) const;
    BOOL IsDirty(int index) const;
    BOOL IsDirty() const;

private:
    std::unordered_map<PVOID, INT> m_data; // map file data to page indexes
    std::vector<IFileView::Ptr> m_views; // file views
};
