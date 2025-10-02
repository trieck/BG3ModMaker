#include "stdafx.h"
#include "IconDlg.h"
#include "Iconizer.h"
#include "Settings.h"
#include "StringHelper.h"

IconDlg::IconDlg(const CString& iconID) : m_iconID(iconID)
{
    Settings settings;
    auto dbPath = settings.GetString("Settings", "IconPath");

    Iconizer iconizer;
    iconizer.open(StringHelper::toUTF8(dbPath).GetString());

    m_image = iconizer.getIcon(StringHelper::toUTF8(iconID).GetString());
}

BOOL IconDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    auto wndFrame = GetDlgItem(IDC_ST_ICON);
    ATLASSERT(wndFrame.IsWindow());

    CRect rcDlg;
    GetClientRect(&rcDlg);

    CRect rcFrame;
    wndFrame.GetWindowRect(&rcFrame);
    ScreenToClient(&rcFrame);

    m_marginLeft = rcFrame.left;
    m_marginTop = rcFrame.top;
    m_marginRight = rcDlg.right - rcFrame.right;
    m_marginBottom = rcDlg.bottom - rcFrame.bottom;

    wndFrame.DestroyWindow(); // Needed only for margin calculation

    m_iconView.Create(*this, rcFrame,
                      WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    ATLASSERT(m_iconView.IsWindow());

    m_iconView.LoadImage(m_image);

    SetWindowText(m_iconID);

    DlgResize_Init();
    CenterWindow(GetParent());

    return TRUE; // Let the system set the focus
}

void IconDlg::OnClose()
{
    Destroy();
}

void IconDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    if (m_iconView.IsWindow()) {
        CRect rc;
        rc.left = m_marginLeft;
        rc.top = m_marginTop;
        rc.right = size.cx - m_marginRight;
        rc.bottom = size.cy - m_marginBottom;
        m_iconView.MoveWindow(&rc);
    }
}
