#pragma once
#include "ImageView.h"
#include "resources/resource.h"

class IconDlg : public CDialogImpl <IconDlg>,
                public CDialogResize<IconDlg>
{
public:
    enum { IDD = IDD_ICON };

    explicit IconDlg(const CString& iconID);

    BEGIN_MSG_MAP(IconDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(CDialogResize)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(IconDlg)
    END_DLGRESIZE_MAP()

    BOOL HasImage() const;

private:
    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
    void OnClose();
    void OnSize(UINT /*uMsg*/, const CSize& size);

    DirectX::ScratchImage m_image;
    ImageView m_iconView;
    CString m_iconID;

    int m_marginLeft = 0;
    int m_marginTop = 0;
    int m_marginRight = 0;
    int m_marginBottom = 0;
    int m_nPage = 0;
};
