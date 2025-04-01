#pragma once

#include "resource.h"
#include "RopeTreeView.h"

class RopePad;

class RopeTreeFrame : public CFrameWindowImpl<RopeTreeFrame>,
                      public CMessageFilter,
                      public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(nullptr, IDR_TREEFRAME)

    BEGIN_UPDATE_UI_MAP(RopeTreeFrame)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(RopeTreeFrame)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_UPDATELAYOUT(OnUpdateLayout)
        CHAIN_MSG_MAP(CFrameWindowImpl)
    END_MSG_MAP()

    explicit RopeTreeFrame(RopePad* pApp);
    BOOL PreTranslateMessage(MSG* pMsg) override;

    BOOL OnIdle() override;
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    void OnClose();
    void OnDestroy();
    void OnSize(UINT nType, CSize size);
    void OnUpdateLayout();

    BOOL DefCreate();

private:
    RopeTreeView m_view;
    CStatusBarCtrl m_statusBar;
    RopePad* m_pApp;
};
