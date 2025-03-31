#include "stdafx.h"
#include "RopeTreeFrame.h"
#include "RopePad.h"

extern CAppModule _Module;

RopeTreeFrame::RopeTreeFrame(RopePad* pApp) : m_view(pApp), m_pApp(pApp)
{
}

BOOL RopeTreeFrame::PreTranslateMessage(MSG* pMsg)
{
    return CFrameWindowImpl::PreTranslateMessage(pMsg);
}

LRESULT RopeTreeFrame::OnCreate(LPCREATESTRUCT pcs)
{
    m_hWndClient = m_view.Create(m_hWnd, rcDefault, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CLIENTEDGE);
    if (m_hWndClient == nullptr) {
        ATLTRACE(_T("Unable to create tree view window.\n"));
        return -1;
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

void RopeTreeFrame::OnAddChar(UINT nChar)
{
    if (m_view.IsWindow()) {
        m_view.SendMessage(WM_ADDCHAR, nChar);
    }
}

void RopeTreeFrame::OnClose()
{
    ShowWindow(SW_HIDE);
}

void RopeTreeFrame::OnDestroy()
{
    // Don't quite -- PostQuitMessage(0);
}

void RopeTreeFrame::OnSize(UINT nType, CSize size)
{
    UpdateLayout();
}

BOOL RopeTreeFrame::OnIdle()
{
    return FALSE; // Return TRUE if you want to continue idle processing
}

BOOL RopeTreeFrame::DefCreate()
{
    RECT rect = { 0, 0, 500, 600 };
    auto hWnd = CreateEx(nullptr, rect);
    if (hWnd == nullptr) {
        ATLTRACE(_T("Unable to create tree frame window.\n"));
        return FALSE;
    }

    return TRUE;
}
