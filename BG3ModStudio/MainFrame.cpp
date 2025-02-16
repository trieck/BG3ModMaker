#include "stdafx.h"
#include "resources/resource.h"
#include "MainFrame.h"

BOOL MainFrame::DefCreate()
{
    RECT rect = { 0, 0, 1024, 600 };
    auto hWnd = CreateEx(nullptr, rect);
    if (hWnd == nullptr) {
        ATLTRACE(_T("Unable to create main frame.\n"));
        return FALSE;
    }

    return TRUE;
}

BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
    return CFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg);
}

LRESULT MainFrame::OnCreate(LPCREATESTRUCT pcs)
{
    auto hWndCmdBar = m_cmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
    m_cmdBar.AttachMenu(GetMenu());
    m_cmdBar.LoadImages(IDR_MAINFRAME);
    SetMenu(nullptr); // remove old menu

    bool bRibbonUI = RunTimeHelper::IsRibbonUIAvailable();
    if (bRibbonUI) {
        UIAddMenu(m_cmdBar.GetMenu(), true);
        UIRemoveUpdateElement(ID_FILE_MRU_FIRST);
    }

    CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
    AddSimpleReBarBand(hWndCmdBar);
    CreateSimpleStatusBar();

    m_hWndClient = m_splitter.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    if (m_hWndClient == nullptr) {
        ATLTRACE(_T("Unable to create splitter window.\n"));
        return -1;
    }

    if (!m_objectPane.Create(m_splitter, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)) {
        return -1;
    }

    m_objectPane.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);

    if (!m_folderView.Create(m_objectPane, rcDefault, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASLINES |
        TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE)) {
        return -1;
    }

    m_objectPane.SetClient(m_folderView);

    if (!m_fileView.Create(m_splitter, rcDefault, nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
        ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE
        | ES_NOOLEDRAGDROP | ES_READONLY, WS_EX_CLIENTEDGE)) {
        return -1;
    }

    m_splitter.SetSplitterPane(0, m_objectPane);
    m_splitter.SetSplitterPane(1, m_fileView);
    m_splitter.SetSplitterPosPct(30);

    UISetCheck(ID_VIEW_STATUS_BAR, 1);
    ShowRibbonUI(TRUE);

    UpdateLayout();
    CenterWindow();

    // register object for message filtering and idle updates
    auto pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    return 1;
}

LRESULT MainFrame::OnFileExit()
{
    PostMessage(WM_CLOSE);
    return 0;
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
    UIUpdateMenuBar();

    return FALSE;
}
