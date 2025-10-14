#pragma once

#include "FileViews.h"
#include "FolderView.h"
#include "PIDL.h"
#include "resources/resource.h"
#include "resources/ribbon.h"
#include "ShellNotifyRegistrar.h"

#define WM_FILE_CHANGED (WM_APP + 1)

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

    BEGIN_UPDATE_UI_MAP(MainFrame)
        UPDATE_ELEMENT(ID_FILE_CLOSE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_NEW, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE_ALL, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TOOL_BG3, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TOOL_LOCA, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TOOL_LSF, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TOOL_PACKAGE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_DELETE_FILE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_DELETE_FOLDER, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_MAKELSFHERE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_NEWFILEHERE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_NEWFOLDERHERE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_RENAME_FILE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(MainFrame)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_COPYDATA(OnCopyData)

        MESSAGE_HANDLER2(WM_FILE_CHANGED, OnFileChanged)
        COMMAND_ID_HANDLER3(ID_APP_ABOUT, OnFileAbout)
        COMMAND_ID_HANDLER3(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER3(ID_FILE_CLOSE, OnFolderClose)
        COMMAND_ID_HANDLER3(ID_FILE_NEW, OnNewFile)
        COMMAND_ID_HANDLER3(ID_FILE_OPEN, OnFolderOpen)
        COMMAND_ID_HANDLER3(ID_FILE_PAK_OPEN, OnPakOpen)
        COMMAND_ID_HANDLER3(ID_FILE_SAVE, OnFileSave)
        COMMAND_ID_HANDLER3(ID_FILE_SAVE_ALL, OnFileSaveAll)
        COMMAND_ID_HANDLER3(ID_TOOL_BG3, OnLaunchGame)
        COMMAND_ID_HANDLER3(ID_TOOL_GAMEOBJECT, OnGameObject)
        COMMAND_ID_HANDLER3(ID_TOOL_ICON_EXPLORER, OnIconExplorer)
        COMMAND_ID_HANDLER3(ID_TOOL_INDEX, OnIndex)
        COMMAND_ID_HANDLER3(ID_TOOL_LOCA, OnConvertLoca)
        COMMAND_ID_HANDLER3(ID_TOOL_LSF, OnConvertLSF)
        COMMAND_ID_HANDLER3(ID_TOOL_PACKAGE, OnFolderPack)
        COMMAND_ID_HANDLER3(ID_TOOL_SEARCH, OnSearch)
        COMMAND_ID_HANDLER3(ID_TOOL_SETTINGS, OnSettings)
        COMMAND_ID_HANDLER3(ID_TOOL_UUID, OnUUID)
        COMMAND_ID_HANDLER3(ID_TREE_DELETE_FILE, OnDeleteFile)
        COMMAND_ID_HANDLER3(ID_TREE_DELETE_FOLDER, OnDeleteFile)
        COMMAND_ID_HANDLER3(ID_TREE_NEWFILEHERE, OnNewFileHere)
        COMMAND_ID_HANDLER3(ID_TREE_NEWFOLDERHERE, OnNewFolderHere)
        COMMAND_ID_HANDLER3(ID_TREE_RENAME_FILE, OnRenameFile)
        COMMAND_ID_HANDLER3(ID_VIEW_STATUS_BAR, OnViewStatusBar)

        REFLECT_NOTIFY_CODE(TVN_ITEMEXPANDING)
        NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnTVDelete)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTVSelChanged)
        NOTIFY_CODE_HANDLER_EX(TVN_BEGINLABELEDIT, OnTVBeginLabelEdit)
        NOTIFY_CODE_HANDLER_EX(TVN_ENDLABELEDIT, OnTVEndLabelEdit)
        NOTIFY_CODE_HANDLER_EX(TBVN_PAGEACTIVATED, OnTabActivated)
        NOTIFY_CODE_HANDLER_EX(TBVN_CONTEXTMENU, OnTabContextMenu)
        NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnRClick)

        CHAIN_MSG_MAP(CRibbonFrameWindowImpl)
    END_MSG_MAP()

private:
    LRESULT OnCopyData(HWND hWnd, PCOPYDATASTRUCT pcds);
    void OnClose();
    void OnConvertLoca();
    void OnConvertLSF();
    void OnDeleteFile();
    void OnFileAbout();
    void OnFileExit();
    void OnFileSave();
    void OnFileSaveAll();
    void OnFolderClose();
    void OnFolderOpen();
    void OnFolderPack();
    void OnGameObject();
    void OnIconExplorer();
    void OnIndex();
    void OnLaunchGame();
    void OnNewFile();
    void OnNewFileHere();
    void OnNewFolderHere();
    void OnPakOpen();
    void OnRenameFile();
    void OnSearch();
    void OnSettings();
    void OnUUID();
    void OnViewStatusBar();

    LRESULT OnRClick(LPNMHDR pnmh);
    LRESULT OnTabActivated(LPNMHDR pnmhdr);
    LRESULT OnTabContextMenu(LPNMHDR pnmh);
    LRESULT OnTVBeginLabelEdit(LPNMHDR pnmhdr);
    LRESULT OnTVDelete(LPNMHDR pnmhdr);
    LRESULT OnTVEndLabelEdit(LPNMHDR pnmhdr);
    LRESULT OnTVSelChanged(LPNMHDR pnmhdr);
    void OnFileChanged(WPARAM wParam, LPARAM lParam);

    using FileCallback = std::function<void(const CStringW& filePath)>;

    BOOL IsFileSelected() const;
    BOOL IsFolderOpen() const;
    BOOL IsFolderSelected() const;
    BOOL IsLSXSelected() const;
    BOOL IsXmlSelected() const;
    BOOL NewFile(LPNMTVDISPINFO pDispInfo);
    BOOL RenameFile(LPNMTVDISPINFO pDispInfo);
    void AddFile(const CString& filename);
    void IterateFiles(HTREEITEM hItem, const FileCallback& callback);
    void ProcessFileChange(LONG event, PIDLIST_ABSOLUTE* pidls);
    void RemoveFile(const CString& filename);
    void RenameFile(const CString& oldname, const CString& newname);
    void UpdateEncodingStatus(FileEncoding encoding);
    void UpdateTitle();

    CCommandBarCtrl m_cmdBar;
    CIcon m_bom, m_nobom;
    CSplitterWindow m_splitter;
    CStatusBarCtrl m_statusBar;
    FilesView m_filesView{};
    FolderView m_folderView{};
    PIDL m_rootPIDL;
    ShellNotifyRegistration m_notify;
};
