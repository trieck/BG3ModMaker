#include "stdafx.h"
#include "BG3ModStudio.h"
#include "DDSFileView.h"
#include "UtilityBase.h"

#include <d2d1helper.h>

DDSFileView::DDSFileView()
{
}

LRESULT DDSFileView::OnCreate(LPCREATESTRUCT pcs)
{
    auto lRet = DefWindowProc();

    return lRet;
}

LRESULT DDSFileView::OnPaint(CPaintDC /*dc*/)
{
    if (!m_pRenderTarget || !m_bitmap) {
        return 0;
    }

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    float w = m_bitmap->GetSize().width * m_zoom;
    float h = m_bitmap->GetSize().height * m_zoom;

    D2D1_RECT_F destRect = D2D1::RectF(
        -static_cast<FLOAT>(m_ScrollPos.x),
        -static_cast<FLOAT>(m_ScrollPos.y),
        -static_cast<FLOAT>(m_ScrollPos.x) + w,
        -static_cast<FLOAT>(m_ScrollPos.y) + h
    );

    m_pRenderTarget->DrawBitmap(m_bitmap, &destRect);
    auto hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    }

    return 0;
}

void DDSFileView::OnSize(UINT nType, CSize size)
{
    DefWindowProc();

    m_nXPageSize = size.cx;
    m_nYPageSize = size.cy;

    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nMin = 0;

    if (m_nDocHeight > m_nYPageSize) {
        si.nMax = m_nDocHeight - 1;
        si.nPage = m_nYPageSize;
        si.nPos = std::min(m_ScrollPos.y, m_nDocHeight - m_nYPageSize);
        m_ScrollPos.y = si.nPos;
    } else {
        si.nMax = 0;
        si.nPage = 0;
        si.nPos = 0;
        m_ScrollPos.y = 0;
    }
    SetScrollInfo(SB_VERT, &si, TRUE);

    if (m_nDocWidth > m_nXPageSize) {
        si.nMax = m_nDocWidth - 1;
        si.nPage = m_nXPageSize;
        si.nPos = std::min(m_ScrollPos.x, m_nDocWidth - m_nXPageSize);
        m_ScrollPos.x = si.nPos;
    } else {
        si.nMax = 0;
        si.nPage = 0;
        si.nPos = 0;
        m_ScrollPos.x = 0;
    }
    SetScrollInfo(SB_HORZ, &si, TRUE);

    if (m_pRenderTarget) {
        (void)m_pRenderTarget->Resize(D2D1::SizeU(size.cx, size.cy));
    }

    Invalidate();
}

LRESULT DDSFileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (nFlags & MK_CONTROL) {
        if (zDelta > 0) {
            m_zoom *= 1.1f; // zoom in
        } else {
            m_zoom *= 0.9f; // zoom out
        }

        // Clamp
        m_zoom = std::clamp(m_zoom, 0.1f, 8.0f);

        // Update doc size so scrollbars resize properly
        m_nDocWidth = static_cast<int>(m_bitmap->GetSize().width * m_zoom);
        m_nDocHeight = static_cast<int>(m_bitmap->GetSize().height * m_zoom);

        OnSize(0, CSize(m_nXPageSize, m_nYPageSize)); // reapply scroll info
        Invalidate();
    } else {
        auto code = zDelta < 0 ? SB_LINEDOWN : SB_LINEUP;

        OnVScroll(code, 0, nullptr);
    }

    return 0;
}

void DDSFileView::OnHScroll(UINT nSBCode, UINT, CScrollBar)
{
    auto pos = m_ScrollPos.x;
    auto orig = pos;
    auto nMaxPos = std::max<LONG>(0, m_nDocWidth - m_nXPageSize);

    switch (nSBCode) {
    case SB_TOP:
        pos = 0;
        break;
    case SB_BOTTOM:
        pos = nMaxPos;
        break;
    case SB_LINEUP:
        pos = std::max(0L, pos - 16);
        break;
    case SB_LINEDOWN:
        pos = std::min(nMaxPos, pos + 16);
        break;
    case SB_PAGEUP:
        pos = std::max(0L, pos - m_nXPageSize);
        break;
    case SB_PAGEDOWN:
        pos = std::min(nMaxPos, pos + m_nXPageSize);
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION: {
        SCROLLINFO info{sizeof(info), SIF_TRACKPOS};
        GetScrollInfo(SB_HORZ, &info);
        pos = info.nTrackPos;
        break;
    }
    default:
        return;
    }

    if (pos != orig) {
        m_ScrollPos.x = pos;
        SetScrollPos(SB_HORZ, pos);
        Invalidate();
    }
}

