#include "stdafx.h"
#include "Exception.h"
#include "OsiFileView.h"
#include "OsiReader.h"
#include "StringHelper.h"
#include "resources/resource.h"

typedef struct OsiTreeNodeData
{
    OsiViewType viewType;
    const void* pdata;
} OSITREENODEDATA, *LPOSITREENODEDATA;

OsiFileView::OsiFileView() : m_funcTree(kMaxNodes), m_dbTree(kMaxNodes)
{
}

LRESULT OsiFileView::OnCreate(LPCREATESTRUCT pcs)
{
    CRect rc{0, 0, pcs->cx, pcs->cy};

    if (!m_splitter.Create(m_hWnd, rc, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)) {
        return -1;
    }
    ATLASSERT(m_splitter.IsWindow());

    if (!m_tree.Create(m_splitter, rcDefault, nullptr,
                       WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                       TVS_HASBUTTONS | TVS_HASLINES | TVS_FULLROWSELECT | TVS_INFOTIP
                       | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE)) {
        return -1;
    }

    ATLASSERT(m_tree.IsWindow());

    if (!m_formView.Create(m_splitter, rcDefault, nullptr,
                           WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE)) {
        return -1;
    }

    m_splitter.SetSplitterPane(0, m_tree);
    m_splitter.SetSplitterPane(1, m_formView);
    m_splitter.SetSplitterPosPct(40);

    static constexpr auto icons = {
        IDI_GOAL,
        IDI_FUNCTION,
        IDI_FOLDER_SMALL,
        IDI_TYPES,
        IDI_DATABASE,
        IDI_ENUM
    };

    m_imageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, static_cast<uint32_t>(icons.size()), 0);
    ATLASSERT(m_imageList);

    for (auto icon : icons) {
        auto hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(icon));
        ATLASSERT(hIcon);
        m_imageList.AddIcon(hIcon);
    }

    m_tree.SetImageList(m_imageList, TVSIL_NORMAL);

    return 0;
}

void OsiFileView::OnSize(UINT nType, CSize size)
{
    if (m_splitter.IsWindow()) {
        m_splitter.SetWindowPos(nullptr, 0, 0, size.cx, size.cy, SWP_NOZORDER);
    }
}

LRESULT OsiFileView::OnItemExpanding(LPNMHDR pnmh)
{
    if (pnmh->hwndFrom != m_tree.m_hWnd) {
        return 0;
    }

    const auto item = MAKE_TREEITEM(pnmh, &m_tree);

    Expand(item);

    return 0;
}

LRESULT OsiFileView::OnDelete(LPNMHDR pnmh)
{
    const auto item = MAKE_OLDTREEITEM(pnmh, &m_tree);

    auto data = std::bit_cast<LPOSITREENODEDATA>(item.GetData());

    delete data;

    return 0;
}

LRESULT OsiFileView::OnSelChanged(LPNMHDR pnmh)
{
    CWaitCursor cursor;

    const auto item = MAKE_TREEITEM(pnmh, &m_tree);

    auto* data = reinterpret_cast<LPOSITREENODEDATA>(item.GetData());
    if (data == nullptr || data->pdata == nullptr) {
        m_formView.DestroyView();
        return 0;
    }

    auto* pData = data->pdata;
    switch (data->viewType) {
    case OVT_DATABASE:
        pData = FindDatabase(data);
        break;
    case OVT_FUNCTION:
        pData = FindFunction(data);
        break;
    default:
        break;
    }

    if (pData == nullptr) {
        m_formView.DestroyView();
        return 0;
    }

    OsiData osiData{.pStory = &m_story, .pdata = pData};
    m_formView.LoadView(data->viewType, &osiData);

    return 0;
}

void OsiFileView::OnContextMenu(const CWindow& wnd, const CPoint& point)
{
}

const OsiDatabase* OsiFileView::FindDatabase(void* data) const
{
    auto* pNodeData = static_cast<LPOSITREENODEDATA>(data);
    if (pNodeData == nullptr) {
        return nullptr;
    }

    auto* pNode = static_cast<const SBNode*>(pNodeData->pdata);
    if (pNode == nullptr) {
        return nullptr;
    }

    if (pNode->next != nullptr) { // not a leaf node
        return nullptr;
    }

    auto* pDb = static_cast<const OsiDatabase*>(pNode->value);

    return pDb;
}

const OsiFunction* OsiFileView::FindFunction(void* data) const
{
    auto* pNodeData = static_cast<LPOSITREENODEDATA>(data);
    if (pNodeData == nullptr) {
        return nullptr;
    }

    auto* pNode = static_cast<const SBNode*>(pNodeData->pdata);
    if (pNode == nullptr) {
        return nullptr;
    }

    if (pNode->next != nullptr) { // not a leaf node
        return nullptr;
    }

    auto* pFunc = static_cast<const OsiFunction*>(pNode->value);

    return pFunc;
}

