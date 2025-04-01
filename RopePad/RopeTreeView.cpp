#include "stdafx.h"
#include "RopePad.h"
#include "RopeTreeView.h"
#include "ScopeGuard.h"
#include "StringHelper.h"

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

        SetScrollSizes();
    } else {
        if (zDelta > 0) {
            ScrollLineUp();
        } else {
            ScrollLineDown();
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

    if (!(m_pDeviceContext && m_pBkgndBrush)) {
        return; // Render target not created
    }

    CRect rc;
    GetClientRect(&rc);

    CPoint ptOffset;
    GetScrollOffset(ptOffset);

    D2D1_SIZE_F offset = {static_cast<FLOAT>(-ptOffset.x), static_cast<FLOAT>(-ptOffset.y)};

    D2D1_RECT_F clipRect = D2D1::RectF(
        static_cast<FLOAT>(rc.left) / m_zoom - offset.width,
        static_cast<FLOAT>(rc.top) / m_zoom - offset.height,
        static_cast<FLOAT>(rc.right) / m_zoom - offset.width,
        static_cast<FLOAT>(rc.bottom) / m_zoom - offset.height
    );

    m_pDeviceContext->BeginDraw();

    D2D1::Matrix3x2F transform =
        D2D1::Matrix3x2F::Translation(offset.width, offset.height) *
        D2D1::Matrix3x2F::Scale(m_zoom, m_zoom);

    m_pDeviceContext->SetTransform(transform);

    m_pDeviceContext->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    m_pDeviceContext->Clear(D2D1::ColorF(0x600000));
    m_pDeviceContext->FillRectangle(clipRect, m_pBkgndBrush);

    if (m_pCommandList) {
        m_pDeviceContext->DrawImage(m_pCommandList);
    }

    m_pDeviceContext->PopAxisAlignedClip();

    m_pDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

    auto hr = m_pDeviceContext->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    } else if (FAILED(hr)) {
        ATLTRACE(_T("Failed to end draw on render target\n"));
    }
}

void RopeTreeView::OnSize(UINT nType, CSize size)
{
    if (!m_pRenderTarget) {
        return;
    }

    auto hr = m_pRenderTarget->Resize(D2D1::SizeU(size.cx, size.cy));
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to resize render target\n"));
    }

    SetScrollSizes();
}

void RopeTreeView::OnUpdateLayout()
{
    (void)RecreateLayout();
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

    hr = m_pRenderTarget->QueryInterface(&m_pDeviceContext);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to get device context\n"));
        return hr;
    }

    hr = m_pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cornsilk), &m_pBkgndBrush);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create solid color brush\n"));
        return hr;
    }

    m_pDWriteFactory = m_pApp->GetDWriteFactory();
    if (!m_pDWriteFactory) {
        ATLTRACE(_T("Failed to get DWrite factory\n"));
        return E_FAIL;
    }

    hr = InitLayout();
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create rope layout\n"));
        return hr;
    }

    return hr;
}

HRESULT RopeTreeView::InitLayout()
{
    if (!m_pDeviceContext || !m_pDWriteFactory) {
        return E_POINTER;
    }

    auto hr = m_layout.Init(m_pDeviceContext, m_pDWriteFactory);

    return hr;
}

HRESULT RopeTreeView::RecreateLayout()
{
    if (!m_pApp || !m_pDeviceContext || !m_pDWriteFactory) {
        return E_POINTER;
    }

    const auto& rope = m_pApp->GetRope();

    m_pCommandList.Release();
    auto hr = m_layout.Render(rope, &m_pCommandList);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to render rope layout\n"));
        return hr;
    }

    SetScrollSizes();

    return hr;
}

void RopeTreeView::SetScrollSizes()
{
    CRect rc;
    GetClientRect(&rc);

    DoSize(rc.Width(), rc.Height());

    auto bounds = m_layout.GetBounds();
    if (bounds.width > 0 && bounds.height > 0) {
        SetScrollSize(static_cast<int>(bounds.width * m_zoom), static_cast<int>(bounds.height * m_zoom));
    }
    
    Invalidate();
}

void RopeTreeView::DiscardDevResources()
{
    m_pCommandList.Release();
    m_pBkgndBrush.Release();
    m_pDeviceContext.Release();
    m_pRenderTarget.Release();
    m_pDWriteFactory.Release();
}
