#include "stdafx.h"
#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter() : m_hwnd(nullptr)
{
}

void SyntaxHighlighter::SetHwnd(HWND hWnd)
{
    m_hwnd = hWnd;
}

uint32_t SyntaxHighlighter::Run()
{
    PeekMessage(nullptr, nullptr, WM_USER, WM_USER, PM_NOREMOVE); // ensures queue exists

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        switch (msg.message) {
        case WM_FORMAT_SPAN:
            FormatSpan(msg.wParam, msg.lParam);
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

void SyntaxHighlighter::FormatSpan(WPARAM wParam, LPARAM lParam)
{
    auto start = static_cast<LONG>(wParam);
    auto end = static_cast<LONG>(lParam);

    HIGHLIGHT_SPAN span;
    span.start = start;
    span.end = end;
    span.color = RGB(0x60, 0, 0);

    SendMessage(m_hwnd, WM_HIGHLIGHT_READY, 0, reinterpret_cast<LPARAM>(&span));
}

void SyntaxHighlighter::Enqueue(LONG min, LONG max)
{
    PostThreadMessage(GetThreadId(), WM_FORMAT_SPAN, min, max);
}
