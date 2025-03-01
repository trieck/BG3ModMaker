#pragma once

#include "TextFileView.h"
#include "TabViewImpl.h"

class FilesView : public TabViewImpl<FilesView>
{
public:
    using Base = TabViewImpl;

    BEGIN_MSG_MAP(FilesView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        REFLECT_NOTIFY_CODE(TBVN_PAGEACTIVATED)
        NOTIFY_CODE_HANDLER(TCN_SELCHANGE, OnTabSelChange)
        CHAIN_MSG_MAP(TabViewImpl)
    END_MSG_MAP()

    DECLARE_WND_CLASS_EX(_T("FilesView"), 0, COLOR_APPWORKSPACE)

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnTabSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    BOOL NewFile();
    BOOL ActivateFile(const CString& path, LPVOID data);
    IFileView::Ptr ActiveFile() const;
    int ActivePage() const;
    const std::vector<IFileView::Ptr>& Files() const;

    PVOID GetData(int index) const;
    PVOID CloseFile(int index);
    BOOL CloseFileByData(LPVOID data);
    PVOID CloseOtherFiles(int index);
    void CloseAllFiles();
    PVOID CloseActiveFile();
    FileEncoding FileEncoding(int index) const;
    BOOL IsDirty(int index) const;
    BOOL IsDirty() const;
    BOOL SaveFile(const IFileView::Ptr& fileView);
    BOOL SaveAll();
    CString GetTitle(const IFileView::Ptr& fileView) const;
    void SetTitle(const IFileView::Ptr& fileView, LPCTSTR lpstrTitle);
    IFileView::Ptr GetFileView(int index) const;
    IFileView::Ptr GetFileView(LPVOID data) const;
    IFileView::Ptr GetFileView(const CString& path) const;
    BOOL ActivateView(const IFileView::Ptr& fileView, LPVOID data);
    int GetViewIndex(const IFileView::Ptr& fileView) const;
    BOOL SetData(const CString& path, LPVOID data);
    BOOL RenameFile(const CString& oldname, const CString& newname);
private:
    BOOL AddPage(const IFileView::Ptr& fileView, LPCTSTR lpstrTitle, LPCTSTR lpstrPath, LPVOID pData = nullptr);    

    CFont m_tabFont;
    std::unordered_map<LPVOID, INT> m_data; // map file data to page indexes
    std::vector<IFileView::Ptr> m_views; // file views
    uint32_t m_nextId{ 0 };
};
