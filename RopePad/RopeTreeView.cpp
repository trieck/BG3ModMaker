#include "stdafx.h"
#include "Rope.h"
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

        if (m_pSvgDoc) {
            D2D1_SIZE_F sizeF = m_pSvgDoc->GetViewportSize();
            SetScrollSize(static_cast<int>(sizeF.width * m_zoom), static_cast<int>(sizeF.height * m_zoom));
        }
        Invalidate();
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

    if (!(m_pRenderTarget && m_pBkgndBrush && m_pWhiteBrush && m_pBlackBrush && m_pDeviceContext && m_pSvgDoc)) {
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

    m_pRenderTarget->BeginDraw();

    D2D1::Matrix3x2F transform =
        D2D1::Matrix3x2F::Translation(offset.width, offset.height) *
        D2D1::Matrix3x2F::Scale(m_zoom, m_zoom);

    m_pRenderTarget->SetTransform(transform);

    m_pRenderTarget->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    m_pRenderTarget->Clear(D2D1::ColorF(0x600000));
    m_pRenderTarget->FillRectangle(clipRect, m_pBkgndBrush);

    m_pDeviceContext->DrawSvgDocument(m_pSvgDoc);

    auto nodeRadius = RopeLayout::NodeRadius();
    for (const auto& label : m_layout.GetLabels()) {
        D2D1_RECT_F rcLabel = {
            label.pos.x - nodeRadius,
            label.pos.y - nodeRadius,
            label.pos.x + nodeRadius,
            label.pos.y + nodeRadius
        };

        // Set correct brush
        ID2D1SolidColorBrush* pBrush = (label.color == "black") ? m_pBlackBrush : m_pWhiteBrush;

        auto wLabel = StringHelper::fromUTF8(label.text.c_str());

        m_pRenderTarget->DrawText(
            wLabel,
            static_cast<UINT32>(wLabel.GetLength()),
            m_pTextFormat, rcLabel,
            pBrush,
            D2D1_DRAW_TEXT_OPTIONS_NONE,
            DWRITE_MEASURING_MODE_NATURAL
        );
    }

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

HRESULT RopeTreeView::BuildSvgDoc()
{
    if (!m_pDeviceContext) {
        return E_FAIL;
    }

    m_pSvgDoc.Release();

    Rope rope(3);
    rope.insert(0, "ABCDEFGHI");

    m_layout.Reset();
    m_layout.Layout({800, 600}, rope);

    auto hr = m_layout.MakeDoc(m_pDeviceContext, &m_pSvgDoc);
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

    hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create solid color brush\n"));
        m_pRenderTarget.Release();
        return hr;
    }

    hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
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

    auto pDWriteFactory = m_pApp->GetDWriteFactory();
    if (!pDWriteFactory) {
        ATLTRACE(_T("Failed to get DWrite factory\n"));
        return E_FAIL;
    }

    hr = pDWriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-us", &m_pTextFormat);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create text format\n"));
        return hr;
    }

    hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to set text alignment\n"));
        return hr;
    }

    hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to set paragraph alignment\n"));
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
    if (m_pTextFormat) {
        m_pTextFormat.Release();
    }

    if (m_pSvgDoc) {
        m_pSvgDoc.Release();
    }

    if (m_pDeviceContext) {
        m_pDeviceContext.Release();
    }

    if (m_pWhiteBrush) {
        m_pWhiteBrush.Release();
    }

    if (m_pBlackBrush) {
        m_pBlackBrush.Release();
    }

    if (m_pBkgndBrush) {
        m_pBkgndBrush.Release();
    }

    if (m_pRenderTarget) {
        m_pRenderTarget.Release();
    }
}
