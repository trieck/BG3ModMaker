#include "stdafx.h"
#include "RopeView.h"
#include "RopePad.h"
#include "ScopeGuard.h"

static constexpr auto BLINK_TIMER_ID = 0xBABE;
static constexpr auto MARGIN_SIZE = 15;
static constexpr auto CARET_START_X = 17.0f;
static constexpr auto CARET_START_Y = 20.0f;
static constexpr auto FONT_SIZE = 20.f;

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

    if (!SetTimer(BLINK_TIMER_ID, 200, nullptr)) {
        ATLTRACE(_T("Failed to set timer\n"));
        return -1; // Fail the creation
    }

    return 0;
}

void RopeView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_RETURN) {
        InsertChar(L'\n');
        m_lineNum++;

        float visibleBottom = m_scrollY + GetViewHeight();
        if (m_caretY > visibleBottom) {
            m_scrollY += LineHeight(); // Scroll down if caret is out of view
            m_scrollY = std::min(m_scrollY, m_textMetrics.height - GetViewHeight());
            Invalidate();
        }

    } else if (nChar >= 32 && nChar <= 127) { // Ignore control characters
        InsertChar(nChar);
    } else if (nChar == VK_BACK && m_insertPos > 0) {
        if (m_text[m_insertPos - 1] == L'\n') {
            m_lineNum--;
            m_text.Delete(--m_insertPos);
        }

        if (m_insertPos > 0) {
            m_text.Delete(--m_insertPos);
        }
    }

    UpdateCaretPos();

    (void)DrawD2DText();
}

FLOAT RopeView::GetViewHeight() const
{
    D2D1_RECT_F rc;
    GetViewRect(rc);

    return rc.bottom - rc.top;
}

void RopeView::InsertChar(UINT nChar)
{
    m_text.Insert(m_insertPos++, static_cast<WCHAR>(nChar));

    (void)UpdateLayout();
}

void RopeView::OnDestroy()
{
    KillTimer(BLINK_TIMER_ID);

    DiscardDevResources();
}

void RopeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_LEFT && m_insertPos > 0) {
        m_insertPos--;
    } else if (nChar == VK_RIGHT && m_insertPos < m_text.GetLength()) {
        m_insertPos++;
    }

    UpdateCaretPos();
    Invalidate();
}

LRESULT RopeView::OnMouseWheel(UINT nFlags, short zDelta, const CPoint& pt)
{
    auto scrollAmount = zDelta > 0 ? -FONT_SIZE : FONT_SIZE;

    auto maxScroll = std::max(0.f, m_textMetrics.height - GetViewHeight());

    m_scrollY = std::clamp(m_scrollY + scrollAmount, 0.f, maxScroll);

    Invalidate();

    return 0;
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

    auto translation = D2D1::Matrix3x2F::Translation(0, -m_scrollY);
    m_pRenderTarget->SetTransform(translation);

    float caretX = std::max(CARET_START_X, m_caretX);
    float caretY = std::max(CARET_START_Y, m_caretY);

    auto pBrush = m_showCaret ? m_pTextBrush : m_pBkgndBrush; // Use text brush if caret is shown, else background brush

    // Draw caret
    m_pRenderTarget->DrawLine(
        D2D1::Point2F(caretX, caretY),
        D2D1::Point2F(caretX, caretY + FONT_SIZE),
        pBrush, 2.0f);

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    HRESULT hr = m_pRenderTarget->EndDraw();
    if (FAILED(hr)) {
        m_pRenderTarget.Release();
    }
}

void RopeView::GetViewRect(D2D1_RECT_F& rc) const
{
    CRect clientRect;
    GetClientRect(&clientRect);

    InflateRect(&clientRect, -MARGIN_SIZE, -MARGIN_SIZE);

    if (clientRect.Width() < 0) {
        clientRect.left = 0;
        clientRect.right = 0;
    }

    if (clientRect.Height() < 0) {
        clientRect.top = 0;
        clientRect.bottom = 0;
    }

    rc.left = static_cast<FLOAT>(clientRect.left);
    rc.top = static_cast<FLOAT>(clientRect.top);
    rc.right = static_cast<FLOAT>(clientRect.right);
    rc.bottom = static_cast<FLOAT>(clientRect.bottom);
}

