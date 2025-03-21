#include "stdafx.h"
#include "RopeFrame.h"
#include "AboutDlg.h"

extern CAppModule _Module;

RopeFrame::RopeFrame(RopePad* pApp) : m_pApp(pApp), m_view(pApp)
{
    ATLASSERT(m_pApp != nullptr);
}

BOOL RopeFrame::PreTranslateMessage(MSG* pMsg)
{
    return CFrameWindowImpl::PreTranslateMessage(pMsg);
}

LRESULT RopeFrame::OnCreate(LPCREATESTRUCT pcs)
{
    SetMenuBitmaps();
    UpdateLayout();
    CenterWindow();

    m_hWndClient = m_view.Create(m_hWnd, rcDefault, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CLIENTEDGE);

    if (m_hWndClient == nullptr) {
        ATLTRACE(_T("Unable to create view window.\n"));
        return -1; // Fail the creation
    }

    if (!CreateSimpleStatusBar()) {
        ATLTRACE("Unable to create status bar.\n");
        return -1;
    }

    ATLASSERT(::IsWindow(m_hWndStatusBar));
    m_statusBar.Attach(m_hWndStatusBar);

    // register object for message filtering and idle updates
    auto pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != nullptr);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    return 0;
}

BOOL RopeFrame::OnIdle()
{
    return FALSE; // Return TRUE if you want to continue idle processing
}

void RopeFrame::OnClose()
{
    DestroyWindow();
}

void RopeFrame::OnSize(UINT nType, const CSize& size)
{
    UpdateLayout();
}

void RopeFrame::OnFileExit()
{
    PostMessage(WM_CLOSE);
}

void RopeFrame::OnAbout()
{
    AboutDlg dlg;
    dlg.DoModal();
}

BOOL RopeFrame::DefCreate()
{
    RECT rect = {0, 0, 600, 400};
    auto hWnd = CreateEx(nullptr, rect);
    if (hWnd == nullptr) {
        ATLTRACE(_T("Unable to create frame window.\n"));
        return FALSE;
    }

    return TRUE;
}

void RopeFrame::SetMenuBitmap(UINT nResourceID)
{
    auto hBmp = ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(nResourceID));
    ATLASSERT(hBmp != nullptr);
    if (hBmp != nullptr) {
        CMenuHandle menu = GetMenu();
        ATLASSERT(menu.m_hMenu != nullptr);
        menu.SetMenuItemBitmaps(nResourceID, MF_BYCOMMAND, hBmp, hBmp);
    }
}

void RopeFrame::SetMenuBitmaps()
{
    SetMenuBitmap(ID_APP_EXIT);
    SetMenuBitmap(ID_HELP_ABOUT);
}
