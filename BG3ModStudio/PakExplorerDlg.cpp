#include "stdafx.h"
#include "PakExplorerDlg.h"
#include "StringHelper.h"
#include "Util.h"

#include <algorithm>

BOOL PakExplorerDlg::OnIdle()
{
    UIUpdateChildWindows();
    return FALSE; // only want to be called once
}

void PakExplorerDlg::SetPAKReader(PAKReader&& reader)
{
    m_pakReader = std::move(reader);
}

void PakExplorerDlg::OnClose()
{
    Destroy();
}

void PakExplorerDlg::OnDestroy()
{
    auto* pLoop = _Module.GetMessageLoop();
    if (pLoop != nullptr) {
        pLoop->RemoveIdleHandler(this);
    }
}

void PakExplorerDlg::OnSize(UINT nType, CSize size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    if (m_splitter.IsWindow()) {
        CRect rc;
        rc.left = m_marginLeft;
        rc.top = m_marginTop;
        rc.right = size.cx - m_marginRight;
        rc.bottom = size.cy - m_marginBottom;

        m_splitter.MoveWindow(&rc);
    }
}

LRESULT PakExplorerDlg::OnDelete(LPNMHDR pnmh)
{
    const auto item = MAKE_OLDTREEITEM(pnmh, &m_treeView);

    auto* pdata = reinterpret_cast<NodeParam*>(item.GetData());

    delete pdata;

    return 0;
}

LRESULT PakExplorerDlg::OnItemExpanding(LPNMHDR pnmh)
{
    const auto item = MAKE_TREEITEM(pnmh, &m_treeView);

    auto data = std::bit_cast<NodeParam*>(item.GetData());
    if (data && data->type == NodeType::Folder) {
        ExpandFolders(item);
    }

    return 0;
}

LRESULT PakExplorerDlg::OnTVSelChanged(LPNMHDR pnmh)
{
    const auto item = MAKE_TREEITEM(pnmh, &m_treeView);

    auto* data = reinterpret_cast<NodeParam*>(item.GetData());
    if (data == nullptr || data->type == NodeType::Folder) {
        return 0;
    }

    CString name;
    item.GetText(name);

    CWaitCursor cursor;

    CString fullPath;
    fullPath.Format(L"%s/%s", data->prefix.GetString(), name.GetString());

    auto utf8File = StringHelper::toUTF8(fullPath);
    try {
        auto contents = m_pakReader.readFile(utf8File.GetString());

        m_fileView.LoadView(fullPath, contents, FileViewFlags::ReadOnly);
    } catch (const std::exception& e) {
        CString msg;
        msg.Format(L"Failed to read file '%s': %S", fullPath.GetString(), e.what());
        AtlMessageBox(*this, msg.GetString());
        return 0;
    }

    return 0;
}

HTREEITEM PakExplorerDlg::Insert(HTREEITEM hParent, const CString& path, uint32_t startIndex, uint32_t endIndex,
                                 const CString& prefix, int iImage, int cChildren)
{
    TVINSERTSTRUCTW tvis{};
    tvis.hParent = hParent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
        TVIF_CHILDREN | TVIF_PARAM;

    tvis.item.pszText = const_cast<LPWSTR>(path.GetString());
    tvis.item.iImage = iImage;
    tvis.item.iSelectedImage = iImage;
    tvis.item.cChildren = cChildren;

    auto* param = new NodeParam{
        .startIndex = startIndex,
        .endIndex = endIndex,
        .prefix = prefix,
        .type = cChildren == 0 ? NodeType::File : NodeType::Folder
    };

    tvis.item.lParam = reinterpret_cast<LPARAM>(param);

    auto hItem = m_treeView.InsertItem(&tvis);

    return hItem;
}

void PakExplorerDlg::Populate()
{
    CWaitCursor cursor;

    m_pakReader.sortFiles();
    auto& files = m_pakReader.files();

    CString currentRoot;
    auto startIndex = 0u;

    for (auto i = 0u; i < files.size(); ++i) {
        const auto& file = files[i];
        auto wideFile = StringHelper::fromUTF8(file.name.c_str());
        auto pos = wideFile.Find(L'/');

        if (pos == -1) { // ignore files in root (rare)
            ATLASSERT(0);
            continue;
        }

        auto root = wideFile.Left(pos);

        if (currentRoot.IsEmpty()) {
            currentRoot = root;
            startIndex = i;
        } else if (root != currentRoot) {
            Insert(TVI_ROOT, currentRoot, startIndex, i, currentRoot, 0, 1);
            currentRoot = root;
            startIndex = i;
        }
    }

    if (!currentRoot.IsEmpty()) {
        Insert(TVI_ROOT, currentRoot, startIndex, static_cast<uint32_t>(files.size()), currentRoot, 0, 1);
    }
}

void PakExplorerDlg::SetTitle()
{
    auto filename = StringHelper::fromUTF8(m_pakReader.filename().c_str());

    CString title;
    title.Format(L"PAK Explorer - %s", filename.GetString());

    SetWindowText(title);
}

