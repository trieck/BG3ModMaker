#include "stdafx.h"
#include "RopeView.h"
#include "RopePad.h"
#include "ScopeGuard.h"

static constexpr auto BLINK_TIMER_ID = 0xBABE;
static constexpr auto MARGIN_SIZE = 15;
static constexpr auto CARET_START = 20;
static constexpr auto FONT_SIZE = 24.0f;
static constexpr auto OVERHANG_WIDTH = 2.0f;

RopeView::RopeView(RopePad* pApp) : m_pApp(pApp)
{
    ATLASSERT(m_pApp != nullptr);
}

LRESULT RopeView::OnCreate(LPCREATESTRUCT pcs)
{
    auto hr = CreateDevResources();
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create device resources\n"));
        return -1; // Fail the creation
    }

    ATLASSERT(IsWindow());

    auto blinkRate = GetCaretBlinkTime();
    if (blinkRate == 0) {
        blinkRate = 500; // Default blink rate if system setting is zero
    }

    if (!SetTimer(BLINK_TIMER_ID, blinkRate, nullptr)) {
        ATLTRACE(_T("Failed to set timer\n"));
        return -1; // Fail the creation
    }

    return 0;
}

void RopeView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar >= 32 && nChar != 127) { // Ignore control characters
        // Insert character at caret position
        m_text.Insert(m_caretPos, static_cast<WCHAR>(nChar));
        (void)UpdateTextWidth();
        m_caretPos++;
        (void)DrawD2DText();
    } else if (nChar == VK_BACK && m_caretPos > 0) {
        // Remove character before caret position
        m_text.Delete(m_caretPos - 1);
        (void)UpdateTextWidth();
        m_caretPos--;
        (void)DrawD2DText();
    }    
}

void RopeView::OnDestroy()
{
    KillTimer(BLINK_TIMER_ID);

    DiscardDevResources();    
}

void RopeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_LEFT && m_caretPos > 0) {
        m_caretPos--;
    } else if (nChar == VK_RIGHT && m_caretPos < m_text.GetLength()) {
        m_caretPos++;
    }
}

void RopeView::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != BLINK_TIMER_ID) {
        return; // Ignore other timers
    }

    if (!m_pRenderTarget) {
        return; // Render target not created
    }

    m_showCaret = !m_showCaret; // Toggle caret visibility

    m_pRenderTarget->BeginDraw();

    auto caretX = static_cast<float>(CARET_START + m_textWidth);
    auto caretY = static_cast<float>(CARET_START);

    if (m_showCaret) {
        // Draw caret
        m_pRenderTarget->DrawLine(
            D2D1::Point2F(caretX, caretY),
            D2D1::Point2F(caretX, caretY + FONT_SIZE),
            m_pTextBrush, 2.0f);
    } else {
        // Erase caret by drawing over it
        m_pRenderTarget->DrawLine(
            D2D1::Point2F(caretX, caretY),
            D2D1::Point2F(caretX, caretY + FONT_SIZE),
            m_pBkgndBrush, 2.0f); // Use background brush to erase
    }

    HRESULT hr = m_pRenderTarget->EndDraw();
    if (FAILED(hr)) {
        m_pRenderTarget.Release();
    }
}

void RopeView::OnPaint()
{
    PAINTSTRUCT ps;
    ScopeGuardSimple guard(
        [&]() { BeginPaint(&ps); },
        [&]() { EndPaint(&ps); }
    );

    if (!(m_pRenderTarget && m_pBkgndBrush && m_pTextBrush && m_pTextFormat)) {
        return;
    }

    auto hr = DrawD2DText();
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to draw text\n"));
    }
}

BOOL RopeView::OnEraseBkgnd(const CDCHandle& dc)
{
    return TRUE;
}

void RopeView::OnSize(UINT nType, CSize size)
{
    if (!m_pRenderTarget) {
        return; // Render target not created
    }

    auto hr = m_pRenderTarget->Resize(D2D1::SizeU(size.cx, size.cy));
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to resize render target\n"));
    }
}

HRESULT RopeView::CreateDevResources()
{
    auto pFactory = m_pApp->GetD2DFactory();
    if (!pFactory) {
        ATLTRACE(_T("Failed to get D2D factory\n"));
        return E_FAIL;
    }

    DiscardDevResources();

    RECT rc;
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

    auto pDWriteFactory = m_pApp->GetDWriteFactory();
    if (!pDWriteFactory) {
        ATLTRACE(_T("Failed to get DWrite factory\n"));
        return E_FAIL;
    }

    hr = pDWriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, FONT_SIZE, L"en-us", &m_pTextFormat);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create text format\n"));
        return hr;
    }

    hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to set text alignment\n"));
        return hr;
    }

    hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to set paragraph alignment\n"));
        return hr;
    }

    hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x600000), &m_pTextBrush);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create text brush\n"));
    }

    return hr;
}

HRESULT RopeView::UpdateTextWidth()
{
    if (!m_pApp) {
        ATLTRACE(_T("RopeView::UpdateTextWidth: m_pApp is null\n"));
        return E_FAIL;
    }

    auto pDWriteFactory = m_pApp->GetDWriteFactory();
    if (!pDWriteFactory) {
        ATLTRACE(_T("Failed to get DWrite factory\n"));
        return E_FAIL;
    }

    CComPtr<IDWriteTextLayout> pTextLayout;
    auto hr = pDWriteFactory->CreateTextLayout(
        m_text.GetString(),
        m_text.GetLength(),
        m_pTextFormat,
        1000.0f, // Arbitrary width
        100.0f,  // Arbitrary height
        &pTextLayout
    );

    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create text layout\n"));
        return hr;
    }

    DWRITE_TEXT_METRICS textMetrics;
    hr = pTextLayout->GetMetrics(&textMetrics);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to get text metrics\n"));
        return hr;
    }

    m_textWidth = textMetrics.width - OVERHANG_WIDTH;

    return S_OK;
}

HRESULT RopeView::DrawD2DText()
{
    if (!m_pRenderTarget || !m_pBkgndBrush || !m_pTextBrush || !m_pTextFormat) {
        return E_FAIL; // Ensure all resources are available
    }

    CRect rc;
    GetClientRect(&rc);

    InflateRect(&rc, -MARGIN_SIZE, -MARGIN_SIZE);

    D2D1_RECT_F clipRect = D2D1::RectF(
        static_cast<FLOAT>(rc.left),
        static_cast<FLOAT>(rc.top),
        static_cast<FLOAT>(rc.right),
        static_cast<FLOAT>(rc.bottom));

    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->Clear(D2D1::ColorF(0x600000));
    m_pRenderTarget->FillRectangle(clipRect, m_pBkgndBrush);

    m_pRenderTarget->DrawText(
        m_text.GetString(),
        m_text.GetLength(),
        m_pTextFormat,
        &clipRect,
        m_pTextBrush
    );

    auto hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    } else if (FAILED(hr)) {
        ATLTRACE(_T("Failed to end draw on render target\n"));
    }

    return hr;
}

void RopeView::DiscardDevResources()
{
    if (m_pRenderTarget) {
        m_pRenderTarget.Release();
    }

    if (m_pBkgndBrush) {
        m_pBkgndBrush.Release();
    }

    if (m_pTextBrush) {
        m_pTextBrush.Release();
    }

    if (m_pTextFormat) {
        m_pTextFormat.Release();
    }
}
