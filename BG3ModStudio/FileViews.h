#pragma once

#include "TextFileView.h"
#include "TabViewImpl.h"

class FilesView : public TabViewImpl<FilesView>
{
public:
    BEGIN_MSG_MAP(FilesView)
        REFLECT_NOTIFY_CODE(TBVN_PAGEACTIVATED)
        CHAIN_MSG_MAP(TabViewImpl)
        ALT_MSG_MAP(ALT_MSG_MAP_TABCTRL) // tab control
    END_MSG_MAP()

    DECLARE_WND_CLASS_EX(_T("FilesView"), 0, COLOR_APPWORKSPACE)

    void UpdateLayout();

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
