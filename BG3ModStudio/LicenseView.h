#pragma once

#include "Direct2D.h"

class LicenseView : public CWindowImpl<LicenseView>
{
public:
    using Base = CWindowImpl;

    DECLARE_WND_CLASS_EX(L"BG3_LicenseView", CS_VREDRAW | CS_HREDRAW, -1)

    BEGIN_MSG_MAP(LicenseView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PAINT2(OnPaint)
        MSG_WM_VSCROLL(OnVScroll)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
    END_MSG_MAP()

    LicenseView() = default;

private:
    void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
    void OnSize(UINT nType, CSize size);
    void OnDestroy();
    LRESULT OnPaint(const CPaintDC& dc);
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    HRESULT CreateDevResources();
    void DiscardDevResources();
    void RecalcLayout();
    void UpdateScrollBars();

    CComPtr<IDWriteFactory> m_dwrite;
    CComPtr<IDWriteTextFormat> m_format;
    CComPtr<IDWriteTextLayout> m_layout;
    CComPtr<ID2D1Factory> m_d2d;
    CComPtr<ID2D1HwndRenderTarget> m_rt;
    CComPtr<ID2D1SolidColorBrush> m_textBrush;

    CString m_text;
    uint32_t m_yScroll = 0;
    float m_docHeight = 0.0f;
    float m_lineHeight = 0.0f;
};