BOOL OsiFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE;

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);
    if (!hWnd) {
        return FALSE;
    }

    return TRUE;
}

BOOL OsiFileView::LoadFile(const CString& path)
{
    auto utf8Path = StringHelper::toUTF8(path);

    try {
        FileStream stream;
        stream.open(utf8Path, "rb");

        OsiReader reader;
        reader.read(stream);
        m_story = std::move(reader).takeStory();
    } catch (const Exception& e) {
        ATLTRACE("Failed to load Osi file: %s\n", e.what());
        return FALSE;
    }

    Populate();

    return TRUE;
}

BOOL OsiFileView::LoadBuffer(const CString& path, const ByteBuffer& buffer)
{
    try {
        OsiReader reader;
        reader.read(buffer);
        m_story = std::move(reader).takeStory();
    } catch (const Exception& e) {
        ATLTRACE("Failed to load Osi buffer: %s\n", e.what());
        return FALSE;
    }

    Populate();

    return TRUE;
}

BOOL OsiFileView::SaveFile()
{
    return TRUE;
}

BOOL OsiFileView::SaveFileAs(const CString& path)
{
    return TRUE;
}

BOOL OsiFileView::Destroy()
{
    return DestroyWindow();
}

BOOL OsiFileView::IsDirty() const
{
    return FALSE;
}

BOOL OsiFileView::IsEditable() const
{
    return FALSE;
}

BOOL OsiFileView::IsText() const
{
    return FALSE;
}

const CString& OsiFileView::GetPath() const
{
    return m_path;
}

void OsiFileView::SetPath(const CString& path)
{
    m_path = path;
}

FileEncoding OsiFileView::GetEncoding() const
{
    return UNKNOWN;
}

OsiFileView::operator HWND() const
{
    return m_hWnd;
}

void OsiFileView::Populate()
{
    m_tree.DeleteAllItems();
    PopulateDatabases();
    PopulateEnums();
    PopulateFunctions();
    PopulateGoals();
    PopulateTypes();
}

void OsiFileView::PopulateDatabases()
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;

    tvis.itemex.cChildren = m_story.goals.empty() ? 0 : 1;
    tvis.itemex.pszText = const_cast<LPWSTR>(L"Databases");
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{.viewType = OVT_DATABASE, .pdata = nullptr});
    tvis.itemex.iImage = 4;
    tvis.itemex.iSelectedImage = 4;
    tvis.itemex.iExpandedImage = 4;

    m_tree.InsertItem(&tvis);
}

void OsiFileView::PopulateEnums()
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;

    tvis.itemex.cChildren = m_story.goals.empty() ? 0 : 1;
    tvis.itemex.pszText = const_cast<LPWSTR>(L"Enums");
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{.viewType = OVT_ENUM, .pdata = nullptr});
    tvis.itemex.iImage = 5;
    tvis.itemex.iSelectedImage = 5;
    tvis.itemex.iExpandedImage = 5;

    m_tree.InsertItem(&tvis);
}

void OsiFileView::PopulateFunctions()
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;

    tvis.itemex.cChildren = m_story.goals.empty() ? 0 : 1;
    tvis.itemex.pszText = const_cast<LPWSTR>(L"Functions");
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{.viewType = OVT_FUNCTION, .pdata = nullptr});
    tvis.itemex.iImage = 1;
    tvis.itemex.iSelectedImage = 1;
    tvis.itemex.iExpandedImage = 1;

    m_tree.InsertItem(&tvis);
}

void OsiFileView::PopulateGoals()
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;

    tvis.itemex.cChildren = m_story.goals.empty() ? 0 : 1;
    tvis.itemex.pszText = const_cast<LPWSTR>(L"Goals");
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{.viewType = OVT_GOAL, .pdata = nullptr});
    tvis.itemex.iImage = 0;
    tvis.itemex.iSelectedImage = 0;
    tvis.itemex.iExpandedImage = 0;
    m_tree.InsertItem(&tvis);
}

void OsiFileView::PopulateTypes()
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;

    tvis.itemex.cChildren = m_story.types.empty() ? 0 : 1;
    tvis.itemex.pszText = const_cast<LPWSTR>(L"Types");
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{.viewType = OVT_TYPE, .pdata = nullptr});
    tvis.itemex.iImage = 3;
    tvis.itemex.iSelectedImage = 3;
    tvis.itemex.iExpandedImage = 3;
    m_tree.InsertItem(&tvis);
}

