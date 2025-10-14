#include "stdafx.h"
#include "Exception.h"
#include "FileStream.h"
#include "IconDlg.h"
#include "LSFFileView.h"
#include "LSFReader.h"
#include "resources/resource.h"
#include "StringHelper.h"
#include "Util.h"

enum NodeItemType
{
    NIT_UNKNOWN = 0,
    NIT_REGION = 1,
    NIT_NODE = 2,
};

typedef struct NodeItemData
{
    NodeItemType type;
    void* pdata;
} NODEITEMDATA, *LPNODEITEMDATA;

static constexpr auto COLUMN_PADDING = 12;

LSFFileView::LSFFileView()
{
}

LRESULT LSFFileView::OnCreate(LPCREATESTRUCT pcs)
{
    CRect rc{0, 0, pcs->cx, pcs->cy};

    m_splitter.Create(m_hWnd, rc, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    ATLASSERT(m_splitter.IsWindow());

    m_tree.Create(m_splitter, rcDefault, nullptr,
                  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                  TVS_HASBUTTONS | TVS_HASLINES | TVS_FULLROWSELECT | TVS_INFOTIP
                  | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
    ATLASSERT(m_tree.IsWindow());

    m_list.Create(m_splitter, rcDefault, nullptr,
                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
                  | LVS_REPORT | LVS_SINGLESEL, WS_EX_CLIENTEDGE);
    ATLASSERT(m_list.IsWindow());

    m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    auto font = AtlCreateControlFont();
    m_list.SetFont(font);

    m_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100);
    m_list.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 150);
    m_list.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 150);

    m_splitter.SetSplitterPane(0, m_tree);
    m_splitter.SetSplitterPane(1, m_list);
    m_splitter.SetSplitterPosPct(40);

    static constexpr auto icons = {
        IDI_REGION,
        IDI_NODE
    };

    CImageList m_ImageList;
    m_ImageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, static_cast<uint32_t>(icons.size()), 0);
    ATLASSERT(m_ImageList);

    for (auto icon : icons) {
        auto hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(icon));
        ATLASSERT(hIcon);
        m_ImageList.AddIcon(hIcon);
    }

    m_tree.SetImageList(m_ImageList, TVSIL_NORMAL);

    return 0;
}

void LSFFileView::OnSize(UINT nType, CSize size)
{
    if (m_splitter.IsWindow()) {
        m_splitter.SetWindowPos(nullptr, 0, 0, size.cx, size.cy, SWP_NOZORDER);
    }
}

LRESULT LSFFileView::OnItemExpanding(LPNMHDR pnmh)
{
    if (pnmh->hwndFrom != m_tree.m_hWnd) {
        return 0;
    }

    const auto item = MAKE_TREEITEM(pnmh, &m_tree);

    Expand(item);

    return 0;
}

LRESULT LSFFileView::OnDelete(LPNMHDR pnmh)
{
    const auto item = MAKE_OLDTREEITEM(pnmh, &m_tree);

    auto data = std::bit_cast<NodeItemData*>(item.GetData());

    delete data;

    return 0;
}

LRESULT LSFFileView::OnSelChanged(LPNMHDR /*pnmh*/)
{
    auto item = m_tree.GetSelectedItem();
    if (!item) {
        return 0;
    }

    m_list.DeleteAllItems();

    auto data = reinterpret_cast<NodeItemData*>(m_tree.GetItemData(item.m_hTreeItem));
    if (data == nullptr || data->type == NIT_UNKNOWN) {
        return 0;
    }

    if (data->type == NIT_REGION || data->type == NIT_NODE) {
        AddAttributes(*static_cast<LSNode*>(data->pdata));
    }

    return 0;
}

