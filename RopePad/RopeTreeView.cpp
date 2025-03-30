#include "stdafx.h"
#include "Rope.h"
#include "RopePad.h"
#include "RopeTreeView.h"
#include "ScopeGuard.h"
#include "SVG.h"

RopeTreeView::RopeTreeView(RopePad* pApp) : m_pApp(pApp)
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

    D2D1_RECT_F d2rc;
    d2rc.left = static_cast<FLOAT>(rc.left);
    d2rc.top = static_cast<FLOAT>(rc.top);
    d2rc.right = static_cast<FLOAT>(rc.right);
    d2rc.bottom = static_cast<FLOAT>(rc.bottom);

    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->Clear(D2D1::ColorF(0x600000));
    m_pRenderTarget->FillRectangle(d2rc, m_pBkgndBrush);

    m_pDeviceContext->DrawSvgDocument(m_pSvgDoc);

    auto hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    } else if (FAILED(hr)) {
        ATLTRACE(_T("Failed to end draw on render target\n"));
    }
}

void RopeTreeView::OnSize(UINT nType, CSize size)
{
    if (!m_pRenderTarget) {
        return; // Render target not created
    }

    auto hr = m_pRenderTarget->Resize(D2D1::SizeU(size.cx, size.cy));
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to resize render target\n"));
    }

    Invalidate();
}

void LayoutRope(Rope::PNode node, float depth, float& nextX, SVG& svg, float parentX = -1, float parentY = -1)
{
    if (!node) {
        return;
    }

    auto x = nextX * 100.0f;
    auto y = depth * 100.0f;
    nextX += 1.0f;

    if (parentX >= 0 && parentY >= 0) {
        svg.Line(parentX, parentY, x, y);
    }

    COLORREF color = node->value.isLeaf() ? RGB(0, 128, 0) : RGB(200, 0, 0);
    svg.Circle(x, y, 20.0f, color);

    LayoutRope(node->left, depth + 1.0f, nextX, svg, x, y);
    LayoutRope(node->right, depth + 1.0f, nextX, svg, x, y);
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
