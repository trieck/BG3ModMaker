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
    void OnClose();
    void OnConvertLoca();
    void OnConvertLSF();
    void OnDeleteFile();
    void OnFileExit();
    void OnFileSave();
    void OnFileSaveAll();
    void OnFolderClose();
    void OnFolderOpen();
    void OnFolderPack();
    void OnIndex();
    void OnNewFile();
    void OnNewFileHere();
    void OnSearch();
    void OnSettings();
    void OnViewOutput();
    void OnViewStatusBar();

    LRESULT OnTVSelChanged(LPNMHDR pnmhdr);
    LRESULT OnTVBeginLabelEdit(LPNMHDR pnmhdr);
    LRESULT OnTVEndLabelEdit(LPNMHDR pnmhdr);
    LRESULT OnTVDelete(LPNMHDR pnmhdr);
    LRESULT OnTabActivated(LPNMHDR pnmhdr);
    LRESULT OnTabContextMenu(LPNMHDR pnmh);
    LRESULT OnRClick(LPNMHDR pnmh);
    LRESULT OnFileChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BEGIN_UPDATE_UI_MAP(MainFrame)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_OUTPUT, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_NEWFILEHERE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_MAKELSFHERE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_DELETE_FOLDER, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TREE_DELETE_FILE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TOOL_PACKAGE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TOOL_LSF, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_TOOL_LOCA, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE_ALL, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_SAVE, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_NEW, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_FILE_CLOSE, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(MainFrame)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CLOSE(OnClose)

        MESSAGE_HANDLER(WM_FILE_CHANGED, OnFileChanged)

        COMMAND_ID_HANDLER3(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER3(ID_FILE_CLOSE, OnFolderClose)
        COMMAND_ID_HANDLER3(ID_FILE_NEW, OnNewFile)
        COMMAND_ID_HANDLER3(ID_FILE_OPEN, OnFolderOpen)
        COMMAND_ID_HANDLER3(ID_FILE_SAVE, OnFileSave)
        COMMAND_ID_HANDLER3(ID_FILE_SAVE_ALL, OnFileSaveAll)
        COMMAND_ID_HANDLER3(ID_TOOL_INDEX, OnIndex)
        COMMAND_ID_HANDLER3(ID_TOOL_LOCA, OnConvertLoca)
        COMMAND_ID_HANDLER3(ID_TOOL_LSF, OnConvertLSF)
        COMMAND_ID_HANDLER3(ID_TOOL_PACKAGE, OnFolderPack)
        COMMAND_ID_HANDLER3(ID_TOOL_SEARCH, OnSearch)
        COMMAND_ID_HANDLER3(ID_TOOL_SETTINGS, OnSettings)
        COMMAND_ID_HANDLER3(ID_TREE_DELETE_FILE, OnDeleteFile)
        COMMAND_ID_HANDLER3(ID_TREE_DELETE_FOLDER, OnDeleteFile)
        COMMAND_ID_HANDLER3(ID_TREE_NEWFILEHERE, OnNewFileHere)
        COMMAND_ID_HANDLER3(ID_VIEW_OUTPUT, OnViewOutput)
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
    using FileCallback = std::function<void(const CStringW& filePath)>;

    BOOL IsFolderOpen() const;
    BOOL IsFolderSelected() const;
    BOOL IsXmlSelected() const;
    BOOL IsLSXSelected() const;
    void AddFile(const CString& filename);
    void IterateFiles(HTREEITEM hItem, const FileCallback& callback);
    void LogMessage(const CString& message);
    void PreloadTree();
    void PreloadTree(HTREEITEM hItem);
    void ProcessFileChange(UINT action, const CString& filename);
    void RemoveFile(const CString& filename);
    void RenameFile(const CString& oldname, const CString& newname);
    void UpdateEncodingStatus(FileEncoding encoding);
    void UpdateTitle();

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
