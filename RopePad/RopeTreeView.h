#pragma once

#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1svg.h>

class RopePad;

class RopeTreeView : public CWindowImpl<RopeTreeView>
{
public:
    DECLARE_WND_CLASS("RopeTreeView")

    BEGIN_MSG_MAP(RopeTreeView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DIRECT2DPAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
    END_MSG_MAP()

    explicit RopeTreeView(RopePad* pApp);

    BOOL OnEraseBkgnd(const CDCHandle& dc);
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, const CPoint& pt);
    void OnDestroy();
    void OnPaint();
    void OnSize(UINT nType, CSize size);

private:
    HRESULT BuildSvgDoc();
    HRESULT CreateDevResources();
    void DiscardDevResources();

    RopePad* m_pApp;
    CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
    CComPtr<ID2D1SolidColorBrush> m_pBkgndBrush;
    CComPtr<ID2D1DeviceContext7> m_pDeviceContext;
    CComPtr<ID2D1SvgDocument> m_pSvgDoc;
};

