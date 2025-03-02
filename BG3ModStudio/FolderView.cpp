#include "stdafx.h"
#include "FolderView.h"

#include "ShellHelper.h"
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
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new TreeItemData{.type = TIT_FOLDER, .path = folder});

    InsertItem(&tvis);

    Expand(GetRootItem(), TVE_EXPAND);
}

void FolderView::ExpandFolders(const CTreeItem& folder)
{
    auto hItem = folder;

    auto child = folder.GetChild();

    if (GetItemState(folder.m_hTreeItem, TVIS_EXPANDED)) {
        if (child.IsNull()) {
            TVITEMEX item{};
            item.mask = TVIF_CHILDREN;
            item.hItem = hItem;
            item.cChildren = 0;
            SetItem(&item);
        }

        return; // already expanded
    }

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
            tvis.itemex.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT |
                TVIF_PARAM;
            tvis.itemex.cChildren = 1;
            tvis.itemex.pszText = findData.cFileName;
            tvis.itemex.iImage = 0;
            tvis.itemex.iSelectedImage = 0;
            tvis.itemex.iExpandedImage = 0;
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new TreeItemData{.type = TIT_FOLDER, .path = path});

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
            tvis.itemex.lParam = std::bit_cast<LPARAM>(new TreeItemData{.type = TIT_FILE, .path = path});
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

HTREEITEM FolderView::FindFile(const CString& filename)
{
    if (!IsWindow()) {
        return nullptr;
    }

    auto hRoot = GetRootItem();
    if (hRoot.IsNull()) {
        return nullptr;
    }

    auto* rootData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hRoot));
    if (!rootData) {
        return nullptr; // unknown
    }

    if (rootData->type != TIT_FOLDER) {
        return nullptr; // not a folder
    }

    if (!PathIsPrefix(rootData->path, filename)) {
        return nullptr; // filename not rooted in tree root
    }

    auto rootComponents = SplitPath(rootData->path);
    auto components = SplitPath(filename);

    while (!rootComponents.empty() && !components.empty()) {
        auto rootComponent = rootComponents.front();
        auto component = components.front();
        if (rootComponent.CompareNoCase(component) != 0) {
            break;  // not rooted
        }
        rootComponents.pop_front();
        components.pop_front();
    }

    if (!rootComponents.empty()) {
        return nullptr; // path is not rooted at tree root.
    }

    if (components.empty()) {
        return hRoot; // the component is the root?
    }

    return FindFile(hRoot, components);
}

HTREEITEM FolderView::AddFile(const CString& filename)
{
    if (!IsWindow()) {
        return nullptr;
    }

    auto hRoot = GetRootItem();
    if (hRoot.IsNull()) {
        return nullptr;
    }

    auto* rootData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hRoot));
    if (!rootData) {
        return nullptr; // unknown
    }

    if (rootData->type != TIT_FOLDER) {
        return nullptr; // not a folder
    }

    if (!PathIsPrefix(rootData->path, filename)) {
        return nullptr; // filename not rooted in tree root
    }

    auto rootComponents = SplitPath(rootData->path);
    auto components = SplitPath(filename);

    while (!rootComponents.empty() && !components.empty()) {
        auto rootComponent = rootComponents.front();
        auto component = components.front();
        if (rootComponent.CompareNoCase(component) != 0) {
            break;  // not rooted
        }
        rootComponents.pop_front();
        components.pop_front();
    }

    if (!rootComponents.empty()) {
        return nullptr; // path is not rooted at tree root.
    }

    if (components.empty()) {
        return nullptr; // the component is the root?
    }

    auto hItem = InsertFile(GetRootItem(), filename, components);
    if (hItem != nullptr) {
        auto hParentItem = GetParentItem(hItem);
        TVITEMEX item{};
        item.mask = TVIF_CHILDREN;
        item.hItem = hParentItem;
        item.cChildren = 1;
        SetItem(&item);

        auto state = GetItemState(hParentItem, TVIF_STATE);
        if (state & TVIS_EXPANDED) {
            Expand(hParentItem, TVE_COLLAPSE);
            Expand(hParentItem, TVE_EXPAND);
        }
    }

    return hItem;
}

HTREEITEM FolderView::InsertFile(HTREEITEM hRoot, const CString& filename, std::deque<CString>& components)
{
    auto* rootData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hRoot));
    if (!rootData) {
        return nullptr; // unknown
    }

    if (rootData->type != TIT_FOLDER) {
        return nullptr; // not a folder
    }

    CString subpath(rootData->path);

    while (!components.empty() && hRoot != nullptr) {
        auto component = components.front();
        components.pop_front();

        subpath.AppendFormat(_T("\\%s"), component.GetString());

        hRoot = InsertSubpath(hRoot, subpath, component);
    }

    return hRoot;
}

