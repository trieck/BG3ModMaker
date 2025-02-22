#pragma once

#include "TextFileView.h"

class FilesView : public CTabViewImpl<FilesView>
{
public:
    BEGIN_MSG_MAP(ComDetailView)
        MSG_WM_DRAWITEM(OnDrawItem)
        REFLECT_NOTIFY_CODE(TBVN_PAGEACTIVATED)
        CHAIN_MSG_MAP(CTabViewImpl)
        ALT_MSG_MAP(1) // tab control
    END_MSG_MAP()

    DECLARE_WND_CLASS_EX(_T("FilesView"), 0, COLOR_APPWORKSPACE)

    bool CreateTabControl();
    void UpdateLayout();
    LRESULT OnDrawItem(int nID, LPDRAWITEMSTRUCT pdis) const;

    BOOL ActivateFile(const CString& path, void* data);
    IFileView::Ptr ActiveFile() const;

    PVOID GetData(int index) const;
    PVOID CloseFile(int index);
    PVOID CloseOtherFiles(int index);
    void CloseAllFiles();
    PVOID CloseActiveFile();
    FileEncoding FileEncoding(int index) const;

private:
    std::unordered_map<PVOID, INT> m_data; // map file data to page indexes
    std::vector<IFileView::Ptr> m_views; // file views
};
