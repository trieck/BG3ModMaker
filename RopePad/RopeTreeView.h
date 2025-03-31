#pragma once

#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1svg.h>

#include "RopeLayout.h"

class RopePad;

class RopeTreeView : public CWindowImpl<RopeTreeView>, public CScrollImpl<RopeTreeView>
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
        CHAIN_MSG_MAP(CScrollImpl)
    END_MSG_MAP()

    explicit RopeTreeView(RopePad* pApp);

    BOOL OnEraseBkgnd(const CDCHandle& dc);
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, const CPoint& pt);
    void OnDestroy();
    void OnPaint();
    void OnSize(UINT nType, CSize size);

private:
    HRESULT CreateDevResources();
    HRESULT CreateRopeLayout();
    void DiscardDevResources();

    CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
    CComPtr<ID2D1SolidColorBrush> m_pBkgndBrush;
    CComPtr<ID2D1DeviceContext7> m_pDeviceContext;
    CComPtr<IDWriteFactory> m_pDWriteFactory;
    CComPtr<ID2D1CommandList> m_pCommandList;

    RopePad* m_pApp;
    RopeLayout m_layout;
    float m_zoom;
};

