#pragma once

#include "D3DModel.h"
#include "Direct3D.h"
#include "IFileView.h"

class GR2FileView : public CWindowImpl<GR2FileView>, public IFileView
{
public:
    using Base = CWindowImpl;

    GR2FileView();
    ~GR2FileView() override = default;

    BEGIN_MSG_MAP(GR2FileView)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT2(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_HSCROLL(OnHScroll)
        MSG_WM_VSCROLL(OnVScroll)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"GR2FileView", nullptr)

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnEraseBkgnd(const CDCHandle& dc);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    LRESULT OnPaint(const CPaintDC& dc);
    void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);
    void OnSize(UINT nType, CSize size);
    void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
    void OnTimer(UINT_PTR nIDEvent);
    void OnDestroy();

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    BOOL IsEditable() const override;
    BOOL IsText() const override;
    BOOL LoadBuffer(const CString& path, const ByteBuffer& buffer) override;
    BOOL LoadFile(const CString& path) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    const CString& GetPath() const override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;
    VOID SetPath(const CString& path) override;

private:
    void UpdateScrollBars();
    void UpdateModelPan();

    static constexpr UINT_PTR RENDER_TIMER_ID = 1;
    static constexpr UINT RENDER_TIMER_MS = 16; // ~60 FPS

    Direct3D m_direct3D;
    CString m_path;
    D3DModel m_model;

    bool m_isDragging{false};
    CPoint m_lastMousePos;
    CPoint m_ScrollPos;
    float m_cameraPitch{0.0f};
    float m_cameraYaw{0.0f};
    float m_zoom = 1.0f;
    int m_nXPageSize = 0;
    int m_nYPageSize = 0;
    LONG m_nDocHeight = 0;
    LONG m_nDocWidth = 0;
};
