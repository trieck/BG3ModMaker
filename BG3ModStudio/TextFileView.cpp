#include "stdafx.h"

#include "Exception.h"
#include "FileStream.h"
#include "resources/resource.h"
#include "ScopeGuard.h"
#include "TextFileView.h"
#include "UTF8Stream.h"
#include "XmlLineFormatter.h"

namespace { // anonymous

constexpr auto EDIT_TIMER_ID = 0xFEED;
constexpr auto EDIT_TIMER_DELAY = 50;
constexpr auto EDIT_TIMER_AUTO_KILL_ID = 0xDEAD;
constexpr auto EDIT_TIMER_AUTO_KILL = 5000;

DWORD CALLBACK StreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
    auto** pptext = reinterpret_cast<LPCSTR*>(dwCookie);

    if (pptext == nullptr || *pptext == nullptr) {
        return E_POINTER;
    }

    auto strSize = strlen(*pptext);

    if (strSize == 0) {
        *pcb = 0;
        return 0;
    }

    auto sz = std::min<LONG>(static_cast<LONG>(strSize), cb);

    memcpy(pbBuff, *pptext, sz);

    *pptext += sz;

    *pcb = sz;

    return 0;
}

DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
    auto** ppStream = reinterpret_cast<IStream**>(dwCookie);

    if (ppStream == nullptr || *ppStream == nullptr) {
        return E_POINTER;
    }

    if (cb == 0) {
        *pcb = 0;
        return 0;
    }

    auto hr = (*ppStream)->Write(pbBuff, cb, reinterpret_cast<ULONG*>(pcb));
    if (FAILED(hr)) {
        return hr;
    }

    if (*pcb == 0) {
        return E_FAIL;
    }

    return 0;
}

auto constexpr BUFFER_SIZE = 4096;
} // namespace

LRESULT TextFileView::OnCreate(LPCREATESTRUCT pcs)
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
        ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE
        | ES_NOOLEDRAGDROP;

    if (!m_richEdit.Create(*this, rcDefault, nullptr, dwStyle, 0, IDC_RICHEDIT)) {
        ATLTRACE("Unable to create rich edit control.\n");
        return -1;
    }

    m_richEdit.SendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);

    return 0;
}

LRESULT TextFileView::OnEditChange(UINT uNotifyCode, int nID, CWindow wndCtl, BOOL& bHandled)
{
    ATLASSERT(nID == IDC_RICHEDIT);
    ATLASSERT(wndCtl == m_richEdit);

    KillTimer(EDIT_TIMER_ID);
    SetTimer(EDIT_TIMER_ID, EDIT_TIMER_DELAY);

    KillTimer(EDIT_TIMER_AUTO_KILL_ID);
    SetTimer(EDIT_TIMER_AUTO_KILL_ID, EDIT_TIMER_AUTO_KILL, nullptr);

    return 0;
}

void TextFileView::OnSize(UINT nType, CSize size)
{
    if (m_richEdit.IsWindow()) {
        m_richEdit.MoveWindow(0, 0, size.cx, size.cy, TRUE);
    }
}

void TextFileView::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent) {
    case EDIT_TIMER_ID:
        KillTimer(EDIT_TIMER_ID);
        ApplySyntaxHighlight();
        break;
    case EDIT_TIMER_AUTO_KILL_ID:
        KillTimer(EDIT_TIMER_ID);
        KillTimer(EDIT_TIMER_AUTO_KILL_ID);
        break;
    default:
        break;
    }
}

BOOL TextFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE;

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);

    return hWnd != nullptr;
}

