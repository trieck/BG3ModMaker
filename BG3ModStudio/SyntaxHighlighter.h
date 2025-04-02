#pragma once

#include "ThreadImpl.h"

typedef struct
{
    LONG start;
    LONG end;
    COLORREF color;
} HIGHLIGHT_RANGE, *LPHILIGHT_RANGE;

typedef struct
{
    LONG start;
    LONG end;
    LPTSTR lpstrText;
} TEXT_RANGE, *LPTEXT_RANGE;

class SyntaxHighlighter : public ThreadImpl<SyntaxHighlighter>
{
public:
    SyntaxHighlighter();

    uint32_t Run();
    void Enqueue(LONG min, LONG max);
    void SetHwnd(HWND hWnd);
    void Shutdown();

private:
    void HighlightRange(LONG start, LONG end);
    void FormatRange(LONG start, LONG end);
    void GetTextRange(LONG start, LONG end);
    void NormalizeRange(LONG& start, LONG& end);

    static constexpr auto MAX_TEXT_SIZE = 4096;
    TCHAR m_buf[MAX_TEXT_SIZE]{};
    HWND m_hWnd;
};
