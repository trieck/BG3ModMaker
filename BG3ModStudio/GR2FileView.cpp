#include "stdafx.h"
#include "GR2FileView.h"
#include "GR2ModelBuilder.h"
#include "StringHelper.h"

GR2FileView::GR2FileView()
{
}

LRESULT GR2FileView::OnCreate(LPCREATESTRUCT pcs)
{
    auto hr = m_direct3D.Initialize(*this);
    if (FAILED(hr)) {
        ATLTRACE(L"Failed to initialize Direct3D: 0x%08X\n", hr);
        return -1;
    }

    // Start continuous rendering timer
    SetTimer(RENDER_TIMER_ID, RENDER_TIMER_MS);

    return 0;
}

LRESULT GR2FileView::OnEraseBkgnd(const CDCHandle& /*dc*/)
{
    return TRUE;
}

LRESULT GR2FileView::OnPaint(const CPaintDC& dc)
{
    // Clear to dark blue-gray
    float clearColor[4] = {0.2f, 0.2f, 0.3f, 1.0f};
    m_direct3D.Clear(clearColor);

    if (m_model.IsValid()) {
        m_model.Render(m_direct3D);
    }

    auto hr = m_direct3D.Present();
    if (FAILED(hr)) {
        ATLTRACE(L"Failed to present Direct3D swap chain: 0x%08X\n", hr);
    }

    ValidateRect(&dc.m_ps.rcPaint);

    return 0;
}

void GR2FileView::OnSize(UINT nType, CSize size)
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

    if (size.cx > 0 && size.cy > 0) {
        (void)m_direct3D.Resize(size.cx, size.cy);
    }

    Invalidate();
}

LRESULT GR2FileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (nFlags & MK_CONTROL) {
        if (zDelta > 0) {
            m_zoom *= 1.1f; // zoom in
        } else {
            m_zoom *= 0.9f; // zoom out
        }

        // Clamp
        m_zoom = std::clamp(m_zoom, 0.1f, 8.0f);
        m_model.SetZoom(m_zoom);

        UpdateScrollBars();
    } else {
        auto code = zDelta < 0 ? SB_LINEDOWN : SB_LINEUP;

        OnVScroll(code, 0, nullptr);
    }

    return 0;
}

void GR2FileView::OnHScroll(UINT nSBCode, UINT, CScrollBar)
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
        SCROLLINFO info;
        info.cbSize = sizeof(info);
        info.fMask = SIF_TRACKPOS;
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
        UpdateModelPan();
    }
}

void GR2FileView::OnVScroll(UINT nSBCode, UINT, CScrollBar)
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
        SCROLLINFO info;
        info.cbSize = sizeof(info);
        info.fMask = SIF_TRACKPOS;
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
        UpdateModelPan();
    }
}

void GR2FileView::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == RENDER_TIMER_ID) {
        Invalidate(FALSE);
    }
}

void GR2FileView::OnDestroy()
{
    // Stop timer
    KillTimer(RENDER_TIMER_ID);
}

void GR2FileView::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_isDragging = true;
    m_lastMousePos = point;
    SetCapture();
}

void GR2FileView::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_isDragging = false;
    ReleaseCapture();
}

void GR2FileView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_isDragging) {
        return;
    }

    // Calculate delta
    auto dx = static_cast<float>(point.x - m_lastMousePos.x);
    auto dy = static_cast<float>(point.y - m_lastMousePos.y);

    // Update rotation (sensitivity: 0.01 radians per pixel)
    m_cameraYaw -= dx * 0.01f;
    m_cameraPitch -= dy * 0.01f;

    // Clamp pitch to avoid gimbal lock
    m_cameraPitch = std::clamp(m_cameraPitch, -1.5f, 1.5f);

    // Update model rotation
    m_model.SetRotation(m_cameraYaw, m_cameraPitch);

    m_lastMousePos = point;
}

BOOL GR2FileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;

    WNDPROC pUnusedWndSuperProc = nullptr;
    GetWndClassInfo().Register(&pUnusedWndSuperProc);

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);

    return hWnd != nullptr;
}

BOOL GR2FileView::Destroy()
{
    m_direct3D.Release();

    if (!IsWindow()) {
        return FALSE;
    }

    return DestroyWindow();
}

BOOL GR2FileView::IsDirty() const
{
    return FALSE;
}

BOOL GR2FileView::IsEditable() const
{
    return FALSE;
}

BOOL GR2FileView::IsText() const
{
    return FALSE;
}

BOOL GR2FileView::LoadBuffer(const CString& path, const ByteBuffer& buffer)
{
    return FALSE;
}

BOOL GR2FileView::LoadFile(const CString& path)
{
    GR2ModelBuilder builder;

    auto utf8Path = StringHelper::toUTF8(path);

    try {
        auto grannyModel = builder.build(utf8Path);
        if (!m_model.Create(m_direct3D, grannyModel)) {
            ATLTRACE(L"Failed to create D3DModel from GR2 file: %s\n", path.GetString());
            return FALSE;
        }
    } catch (const std::exception& ex) {
        ATLTRACE(L"Failed to load GR2 file: %s. Error: %S\n", path.GetString(), ex.what());
        return FALSE;
    }

    m_ScrollPos = {0, 0};

    UpdateScrollBars();

    return TRUE;
}

BOOL GR2FileView::SaveFile()
{
    return FALSE;
}

BOOL GR2FileView::SaveFileAs(const CString& path)
{
    return FALSE;
}

const CString& GR2FileView::GetPath() const
{
    return m_path;
}

FileEncoding GR2FileView::GetEncoding() const
{
    return UNKNOWN;
}

GR2FileView::operator HWND() const
{
    return m_hWnd;
}

void GR2FileView::SetPath(const CString& path)
{
    m_path = path;
}

void GR2FileView::UpdateScrollBars()
{
    if (!m_model.IsValid()) {
        return;
    }

    // Get model dimensions at current zoom
    // Scale from normalized [-1,1] space to pixel space
    // Assuming viewport is roughly 1000px, scale accordingly
    auto modelWidth = m_model.GetScreenWidth() * (static_cast<float>(m_nXPageSize) / 2.0f);
    auto modelHeight = m_model.GetScreenHeight() * (static_cast<float>(m_nYPageSize) / 2.0f);

    m_nDocWidth = static_cast<LONG>(modelWidth);
    m_nDocHeight = static_cast<LONG>(modelHeight);

    // Trigger OnSize to update scrollbar info
    CSize size(m_nXPageSize, m_nYPageSize);

    OnSize(0, size);
}

void GR2FileView::UpdateModelPan()
{
    if (!m_model.IsValid()) {
        return;
    }

    // Convert scroll position to normalized pan offset
    // Scroll position is in pixels, need to convert to [-1, 1] space
    float panX = 0.0f;
    float panY = 0.0f;

    if (m_nDocWidth > m_nXPageSize) {
        // Map scroll position to pan range
        float scrollRatio = static_cast<float>(m_ScrollPos.x) / (m_nDocWidth - m_nXPageSize);
        panX = -scrollRatio * 2.0f; // -2 to +2 range in normalized space
    }

    if (m_nDocHeight > m_nYPageSize) {
        float scrollRatio = static_cast<float>(m_ScrollPos.y) / (m_nDocHeight - m_nYPageSize);
        panY = scrollRatio * 2.0f;
    }

    m_model.SetPan(panX, panY);
}
