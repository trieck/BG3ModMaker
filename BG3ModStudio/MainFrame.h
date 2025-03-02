#pragma once

#include "FileViews.h"
#include "FolderMonitor.h"
#include "FolderView.h"
#include "OutputWindow.h"
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
    void OnViewStatusBar();
    void OnViewOutput();
    void OnFolderOpen();
    void OnFolderClose();
    void OnFolderPack();
    void OnFileSave();
    void OnFileSaveAll();
    void OnFileExit();
    void OnDeleteFile();
    void OnNewFile();
    void OnNewFileHere();
    void OnClose();

    LRESULT OnTVSelChanged(LPNMHDR pnmhdr);
    LRESULT OnTVDelete(LPNMHDR pnmhdr);
    LRESULT OnTabActivated(LPNMHDR pnmhdr);
    LRESULT OnTabContextMenu(LPNMHDR pnmh);
    LRESULT OnRClick(LPNMHDR pnmh);
    LRESULT OnFileChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BEGIN_RIBBON_CONTROL_MAP(MainFrame)
    END_RIBBON_CONTROL_MAP()

    BEGIN_UPDATE_UI_MAP(MainFrame)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_OUTPUT, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_NEW, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_CLOSE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_PACKAGE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE_ALL, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(MainFrame)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CLOSE(OnClose)
        MESSAGE_HANDLER(WM_FILE_CHANGED, OnFileChanged)

        COMMAND_ID_HANDLER3(ID_VIEW_STATUS_BAR, OnViewStatusBar)
        COMMAND_ID_HANDLER3(ID_VIEW_OUTPUT, OnViewOutput)
        COMMAND_ID_HANDLER3(ID_FILE_NEW, OnNewFile)
        COMMAND_ID_HANDLER3(ID_FILE_OPEN, OnFolderOpen)
        COMMAND_ID_HANDLER3(ID_FILE_CLOSE, OnFolderClose)
        COMMAND_ID_HANDLER3(ID_FILE_PACKAGE, OnFolderPack)
        COMMAND_ID_HANDLER3(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER3(ID_FILE_SAVE, OnFileSave)
        COMMAND_ID_HANDLER3(ID_FILE_SAVE_ALL, OnFileSaveAll)
        COMMAND_ID_HANDLER3(ID_TREE_DELETE_FILE, OnDeleteFile)
        COMMAND_ID_HANDLER3(ID_TREE_NEWFILEHERE, OnNewFileHere)

        REFLECT_NOTIFY_CODE(TVN_ITEMEXPANDING)
        NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnTVDelete)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTVSelChanged)
        NOTIFY_CODE_HANDLER_EX(TBVN_PAGEACTIVATED, OnTabActivated)
        NOTIFY_CODE_HANDLER_EX(TBVN_CONTEXTMENU, OnTabContextMenu)
        NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnRClick)
        CHAIN_MSG_MAP(CRibbonFrameWindowImpl)
    END_MSG_MAP()

private:
    using FileCallback = std::function<void(const CStringW& filePath)>;

    void ProcessFileChange(UINT action, const CString& filename);
    void AddFile(const CString& filename);
    void RemoveFile(const CString& filename);
    void RenameFile(const CString& oldname, const CString& newname);
    BOOL FolderIsOpen() const;
    void UpdateTitle();
    void UpdateEncodingStatus(FileEncoding encoding);
    void IterateFiles(HTREEITEM hItem, const FileCallback& callback);
    void PreloadTree();
    void PreloadTree(HTREEITEM hItem);
    void LogMessage(const CString& message);

    CHorSplitterWindow m_hSplitter;
    CSplitterWindow m_vSplitter;
    CCommandBarCtrl m_cmdBar;
    CStatusBarCtrl m_statusBar;
    FolderView m_folderView{};
    FilesView m_filesView{};
    OutputWindow m_output{};
    CIcon m_bom, m_nobom;
    FolderMonitor::Ptr m_folderMonitor;
    std::stack<std::wstring> m_oldnames; // old file names for renaming
};