BOOL PakExplorerDlg::OnInitDialog(HWND, LPARAM lParam)
{
    UIAddChildWindowContainer(m_hWnd);

    DlgResize_Init();

    auto wndFrame = GetDlgItem(IDC_ST_PAKEXPLORER);
    ATLASSERT(wndFrame.IsWindow());

    CRect rcDlg;
    GetClientRect(&rcDlg);

    CRect rcFrame;
    wndFrame.GetWindowRect(&rcFrame);
    ScreenToClient(&rcFrame);

    m_marginLeft = rcFrame.left;
    m_marginTop = rcFrame.top;
    m_marginRight = rcDlg.right - rcFrame.right;
    m_marginBottom = rcDlg.bottom - rcFrame.bottom;

    wndFrame.DestroyWindow(); // Needed only for margin calculation

    m_splitter.Create(m_hWnd, rcFrame, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    ATLASSERT(m_splitter.IsWindow());

    if (!m_treeView.Create(m_splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                           WS_EX_CLIENTEDGE)) {
        ATLTRACE("Unable to create tree view window.\n");
        return -1;
    }

    static constexpr auto icons = {
        IDI_FOLDER,
        IDI_FILE
    };

    m_imageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, static_cast<uint32_t>(icons.size()), 0);
    for (auto icon : icons) {
        auto hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(icon));
        ATLASSERT(hIcon);
        m_imageList.AddIcon(hIcon);
    }

    m_treeView.SetImageList(m_imageList, TVSIL_NORMAL);

    auto style = TVS_HASBUTTONS | TVS_HASLINES | TVS_FULLROWSELECT | TVS_INFOTIP
        | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_EDITLABELS;

    m_treeView.ModifyStyle(0, style);

    if (!m_fileView.Create(m_splitter, rcDefault, nullptr,
                           WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE)) {
        ATLTRACE("Unable to create files view window.\n");
        return -1;
    }

    m_splitter.SetSplitterPane(0, m_treeView);
    m_splitter.SetSplitterPane(1, m_fileView);
    m_splitter.SetSplitterPosPct(40);

    UIAddChildWindowContainer(m_hWnd);
    DlgResize_Init();

    CenterWindow(GetParent());

    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddIdleHandler(this);

    auto icon = Util::LoadBitmapAsIcon(ID_FILE_PAK_OPEN, 32, 32);
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    Populate();

    SetTitle();

    return TRUE; // Let the system set the focus
}

void PakExplorerDlg::ExpandFolders(const CTreeItem& folder)
{
    auto child = folder.GetChild();

    if (m_treeView.GetItemState(folder.m_hTreeItem, TVIS_EXPANDED)) {
        if (child.IsNull()) {
            TVITEMEX item{};
            item.mask = TVIF_CHILDREN;
            item.hItem = folder.m_hTreeItem;
            item.cChildren = 0; // no children
            m_treeView.SetItem(&item);
        }

        return; // already expanded
    }

    if (!child.IsNull()) {
        return; // already have children
    }

    CWaitCursor cursor;

    auto* np = reinterpret_cast<NodeParam*>(m_treeView.GetItemData(folder.m_hTreeItem));
    if (np == nullptr || np->type != NodeType::Folder) {
        return;
    }

    const auto& files = m_pakReader.files();

    auto prefix = np->prefix;

    std::unordered_set<std::wstring> seenFolders; // avoid duplicates

    auto inserted = 0;

    for (auto i = np->startIndex; i < np->endIndex; ++i) {
        const auto& full = files[i].name;
        auto wideFile = StringHelper::fromUTF8(full.c_str());
        if (wideFile.Left(prefix.GetLength()) != prefix) {
            continue;
        }

        auto rel = wideFile.Mid(prefix.GetLength() + 1); // +1 to skip '/'
        auto pos = rel.Find(L'/');
        if (pos == -1) { // file
            // Insert file
            Insert(folder.m_hTreeItem, rel, i, i + 1, prefix, 1, 0);
            ++inserted;
        } else { // folder
            rel = rel.Left(pos);
            auto subPrefix = np->prefix + L"/" + rel;

            if (seenFolders.insert(rel.GetString()).second) {
                auto end = i;

                while (end < np->endIndex) {
                    auto nextWideFile = StringHelper::fromUTF8(files[end].name.c_str());
                    if (nextWideFile.Left(subPrefix.GetLength()) != subPrefix) {
                        break;
                    }
                    ++end;
                }

                Insert(folder.m_hTreeItem, rel, i, end, subPrefix, 0, 1);
                ++inserted;
            }
        }
    }

    if (inserted == 0) {
        TVITEMEX item{};
        item.mask = TVIF_CHILDREN;
        item.hItem = folder.m_hTreeItem;
        item.cChildren = 0; // no children
        m_treeView.SetItem(&item);
    }
}
