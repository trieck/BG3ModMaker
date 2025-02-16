#pragma once

class FileView : public CWindowImpl<FileView, CRichEditCtrl>
{
public:
    BEGIN_MSG_MAP(FileView)
        MSG_WM_CREATE(OnCreate)
        MESSAGE_HANDLER(WM_SELCHANGED, OnSelChanged)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(NULL, CRichEditCtrl::GetWndClassName())

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnSelChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
};