BOOL TextFileView::LoadFile(const CString& path)
{
    m_stream.Reset();

    m_path = path;
    m_encoding = UNKNOWN;

    FileStream file;

    CStringA strPath(path);

    try {
        file.open(strPath, "rb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return FALSE;
    }

    char buf[BUFFER_SIZE];
    auto read = file.read(buf, sizeof(buf));

    LPSTR pb = buf;
    if (SkipBOM(pb, read)) {
        read -= 3;
        m_encoding = UTF8BOM;
    } else {
        m_encoding = UTF8; // nothing else supported for now
    }

    Write(pb, read);

    for (;;) {
        read = file.read(buf, sizeof(buf));
        if (read == 0) {
            break;
        }

        Write(buf, read);
    }

    m_formatter = RTFFormatterRegistry::GetInstance().GetFormatter(path);

    Flush();

    m_richEdit.SetModify(FALSE);

    return TRUE;
}

BOOL TextFileView::SaveFile()
{
    if (m_path.IsEmpty()) {
        return FALSE;
    }

    if (!SaveFileAs(m_path)) {
        return FALSE;
    }

    m_richEdit.SetModify(FALSE);

    return TRUE;
}

BOOL TextFileView::SaveFileAs(const CString& path)
{
    CComObjectStack<UTF8Stream> stream;
    IStream* pStream = &stream;

    EDITSTREAM es{};
    es.dwCookie = reinterpret_cast<DWORD_PTR>(&pStream);
    es.pfnCallback = StreamOutCallback;

    m_richEdit.StreamOut(CP_UTF8 << 16 | SF_USECODEPAGE | SF_TEXT, es);

    if (es.dwError != NOERROR) {
        ATLTRACE("Failed to stream in text.\n");
        return FALSE;
    }

    LARGE_INTEGER li{};
    auto hr = pStream->Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        ATLTRACE("Failed to seek stream.\n");
        return FALSE;
    }

    CStringA strPath(path);

    FileStream file;
    try {
        file.open(strPath, "wb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return FALSE;
    }

    WriteBOM(file);

    for (;;) {
        char buf[BUFFER_SIZE];
        ULONG read;

        hr = pStream->Read(buf, sizeof(buf), &read);
        if (FAILED(hr)) {
            ATLTRACE("Failed to read stream.\n");
            return FALSE;
        }

        if (read == 0) {
            break;
        }

        file.write(buf, read);
    }

    file.close();

    return TRUE;
}

BOOL TextFileView::Destroy()
{
    return DestroyWindow();
}

BOOL TextFileView::IsDirty() const
{
    return m_richEdit.GetModify();
}

const CString& TextFileView::GetPath() const
{
    return m_path;
}

void TextFileView::SetPath(const CString& path)
{
    m_path = path;
}

FileEncoding TextFileView::GetEncoding() const
{
    return m_encoding;
}

TextFileView::operator HWND() const
{
    return m_hWnd;
}

BOOL TextFileView::Write(LPCWSTR text) const
{
    auto hr = m_stream.Write(text);

    return SUCCEEDED(hr);
}

BOOL TextFileView::Write(LPCWSTR text, size_t length) const
{
    CStringW str(text, static_cast<int>(length));

    return Write(str);
}

BOOL TextFileView::Write(LPCSTR text) const
{
    auto hr = m_stream.Write(text);
    return SUCCEEDED(hr);
}

BOOL TextFileView::Write(LPCSTR text, size_t length) const
{
    auto hr = m_stream.Write(text, length);
    return SUCCEEDED(hr);
}

BOOL TextFileView::SkipBOM(LPSTR& str, size_t size)
{
    if (size < 3) {
        return FALSE;
    }

    if (str[0] == '\xEF' && str[1] == '\xBB' && str[2] == '\xBF') {
        str += 3;
        return TRUE;
    }

    return FALSE;
}

BOOL TextFileView::WriteBOM(StreamBase& stream) const
{
    switch (m_encoding) {
    case UTF8BOM:
        stream.write("\xEF\xBB\xBF", 3);
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

BOOL TextFileView::Flush()
{
    auto str = m_formatter->Format(m_stream);

    LPCSTR pStr = str.GetString();

    EDITSTREAM es{};
    es.dwCookie = reinterpret_cast<DWORD_PTR>(&pStr);
    es.pfnCallback = StreamInCallback;

    m_richEdit.StreamIn(CP_UTF8 << 16 | SF_USECODEPAGE | SF_RTF, es);
    if (es.dwError != NOERROR) {
        ATLTRACE("Failed to stream in text.\n");
        return FALSE;
    }

    CPoint origin;
    m_richEdit.SetScrollPos(&origin);

    auto hr = m_stream.Reset();

    return SUCCEEDED(hr);
}


void TextFileView::ApplySyntaxHighlight()
{
    CString text;
    m_richEdit.GetWindowText(text);

    BOOL isDirty;
    ScopeGuardSimple modifyGuard(
        [&] { isDirty = m_richEdit.GetModify(); },
        [&] { m_richEdit.SetModify(isDirty); });

    DWORD prevMask = m_richEdit.GetEventMask();

    ScopeGuard maskGuard(m_richEdit, 
        &CRichEditCtrl::SetEventMask, 
        &CRichEditCtrl::SetEventMask,
        std::make_tuple(prevMask & ~ENM_CHANGE),
        std::make_tuple(prevMask));

    ScopeGuard redraw(m_richEdit,
        &CRichEditCtrl::SetRedraw,
        &CRichEditCtrl::SetRedraw,
        std::make_tuple(FALSE),
        std::make_tuple(TRUE));
 
    if (text.IsEmpty()) {
        auto format = m_formatter->GetDefaultFormat();
        auto pFormat = format.GetString();
        EDITSTREAM es{};
        es.dwCookie = reinterpret_cast<DWORD_PTR>(&pFormat);
        es.pfnCallback = StreamInCallback;
        m_richEdit.StreamIn(CP_UTF8 << 16 | SF_USECODEPAGE | SF_RTF, es);
        m_lineFormatter.Reset();
        return;
    }

    CHARRANGE prevSel;
    m_richEdit.GetSel(prevSel); // Get cursor position

    LONG lineIndex = m_richEdit.LineFromChar(prevSel.cpMin);
    LONG lineStart = m_richEdit.LineIndex(lineIndex);
    LONG lineLength = m_richEdit.LineLength(lineStart);

    if (lineLength == 0) {
        return;
    }

    CString line;
    LPTSTR pline = line.GetBufferSetLength(lineLength + 1);

    auto len = m_richEdit.GetLine(lineIndex, pline, lineLength);
    line.ReleaseBuffer(len);

    if (len == 0) {
        return;
    }

    uint32_t state = 0;

    PARAFORMAT2 pf{};
    pf.cbSize = sizeof(PARAFORMAT2);
    pf.dwMask = PFM_OFFSET;

    if (lineIndex > 0) {
        auto preLineIndex = lineIndex - 1;
        auto prevLineStart = m_richEdit.LineIndex(preLineIndex);
        auto prevLineLength = m_richEdit.LineLength(prevLineStart);
        auto prevLineEnd = prevLineStart + prevLineLength;

        m_richEdit.HideSelection(TRUE, FALSE);
        m_richEdit.SetSel(prevLineStart, prevLineEnd);
        m_richEdit.GetParaFormat(pf);
        m_richEdit.SetSel(prevSel);
        m_richEdit.HideSelection(FALSE, FALSE);

        //state = static_cast<uint32_t>(pf.dxOffset);

        ATLTRACE("Previous line %d has state %d\n", preLineIndex, state);
    }

    m_lineFormatter.SetState(state);

    // Just use XML for this example for everything
    auto tokens = m_lineFormatter.Format(line);

    while (!tokens.empty()) {
        auto token = tokens.back();
        tokens.pop_back();

        CHARRANGE cr{};
        cr.cpMin = lineStart + token.start;
        cr.cpMax = lineStart + token.end;

        m_richEdit.SetSel(cr);

        CHARFORMAT2 cf{};
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_ALL;

        m_richEdit.GetSelectionCharFormat(cf);

        cf.dwMask = CFM_COLOR;
        cf.crTextColor = token.color;
        m_richEdit.SetSelectionCharFormat(cf);
    }

    // Store parser state of the current line in the para format
    m_richEdit.HideSelection(TRUE, FALSE);
    m_richEdit.SetSel(lineIndex, lineIndex + lineLength);

    ATLTRACE("Setting state %d for line %d\n", m_lineFormatter.GetState(), lineIndex);

    pf.dxOffset = static_cast<LONG>(m_lineFormatter.GetState());
    m_richEdit.SetParaFormat(pf);

    m_richEdit.SetSel(prevSel); // Restore cursor position
    m_richEdit.HideSelection(FALSE, FALSE);

    Invalidate();
}
