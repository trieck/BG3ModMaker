#include "stdafx.h"
#include "RopePad.h"
#include "MessageLoopEx.h"
#include "RopeFrame.h"
#include "RopeTreeFrame.h"

extern CAppModule _Module;

RopePad::RopePad() : m_treeFrame(this)
{
}

RopePad::~RopePad()
{
}

BOOL RopePad::Init(HINSTANCE hInstance, LPSTR lpCmdLine)
{
    // Initialize the application
    if (!hInstance || !lpCmdLine) {
        return FALSE;
    }

    // Initialize the ATL module
    auto result = _Module.Init(nullptr, hInstance);
    if (FAILED(result)) {
        return FALSE;
    }

    // Initialize COM library
    auto hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        return FALSE;
    }

    // Initialize Direct2D
    hr = m_direct2D.Initialize();
    if (FAILED(hr)) {
        ATLTRACE(_T("Direct2D initialization failed\n"));
        return -1;
    }

    // Initialize common controls
    INITCOMMONCONTROLSEX iex = { sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES | ICC_COOL_CLASSES };
    if (!InitCommonControlsEx(&iex)) {
        return FALSE;
    }
    
    return TRUE;
}

int RopePad::Run(int nShowCmd)
{
    MessageLoopEx msgLoop;
    _Module.AddMessageLoop(&msgLoop);

    RopeFrame frame(this);
    if (!frame.DefCreate()) {
        ATLTRACE(_T("Main window creation failed\n"));
        return -1;
    }
    
    frame.ShowWindow(nShowCmd);

    if (!m_treeFrame.DefCreate()) {
        ATLTRACE(_T("Tree window creation failed\n"));
        return -1;
    }

    m_treeFrame.ShowWindow(nShowCmd);

    auto result = msgLoop.Run();

    _Module.RemoveMessageLoop();

    return result;
}

void RopePad::Term()
{
    if (m_treeFrame.IsWindow()) {
        m_treeFrame.DestroyWindow();
    }

    m_direct2D.Terminate();

    _Module.Term();

    CoUninitialize();
}

ID2D1Factory* RopePad::GetD2DFactory() const
{
    return m_direct2D.GetD2DFactory();
}

IDWriteFactory* RopePad::GetDWriteFactory() const
{
    return m_direct2D.GetDWriteFactory();
}

void RopePad::ToggleTreeView()
{
    if (!m_treeFrame.IsWindow()) {
        return;
    }

    m_treeFrame.ShowWindow(m_treeFrame.IsWindowVisible() ? SW_HIDE : SW_SHOW);
}
