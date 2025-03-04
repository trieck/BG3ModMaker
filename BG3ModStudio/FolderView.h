#pragma once

enum TreeItemType {
    TIT_UNKNOWN = 0,
    TIT_FOLDER = 1,
    TIT_FILE = 2
};

typedef struct TreeItemData {
    TreeItemType type;
    CString path;
} TREEITEMDATA, *LPTREEITEMDATA;

class FolderView : public CWindowImpl<FolderView, CTreeViewCtrlEx>
{
public:
    DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())

    BEGIN_MSG_MAP_EX(FolderView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnItemExpanding)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDelete)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    FolderView() = default;
    LRESULT OnCreate(LPCREATESTRUCT /*pcs*/);
    void OnDestroy();
    LRESULT OnItemExpanding(LPNMHDR pnmh);
    LRESULT OnDelete(LPNMHDR pnmh);

    void SetFolder(const CString& folder);
    void ExpandFolders(const CTreeItem& folder);
    HTREEITEM AddFile(const CString& filename);
    HTREEITEM FindFile(const CString& filename);
    HTREEITEM RenameFile(const CString& oldname, const CString& newname);
    CString GetRootPath() const;

private:
    HTREEITEM InsertFile(HTREEITEM hRoot, const CString& filename, std::deque<CString>& components);
    HTREEITEM InsertSubpath(HTREEITEM hRoot, const CString& subpath, const CString& component);
    HTREEITEM FindFile(HTREEITEM hRoot, std::deque<CString>& components);
    HTREEITEM FindSubpath(HTREEITEM hRoot, const CString& subpath);

    CImageList m_ImageList;
};
