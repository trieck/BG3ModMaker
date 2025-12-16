#include "stdafx.h"
#include "BG3ModStudio.h"
#include "LicenseView.h"
#include "ResourceHelper.h"
#include "resources/resource.h"

LRESULT LicenseView::OnCreate(LPCREATESTRUCT pcs)
{
    m_text = ResourceHelper::LoadString(IDR_LICENSE);
    if (m_text.IsEmpty()) {
        m_text = L"License text could not be loaded.";
    }

    auto hr = CreateDevResources();
    if (FAILED(hr)) {
        ATLTRACE("Failed to create Direct2D resources: 0x%08X\n", hr);
        return -1;
    }

    RecalcLayout();

    return 0;
}

void LicenseView::OnDestroy()
{
    DiscardDevResources();
}

void LicenseView::OnSize(UINT nType, CSize size)
{
    RecalcLayout();
}

LRESULT LicenseView::OnPaint(const CPaintDC& dc)
{
    ValidateRect(&dc.m_ps.rcPaint);

    if (!m_rt || !m_layout || !m_textBrush) {
        return 0;
    }

    m_rt->BeginDraw();

    m_rt->SetTransform(D2D1::Matrix3x2F::Identity());

    m_rt->Clear(D2D1::ColorF(0.05f, 0.05f, 0.05f));

    m_rt->SetTransform(
        D2D1::Matrix3x2F::Translation(0.0f, -static_cast<FLOAT>(m_yScroll))
    );

    m_rt->DrawTextLayout(
        D2D1::Point2F(0, 0),
        m_layout,
        m_textBrush
    );

    auto hr = m_rt->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
        RecalcLayout();
    }

    return 0;
}

void LicenseView::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar /*pScrollBar*/)
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    GetScrollInfo(SB_VERT, &si);

    auto pos = static_cast<int32_t>(m_yScroll);
    auto orig = pos;
    auto nMaxPos = static_cast<int32_t>(
        std::max(0.0f, m_docHeight - static_cast<float>(si.nPage)));

    auto lineHeight = static_cast<int32_t>(m_lineHeight);

    switch (nSBCode) {
    case SB_TOP:
        pos = 0;
        break;
    case SB_BOTTOM:
        pos = static_cast<int32_t>(nMaxPos);
        break;
    case SB_LINEUP:
        if (pos <= 0)
            return;
        pos -= lineHeight;
        break;
    case SB_LINEDOWN:
        pos += lineHeight;
        break;
    case SB_PAGEUP:
        if (pos <= 0)
            return;
        pos -= static_cast<int32_t>(si.nPage);
        break;
    case SB_PAGEDOWN:
        pos += static_cast<int32_t>(si.nPage);
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        pos = si.nTrackPos;
        break;
    default:
        return;
    }

    pos = std::clamp<int32_t>(pos, 0, static_cast<int>(nMaxPos));

    auto delta = static_cast<int32_t>(pos - orig);
    if (delta == 0) {
        return; // no change
    }

    ScrollWindow(0, -delta);
    m_yScroll = pos;
    SetScrollPos(SB_VERT, pos);
}

