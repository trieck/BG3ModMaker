#include "stdafx.h"
#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter() : m_hWnd(nullptr)
{
}

void SyntaxHighlighter::SetHwnd(HWND hWnd)
{
    m_hWnd = hWnd;
}

uint32_t SyntaxHighlighter::Run()
{
    PeekMessage(nullptr, nullptr, WM_USER, WM_USER, PM_NOREMOVE); // ensures queue exists

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        switch (msg.message) {
        case WM_FORMAT_RANGE:
            FormatRange(static_cast<LONG>(msg.wParam), static_cast<LONG>(msg.lParam));
            break;
        case WM_QUIT:
            return 0;
        }
    }

    return 0;
}

void SyntaxHighlighter::Shutdown()
{
    PostThreadMessage(GetThreadId(), WM_QUIT, 0, 0);
}

void SyntaxHighlighter::HighlightRange(LONG start, LONG end)
{
    NormalizeRange(start, end);

    COLORREF color = RGB(0, 0, 0x80);

    // something completely stupid
    if (m_buf[end - start - 1] == _T('<') || m_buf[end - start - 1] == _T('>')) {
        color = RGB(0x80, 0, 0);
    }

    HIGHLIGHT_RANGE hrange;
    hrange.start = start;
    hrange.end = end;
    hrange.color = color;

    SendMessage(m_hWnd, WM_HIGHLIGHT_READY, 0, reinterpret_cast<LPARAM>(&hrange));
}

void SyntaxHighlighter::FormatRange(LONG start, LONG end)
{
    NormalizeRange(start, end);
    if (end - start == 0) {
        return;
    }

    GetTextRange(start, end);
    HighlightRange(start, end);
}

void SyntaxHighlighter::GetTextRange(LONG start, LONG end)
{
    NormalizeRange(start, end);

    TEXT_RANGE range;
    range.start = start;
    range.end = end;
    range.lpstrText = m_buf;

    SendMessage(m_hWnd, WM_GET_TEXT_RANGE, 0, reinterpret_cast<LPARAM>(&range));

    m_buf[end - start] = _T('\0');
}

void SyntaxHighlighter::NormalizeRange(LONG& start, LONG& end)
{
    if (start > end) {
        std::swap(start, end);
    }

    start = std::max<LONG>(start, 0);
    end = std::max<LONG>(end, 0);
    end = std::max(end, start);
    end = std::min(end, start + MAX_TEXT_SIZE - 1);
}

void SyntaxHighlighter::Enqueue(LONG min, LONG max)
{
    PostThreadMessage(GetThreadId(), WM_FORMAT_RANGE, min, max);
}