void OsiFileView::Expand(const CTreeItem& item)
{
    CWaitCursor cursor;
    auto child = item.GetChild();

    if (m_tree.GetItemState(child, TVIS_EXPANDED)) {
        if (!child) {
            TVITEMEX newItem{};
            newItem.mask = TVIF_CHILDREN;
            newItem.hItem = child;
            newItem.cChildren = 0;
            m_tree.SetItem(&newItem);
            return;
        }

        return; // already expanded
    }

    if (child) {
        return; // already populated
    }

    auto data = reinterpret_cast<LPOSITREENODEDATA>(m_tree.GetItemData(item.m_hTreeItem));
    if (data == nullptr) {
        return;
    }

    if (data->viewType == OVT_DATABASE) {
        ExpandDatabase(item, static_cast<const SBNode*>(data->pdata));
    } else if (data->viewType == OVT_ENUM) {
        ExpandEnum(item, static_cast<const OsiEnum*>(data->pdata));
    } else if (data->viewType == OVT_FUNCTION) {
        ExpandFunction(item, static_cast<const SBNode*>(data->pdata));
    } else if (data->viewType == OVT_GOAL) {
        ExpandGoal(item, static_cast<const OsiGoal*>(data->pdata));
    } else if (data->viewType == OVT_TYPE) {
        ExpandType(item);
    }
}

void OsiFileView::ExpandDatabase(const CTreeItem& item, const SBNode* pDatabase)
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = item.m_hTreeItem;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;
    tvis.itemex.iImage = 2;
    tvis.itemex.iSelectedImage = 2;
    tvis.itemex.iExpandedImage = 2;

    if (pDatabase == nullptr) {
        m_dbTree.clear();

        for (const auto& db : m_story.databases) {
            std::string name = "(Unnamed)";
            if (db.ownerNode != INVALID_REF) {
                const auto& owner = m_story.nodes[db.ownerNode];
                if (!owner->name.empty()) {
                    name = std::format("{}({})", owner->name, owner->numParams);
                } else {
                    name = std::format("<{}>", owner->typeName());
                }
            }
            name += std::format(" #{}", db.index);
            m_dbTree.insert(name, &db);
        }

        const auto* root = m_dbTree.root();
        for (const auto& node : root->nodes) {
            tvis.itemex.cChildren = node.next ? 1 : 0;
            auto label = MakeNodeLabel(node);
            tvis.itemex.pszText = const_cast<LPWSTR>(label.GetString());
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
                .viewType = OVT_DATABASE, .pdata = &node
            });
            m_tree.InsertItem(&tvis);
        }
    } else {
        for (const auto& node : pDatabase->next->nodes) {
            if (!node.next) {
                tvis.itemex.iImage = 4;
                tvis.itemex.iSelectedImage = 4;
                tvis.itemex.iExpandedImage = 4;
            }
            tvis.itemex.cChildren = node.next ? 1 : 0;
            auto label = MakeNodeLabel(node);
            tvis.itemex.pszText = const_cast<LPWSTR>(label.GetString());
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
                .viewType = OVT_DATABASE, .pdata = &node
            });
            m_tree.InsertItem(&tvis);
        }
    }
}

void OsiFileView::ExpandEnum(const CTreeItem& item, const OsiEnum* pEnum)
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = item.m_hTreeItem;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;
    tvis.itemex.iImage = 5;
    tvis.itemex.iSelectedImage = 5;
    tvis.itemex.iExpandedImage = 5;

    if (pEnum == nullptr) {
        for (const auto& e : m_story.enums | std::views::values) {
            tvis.itemex.cChildren = 1;
            auto typeName = m_story.typeName(e.type);
            auto wTypeName = StringHelper::fromUTF8(typeName.c_str());
            tvis.itemex.pszText = const_cast<LPWSTR>(wTypeName.GetString());
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
                .viewType = OVT_ENUM, .pdata = &e
            });
            m_tree.InsertItem(&tvis);
        }
    } else {
        for (const auto& entry : pEnum->elements) {
            const auto& name = entry.first;
            auto wName = StringHelper::fromUTF8(name.c_str());

            CString label;
            label.Format(L"%s = %llu", wName.GetString(), entry.second);
            tvis.itemex.cChildren = 0;
            tvis.itemex.pszText = const_cast<LPWSTR>(label.GetString());
            tvis.itemex.lParam = 0;
            m_tree.InsertItem(&tvis);
        }
    }

    m_tree.SortChildren(item.m_hTreeItem);
}

