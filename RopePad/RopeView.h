#pragma once

#include <d2d1.h>
#include <dwrite.h>

class RopePad;

class RopeView : public CWindowImpl<RopeView>
{
public:
    DECLARE_WND_CLASS("RopeView")

    BEGIN_MSG_MAP(RopeView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_DIRECT2DPAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_CHAR(OnChar)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
    END_MSG_MAP()

    explicit RopeView(RopePad* pApp);

    BOOL OnEraseBkgnd(const CDCHandle& dc);
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, const CPoint& pt);
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnDestroy();
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnPaint();
    void OnSize(UINT nType, CSize size);
    void OnTimer(UINT_PTR nIDEvent);
    void GetViewRect(D2D1_RECT_F& rc) const;
    FLOAT GetViewHeight() const;

private:
    void InsertChar(UINT nChar);
    HRESULT CreateDevResources();
    HRESULT DrawD2DText();
    HRESULT UpdateLayout();
    void DiscardDevResources();
    void UpdateCaretPos();
    FLOAT LineHeight() const;

    RopePad* m_pApp;
    int32_t m_insertPos = 0;
    float m_scrollY = 0;
    float m_caretY = 0;
    float m_caretX = 0;
    CStringW m_text;
    BOOL m_showCaret = FALSE;
    DWRITE_TEXT_METRICS m_textMetrics{};

    CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
    CComPtr<ID2D1SolidColorBrush> m_pBkgndBrush;
    CComPtr<ID2D1SolidColorBrush> m_pTextBrush;
    CComPtr<IDWriteTextFormat> m_pTextFormat;
    CComPtr<IDWriteTextLayout> m_pTextLayout;
};
