#include "stdafx.h"
#include "Rope.h"
#include "RopePad.h"
#include "RopeTreeView.h"
#include "ScopeGuard.h"
#include "SVG.h"

RopeTreeView::RopeTreeView(RopePad* pApp) : m_pApp(pApp), m_zoom(1.0f)
{
}

BOOL RopeTreeView::OnEraseBkgnd(const CDCHandle& dc)
{
    return TRUE;
}

LRESULT RopeTreeView::OnCreate(LPCREATESTRUCT pcs)
{
    auto hr = CreateDevResources();
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create device resources\n"));
        return -1; // Fail the creation
    }

    return 0;
}

LRESULT RopeTreeView::OnMouseWheel(UINT nFlags, short zDelta, const CPoint& pt)
{
    if (nFlags & MK_CONTROL) {
        m_zoom += zDelta > 0 ? 0.1f : -0.1f;
        m_zoom = std::clamp(m_zoom, 0.1f, 10.0f);

        if (m_pSvgDoc) {
            D2D1_SIZE_F sizeF = m_pSvgDoc->GetViewportSize();
            SetScrollSize(static_cast<int>(sizeF.width * m_zoom), static_cast<int>(sizeF.height * m_zoom));
        }
        Invalidate();
    } else {
        if (zDelta > 0) {
            ScrollPageUp();
        } else {
            ScrollPageDown();
        }
    }

    return 0;
}

void RopeTreeView::OnDestroy()
{
    DiscardDevResources();
}

void RopeTreeView::OnPaint()
{
    PAINTSTRUCT ps;
    ScopeGuardSimple guard(
        [&] { BeginPaint(&ps); },
        [&] { EndPaint(&ps); }
    );

    if (!m_pRenderTarget || !m_pBkgndBrush || !m_pDeviceContext || !m_pSvgDoc) {
        return; // Render target not created
    }

    CRect rc;
    GetClientRect(&rc);

    CPoint ptOffset;
    GetScrollOffset(ptOffset);

    D2D1_SIZE_F offset = { static_cast<FLOAT>(-ptOffset.x), static_cast<FLOAT>(-ptOffset.y) };

    D2D1_RECT_F clipRect = D2D1::RectF(
        static_cast<FLOAT>(rc.left) / m_zoom - offset.width,
        static_cast<FLOAT>(rc.top) / m_zoom - offset.height,
        static_cast<FLOAT>(rc.right) / m_zoom - offset.width,
        static_cast<FLOAT>(rc.bottom) / m_zoom - offset.height
    );

    m_pRenderTarget->BeginDraw();

    D2D1::Matrix3x2F transform =
        D2D1::Matrix3x2F::Translation(offset.width, offset.height) *
        D2D1::Matrix3x2F::Scale(m_zoom, m_zoom);

    m_pRenderTarget->SetTransform(transform);

    m_pRenderTarget->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    m_pRenderTarget->Clear(D2D1::ColorF(0x600000));
    m_pRenderTarget->FillRectangle(clipRect, m_pBkgndBrush);

    m_pDeviceContext->DrawSvgDocument(m_pSvgDoc);

    m_pRenderTarget->PopAxisAlignedClip();

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    auto hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    } else if (FAILED(hr)) {
        ATLTRACE(_T("Failed to end draw on render target\n"));
    }
}

void RopeTreeView::OnSize(UINT nType, CSize size)
{
    if (!m_pRenderTarget || !m_pSvgDoc) {
        return;
    }

    auto hr = m_pRenderTarget->Resize(D2D1::SizeU(size.cx, size.cy));
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to resize render target\n"));
    }

    DoSize(size.cx, size.cy);

    D2D1_SIZE_F sizeF = m_pSvgDoc->GetViewportSize();

    SetScrollSize(static_cast<int>(sizeF.width * m_zoom), static_cast<int>(sizeF.height * m_zoom));
}

void LayoutRope(Rope::PNode node, int depth, float& nextX, SVG& svg, float parentX = -1, float parentY = -1)
{
    if (!node) {
        return;
    }

    constexpr auto levelSpacingX = 100.0f;
    constexpr auto levelSpacingY = 100.0f;
    constexpr auto nodeRadius = 30.0f;
    constexpr auto topPadding = nodeRadius + 10.0f;

    auto x = nextX * levelSpacingX;
    auto y = static_cast<float>(depth) * levelSpacingY + topPadding;

    nextX += 1.0f;

    constexpr auto radius = 30.0f;
    if (parentX >= 0 && parentY >= 0) {
        svg.EdgeLine(parentX, parentY, x, y, radius);
    }

    COLORREF color = node->value.isLeaf() ? RGB(0, 0x80, 0) : RGB(0x80, 0, 0);
    svg.Circle(x, y, radius, color);

    LayoutRope(node->left, depth + 1, nextX, svg, x, y);
    LayoutRope(node->right, depth + 1, nextX, svg, x, y);
}

HRESULT RopeTreeView::BuildSvgDoc()
{
    if (!m_pDeviceContext) {
        return E_FAIL;
    }

    m_pSvgDoc.Release();

    SVG svg;
    Rope rope(3);
    rope.insert(0, "ABCDEFGHI");

    svg.Begin(800, 600);
    float nextX = 1.f;
    LayoutRope(rope.root(), 0, nextX, svg);
    svg.End();

    auto hr = svg.Make(m_pDeviceContext, &m_pSvgDoc);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create SVG document\n"));
        return hr;
    }

    return S_OK;
}

HRESULT RopeTreeView::CreateDevResources()
{
    auto pFactory = m_pApp->GetD2DFactory();
    if (!pFactory) {
        ATLTRACE(_T("Failed to get D2D factory\n"));
        return E_FAIL;
    }

    DiscardDevResources();

    CRect rc;
    GetClientRect(&rc);

    auto hwndProps = D2D1::HwndRenderTargetProperties(m_hWnd, D2D1::SizeU(rc.right, rc.bottom));
    auto renderTargetProps = D2D1::RenderTargetProperties();

    auto hr = pFactory->CreateHwndRenderTarget(renderTargetProps, hwndProps, &m_pRenderTarget);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create Direct2D render target\n"));
        return hr;
    }

    hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cornsilk), &m_pBkgndBrush);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create solid color brush\n"));
        m_pRenderTarget.Release();
        return hr;
    }

    hr = m_pRenderTarget->QueryInterface(&m_pDeviceContext);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to get device context\n"));
        return hr;
    }

    hr = BuildSvgDoc();
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to build SVG document\n"));
        return hr;
    }
    return hr;
}

void RopeTreeView::DiscardDevResources()
{
    if (m_pSvgDoc) {
        m_pSvgDoc.Release();
    }

    if (m_pDeviceContext) {
        m_pDeviceContext.Release();
    }

    if (m_pBkgndBrush) {
        m_pBkgndBrush.Release();
    }

    if (m_pRenderTarget) {
        m_pRenderTarget.Release();
    }
}
