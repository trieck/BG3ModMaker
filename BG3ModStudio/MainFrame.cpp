#include "stdafx.h"

#include "COMError.h"
#include "FileDialogEx.h"
#include "FileOperation.h"
#include "MainFrame.h"

BOOL MainFrame::DefCreate()
{
    RECT rect = {0, 0, 1024, 600};
    auto hWnd = CreateEx(nullptr, rect);
    if (hWnd == nullptr) {
        ATLTRACE(_T("Unable to create main frame.\n"));
        return FALSE;
    }

    return TRUE;
}

BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
    return CFrameWindowImpl::PreTranslateMessage(pMsg);
}

LRESULT MainFrame::OnCreate(LPCREATESTRUCT pcs)
{
    m_cmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
    m_cmdBar.AttachMenu(GetMenu());
    m_cmdBar.LoadImages(IDR_MAINFRAME);
    SetMenu(nullptr); // remove old menu

    if (RunTimeHelper::IsRibbonUIAvailable()) {
        UIAddMenu(m_cmdBar.GetMenu(), true);
        UIRemoveUpdateElement(ID_FILE_MRU_FIRST);
    }

    if (!CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE)) {
        ATLTRACE("Unable to create rebar.\n");
        return -1;
    }

    if (!AddSimpleReBarBand(m_cmdBar)) {
        ATLTRACE("Unable to add rebar band.\n");
        return -1;
    }

    if (!CreateSimpleStatusBar()) {
        ATLTRACE("Unable to create status bar.\n");
        return -1;
    }

    ATLASSERT(::IsWindow(m_hWndStatusBar));
    m_statusBar.Attach(m_hWndStatusBar);

    int parts[] = { 200, -1 };
    m_statusBar.SetParts(2, parts);
    m_statusBar.SetSimple(FALSE);
    
    m_bom.LoadIcon(IDI_BOM, 12, 12);
    m_nobom.LoadIcon(IDI_NOBOM, 12, 12);
    
    m_hWndClient = m_splitter.Create(*this, rcDefault, nullptr,
                                     WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    if (m_hWndClient == nullptr) {
        ATLTRACE("Unable to create splitter window.\n");
        return -1;
    }

    if (!m_folderView.Create(m_splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                             WS_EX_CLIENTEDGE)) {
        return -1;
    }

    if (!m_filesView.Create(m_splitter, rcDefault, nullptr, 
                            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE)) {
        return -1;
    }

    m_splitter.SetSplitterPane(0, m_folderView);
    m_splitter.SetSplitterPane(1, m_filesView);
    m_splitter.SetSplitterPosPct(30);

    UISetCheck(ID_VIEW_STATUS_BAR, 1);

    ShowRibbonUI(TRUE);

    UpdateLayout();
    CenterWindow();

    // register object for message filtering and idle updates
    auto pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != nullptr);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    return 1;
}

LRESULT MainFrame::OnFolderOpen()
{
    FileDialogEx dlg(FileDialogEx::Folder, *this);
    if (dlg.DoModal() != IDOK) {
        return 0;
    }

    const auto& paths = dlg.paths();
    ATLASSERT(!paths.empty());

    m_filesView.CloseAllFiles();

    m_folderView.SetFolder(paths[0]);

    UpdateTitle();

    return 0;
}

LRESULT MainFrame::OnFolderClose()
{
    m_folderView.DeleteAllItems();
    m_folderView.RedrawWindow();

    m_filesView.CloseAllFiles();

    UpdateTitle();

    return 0;
}

LRESULT MainFrame::OnFileSave()
{
    auto activeFile = m_filesView.ActiveFile();
    ATLASSERT(activeFile != nullptr);

    m_filesView.SaveFile(activeFile);

    return 0;
}

LRESULT MainFrame::OnFileSaveAll()
{
    m_filesView.SaveAll();

    return 0;
}

LRESULT MainFrame::OnFileExit()
{
    PostMessage(WM_CLOSE);
    return 0;
}

