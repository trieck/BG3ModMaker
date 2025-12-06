#pragma once

#include "Cataloger.h"
#include "ModelessDialog.h"
#include "resources/resource.h"

#include <nlohmann/json.hpp>

class GameObjectDlg : public ModelessDialog<GameObjectDlg>,
                      public CDialogResize<GameObjectDlg>,
                      public CUpdateUI<GameObjectDlg>,
                      public CIdleHandler
{
public:
    enum { IDD = IDD_GAMEOBJECT };

    BEGIN_MSG_MAP(GameObjectDlg)
        COMMAND_HANDLER3(IDC_E_QUERY, EN_CHANGE, OnQueryChange)
        COMMAND_ID_HANDLER3(IDC_B_SEARCH, OnSearch)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_CONTEXTMENU(OnContextMenu)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_SIZE(OnSize)
        NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDelete)
        NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnItemExpanding)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTVSelChanged)
        NOTIFY_HANDLER(ID_ATTRIBUTE_LIST, NM_DBLCLK, OnDoubleClick)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(GameObjectDlg)
    END_DLGRESIZE_MAP()

    BEGIN_UPDATE_UI_MAP(GameObjectDlg)
    END_UPDATE_UI_MAP()

    BOOL OnIdle() override;

private:
    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    CString GetAttribute(const nlohmann::json& obj, const CString& key);
    HTREEITEM GetChild(HTREEITEM hParent, const CString& uuid);
    HTREEITEM GetTypeRoot(const CString& type);
    HTREEITEM InsertHierarchy(HTREEITEM hRoot, const CString& uuid);
    HTREEITEM InsertNode(HTREEITEM hParent, const CString& key, LPARAM lparam = 0);
    HTREEITEM InsertUUID(HTREEITEM hParent, const CString& uuid);
    LRESULT OnDelete(LPNMHDR pnmh);
    LRESULT OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnItemExpanding(LPNMHDR pnmh);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, const CPoint& /*pt*/);
    LRESULT OnTVSelChanged(LPNMHDR pnmh);
    void AutoAdjustAttributes();
    void DeleteAll();
    void ExpandNode(const CTreeItem& node);
    void OnClose();
    void OnContextAttributes(const CPoint& point);
    void OnContextMenu(const CWindow& wnd, const CPoint& point);
    void OnDestroy();
    void OnQueryChange();
    void OnSearch();
    void OnSize(UINT /*uMsg*/, const CSize& size);
    void Populate();
    void PopulateDoc(const std::pair<const std::string, nlohmann::json>& doc);
    void PopulateDocs(const std::unordered_map<std::string, nlohmann::json>& docs);
    void PopulateTypes();
    void PopulateUUIDs(const std::unordered_set<std::string>& uuids);
    void ViewValue();

    Cataloger m_cataloger;
    CFont m_treeFont, m_attributeFont;
    CImageList m_imageList;
    CListViewCtrl m_attributes;
    CSplitterWindow m_splitter;
    CString m_dbPath, m_indexPath;
    CTreeViewCtrlEx m_tree;

    bool m_deleting = false;
    int m_fontSize = 8;
    int m_marginBottom = 0;
    int m_marginLeft = 0;
    int m_marginRight = 0;
    int m_marginTop = 0;
    int m_nPage = 0;
};