HRESULT LicenseView::CreateDevResources()
{
    DiscardDevResources();

    if (!IsWindow()) {
        ATLTRACE("Window not created\n");
        return E_FAIL;
    }

    m_dwrite = BG3ModStudio::instance().GetDWriteFactory();
    if (!m_dwrite) {
        ATLTRACE("DirectWrite factory not available\n");
        return E_FAIL;
    }

    auto hr = m_dwrite->CreateTextFormat(
        L"Cascadia Mono",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        12.0f,
        L"",
        &m_format
    );

    if (FAILED(hr)) {
        ATLTRACE("Failed to create text format: 0x%08X\n", hr);
        return hr;
    }

    hr = m_format->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    if (FAILED(hr)) {
        ATLTRACE("Failed to set word wrapping: 0x%08X\n", hr);
        return hr;
    }

    m_d2d = BG3ModStudio::instance().GetD2DFactory();
    if (!m_d2d) {
        ATLTRACE("Direct2D factory not available\n");
        return E_FAIL;
    }

    D2D1_RENDER_TARGET_PROPERTIES rtProps =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(
                DXGI_FORMAT_UNKNOWN,
                D2D1_ALPHA_MODE_IGNORE
            )
        );

    CRect rc{};
    GetClientRect(&rc);

    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps =
        D2D1::HwndRenderTargetProperties(
            m_hWnd,
            D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
        );

    hr = m_d2d->CreateHwndRenderTarget(
        rtProps,
        hwndProps,
        &m_rt
    );
    if (FAILED(hr)) {
        ATLTRACE("Failed to create render target: 0x%08X\n", hr);
        return hr;
    }

    hr = m_rt->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightGreen),
        &m_textBrush
    );
    if (FAILED(hr)) {
        ATLTRACE("Failed to create text brush: 0x%08X\n", hr);
        return hr;
    }

    return hr;
}

void LicenseView::DiscardDevResources()
{
    m_layout.Release();
    m_format.Release();
    m_textBrush.Release();
    m_rt.Release();
    m_d2d.Release();
}

void LicenseView::RecalcLayout()
{
    if (!m_dwrite) {
        ATLTRACE("DirectWrite factory not available\n");
        return;
    }

    CRect rc{};
    GetClientRect(&rc);

    auto width = static_cast<FLOAT>(rc.right - rc.left);

    m_layout.Release();
    auto hr = m_dwrite->CreateTextLayout(
        m_text,
        m_text.GetLength(),
        m_format,
        width,
        FLT_MAX,
        &m_layout
    );

    if (FAILED(hr)) {
        ATLTRACE("Failed to create text layout: 0x%08X\n", hr);
        return;
    }

    DWRITE_TEXT_METRICS tm{};
    hr = m_layout->GetMetrics(&tm);
    if (FAILED(hr)) {
        ATLTRACE("Failed to get text metrics: 0x%08X\n", hr);
        return;
    }

    auto dpiX = 96.0f, dpiY = 96.0f;
    if (m_rt) {
        m_rt->GetDpi(&dpiX, &dpiY);
    }

    m_docHeight = tm.height * (dpiY / 96.0f);

    auto lineCount = 0u;
    hr = m_layout->GetLineMetrics(nullptr, 0, &lineCount);
    if (FAILED(hr) && hr != E_NOT_SUFFICIENT_BUFFER) {
        ATLTRACE("Failed to get line metrics count: 0x%08X\n", hr);
        return;
    }

    if (lineCount > 0) {
        std::vector<DWRITE_LINE_METRICS> lines(lineCount);
        hr = m_layout->GetLineMetrics(lines.data(), lineCount, &lineCount);
        if (FAILED(hr)) {
            ATLTRACE("Failed to get line metrics: 0x%08X\n", hr);
        }

        m_lineHeight = std::ceil(lines[0].height * (dpiY / 96.0f));
    } else {
        m_lineHeight = 14.0f;
    }

    if (m_rt) {
        auto iWidth = rc.right - rc.left;
        auto iHeight = rc.bottom - rc.top;
        hr = m_rt->Resize(D2D1::SizeU(iWidth, iHeight));
        if (FAILED(hr)) {
            ATLTRACE("Failed to resize render target: 0x%08X\n", hr);
            return;
        }
    }

    UpdateScrollBars();
}

void LicenseView::UpdateScrollBars()
{
    RECT rc{};
    GetClientRect(&rc);

    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nPage = rc.bottom - rc.top;
    si.nMax = static_cast<int>(m_docHeight);
    si.nPos = static_cast<int>(m_yScroll);

    SetScrollInfo(SB_VERT, &si, TRUE);
}

BOOL LicenseView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    auto code = zDelta < 0 ? SB_LINEDOWN : SB_LINEUP;

    OnVScroll(code, 0, nullptr);

    return TRUE;
}
