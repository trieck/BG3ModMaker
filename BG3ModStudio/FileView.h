#pragma once
#include "textstream.h"

class FileView : public CWindowImpl<FileView, CRichEditCtrl>
{
public:
    using Ptr = std::unique_ptr<FileView>;

    BEGIN_MSG_MAP(FileView)
        MSG_WM_CREATE(OnCreate)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(NULL, CRichEditCtrl::GetWndClassName())

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    void LoadFile(const CString& path);
    LPCTSTR GetPath() const;

private:
    BOOL Write(LPCWSTR text) const;
    BOOL Write(LPCWSTR text, size_t length) const;
    BOOL Write(LPCSTR text) const;
    BOOL Write(LPCSTR text, size_t length) const;

    BOOL Flush();

    CString m_path;
    CComObjectStack<TextStream> m_stream;
};

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

    void ActivateFile(const CString& path, void* data);
    PVOID GetData(int index) const;
    PVOID CloseFile(int index);
    PVOID CloseOtherFiles(int index);
    void CloseAllFiles();    
    PVOID CloseActiveFile();

private:
    std::unordered_map<PVOID, INT> m_data;  // map file data to page indexes
    std::vector<FileView::Ptr> m_views;     // file views
};