void DDSFileView::OnVScroll(UINT nSBCode, UINT, CScrollBar)
{
    auto pos = m_ScrollPos.y;
    auto orig = pos;
    auto nMaxPos = std::max<LONG>(0, m_nDocHeight - m_nYPageSize);

    switch (nSBCode) {
    case SB_TOP:
        pos = 0;
        break;
    case SB_BOTTOM:
        pos = nMaxPos;
        break;
    case SB_LINEUP:
        pos = std::max(0L, pos - 16); // 16 px step up
        break;
    case SB_LINEDOWN:
        pos = std::min(nMaxPos, pos + 16); // 16 px step down
        break;
    case SB_PAGEUP:
        pos = std::max(0L, pos - m_nYPageSize);
        break;
    case SB_PAGEDOWN:
        pos = std::min(nMaxPos, pos + m_nYPageSize);
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION: {
        SCROLLINFO info{sizeof(info), SIF_TRACKPOS};
        GetScrollInfo(SB_VERT, &info);
        pos = info.nTrackPos;
        break;
    }
    default:
        return;
    }

    if (pos != orig) {
        m_ScrollPos.y = pos;
        SetScrollPos(SB_VERT, pos);
        Invalidate();
    }
}


BOOL DDSFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;

    WNDPROC pUnusedWndSuperProc = nullptr;
    GetWndClassInfo().Register(&pUnusedWndSuperProc);

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);

    return hWnd != nullptr;
}

BOOL DDSFileView::LoadFile(const CString& path)
{
    auto hr = CreateDevResources();
    if (FAILED(hr)) {
        ATLTRACE("Failed to create device resources\n");
        return FALSE;
    }

    DirectX::ScratchImage image;
    hr = LoadFromDDSFile(path.GetString(),
                         DirectX::DDS_FLAGS_NONE,
                         nullptr, image);
    if (FAILED(hr)) {
        ATLTRACE("Failed to load DDS file: %s\n", path.GetString());
        return FALSE;
    }

    const auto* img = image.GetImage(0, 0, 0);
    if (!img) {
        ATLTRACE("No image in DDS file\n");
        return FALSE;
    }

    DirectX::ScratchImage decompressed;
    hr = Decompress(
        *img,
        DXGI_FORMAT_UNKNOWN, // let it choose a good format
        decompressed
    );

    if (FAILED(hr)) {
        ATLTRACE("Failed to decompress image\n");
        return FALSE;
    }

    const auto* tmpImg = decompressed.GetImage(0, 0, 0);
    if (!tmpImg) {
        ATLTRACE("No decompressed image\n");
        return FALSE;
    }

    DirectX::ScratchImage converted;
    hr = Convert(
        *tmpImg,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DirectX::TEX_FILTER_DEFAULT,
        DirectX::TEX_THRESHOLD_DEFAULT,
        converted
    );

    if (FAILED(hr)) {
        ATLTRACE("Failed to convert image to BGRA\n");
        return FALSE;
    }

    D2D1_BITMAP_PROPERTIES props;
    props.dpiX = 96.0f;
    props.dpiY = 96.0f;
    props.pixelFormat = D2D1::PixelFormat(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        D2D1_ALPHA_MODE_IGNORE
    );

    const auto* imgBGRA = converted.GetImage(0, 0, 0);
    if (!imgBGRA) {
        ATLTRACE("No converted image\n");
        return FALSE;
    }

    hr = m_pRenderTarget->CreateBitmap(
        D2D1::SizeU(static_cast<UINT32>(imgBGRA->width), static_cast<UINT32>(imgBGRA->height)),
        imgBGRA->pixels,
        static_cast<UINT32>(imgBGRA->rowPitch),
        &props,
        &m_bitmap
    );

    if (FAILED(hr)) {
        ATLTRACE("Failed to create bitmap\n");
        return FALSE;
    }

    m_nDocWidth = static_cast<LONG>(imgBGRA->width);
    m_nDocHeight = static_cast<LONG>(imgBGRA->height);
    m_ScrollPos = {0, 0};

    return TRUE;
}

BOOL DDSFileView::SaveFile()
{
    return TRUE;
}

BOOL DDSFileView::SaveFileAs(const CString& path)
{
    return TRUE;
}

BOOL DDSFileView::Destroy()
{
    DiscardDevResources();
    m_bitmap.Release();

    return DestroyWindow();
}

BOOL DDSFileView::IsDirty() const
{
    return FALSE;
}

const CString& DDSFileView::GetPath() const
{
    return m_path;
}

void DDSFileView::SetPath(const CString& path)
{
    m_path = path;
}

FileEncoding DDSFileView::GetEncoding() const
{
    return UNKNOWN;
}

DDSFileView::operator HWND() const
{
    return m_hWnd;
}

HRESULT DDSFileView::CreateDevResources()
{
    DiscardDevResources();

    if (!IsWindow()) {
        ATLTRACE("Window not created\n");
        return E_FAIL;
    }

    auto* factory = BG3ModStudio::instance().GetD2DFactory();
    if (!factory) {
        ATLTRACE("No Direct2D factory\n");
        return E_FAIL;
    }

    CRect rc;
    GetClientRect(&rc);

    auto width = rc.Width();
    auto height = rc.Height();

    auto hr = factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hWnd, D2D1::SizeU(width, height)),
        &m_pRenderTarget
    );

    if (FAILED(hr)) {
        ATLTRACE("Failed to create render target\n");
        return hr;
    }

    return S_OK;
}

void DDSFileView::DiscardDevResources()
{
    m_pRenderTarget.Release();
}
