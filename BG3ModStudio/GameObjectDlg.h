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
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
        NOTIFY_HANDLER(ID_ATTRIBUTE_LIST, NM_DBLCLK, OnDoubleClick)
        NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDelete)
        NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTVSelChanged)
        NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnItemExpanding)
        MSG_WM_CONTEXTMENU(OnContextMenu)
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
    HTREEITEM InsertNode(HTREEITEM hParent, const CString& key, LPARAM lparam = 0);
    LRESULT OnDelete(LPNMHDR pnmh);
    LRESULT OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnItemExpanding(LPNMHDR pnmh);
    LRESULT OnTVSelChanged(LPNMHDR pnmh);
    void AutoAdjustAttributes();
    void ExpandNode(const CTreeItem& node);
    void OnClose();
    void OnContextAttributes(const CPoint& point);
    void OnContextMenu(const CWindow& wnd, const CPoint& point);
    void OnDestroy();
    void OnSize(UINT /*uMsg*/, const CSize& size);
    void Populate();
    void PopulateTypes();
    void ViewValue();

    Cataloger m_cataloger;
    CFont m_font;
    CImageList m_imageList;
    CListViewCtrl m_attributes;
    CSplitterWindow m_splitter;
    CString m_dbPath;
    CTreeViewCtrlEx m_tree;

    int m_marginLeft = 0;
    int m_marginTop = 0;
    int m_marginRight = 0;
    int m_marginBottom = 0;
    int m_nPage = 0;
};
