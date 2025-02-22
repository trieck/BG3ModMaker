#include "stdafx.h"
#include "BinaryFileView.h"

#include "Exception.h"
#include "FileStream.h"
#include "TextFileView.h"

namespace { // anonymous
constexpr auto BUFFER_SIZE = 4096;
constexpr auto LINESIZE = 16;
constexpr auto BUFFSIZE = 81;
constexpr auto DATAOFFSET = 13;
constexpr auto CHAROFFSET = 5;

int32_t FormatLine(uint32_t line, LPSTR buffer, LPCSTR pdata, size_t size)
{
    auto pbuffer = buffer;
    size_t left = BUFFSIZE + 1;

    left -= sprintf_s(pbuffer, left, "%0.8x     ", line * LINESIZE);

    pbuffer += DATAOFFSET;
    left -= DATAOFFSET;

    for (auto i = 0u; i < size; i++) {
        if (i > 0) {
            *pbuffer++ = ' ';
            left--;
        }

        sprintf_s(pbuffer, left, "%0.2x", pdata[i]);

        pbuffer += 2;
        left -= 2;
    }

    auto diff = LINESIZE - size;
    while (diff) {
        pbuffer[0] = ' ';
        pbuffer[1] = ' ';
        pbuffer[2] = ' ';
        pbuffer += 3;
        left -= 3;
        diff--;
    }

    strcpy_s(pbuffer, left, "     ");
    pbuffer += CHAROFFSET;

    for (auto i = 0u; i < size; ++i) {
        if (_istprint(pdata[i])) {
            pbuffer[i] = pdata[i];
        } else {
            pbuffer[i] = '.';
        }
    }

    pbuffer[size] = '\0';

    return static_cast<int>((pbuffer + size) - buffer);
}

} // anonymous namespace

BinaryFileView::BinaryFileView() : m_cxChar(0), m_cyChar(0), m_nLinesTotal(0), m_nXPageSize(0), m_nYPageSize(0),
                                   m_nDocHeight(0), m_nDocWidth(0), m_ScrollPos(0, 0)
{
}

LRESULT BinaryFileView::OnCreate(LPCREATESTRUCT pcs)
{
    auto lRet = DefWindowProc();

    if (!m_font.CreatePointFont(120, L"Cascadia Mono") &&
        !m_font.CreatePointFont(120, L"Consolas") &&
        !m_font.CreatePointFont(120, L"Courier New")) {
        ATLTRACE("Failed to create a font. Using system default.\n");
        m_font.Attach(static_cast<HFONT>(GetStockObject(SYSTEM_FIXED_FONT)));
    }

    if (!m_bkgndBrush.CreateSolidBrush(RGB(255, 255, 255))) {
        ATLTRACE("Failed to create brush.\n");
        return -1;
    }

    if (!m_pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0))) {
        ATLTRACE("Failed to create pen.\n");
        return -1;
    }

    SetSizes();

    return lRet;
}

LRESULT BinaryFileView::OnPaint(CPaintDC dc)
{
    dc.SetViewportOrg(-m_ScrollPos.x, -m_ScrollPos.y);
    dc.SetBkMode(TRANSPARENT);

    CRect rc;
    dc.GetClipBox(&rc);

    auto oldFont = dc.SelectFont(m_font);
    auto oldPen = dc.SelectPen(m_pen);

    auto size = m_buffer.second;
    auto nstart = std::max<int>(0, rc.top / m_cyChar);
    auto nend = std::max<int>(0, std::min<int>(m_nLinesTotal - 1, (rc.bottom + m_cyChar - 1) / m_cyChar));

    static CHAR buffer[BUFFSIZE + 1];
    auto pdata = reinterpret_cast<LPCSTR>(m_buffer.first.get());

    for (auto i = nstart; i <= nend; ++i) {
        auto nlength = std::min<size_t>(LINESIZE, size - (static_cast<size_t>(i) * LINESIZE));
        auto j = FormatLine(i, buffer, pdata + static_cast<ptrdiff_t>(i * LINESIZE), nlength);
        TextOutA(dc, 0, i * m_cyChar, buffer, j);
    }

    dc.SelectFont(oldFont);
    dc.SelectPen(oldPen);

    return 0;
}