void LSFFileView::OnContextMenu(const CWindow& wnd, const CPoint& point)
{
    CRect rc;
    m_list.GetWindowRect(&rc);
    if (!rc.PtInRect(point)) {
        return; // Click was outside the list
    }

    CMenu menu;
    menu.LoadMenuW(IDR_ATTRIBUTE_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);
    auto cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, *this);
    if (cmd == 0) {
        return; // No command selected
    }

    auto selectedRow = m_list.GetSelectedIndex();
    if (selectedRow < 0) {
        return; // No item selected
    }

    CString text;
    switch (cmd) {
    case ID_ATTRIBUTE_COPYNAME: // Copy Name
        m_list.GetItemText(selectedRow, 0, text);
        break;
    case ID_ATTRIBUTE_COPYVALUE: // Copy Value
        m_list.GetItemText(selectedRow, 1, text);
        break;
    case ID_ATTRIBUTE_COPYTYPE: // Copy Type
        m_list.GetItemText(selectedRow, 2, text);
        break;
    default:
        return; // Unknown command
    }

    if (text.IsEmpty()) {
        return; // Nothing to copy
    }

    Util::CopyToClipboard(*this, text);
}

LRESULT LSFFileView::OnDoubleClick(LPNMHDR pnmh)
{
    if (pnmh->hwndFrom != m_list.m_hWnd) {
        return 0;
    }

    auto pia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
    if (!pia || pia->iItem < 0) {
        return 0;
    }

    CString id, value;
    m_list.GetItemText(pia->iItem, 1, value);
    if (value.IsEmpty()) {
        return 0;
    }

    CWaitCursor cursor;

    IconDlg dlg(value);
    if (!dlg.HasImage()) {
        return 0;
    }

    dlg.DoModal(*this);

    return 0;
}

BOOL LSFFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE;

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);
    if (!hWnd) {
        return FALSE;
    }

    return TRUE;
}

BOOL LSFFileView::LoadFile(const CString& path)
{
    auto utf8Path = StringHelper::toUTF8(path);

    FileStream file;

    try {
        file.open(utf8Path, "rb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return FALSE;
    }

    LSFReader reader;

    try {
        m_resource = reader.read(file);
        Populate();
    } catch (const Exception& e) {
        ATLTRACE("Failed to read LSF file: %s\n", e.what());
        return FALSE;
    }

    return TRUE;
}

BOOL LSFFileView::LoadBuffer(const CString& /*path*/, const ByteBuffer& buffer)
{
    LSFReader reader;

    try {
        m_resource = reader.read(buffer);
        Populate();
    } catch (const Exception& e) {
        ATLTRACE("Failed to read LSF buffer: %s\n", e.what());
        return FALSE;
    }

    return TRUE;
}

BOOL LSFFileView::SaveFile()
{
    return TRUE;
}

BOOL LSFFileView::SaveFileAs(const CString& path)
{
    return TRUE;
}

BOOL LSFFileView::Destroy()
{
    return DestroyWindow();
}

BOOL LSFFileView::IsDirty() const
{
    return FALSE;
}

const CString& LSFFileView::GetPath() const
{
    return m_path;
}

void LSFFileView::SetPath(const CString& path)
{
    m_path = path;
}

FileEncoding LSFFileView::GetEncoding() const
{
    return UNKNOWN;
}

LSFFileView::operator HWND() const
{
    return m_hWnd;
}

void LSFFileView::Populate()
{
    m_tree.DeleteAllItems();
    m_list.DeleteAllItems();

    if (!m_resource) {
        return;
    }

    TV_INSERTSTRUCT tvis{};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;

    for (const auto& region : m_resource->regions) {
        auto wideName = StringHelper::fromUTF8(region.first.c_str());
        tvis.itemex.cChildren = region.second->childCount() > 0 ? 1 : 0;
        tvis.itemex.pszText = const_cast<LPTSTR>(wideName.GetString());
        tvis.itemex.lParam = std::bit_cast<LPARAM>(new NodeItemData{.type = NIT_REGION, .pdata = region.second.get()});
        auto hItem = m_tree.InsertItem(&tvis);
        m_tree.Expand(hItem, TVE_EXPAND);
    }
}

void LSFFileView::Expand(const CTreeItem& item)
{
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

    auto data = reinterpret_cast<NodeItemData*>(m_tree.GetItemData(item.m_hTreeItem));
    if (data == nullptr || data->type == NIT_UNKNOWN) {
        return;
    }

    if (data->type == NIT_REGION) {
        ExpandRegion(item, *static_cast<Region*>(data->pdata));
    } else if (data->type == NIT_NODE) {
        ExpandNode(item, *static_cast<LSNode*>(data->pdata));
    }
}

void LSFFileView::ExpandRegion(const CTreeItem& item, const Region& region)
{
    auto child = item.GetChild();

    if (region.childCount() == 0) {
        TVITEMEX newItem{};
        newItem.mask = TVIF_CHILDREN;
        newItem.hItem = child;
        newItem.cChildren = 0;
        m_tree.SetItem(&newItem);
        return;
    }

    for (const auto& nodes : region.children | std::views::values) {
        for (const auto& node : nodes) {
            const auto& nodeName = node->name;
            auto wideName = StringHelper::fromUTF8(nodeName.c_str());
            TV_INSERTSTRUCT tvis{};
            tvis.hParent = item.m_hTreeItem;
            tvis.hInsertAfter = TVI_LAST;
            tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
                TVIF_PARAM;
            tvis.itemex.cChildren = node->childCount() > 0 ? 1 : 0;
            tvis.itemex.pszText = const_cast<LPTSTR>(wideName.GetString());
            tvis.itemex.iImage = 1;
            tvis.itemex.iSelectedImage = 1;
            tvis.itemex.iExpandedImage = 1;
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new NodeItemData{.type = NIT_NODE, .pdata = node.get()});
            m_tree.InsertItem(&tvis);
        }
    }
}

