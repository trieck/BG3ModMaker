#include "stdafx.h"
#include "FileDialogEx.h"
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

LRESULT MainFrame::OnFileOpen()
{
    FileDialogEx dlg(FileDialogEx::Folder, *this);

    if (dlg.DoModal() == IDOK) {
        const auto& paths = dlg.paths();
        ATLASSERT(!paths.empty());

        m_filesView.CloseAllFiles();

        m_folderView.SetFolder(paths[0]);
    }

    return 0;
}

LRESULT MainFrame::OnFileSave() const
{
    auto activeFile = m_filesView.ActiveFile();
    ATLASSERT(activeFile != nullptr);

    if (!activeFile->SaveFile()) {
        CString message;
        message.Format(L"Unable to save file \"%s\".", activeFile->GetPath());
        AtlMessageBox(*this, static_cast<LPCWSTR>(message), nullptr, MB_ICONERROR);
    }

    return 0;
}

LRESULT MainFrame::OnFileSaveAll() const
{
    for (auto& file : m_filesView.Files()) {
        if (file->IsDirty()) {
            if (!file->SaveFile()) {
                CString message;
                message.Format(L"Unable to save file \"%s\".", file->GetPath());
                AtlMessageBox(*this, static_cast<LPCWSTR>(message), nullptr, MB_ICONERROR);
            }
        }
    }

    return 0;
}

LRESULT MainFrame::OnFileExit()
{
    PostMessage(WM_CLOSE);
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
    UIEnable(ID_FILE_SAVE, m_filesView.IsDirty(m_filesView.ActivePage()));
    UIEnable(ID_FILE_SAVE_ALL, m_filesView.IsDirty());
    UIUpdateMenuBar();

    return FALSE;
}
