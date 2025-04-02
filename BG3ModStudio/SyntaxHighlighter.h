#pragma once

#include "ThreadImpl.h"

typedef struct _HIGHLIGHT_SPAN
{
    LONG start;
    LONG end;
    COLORREF color;
} HIGHLIGHT_SPAN, *LPHILIGHT_SPAN;

class SyntaxHighlighter : public ThreadImpl<SyntaxHighlighter>
{
public:
    SyntaxHighlighter();

    uint32_t Run();
    void Enqueue(LONG min, LONG max);
    void SetHwnd(HWND hWnd);
    void Shutdown();

private:
    void FormatSpan(WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
};
