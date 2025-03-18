#include "stdafx.h"
#include "RopeView.h"

#include <algorithm>
#include "RopePad.h"
#include "ScopeGuard.h"

static constexpr auto BLINK_TIMER_ID = 0xBABE;
static constexpr auto MARGIN_SIZE = 15;
static constexpr auto CARET_START_X = 17.0f;
static constexpr auto CARET_START_Y = 20.0f;
static constexpr auto FONT_SIZE = 20.f;
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

    if (!SetTimer(BLINK_TIMER_ID, 200, nullptr)) {
        ATLTRACE(_T("Failed to set timer\n"));
        return -1; // Fail the creation
    }

    return 0;
}

void RopeView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_RETURN) {
        m_text.Insert(m_insertPos++, L"\n");
        m_lineNum++;
    } else if (nChar >= 32 && nChar <= 127) { // Ignore control characters

        // FIXME: Move care to new line if it exceeds the width of the window



        m_text.Insert(m_insertPos++, static_cast<WCHAR>(nChar));
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

    float caretX = std::max(CARET_START_X, m_caretX);
    float caretY = std::max(CARET_START_Y, m_caretY);

    auto pBrush = m_showCaret ? m_pTextBrush : m_pBkgndBrush; // Use text brush if caret is shown, else background brush

    // Draw caret
    m_pRenderTarget->DrawLine(
        D2D1::Point2F(caretX, caretY),
        D2D1::Point2F(caretX, caretY + FONT_SIZE),
        pBrush, 2.0f);

    HRESULT hr = m_pRenderTarget->EndDraw();
    if (FAILED(hr)) {
        m_pRenderTarget.Release();
    }
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
        m_pTextBrush,
        D2D1_DRAW_TEXT_OPTIONS_CLIP,
        DWRITE_MEASURING_MODE_NATURAL
    );

    auto hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    } else if (FAILED(hr)) {
        ATLTRACE(_T("Failed to end draw on render target\n"));
    }

    return hr;
}

void RopeView::UpdateCaretPos()
{
    UpdateCaretPosX();
    UpdateCaretPosY();
}

void RopeView::UpdateCaretPosX()
{
    m_caretX = CARET_START_X;

    if (!m_pRenderTarget) {
        return; // Render target not created
    }

    if (!(m_pApp && m_pTextFormat)) {
        return;
    }

    auto pos = std::max(0, m_insertPos - 1);

    LPCWSTR start = m_text;
    LPCWSTR end = start + pos;

    while (end > start && *end != L'\n') {
        --end; // Move back to the start of the line
    }

    if (*end == L'\n') {
        ++end; // Move past the newline character
    }

    auto lineStart = static_cast<int32_t>(end - start);

    auto lineLength = std::max(m_insertPos - lineStart, 0);
    if (lineLength == 0) {
        return;
    }

    auto text = m_text.Mid(lineStart, lineLength);
    if (text.IsEmpty()) {
        return;
    }

    auto pWriteFactory = m_pApp->GetDWriteFactory();
    if (!pWriteFactory) {
        ATLTRACE(_T("Failed to get DWrite factory\n"));
        return;
    }

    CComPtr<IDWriteTextLayout> pTextLayout;

    auto hr = pWriteFactory->CreateTextLayout(
        text.GetString(), text.GetLength(),
        m_pTextFormat, 1000.0f, 1000.0f, &pTextLayout);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create text layout\n"));
        return;
    }

    DWRITE_TEXT_METRICS tm;
    hr = pTextLayout->GetMetrics(&tm);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to get text metrics\n"));
        return;
    }

    m_caretX = std::max(CARET_START_X, CARET_START_X + tm.width - OVERHANG_WIDTH);
}

void RopeView::UpdateCaretPosY()
{
    m_caretY = CARET_START_Y;

    if (!m_pRenderTarget) {
        return; // Render target not created
    }

    if (!(m_pApp && m_pTextFormat)) {
        return;
    }

    if (m_lineNum <= 0) {
        return;
    }

    auto pWriteFactory = m_pApp->GetDWriteFactory();
    if (!pWriteFactory) {
        ATLTRACE(_T("Failed to get DWrite factory\n"));
        return;
    }

    CComPtr<IDWriteTextLayout> pTextLayout;

    auto hr = pWriteFactory->CreateTextLayout(
        m_text.GetString(), // Full text
        m_text.GetLength(),
        m_pTextFormat,
        1000.0f, 1000.0f, // Large enough layout box
        &pTextLayout);

    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to create text layout\n"));
        return;
    }

    uint32_t lineCount = 0;
    hr = pTextLayout->GetLineMetrics(nullptr, 0, &lineCount);

    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ATLTRACE(_T("Failed to retrieve line count\n"));
        return;
    }

    std::vector<DWRITE_LINE_METRICS> lineMetrics(lineCount);
    hr = pTextLayout->GetLineMetrics(lineMetrics.data(), lineCount, &lineCount);
    if (FAILED(hr)) {
        ATLTRACE(_T("Failed to get line metrics\n"));
        return;
    }

    if (m_lineNum >= static_cast<int32_t>(lineCount)) {
        m_lineNum = static_cast<int32_t>(lineCount) - 1;
    }

    for (auto i = 0; i < m_lineNum; ++i) {
        m_caretY += lineMetrics[i].height;
    }
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
