
#include "stdafx.h"
#include "MainFrame.h"
#include "BG3ModStudio.h"
#include "MessageLoopEx.h"

CAppModule _Module;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nShowCmd)
{
    BG3ModStudio app;
    if (!app.init())     {
        ATLTRACE(_T("Failed to initialize application\n"));
        return 1;
    }

    return BG3ModStudio::run(hInstance, lpCmdLine, nShowCmd);
}

BG3ModStudio::~BG3ModStudio()
{
    if (m_hInstRich != nullptr) {
        FreeLibrary(m_hInstRich);
    }
    CoUninitialize();
}

bool BG3ModStudio::init()
{
    auto hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        ATLTRACE(_T("Cannot initialize COM libraries.\n"));
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

    m_hInstRich = LoadLibrary(CRichEditCtrl::GetLibraryName());
    if (m_hInstRich == nullptr) {
        ATLTRACE(_T("Unable to load rich edit library.\n"));
        return FALSE;
    }

    return true;
}

int BG3ModStudio::run(HINSTANCE hInstance, LPWSTR lpCmdLine, int nShowCmd)
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

    wndMain.ShowWindow(nShowCmd);

    auto result = theLoop.Run();

    _Module.RemoveMessageLoop();

    return result;
}
