#include "stdafx.h"
#include "RopeFrame.h"

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
    UpdateLayout();
    CenterWindow();

    m_hWndClient = m_view.Create(m_hWnd, rcDefault, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CLIENTEDGE);

    if (m_hWndClient == nullptr) {
        ATLTRACE(_T("Unable to create view window.\n"));
        return -1; // Fail the creation
    }

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
    if (m_view.IsWindow()) {
        m_view.SetWindowPos(nullptr, 0, 0, size.cx, size.cy, SWP_NOZORDER);
    }
}

BOOL RopeFrame::DefCreate()
{
    RECT rect = {0, 0, 400, 500};
    auto hWnd = CreateEx(nullptr, rect);
    if (hWnd == nullptr) {
        ATLTRACE(_T("Unable to create frame window.\n"));
        return FALSE;
    }

    return TRUE;
}