void LSFFileView::ExpandNode(const CTreeItem& item, const LSNode& node)
{
    auto child = item.GetChild();

    if (node.childCount() == 0) {
        TVITEMEX newItem{};
        newItem.mask = TVIF_CHILDREN;
        newItem.hItem = child;
        newItem.cChildren = 0;
        m_tree.SetItem(&newItem);
        return;
    }

    for (const auto& nodes : node.children | std::views::values) {
        for (const auto& childNode : nodes) {
            const auto& nodeName = childNode->name;
            auto wideName = StringHelper::fromUTF8(nodeName.c_str());
            TV_INSERTSTRUCT tvis{};
            tvis.hParent = item.m_hTreeItem;
            tvis.hInsertAfter = TVI_LAST;
            tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
                TVIF_PARAM;
            tvis.itemex.cChildren = childNode->childCount() > 0 ? 1 : 0;
            tvis.itemex.pszText = const_cast<LPTSTR>(wideName.GetString());
            tvis.itemex.iImage = 1;
            tvis.itemex.iSelectedImage = 1;
            tvis.itemex.iExpandedImage = 1;
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new NodeItemData{.type = NIT_NODE, .pdata = childNode.get()});
            m_tree.InsertItem(&tvis);
        }
    }
}

void LSFFileView::AddAttributes(const LSNode& node)
{
    int index = 0;
    for (const auto& [name, attr] : node.attributes) {
        auto wideName = StringHelper::fromUTF8(name.c_str());
        auto wideValue = StringHelper::fromUTF8(attr.str().c_str());
        auto wideType = StringHelper::fromUTF8(attr.typeStr().c_str());
        index = m_list.InsertItem(index, wideName);
        m_list.SetItemText(index, 1, wideValue);
        m_list.SetItemText(index, 2, wideType);
        ++index;
    }

    AutoAdjustColumns();
}

void LSFFileView::AutoAdjustColumns()
{
    CClientDC dc(m_list);
    auto hFont = m_list.GetFont();
    auto hOldFont = dc.SelectFont(hFont);

    auto header = m_list.GetHeader();

    for (auto col = 0; col < header.GetItemCount(); ++col) {
        LVCOLUMN lvc{};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        TCHAR textBuf[256]{};
        lvc.pszText = textBuf;
        lvc.cchTextMax = _countof(textBuf);
        m_list.GetColumn(col, &lvc);

        CString headerText = lvc.pszText;

        CSize sz;
        dc.GetTextExtent(headerText, headerText.GetLength(), &sz);
        auto maxWidth = sz.cx;

        auto rowCount = m_list.GetItemCount();
        for (auto row = 0; row < rowCount; ++row) {
            CString cellText;
            m_list.GetItemText(row, col, cellText);

            dc.GetTextExtent(cellText, cellText.GetLength(), &sz);
            maxWidth = std::max(sz.cx, maxWidth);
        }

        maxWidth += COLUMN_PADDING;
        m_list.SetColumnWidth(col, maxWidth);
    }

    dc.SelectFont(hOldFont);
}