void OsiFileView::ExpandFunction(const CTreeItem& item, const SBNode* pFunc)
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = item.m_hTreeItem;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;
    tvis.itemex.iImage = 2;
    tvis.itemex.iSelectedImage = 2;
    tvis.itemex.iExpandedImage = 2;

    if (pFunc == nullptr) {
        m_funcTree.clear();
        for (const auto& function : m_story.functions) {
            const auto& name = function.name.name;
            m_funcTree.insert(name, &function);
        }

        const auto* root = m_funcTree.root();
        for (const auto& node : root->nodes) {
            tvis.itemex.cChildren = node.next ? 1 : 0;
            auto label = MakeNodeLabel(node);
            tvis.itemex.pszText = const_cast<LPWSTR>(label.GetString());
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
                .viewType = OVT_FUNCTION, .pdata = &node
            });
            m_tree.InsertItem(&tvis);
        }
    } else {
        for (const auto& node : pFunc->next->nodes) {
            if (!node.next) {
                tvis.itemex.iImage = 1;
                tvis.itemex.iSelectedImage = 1;
                tvis.itemex.iExpandedImage = 1;
            }
            tvis.itemex.cChildren = node.next ? 1 : 0;
            auto label = MakeNodeLabel(node);
            tvis.itemex.pszText = const_cast<LPWSTR>(label.GetString());
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
                .viewType = OVT_FUNCTION, .pdata = &node
            });
            m_tree.InsertItem(&tvis);
        }
    }
}

void OsiFileView::ExpandGoal(const CTreeItem& item, const OsiGoal* pGoal)
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = item.m_hTreeItem;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;
    tvis.itemex.iImage = 0;
    tvis.itemex.iSelectedImage = 0;
    tvis.itemex.iExpandedImage = 0;

    if (pGoal == nullptr) {
        for (const auto& goal : m_story.goals) {
            tvis.itemex.cChildren = goal.subGoals.empty() ? 0 : 1;
            auto goalName = StringHelper::fromUTF8(goal.name.c_str());
            tvis.itemex.pszText = const_cast<LPWSTR>(goalName.GetString());
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
                .viewType = OVT_GOAL, .pdata = &goal
            });
            m_tree.InsertItem(&tvis);
        }
    } else {
        for (auto subGoalId : pGoal->subGoals) {
            const auto& subGoal = m_story.goals[subGoalId];

            tvis.itemex.cChildren = subGoal.subGoals.empty() ? 0 : 1;
            auto goalName = StringHelper::fromUTF8(subGoal.name.c_str());
            tvis.itemex.pszText = const_cast<LPWSTR>(goalName.GetString());
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
                .viewType = OVT_GOAL, .pdata = &subGoal
            });
            m_tree.InsertItem(&tvis);
        }
    }
}

void OsiFileView::ExpandType(const CTreeItem& item)
{
    TV_INSERTSTRUCT tvis{};
    tvis.hParent = item.m_hTreeItem;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;
    tvis.itemex.iImage = 3;
    tvis.itemex.iSelectedImage = 3;
    tvis.itemex.iExpandedImage = 3;

    for (const auto& type : m_story.types | std::views::values) {
        tvis.itemex.cChildren = 0;
        auto typeName = StringHelper::fromUTF8(type.name.c_str());
        tvis.itemex.pszText = const_cast<LPWSTR>(typeName.GetString());
        tvis.itemex.lParam = std::bit_cast<LPARAM>(new OsiTreeNodeData{
            .viewType = OVT_TYPE, .pdata = &type
        });
        m_tree.InsertItem(&tvis);
    }

    m_tree.SortChildren(item.m_hTreeItem);
}

CString OsiFileView::FindMinKey(const SBPage* pPage) const
{
    while (pPage->type == SBPage::PageType::Internal) {
        // leftmost descent
        const auto& n = pPage->nodes.front();
        pPage = n.next.get();
    }

    // leaf
    return pPage->nodes.front().key.c_str();
}

CString OsiFileView::FindMaxKey(const SBPage* pPage) const
{
    while (pPage->type == SBPage::PageType::Internal) {
        // rightmost descent
        const auto& n = pPage->nodes.back();
        pPage = n.next.get();
    }

    // leaf
    return pPage->nodes.back().key.c_str();
}

CString OsiFileView::MakeNodeLabel(const SBNode& node) const
{
    CString label;

    if (!node.next) {
        label = node.key.c_str();
        return label;
    }

    // Non-leaf: compute true descendant range
    const auto* child = node.next.get();

    CString minKey = FindMinKey(child);
    auto count = CountLeafItems(child);

    label.Format(L"%s [%llu]...", minKey.GetString(), count);

    return label;
}

size_t OsiFileView::CountLeafItems(const SBPage* page) const
{
    if (!page) {
        return 0;
    }

    if (page->type == SBPage::PageType::Leaf) {
        return page->nodes.size();
    }

    size_t count = 0u;
    for (const auto& node : page->nodes) {
        count += CountLeafItems(node.next.get());
    }

    return count;
}
