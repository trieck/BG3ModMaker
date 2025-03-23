#pragma once
#include <d2d1.h>

#include "Resource.h"
#include "RopeView.h"

class RopePad;

class RopeFrame : public CFrameWindowImpl<RopeFrame>,
                  public CMessageFilter,
                  public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(nullptr, IDR_MAINFRAME)

    BEGIN_UPDATE_UI_MAP(RopeFrame)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(RopeFrame)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER3(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER3(ID_HELP_ABOUT, OnAbout)
        CHAIN_MSG_MAP(CFrameWindowImpl)
    END_MSG_MAP()

    explicit RopeFrame(RopePad* pApp);
    BOOL PreTranslateMessage(MSG* pMsg) override;
    LRESULT OnCreate(LPCREATESTRUCT pcs);

    BOOL OnIdle() override;
    void OnClose();
    void OnSize(UINT nType, const CSize& size);
    void OnFileExit();
    void OnAbout();

    BOOL DefCreate();

private:
    void SetMenuBitmap(UINT nResourceID);
    void SetMenuBitmaps();

    RopePad* m_pApp;
    RopeView m_view;
    CStatusBarCtrl m_statusBar;
};