HTREEITEM FolderView::InsertSubpath(HTREEITEM hRoot, const CString& subpath, const CString& component)
{
    auto* pData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hRoot));
    if (!pData) {
        return nullptr; // unknown
    }

    if (pData->type != TIT_FOLDER) {
        return nullptr; // not a folder
    }

    auto rootPath = pData->path;

    if (_tcsicmp(rootPath, subpath) == 0) {
        return hRoot; // root is already the subpath, return it
    }

    HTREEITEM hChild = GetNextItem(hRoot, TVGN_CHILD);
    while (hChild != nullptr) {
        HTREEITEM hNext = GetNextItem(hChild, TVGN_NEXT);
        pData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hChild));
        if (!pData) {
            continue;
        }

        if (_tcsicmp(pData->path, subpath) == 0) {
            return hChild; // already exists, return it
        }

        hChild = hNext;
    }

    auto attributes = GetFileAttributes(subpath);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return nullptr; // no such subpath
    }

    auto isDir = attributes & FILE_ATTRIBUTE_DIRECTORY;

    auto mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_EXPANDEDIMAGE | TVIF_TEXT | TVIF_PARAM;
    if (isDir) {
        mask |= TVIF_CHILDREN;
    }

    TreeItemType type = isDir ? TIT_FOLDER : TIT_FILE;

    TV_INSERTSTRUCT tvis{};
    tvis.hParent = hRoot;
    tvis.hInsertAfter = TVI_LAST;
    tvis.itemex.mask = mask;
    tvis.itemex.cChildren = isDir ? 1 : 0;
    tvis.itemex.pszText = const_cast<LPWSTR>(component.GetString());
    tvis.itemex.iImage = isDir ? 0 : 1;
    tvis.itemex.iSelectedImage = isDir ? 0 : 1;
    tvis.itemex.iExpandedImage = isDir ? 0 : 1;
    tvis.itemex.lParam = std::bit_cast<LPARAM>(new TreeItemData{.type = type, .path = subpath});

    return InsertItem(&tvis);
}

HTREEITEM FolderView::FindFile(HTREEITEM hRoot, std::deque<CString>& components)
{
    auto* pData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hRoot));
    if (!pData) {
        return nullptr; // unknown
    }

    if (pData->type != TIT_FOLDER) {
        return nullptr; // not a folder
    }

    auto rootPath = pData->path;

    CString subpath(rootPath);

    while (!components.empty() && hRoot != nullptr) {
        auto component = components.front();
        components.pop_front();

        subpath.AppendFormat(_T("\\%s"), component.GetString());

        hRoot = FindSubpath(hRoot, subpath);
    }

    return hRoot;
}

HTREEITEM FolderView::FindSubpath(HTREEITEM hRoot, const CString& subpath)
{
    auto* pData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hRoot));
    if (!pData) {
        return nullptr; // unknown
    }

    if (pData->type != TIT_FOLDER) {
        return nullptr; // not a folder
    }

    if (_tcsicmp(pData->path, subpath) == 0) {
        return hRoot; // found it!
    }

    HTREEITEM hChild = GetNextItem(hRoot, TVGN_CHILD);
    while (hChild != nullptr) {
        HTREEITEM hNext = GetNextItem(hChild, TVGN_NEXT);

        pData = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hChild));
        if (!pData) {
            continue;
        }

        if (_tcsicmp(pData->path, subpath) == 0) {
            return hChild; // found it!
        }

        hChild = hNext;
    }

    return nullptr;
}

HTREEITEM FolderView::RenameFile(const CString& oldname, const CString& newname)
{
    auto components = SplitPath(newname);
    if (components.empty()) {
        return nullptr;
    }

    const auto& filepart = components.back();

    auto hItem = FindFile(oldname);

    if (hItem != nullptr) {
        auto data = reinterpret_cast<LPTREEITEMDATA>(GetItemData(hItem));
        if (data) {
            data->path = newname;
        }

        TVITEMEX item{};
        item.mask = TVIF_TEXT | TVIF_HANDLE;
        item.hItem = hItem;
        item.pszText = const_cast<LPWSTR>(filepart.GetString());
        if (!SetItem(&item)) {
            return nullptr;
        }
    }

    return hItem;
}
