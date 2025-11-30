#include "stdafx.h"
#include "BG3ModStudio.h"
#include "ImageView.h"

#include <d2d1helper.h>

ImageView::ImageView()
{
}

LRESULT ImageView::OnCreate(LPCREATESTRUCT pcs)
{
    auto lRet = DefWindowProc();

    (void)CreateDevResources();

    return lRet;
}

LRESULT ImageView::OnPaint(const CPaintDC& /*dc*/)
{
    if (!m_pRenderTarget) {
        return 0;
    }

    m_pRenderTarget->BeginDraw();

    auto cref = GetSysColor(COLOR_APPWORKSPACE);
    auto r = GetRValue(cref) / 255.0f;
    auto g = GetGValue(cref) / 255.0f;
    auto b = GetBValue(cref) / 255.0f;
    m_pRenderTarget->Clear(D2D1::ColorF(r, g, b));

    if (m_bitmap) {
        auto w = m_bitmap->GetSize().width * m_zoom;
        auto h = m_bitmap->GetSize().height * m_zoom;

        D2D1_RECT_F destRect = D2D1::RectF(
            -static_cast<FLOAT>(m_ScrollPos.x),
            -static_cast<FLOAT>(m_ScrollPos.y),
            -static_cast<FLOAT>(m_ScrollPos.x) + w,
            -static_cast<FLOAT>(m_ScrollPos.y) + h
        );

        m_pRenderTarget->DrawBitmap(m_bitmap, &destRect);
    }

    auto hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        (void)CreateDevResources();
    }

    return 0;
}

void ImageView::OnSize(UINT nType, CSize size)
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

LRESULT ImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

void ImageView::OnHScroll(UINT nSBCode, UINT, CScrollBar)
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

void ImageView::OnVScroll(UINT nSBCode, UINT, CScrollBar)
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

BOOL ImageView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;

    WNDPROC pUnusedWndSuperProc = nullptr;
    GetWndClassInfo().Register(&pUnusedWndSuperProc);

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);

    return hWnd != nullptr;
}

BOOL ImageView::LoadFile(const CString& path)
{
    DirectX::ScratchImage image;

    CString extension = ATLPath::FindExtension(path);

    HRESULT hr;
    if (extension.CompareNoCase(L".dds") == 0) {
        hr = LoadFromDDSFile(path.GetString(),
                             DirectX::DDS_FLAGS_NONE,
                             nullptr, image);
    } else {
        hr = LoadFromWICFile(path.GetString(),
                             DirectX::WIC_FLAGS_NONE,
                             nullptr, image);
    }

    if (FAILED(hr)) {
        ATLTRACE("Failed to load image file: %s\n", path.GetString());
        return FALSE;
    }

    m_path = path;

    return LoadImage(image);
}

BOOL ImageView::LoadBuffer(const CString& /*path*/, const ByteBuffer& buffer)
{
    if (!buffer.first || buffer.second == 0) {
        ATLTRACE("Empty buffer\n");
        return FALSE;
    }

    DirectX::ScratchImage image;

    // Try DDS first
    auto hr = LoadFromDDSMemory(
        buffer.first.get(),
        buffer.second,
        DirectX::DDS_FLAGS_NONE,
        nullptr,
        image
    );

    if (FAILED(hr)) {
        hr = LoadFromWICMemory(buffer.first.get(), buffer.second,
                               DirectX::WIC_FLAGS_NONE,
                               nullptr, image);
        if (FAILED(hr)) {
            ATLTRACE("Failed to load image from memory\n");
            return FALSE;
        }
    }

    m_path.Empty();

    return LoadImage(image);
}

BOOL ImageView::SaveFile()
{
    return FALSE;
}

BOOL ImageView::SaveFileAs(const CString& path)
{
    return FALSE;
}