LRESULT MainFrame::OnDeleteFile()
{
    auto item = m_folderView.GetSelectedItem();
    if (item.IsNull()) {
        return 0;
    }

    auto data = std::bit_cast<TreeItemData*>(item.GetData());
    CString filename = data ? data->path : CString();
    if (filename.IsEmpty()) {
        return 0;
    }

    auto isFolder = data && data->type == TIT_FOLDER;

    CString message;
    if (isFolder) {
        message.Format(L"Are you sure you want to delete the folder \"%s\"?", filename);
    } else {
        message.Format(L"Are you sure you want to delete the file \"%s\"?", filename);
    }

    auto result = AtlMessageBox(*this, static_cast<LPCTSTR>(message), nullptr, MB_ICONQUESTION | MB_YESNO);
    if (result != IDYES) {
        return 0;
    }

    FileOperation op;
    auto hr = op.Create();
    if (FAILED(hr)) {
        CoMessageBox(*this, hr, nullptr, MB_ICONERROR);
        return 0;
    }

    CWaitCursor wait;
    hr = op.DeleteItem(filename);
    if (FAILED(hr)) {
        CoMessageBox(*this, hr, nullptr, MB_ICONERROR);
        return 0;
    }

    auto hItem = item.m_hTreeItem;

    m_folderView.DeleteItem(hItem);
    m_folderView.RedrawWindow();

    return 0;
}

LRESULT MainFrame::OnNewFile()
{
    ATLASSERT(m_filesView.IsWindow());

    m_filesView.NewFile();

    return 0;
}

LRESULT MainFrame::OnNewFileHere()
{
    FileDialogEx dlg(FileDialogEx::Save, *this, L"*.*", nullptr, 0, L"*.*");
        
    dlg.DoModal();

    return 0;
}

void MainFrame::OnClose()
{
    if (m_filesView.IsWindow()) {
        m_filesView.CloseAllFiles();
    }

    if (m_folderView.IsWindow()) {
        m_folderView.DeleteAllItems();
    }
    
    DestroyWindow();
}

LRESULT MainFrame::OnTVDelete(LPNMHDR pnmhdr)
{
    const auto item = MAKE_OLDTREEITEM(pnmhdr, &m_folderView);

    if (m_filesView.IsWindow()) {
        m_filesView.CloseFileByData(item.m_hTreeItem);
    }

    BOOL bHandled;
    ReflectNotifications(WM_NOTIFY, 0, reinterpret_cast<LPARAM>(pnmhdr), bHandled);

    return 0;
}

LRESULT MainFrame::OnTVSelChanged(LPNMHDR /*pnmhdr*/)
{
    auto item = m_folderView.GetSelectedItem();
    if (item.IsNull()) {
        return 0;
    }

    CWaitCursor wait;

    auto data = std::bit_cast<TreeItemData*>(item.GetData());
    if (data && data->type == TIT_FILE) {
        if (!m_filesView.ActivateFile(data->path, item.m_hTreeItem)) {
            // Select the item's parent if unable to be activated (guaranteed).
            m_folderView.SelectItem(m_folderView.GetParentItem(item.m_hTreeItem));
        }
    }

    return 0;
}

LRESULT MainFrame::OnTabActivated(LPNMHDR pnmhdr)
{
    auto page = static_cast<int>(pnmhdr->idFrom);
    if (page < 0) {
        UpdateEncodingStatus(UNKNOWN);
        return 0;
    }

    auto encoding = m_filesView.FileEncoding(page);
    UpdateEncodingStatus(encoding);

    auto hTreeItem = static_cast<HTREEITEM>(m_filesView.GetData(page));
    if (hTreeItem == nullptr) {
        return 0;
    }

    if (m_folderView.GetSelectedItem() == hTreeItem) {
        return 0;
    }

    m_folderView.Expand(hTreeItem);
    m_folderView.SelectItem(hTreeItem);
    m_folderView.EnsureVisible(hTreeItem);

    return 0;
}

