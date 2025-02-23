#pragma once

#include "IFileView.h"
#include "UTF8Stream.h"

class BinaryFileView : public CWindowImpl<BinaryFileView>, public IFileView
{
public:
    using Base = CWindowImpl;
    using Ptr = std::shared_ptr<BinaryFileView>;

    BinaryFileView();
    ~BinaryFileView() override = default;

    BEGIN_MSG_MAP(BinaryFileView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT2(OnPaint)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_SIZE(OnSize)
        MSG_WM_HSCROLL(OnHScroll)
        MSG_WM_VSCROLL(OnVScroll)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"BinaryFilewView", nullptr)

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnPaint(CPaintDC dc);
    LRESULT OnEraseBkgnd(CDCHandle dc);
    LRESULT OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    void OnSize(UINT nType, CSize size);
    void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
    void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL LoadFile(const CString& path) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    LPCTSTR GetPath() const override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;

private:
    void DrawGridLine(CPaintDC& dc, int32_t vpos, LONG xextent);
    BOOL Write(LPCSTR text) const;
    BOOL Write(LPCSTR text, size_t length) const;
    BOOL Flush();
    void SetSizes();

    CComObjectStack<UTF8Stream> m_stream;
    ByteBuffer m_buffer;  // THIS IS A PLACEHOLDER!!!

    CFont m_font;
    CBrush m_bkgndBrush;
    CPen m_gridPen;
    CString m_path;
    int32_t m_cxChar, m_cyChar;
    int32_t m_nLinesTotal;
    int32_t m_nXPageSize;
    int32_t m_nYPageSize;
    int32_t m_nDocHeight;
    int32_t m_nDocWidth;
    CPoint m_ScrollPos;
};
