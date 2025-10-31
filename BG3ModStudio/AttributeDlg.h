#pragma once

#include "resources/resource.h"
#include <nlohmann/json.hpp>

class AttributeDlg : public CDialogImpl<AttributeDlg>,
                     public CDialogResize<AttributeDlg>
{
public:
    enum { IDD = IDD_ATTRIBUTES };

    BEGIN_MSG_MAP(AttributeDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_CONTEXTMENU(OnContextMenu)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
        NOTIFY_HANDLER(IDC_LST_ATTRIBUTES, NM_DBLCLK, OnDoubleClick)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(AttributeDlg)
        DLGRESIZE_CONTROL(IDC_LST_ATTRIBUTES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    void SetAttributeJson(const std::string& json);

private:
    void AutoAdjustColumns();

    BOOL OnInitDialog(HWND /* hWnd */, LPARAM /*lParam*/);
    LRESULT OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, const CPoint& /*pt*/);
    void OnClose();
    void OnContextMenu(const CWindow& wnd, const CPoint& point);
    void OnSize(UINT /*uMsg*/, const CSize& size);
    void ViewValue();

    CListViewCtrl m_list;
    CFont m_font;
    int m_fontSize = 8;
    nlohmann::json m_attributes;
};