LRESULT MainFrame::OnTabContextMenu(LPNMHDR pnmh)
{
    auto lpcmi = reinterpret_cast<LPTBVCONTEXTMENUINFO>(pnmh);

    auto tab = static_cast<int>(pnmh->idFrom);

    CMenu menu;
    menu.LoadMenuW(IDR_TAB_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);

    HTREEITEM hTreeItem = nullptr;

    switch (popup.TrackPopupMenu(
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | TPM_RETURNCMD | TPM_NONOTIFY, lpcmi->pt.x, lpcmi->pt.y,
        *this)) {
    case ID_TAB_CLOSE:
        hTreeItem = static_cast<HTREEITEM>(m_filesView.CloseFile(tab));
        break;
    case ID_TAB_CLOSE_ALL:
        m_filesView.CloseAllFiles();
        hTreeItem = m_folderView.GetSelectedItem();
        break;
    case ID_TAB_CLOSE_OTHERS:
        m_filesView.CloseOtherFiles(tab);
        break;
    default:
        break;
    }

    if (hTreeItem) {
        if (m_folderView.GetSelectedItem() == hTreeItem) {
            m_folderView.SelectItem(m_folderView.GetRootItem());
        }
    }

    return 0;
}

LRESULT MainFrame::OnRClick(LPNMHDR pnmh)
{
    if (pnmh->hwndFrom != m_folderView) {
        return 0;
    }

    auto item = m_folderView.GetSelectedItem();
    if (item.IsNull()) {
        return 0;
    }

    CMenu menu;
    menu.LoadMenu(IDR_TREE_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);

    CPoint pt;
    GetCursorPos(&pt);

    auto data = std::bit_cast<TreeItemData*>(item.GetData());
    auto isFolder = data && data->type == TIT_FOLDER;

    if (isFolder) {
        popup.ModifyMenu(ID_TREE_DELETE_FILE, MF_BYCOMMAND | MF_ENABLED, ID_TREE_DELETE_FILE, L"Delete Folder");
    } else {
        popup.EnableMenuItem(ID_TREE_NEWFILEHERE, MF_GRAYED);
    }

    popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_VERTICAL, pt.x, pt.y, *this);

    return 0;
}

BOOL MainFrame::FolderIsOpen() const
{
    if (!m_folderView.IsWindow()) {
        return FALSE;
    }

    auto root = m_folderView.GetRootItem();
    return !root.IsNull();
}

void MainFrame::UpdateTitle()
{
    CString strTitle;

    auto* title = AtlGetStringPtr(IDR_MAINFRAME);
    ATLASSERT(title != nullptr);

    if (FolderIsOpen()) {
        auto root = m_folderView.GetRootItem();
        ATLASSERT(root.m_hTreeItem != nullptr);
        auto data = std::bit_cast<TreeItemData*>(root.GetData());
        if (data != nullptr && data->path.GetLength()) {
            strTitle.Format(_T("%s : %s"), title, data->path);
        }
    } else {
        strTitle = title;
    }

    SetWindowText(strTitle);
}

void MainFrame::UpdateEncodingStatus(FileEncoding encoding)
{
    CString status;
    HICON hIcon = nullptr;

    switch (encoding) {
    case UTF8:
        status = L"UTF-8";
        hIcon = m_nobom;
        break;
    case UTF8BOM:
        status = L"UTF-8 with BOM";
        hIcon = m_bom;
        break;
    default:
        status = L"";   // Unknown
        break;
    }

    m_statusBar.SetIcon(1, hIcon);
    m_statusBar.SetText(1, status);
}

LRESULT MainFrame::OnViewStatusBar()
{
    BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
    ::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
    UpdateLayout();
    return 0;
}

BOOL MainFrame::OnIdle()
{
    UIEnable(ID_FILE_NEW, FolderIsOpen());
    UIEnable(ID_FILE_CLOSE, FolderIsOpen());

    if (m_filesView.IsWindow()) {
        UIEnable(ID_FILE_SAVE, m_filesView.IsDirty(m_filesView.ActivePage()));
        UIEnable(ID_FILE_SAVE_ALL, m_filesView.IsDirty());
    }

    UIUpdateMenuBar();

    return FALSE;
}