LRESULT BinaryFileView::OnEraseBkgnd(CDCHandle dc)
{
    CRect rc;
    dc.GetClipBox(rc);

    auto oldBrush = dc.SelectBrush(m_bkgndBrush);

    dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);

    dc.SelectBrush(oldBrush);

    return 1;
}

void BinaryFileView::OnSize(UINT nType, CSize size)
{
    DefWindowProc();

    auto [cx, cy] = size;

    int32_t scrollMax;
    if (cy < m_nDocHeight) {
        scrollMax = m_nDocHeight;
        m_nYPageSize = cy;
        m_ScrollPos.y = std::min<LONG>(m_ScrollPos.y, m_nDocHeight - m_nYPageSize);
    } else {
        scrollMax = m_ScrollPos.y = m_nYPageSize = 0;
    }

    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nMin = 0;
    si.nMax = scrollMax;
    si.nPos = m_ScrollPos.y;
    si.nPage = m_nYPageSize;

    SetScrollInfo(SB_VERT, &si, TRUE);

    if (cx < m_nDocWidth) {
        scrollMax = m_nDocWidth;
        m_nXPageSize = cx;
        m_ScrollPos.x = std::min<LONG>(m_ScrollPos.x, m_nDocWidth - m_nXPageSize);
    } else {
        scrollMax = m_ScrollPos.x = m_nXPageSize = 0;
    }

    si.nMax = scrollMax;
    si.nPos = m_ScrollPos.x;
    si.nPage = m_nXPageSize;

    SetScrollInfo(SB_HORZ, &si, TRUE);

    Invalidate();
}

void BinaryFileView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
{
    auto pos = m_ScrollPos.x;
    auto orig = pos;
    auto nMaxPos = m_nDocWidth - m_nXPageSize;

    SCROLLINFO info;

    switch (nSBCode) {
    case SB_TOP:
        pos = 0;
        break;
    case SB_BOTTOM:
        pos = nMaxPos;
        break;
    case SB_LINEUP:
        if (pos <= 0)
            return;
        pos -= std::min<LONG>(m_cxChar, m_ScrollPos.x);
        break;
    case SB_PAGEUP:
        if (pos <= 0)
            return;
        pos -= std::min<LONG>(m_nXPageSize, m_ScrollPos.x);
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        // Adjust for 32bit scroll coordinates
        info.cbSize = sizeof(SCROLLINFO);
        info.fMask = SIF_TRACKPOS;
        GetScrollInfo(SB_HORZ, &info);
        pos = info.nTrackPos;
        break;
    case SB_PAGEDOWN:
        if (pos >= nMaxPos)
            return;
        pos += std::min<LONG>(m_nXPageSize, nMaxPos - m_ScrollPos.x);
        break;
    case SB_LINEDOWN:
        if (pos >= nMaxPos)
            return;
        pos += std::min<LONG>(m_cxChar, nMaxPos - m_ScrollPos.x);
        break;
    default:
        return;
    }

    auto delta = pos - orig;
    if (delta == 0) {
        return; // no change
    }

    ScrollWindow(-delta, 0);

    SetScrollPos(SB_HORZ, m_ScrollPos.x = pos);
    UpdateWindow();
}

