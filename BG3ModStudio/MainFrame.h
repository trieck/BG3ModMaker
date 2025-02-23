#pragma once

#include "FileViews.h"
#include "FolderView.h"
#include "resources/resource.h"
#include "resources/ribbon.h"

class MainFrame : public CRibbonFrameWindowImpl<MainFrame>,
    public CMessageFilter,
    public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(nullptr, IDR_MAINFRAME)

    BOOL PreTranslateMessage(MSG* pMsg) override;
    BOOL OnIdle() override;
    BOOL DefCreate();

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnViewStatusBar();
    LRESULT OnFileOpen();
    LRESULT OnFileSave() const;
    LRESULT OnFileSaveAll() const;
    LRESULT OnFileExit();

    LRESULT OnTVSelChanged(LPNMHDR pnmhdr);
    LRESULT OnTabActivated(LPNMHDR pnmhdr);
    LRESULT OnTabContextMenu(LPNMHDR pnmh);

    BEGIN_RIBBON_CONTROL_MAP(MainFrame)
    END_RIBBON_CONTROL_MAP()

    BEGIN_UPDATE_UI_MAP(MainFrame)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE_ALL, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(MainFrame)
        MSG_WM_CREATE(OnCreate)
        COMMAND_ID_HANDLER2(ID_VIEW_STATUS_BAR, OnViewStatusBar)
        COMMAND_ID_HANDLER2(ID_FILE_OPEN, OnFileOpen)
        COMMAND_ID_HANDLER2(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER2(ID_FILE_SAVE, OnFileSave)
        COMMAND_ID_HANDLER2(ID_FILE_SAVE_ALL, OnFileSaveAll)
        REFLECT_NOTIFY_CODE(TVN_ITEMEXPANDING)
        REFLECT_NOTIFY_CODE(TVN_DELETEITEM)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTVSelChanged)
        NOTIFY_CODE_HANDLER_EX(TBVN_PAGEACTIVATED, OnTabActivated)
        NOTIFY_CODE_HANDLER_EX(TBVN_CONTEXTMENU, OnTabContextMenu)
        CHAIN_MSG_MAP(CRibbonFrameWindowImpl)
    END_MSG_MAP()

private:
    void UpdateEncodingStatus(FileEncoding encoding);

    CSplitterWindow m_splitter;
    CCommandBarCtrl m_cmdBar;
    CStatusBarCtrl m_statusBar;
    FolderView m_folderView{};
    FilesView m_filesView;
    CIcon m_bom, m_nobom;
};

