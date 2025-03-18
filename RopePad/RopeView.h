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
    END_MSG_MAP()

    explicit RopeView(RopePad* pApp);

    BOOL OnEraseBkgnd(const CDCHandle& dc);
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnDestroy();
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnPaint();
    void OnSize(UINT nType, CSize size);
    void OnTimer(UINT_PTR nIDEvent);

private:
    void DiscardDevResources();
    HRESULT CreateDevResources();
    HRESULT DrawD2DText();
    void UpdateCaretPos();
    void UpdateCaretPosX();
    void UpdateCaretPosY();

    RopePad* m_pApp;
    CStringW m_text;
    float m_caretX = 0;
    float m_caretY = 0;
    int32_t m_insertPos = 0;
    int32_t m_lineNum = 0;
    BOOL m_showCaret = FALSE;

    CComPtr<ID2D1Factory> m_pD2DFactory;
    CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
    CComPtr<ID2D1SolidColorBrush> m_pBkgndBrush;
    CComPtr<ID2D1SolidColorBrush> m_pTextBrush;
    CComPtr<IDWriteTextFormat> m_pTextFormat;
};

