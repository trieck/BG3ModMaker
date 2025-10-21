#include "stdafx.h"
#include "BG3ModStudio.h"
#include "MainFrame.h"
#include "MessageLoopEx.h"
#include "TabView.h"

CAppModule _Module;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nShowCmd)
{
    auto& app = BG3ModStudio::instance();
    if (!app.Init()) {
        ATLTRACE(_T("Failed to initialize application\n"));
        return 1;
    }

    return BG3ModStudio::Run(hInstance, lpCmdLine, nShowCmd);
}

BG3ModStudio& BG3ModStudio::instance()
{
    static BG3ModStudio instance;
    return instance;
}

BG3ModStudio::~BG3ModStudio()
{
    m_direct2D.Terminate();
}

BOOL BG3ModStudio::Init()
{
    auto hr = m_direct2D.Initialize();
    if (FAILED(hr)) {
        ATLTRACE(_T("Cannot initialize Direct2D.\n"));
        CoUninitialize();
        return FALSE;
    }

    INITCOMMONCONTROLSEX iex = {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_USEREX_CLASSES |
        ICC_LINK_CLASS | ICC_BAR_CLASSES
    };

    if (!InitCommonControlsEx(&iex)) {
        ATLTRACE(_T("Cannot initialize common controls.\n"));
        return FALSE;
    }

    DWORD param = FE_FONTSMOOTHINGCLEARTYPE;
    SystemParametersInfo(SPI_SETFONTSMOOTHING, 1, nullptr, 0);
    SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 1, &param, 0);

    if (!InitTabView()) {
        ATLTRACE(_T("Unable to initialize tab view.\n"));
        return FALSE;
    }

    return TRUE;
}

int BG3ModStudio::Run(HINSTANCE hInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    auto hr = _Module.Init(nullptr, hInstance);
    if (FAILED(hr)) {
        ATLTRACE(_T("Unable to initialize module.\n"));
        return -1;
    }

    MessageLoopEx theLoop;
    _Module.AddMessageLoop(&theLoop);

    MainFrame wndMain;
    if (!wndMain.DefCreate()) {
        ATLTRACE(_T("Main window creation failed\n"));
        return -1;
    }

    wndMain.ShowWindow(SW_SHOWNORMAL/*nShowCmd*/);
    wndMain.UpdateWindow();

    auto result = theLoop.Run();

    _Module.RemoveMessageLoop();

    return result;
}

ID2D1Factory* BG3ModStudio::GetD2DFactory() const
{
    return m_direct2D.GetD2DFactory();
}
