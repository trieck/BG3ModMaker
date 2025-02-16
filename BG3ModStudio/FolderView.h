#pragma once

class FolderView : public CWindowImpl<FolderView, CTreeViewCtrlEx>
{
public:
    DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())

    BEGIN_MSG_MAP_EX(FolderView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnItemExpanding)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnDelete)
        DEFAULT_REFLECTION_HANDLER()
        END_MSG_MAP()

    FolderView() = default;
    LRESULT OnCreate(LPCREATESTRUCT /*pcs*/);
    void OnDestroy();
    LRESULT OnItemExpanding(LPNMHDR pnmh);
    LRESULT OnDelete(LPNMHDR pnmh);
};

