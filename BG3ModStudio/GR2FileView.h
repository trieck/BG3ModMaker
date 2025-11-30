#pragma once
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
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"GR2FileView", nullptr)

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnEraseBkgnd(const CDCHandle& dc);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    LRESULT OnPaint(const CPaintDC& dc);
    void OnSize(UINT nType, CSize size);

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
    Direct3D m_direct3D;
    CString m_path;
    D3DModel m_model;
};
