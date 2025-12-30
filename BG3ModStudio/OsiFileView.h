#pragma once

#include "BTree.h"
#include "FormViewContainer.h"
#include "IFileView.h"
#include "OsiStory.h"
#include "OsiViewFactory.h"

class OsiFileView : public CWindowImpl<OsiFileView>, public IFileView
{
public:
    using Base = CWindowImpl;

    OsiFileView();
    ~OsiFileView() override = default;

    BEGIN_MSG_MAP(OsiFileView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnItemExpanding)
        NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDelete)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnSelChanged)
        MSG_WM_CONTEXTMENU(OnContextMenu)
    END_MSG_MAP()

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL LoadFile(const CString& path) override;
    BOOL LoadBuffer(const CString& path, const ByteBuffer& buffer) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    BOOL IsEditable() const override;
    BOOL IsText() const override;
    const CString& GetPath() const override;
    VOID SetPath(const CString& path) override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;

private:
    using SBTree = BTree<std::string, LPCVOID>;
    using SBPage = SBTree::Page;
    using SBNode = SBTree::Node;

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    void OnSize(UINT nType, CSize size);
    LRESULT OnItemExpanding(LPNMHDR pnmh);
    LRESULT OnDelete(LPNMHDR pnmh);
    LRESULT OnSelChanged(LPNMHDR pnmh);
    void OnContextMenu(const CWindow& wnd, const CPoint& point);

    CString FindMaxKey(const SBPage* pPage) const;
    CString FindMinKey(const SBPage* pPage) const;
    CString MakeNodeLabel(const SBNode& node) const;
    const OsiDatabase* FindDatabase(void* data) const;
    const OsiFunction* FindFunction(void* data) const;
    size_t CountLeafItems(const SBPage* page) const;
    void Expand(const CTreeItem& item);
    void ExpandDatabase(const CTreeItem& item, const SBNode* pDatabase);
    void ExpandEnum(const CTreeItem& item, const OsiEnum* pEnum);
    void ExpandFunction(const CTreeItem& item, const SBNode* pFunc);
    void ExpandGoal(const CTreeItem& item, const OsiGoal* pGoal);
    void ExpandType(const CTreeItem& item);
    void Populate();
    void PopulateDatabases();
    void PopulateEnums();
    void PopulateFunctions();
    void PopulateGoals();
    void PopulateTypes();

    OsiStory m_story;
    CSplitterWindow m_splitter;
    CTreeViewCtrlEx m_tree;
    FormViewContainer m_formView;

    CImageList m_imageList;
    CString m_path;
    OsiViewFactory m_viewFactory;
    SBTree m_funcTree;
    SBTree m_dbTree;

    static constexpr auto kMaxNodes = 64;
};
