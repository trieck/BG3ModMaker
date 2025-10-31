#pragma once

#include "FileViewContainer.h"
#include "ModelessDialog.h"
#include "PAKReader.h"
#include "resources/resource.h"

enum class NodeType : uint8_t
{
    Folder,
    File
};

struct NodeParam
{
    uint32_t startIndex; // inclusive range in sorted vector
    uint32_t endIndex; // exclusive range
    CString prefix; // path prefix
    NodeType type; // folder or file
};

class PakExplorerDlg : public ModelessDialog<PakExplorerDlg>,
                       public CDialogResize<PakExplorerDlg>,
                       public CUpdateUI<PakExplorerDlg>,
                       public CIdleHandler
{
public:
    enum { IDD = IDD_PAK_EXPLORER };

    BEGIN_MSG_MAP(PakExplorerDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_SIZE(OnSize)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_DESTROY(OnDestroy)
        NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDelete)
        NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnItemExpanding)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTVSelChanged)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(PakExplorerDlg)
    END_DLGRESIZE_MAP()

    BEGIN_UPDATE_UI_MAP(PakExplorerDlg)
    END_UPDATE_UI_MAP()

    BOOL OnIdle() override;
    void SetPAKReader(PAKReader&& reader);

private:
    void OnClose();
    void OnDestroy();
    void OnSize(UINT nType, CSize size);
    LRESULT OnDelete(LPNMHDR pnmh);
    LRESULT OnItemExpanding(LPNMHDR pnmh);
    LRESULT OnTVSelChanged(LPNMHDR pnmh);

    HTREEITEM Insert(HTREEITEM hParent, const CString& path, uint32_t startIndex, uint32_t endIndex,
                     const CString& prefix, int iImage, int cChildren);

    void Populate();
    void SetTitle();
    BOOL OnInitDialog(HWND, LPARAM);
    void ExpandFolders(const CTreeItem& folder);

    CSplitterWindow m_splitter;
    CTreeViewCtrlEx m_treeView;
    FileViewContainer m_fileView;
    PAKReader m_pakReader;
    CImageList m_imageList;

    int m_marginLeft = 0;
    int m_marginTop = 0;
    int m_marginRight = 0;
    int m_marginBottom = 0;
    int m_nPage = 0;
};