void BinaryFileView::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar /*pScrollBar*/)
{
    auto pos = m_ScrollPos.y;
    auto orig = pos;
    auto nMaxPos = m_nDocHeight - m_nYPageSize;

    SCROLLINFO info;

    switch (nSBCode) {
    case SB_TOP:
        pos = 0;
        break;
    case SB_BOTTOM:
        pos = nMaxPos;
        break;
    case SB_LINEUP:
        if (pos <= 0)
            return;
        pos -= std::min<LONG>(m_cyChar, m_ScrollPos.y);
        break;
    case SB_PAGEUP:
        if (pos <= 0)
            return;
        pos -= std::min<LONG>(m_nYPageSize, m_ScrollPos.y);
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        // Adjust for 32bit scroll coordinates
        info.cbSize = sizeof(SCROLLINFO);
        info.fMask = SIF_TRACKPOS;
        GetScrollInfo(SB_VERT, &info);
        pos = info.nTrackPos;
        break;
    case SB_PAGEDOWN:
        if (pos >= nMaxPos)
            return;
        pos += std::min<LONG>(m_nYPageSize, nMaxPos - m_ScrollPos.y);
        break;
    case SB_LINEDOWN:
        if (pos >= nMaxPos)
            return;
        pos += std::min<LONG>(m_cyChar, nMaxPos - m_ScrollPos.y);
        break;
    default:
        return;
    }

    auto delta = pos - orig;
    if (delta == 0) {
        return; // no change
    }

    ScrollWindow(0, -delta);
    SetScrollPos(SB_VERT, m_ScrollPos.y = pos);
}

LRESULT BinaryFileView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
    UINT code = zDelta < 0 ? SB_LINEDOWN : SB_LINEUP;

    OnVScroll(code, 0, nullptr);

    return 0;
}

BOOL BinaryFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;

    WNDPROC pUnusedWndSuperProc = nullptr;
    GetWndClassInfo().Register(&pUnusedWndSuperProc);

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);

    return hWnd != nullptr;
}

BOOL BinaryFileView::Write(LPCSTR text) const
{
    auto hr = m_stream.Write(text);
    return SUCCEEDED(hr);
}

BOOL BinaryFileView::Write(LPCSTR text, size_t length) const
{
    auto hr = m_stream.Write(text, length);
    return SUCCEEDED(hr);
}

BOOL BinaryFileView::Flush()
{
    // THIS IS A PLACEHOLDER!!!
    m_buffer = m_stream.ReadBytes();

    auto hr = m_stream.Reset();

    return SUCCEEDED(hr);
}

BOOL BinaryFileView::LoadFile(const CString& path)
{
    m_stream.Reset();

    m_path = path;

    FileStream file;
    CStringA strPath(path);

    try {
        file.open(strPath, "rb");
    }
    catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return FALSE;
    }

    char buf[BUFFER_SIZE];
    auto read = file.read(buf, sizeof(buf));

    Write(buf, read);

    for (;;) {
        read = file.read(buf, sizeof(buf));
        if (read == 0) {
            break;
        }

        Write(buf, read);
    }

    Flush();

    SetSizes();

    return TRUE;
}

BOOL BinaryFileView::SaveFile(const CString& path)
{
    return TRUE;
}

BOOL BinaryFileView::Destroy()
{
    return DestroyWindow();
}

LPCTSTR BinaryFileView::GetPath() const
{
    return m_path.GetString();
}

FileEncoding BinaryFileView::GetEncoding() const
{
    return UNKNOWN;
}

BinaryFileView::operator HWND() const
{
    return m_hWnd;
}

void BinaryFileView::SetSizes()
{
    CClientDC dc(*this);

    auto oldFont = dc.SelectFont(m_font);
    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);

    GetCharWidth32(dc, '0', '0', &m_cxChar);
    m_cyChar = tm.tmHeight + tm.tmExternalLeading;

    auto size = m_buffer.second;

    m_nLinesTotal = (static_cast<int>(size) + LINESIZE - 1) / LINESIZE;
    m_nDocHeight = m_nLinesTotal * m_cyChar;
    m_nDocWidth = size == 0 ? 0 : BUFFSIZE * m_cxChar;

    m_ScrollPos.x = m_ScrollPos.y = 0;

    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_POS;
    si.nMin = 0;
    si.nMax = m_nDocHeight;
    si.nPos = 0;

    SetScrollInfo(SB_VERT, &si, TRUE);

    si.nMax = m_nDocWidth;
    SetScrollInfo(SB_HORZ, &si, TRUE);

    dc.SelectFont(oldFont);
}
