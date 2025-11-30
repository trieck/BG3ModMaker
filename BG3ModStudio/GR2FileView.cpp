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
    if (size.cx > 0 && size.cy > 0) {
        (void)m_direct3D.Resize(size.cx, size.cy);
    }
}

LRESULT GR2FileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    return 0;
}

BOOL GR2FileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);
    if (!hWnd) {
        return FALSE;
    }

    return TRUE;
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
