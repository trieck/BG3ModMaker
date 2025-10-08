#pragma once

#include "IFileView.h"
#include "Resource.h"

class LSFFileView : public CWindowImpl<LSFFileView>, public IFileView
{
public:
    using Base = CWindowImpl;

    LSFFileView();
    ~LSFFileView() override = default;

    BEGIN_MSG_MAP(IconView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnItemExpanding)
        NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDelete)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnSelChanged)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"LSFFileView", nullptr)

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    void OnSize(UINT nType, CSize size);
    LRESULT OnItemExpanding(LPNMHDR pnmh);
    LRESULT OnDelete(LPNMHDR pnmh);
    LRESULT OnSelChanged(LPNMHDR pnmh);

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL LoadFile(const CString& path) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    const CString& GetPath() const override;
    VOID SetPath(const CString& path) override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;

private:
    void Populate();
    void Expand(const CTreeItem& item);
    void ExpandResource(const CTreeItem& item);
    void ExpandRegion(const CTreeItem& item, const Region& region);
    void ExpandNode(const CTreeItem& item, const LSNode& node);
    void AddAttributes(const LSNode& node);
    void AutoAdjustColumns();

    CImageList m_imageList;
    CTreeViewCtrlEx m_tree;
    CListViewCtrl m_list;
    CSplitterWindow m_splitter;
    Resource::Ptr m_resource;
    CString m_path;
};