BOOL ImageView::LoadImage(const DirectX::ScratchImage& image)
{
    auto hr = CreateDevResources();
    if (FAILED(hr)) {
        ATLTRACE("Failed to create device resources\n");
        return FALSE;
    }

    const auto* img = image.GetImage(0, 0, 0);
    if (!img) {
        ATLTRACE("No image found\n");
        return FALSE;
    }

    DirectX::ScratchImage decompressed;
    if (DirectX::IsCompressed(img->format)) {
        hr = Decompress(*img,
                        DXGI_FORMAT_UNKNOWN, // let it choose a good format
                        decompressed);
        if (FAILED(hr)) {
            ATLTRACE("Failed to decompress image\n");
            return FALSE;
        }

        img = decompressed.GetImage(0, 0, 0);
        if (!img) {
            ATLTRACE("No decompressed image\n");
            return FALSE;
        }
    }

    DirectX::ScratchImage converted;
    if (img->format != DXGI_FORMAT_B8G8R8A8_UNORM) {
        hr = Convert(*img,
                     DXGI_FORMAT_B8G8R8A8_UNORM,
                     DirectX::TEX_FILTER_DEFAULT,
                     DirectX::TEX_THRESHOLD_DEFAULT,
                     converted);
        if (FAILED(hr)) {
            ATLTRACE("Failed to convert image to BGRA\n");
            return FALSE;
        }

        img = converted.GetImage(0, 0, 0);
        if (!img) {
            ATLTRACE("No converted image\n");
            return FALSE;
        }
    }

    Release();

    D2D1_BITMAP_PROPERTIES props;
    props.dpiX = 96.0f;
    props.dpiY = 96.0f;
    props.pixelFormat = D2D1::PixelFormat(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        D2D1_ALPHA_MODE_IGNORE
    );

    hr = m_pRenderTarget->CreateBitmap(
        D2D1::SizeU(static_cast<UINT32>(img->width), static_cast<UINT32>(img->height)),
        img->pixels,
        static_cast<UINT32>(img->rowPitch),
        &props,
        &m_bitmap
    );

    if (FAILED(hr)) {
        ATLTRACE("Failed to create bitmap\n");
        return FALSE;
    }

    m_nDocWidth = static_cast<LONG>(img->width);
    m_nDocHeight = static_cast<LONG>(img->height);
    m_ScrollPos = {0, 0};

    Invalidate();

    return TRUE;
}

void ImageView::Release()
{
    m_bitmap.Release();
    Invalidate();
}

BOOL ImageView::IsRenderable(const CString& path)
{
    CString extension = ATLPath::FindExtension(path);

    if (extension.CompareNoCase(L".dds") == 0 ||
        extension.CompareNoCase(L".png") == 0 ||
        extension.CompareNoCase(L".jpg") == 0 ||
        extension.CompareNoCase(L".jpeg") == 0 ||
        extension.CompareNoCase(L".bmp") == 0 ||
        extension.CompareNoCase(L".tga") == 0 ||
        extension.CompareNoCase(L".gif") == 0 ||
        extension.CompareNoCase(L".hdr") == 0 ||
        extension.CompareNoCase(L".tiff") == 0 ||
        extension.CompareNoCase(L".tif") == 0) {
        return TRUE;
    }

    return FALSE;
}

const CString& ImageView::GetPath() const
{
    return m_path;
}

void ImageView::SetPath(const CString& path)
{
    m_path = path;
}

FileEncoding ImageView::GetEncoding() const
{
    return UNKNOWN;
}

ImageView::operator HWND() const
{
    return m_hWnd;
}

BOOL ImageView::Destroy()
{
    if (!IsWindow()) {
        return FALSE;
    }

    Release();
    DiscardDevResources();

    return DestroyWindow();
}

BOOL ImageView::IsDirty() const
{
    return FALSE;
}

BOOL ImageView::IsEditable() const
{
    return FALSE;
}

BOOL ImageView::IsText() const
{
    return FALSE;
}

HRESULT ImageView::CreateDevResources()
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

void ImageView::DiscardDevResources()
{
    m_pRenderTarget.Release();
}
