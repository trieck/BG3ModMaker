#pragma once
#pragma once

#include "IFileView.h"

#include <d2d1.h>
#include <DirectXTex.h>

class ImageView : public CWindowImpl<ImageView>, public IFileView
{
public:
    using Base = CWindowImpl;

    ImageView();
    ~ImageView() override = default;

    BEGIN_MSG_MAP(ImageView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT2(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_HSCROLL(OnHScroll)
        MSG_WM_VSCROLL(OnVScroll)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"ImageView", nullptr)

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnPaint(const CPaintDC& dc);
    void OnSize(UINT nType, CSize size);

    LRESULT OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
    void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL LoadFile(const CString& path) override;
    BOOL LoadBuffer(const CString& path, const ByteBuffer& buffer) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    BOOL IsEditable() const override;
    BOOL IsText() const override;
    const CString& GetPath() const override;
    VOID SetPath(const CString& path) override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;

    BOOL LoadImage(const DirectX::ScratchImage& image);
    void Release();

    static BOOL IsRenderable(const CString& path);

private:
    HRESULT CreateDevResources();
    void DiscardDevResources();

    CString m_path;
    CComPtr<ID2D1Bitmap> m_bitmap;
    CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;

    CPoint m_ScrollPos;
    LONG m_nDocWidth = 0;
    LONG m_nDocHeight = 0;
    int m_nXPageSize = 0;
    int m_nYPageSize = 0;
    float m_zoom = 1.0f;
};
