#include "stdafx.h"
#include "FolderView.h"
#include "resources/resource.h"

LRESULT FolderView::OnCreate(LPCREATESTRUCT)
{
    auto bResult = DefWindowProc();

    static constexpr auto icons = {
        IDI_FOLDER,
        IDI_FILE
    };

    m_ImageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, static_cast<uint32_t>(icons.size()), 0);

    for (auto icon : icons) {
        auto hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(icon));
        ATLASSERT(hIcon);
        m_ImageList.AddIcon(hIcon);
    }

    SetImageList(m_ImageList, TVSIL_NORMAL);

    auto style = TVS_HASBUTTONS | TVS_HASLINES | TVS_FULLROWSELECT | TVS_INFOTIP
        | TVS_LINESATROOT | TVS_SHOWSELALWAYS;

    ModifyStyle(0, style);

    SetMsgHandled(FALSE);

    return bResult;
}

void FolderView::OnDestroy()
{
    DeleteAllItems();
}

LRESULT FolderView::OnItemExpanding(LPNMHDR pnmh)
{
    const auto item = MAKE_TREEITEM(pnmh, this);

    auto data = std::bit_cast<TreeItemData*>(item.GetData());
    if (data && data->type == TIT_FOLDER) {
        ExpandFolders(item);
    }

    return 0;
}

LRESULT FolderView::OnDelete(LPNMHDR pnmh)
{
    const auto item = MAKE_OLDTREEITEM(pnmh, this);

    auto data = std::bit_cast<TreeItemData*>(item.GetData());

    delete data;

    return 0;
}

void FolderView::SetFolder(const CString& folder)
{
    DeleteAllItems();

    TV_INSERTSTRUCT tvis{};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_FIRST;
    tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
        TVIF_PARAM;
    tvis.itemex.cChildren = 1;
    tvis.itemex.pszText = const_cast<LPTSTR>(folder.GetString());
    tvis.itemex.iImage = 0;
    tvis.itemex.iSelectedImage = 0;
    tvis.itemex.iExpandedImage = 0;
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new TreeItemData{TIT_FOLDER, folder});

    InsertItem(&tvis);

    Expand(GetRootItem(), TVE_EXPAND);
}

void FolderView::ExpandFolders(const CTreeItem& folder)
{
    auto hItem = folder;

    if (GetItemState(folder.m_hTreeItem, TVIS_EXPANDED)) {
        return; // already expanded
    }

    auto child = folder.GetChild();
    if (!child.IsNull()) {
        return; // already have children
    }

    auto data = std::bit_cast<TreeItemData*>(folder.GetData());
    auto parentPath = data->path;

    CString pattern;
    pattern.Format(_T("%s\\*"), parentPath.GetString());
    
    WIN32_FIND_DATA findData{};
    auto hFind = FindFirstFile(pattern, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    auto hasChildren = false;

    do {
        CString path;
        path.Format(_T("%s\\%s"), parentPath.GetString(), findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (_tcscmp(findData.cFileName, _T(".")) == 0 || _tcscmp(findData.cFileName, _T("..")) == 0) {
                continue;
            }

            TV_INSERTSTRUCT tvis{};
            tvis.hParent = hItem;
            tvis.hInsertAfter = TVI_LAST;
            tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT | TVIF_PARAM;
            tvis.itemex.cChildren = 1;
            tvis.itemex.pszText = findData.cFileName;
            tvis.itemex.iImage = 0;
            tvis.itemex.iSelectedImage = 0;
            tvis.itemex.iExpandedImage = 0;
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new TreeItemData{ .type= TIT_FOLDER, .path= path});

            InsertItem(&tvis);

            hasChildren = true;
        } else {
            TV_INSERTSTRUCT tvis{};
            tvis.hParent = hItem;
            tvis.hInsertAfter = TVI_LAST;
            tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
            tvis.itemex.cChildren = 0;
            tvis.itemex.pszText = findData.cFileName;
            tvis.itemex.iImage = 1;
            tvis.itemex.iSelectedImage = 1;
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new TreeItemData{ .type = TIT_FILE, .path = path });
            InsertItem(&tvis);

            hasChildren = true;
        }

    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);

    if (!hasChildren) {
        TVITEMEX item{};
        item.mask = TVIF_CHILDREN;
        item.hItem = hItem;
        item.cChildren = 0;
        SetItem(&item);
    }
}