void RopeView::OnPaint()
{
    PAINTSTRUCT ps;
    ScopeGuardSimple guard(
        [&] { BeginPaint(&ps); },
        [&] { EndPaint(&ps); }
    );

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

    (void)UpdateLayout();
}

HRESULT RopeView::CreateDevResources()
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

    hr = UpdateLayout();
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to update layout\n"));
        return hr;
    }

    return hr;
}

HRESULT RopeView::DrawD2DText()
{
    if (!m_pRenderTarget || !m_pBkgndBrush || !m_pTextBrush || !m_pTextFormat) {
        return E_FAIL; // Ensure all resources are available
    }

    D2D1_RECT_F rc;
    GetViewRect(rc);

    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->Clear(D2D1::ColorF(0x600000));
    m_pRenderTarget->FillRectangle(rc, m_pBkgndBrush);

    auto translation = D2D1::Matrix3x2F::Translation(0, -m_scrollY);
    m_pRenderTarget->SetTransform(translation);

    m_pRenderTarget->DrawText(
        m_text.GetString(),
        m_text.GetLength(),
        m_pTextFormat,
        &rc,
        m_pTextBrush,
        D2D1_DRAW_TEXT_OPTIONS_CLIP,
        DWRITE_MEASURING_MODE_NATURAL
    );

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    auto hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    } else if (FAILED(hr)) {
        ATLTRACE(_T("Failed to end draw on render target\n"));
    }

    return hr;
}

HRESULT RopeView::UpdateLayout()
{
    if (!(m_pApp && m_pTextFormat)) {
        return E_FAIL;
    }

    auto pWriteFactory = m_pApp->GetDWriteFactory();
    if (!pWriteFactory) {
        ATLTRACE(_T("Failed to get DWrite factory\n"));
        return E_FAIL;
    }

    D2D1_RECT_F rc;
    GetViewRect(rc);

    auto width = rc.right - rc.left;
    auto height = rc.bottom - rc.top;

    m_pTextLayout.Release();

    auto hr = pWriteFactory->CreateTextLayout(
        m_text.GetString(),
        m_text.GetLength(),
        m_pTextFormat,
        width,
        height,
        &m_pTextLayout);

    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create text layout\n"));
        return hr;
    }

    hr = m_pTextLayout->GetMetrics(&m_textMetrics);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to get text metrics\n"));
        return hr;
    }

    UpdateCaretPos();

    return hr;
}

void RopeView::UpdateCaretPos()
{
    if (!m_pTextLayout) {
        return;
    }

    D2D1_POINT_2F caret;
    DWRITE_HIT_TEST_METRICS hm{};

    auto hr = m_pTextLayout->HitTestTextPosition(
        m_insertPos,
        FALSE,
        &caret.x,
        &caret.y,
        &hm
    );

    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to hit test text position\n"));
        return;
    }

    D2D1_RECT_F rc;
    GetViewRect(rc);

    if (caret.x + CARET_START_X >= rc.right) {
        m_caretX = CARET_START_X;
        m_caretY = CARET_START_Y + caret.y + hm.height/* - m_scrollY*/;
    } else {
        m_caretX = CARET_START_X + caret.x;
        m_caretY = CARET_START_Y + caret.y/* - m_scrollY*/;
    }
}

FLOAT RopeView::LineHeight() const
{
    if (m_textMetrics.lineCount == 0) {
        return 0.0f;
    }

    return m_textMetrics.height / static_cast<FLOAT>(m_textMetrics.lineCount);
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

    if (m_pTextLayout) {
        m_pTextLayout.Release();
    }

    if (m_pTextFormat) {
        m_pTextFormat.Release();
    }
}
