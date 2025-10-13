#include "stdafx.h"

#include "Exception.h"
#include "FileStream.h"
#include "TextFileView.h"
#include "UTF8Stream.h"

auto constexpr BUFFER_SIZE = 4096;

LRESULT TextFileView::OnCreate(LPCREATESTRUCT pcs)
{
    if (!m_edit.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL)) {
        ATLTRACE("Unable to edit control.\n");
        return -1;
    }

    m_edit.SetDefaultFontFace("Cascadia Mono");
    m_edit.SetDefaultFontSize(12);

    m_styler = DocStylerRegistry::GetDefaultStyler();
    m_styler->Apply(m_edit);

    return 0;
}

LRESULT TextFileView::OnModified(LPNMHDR pnmh)
{
    if (pnmh->hwndFrom != m_edit) {
        return 0;
    }

    auto scn = reinterpret_cast<SCNotification*>(pnmh);

    if (scn->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) {
        m_bDirty = true;
    }

    return 0;
}

void TextFileView::OnSize(UINT nType, CSize size)
{
    if (m_edit.IsWindow()) {
        m_edit.MoveWindow(0, 0, size.cx, size.cy, TRUE);
    }
}

BOOL TextFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE;

    auto hWnd = Base::Create(parent, rect, nullptr, dwStyle, dwStyleEx);
    if (!hWnd) {
        return FALSE;
    }

    return TRUE;
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

    m_styler = DocStylerRegistry::GetStyler(path);
    m_styler->Apply(m_edit);

    Flush();

    m_edit.SetReadOnly(m_bReadOnly);

    return TRUE;
}

BOOL TextFileView::LoadBuffer(const CString& path, const ByteBuffer& buffer)
{
    m_stream.Reset();
    m_path.Empty();

    m_encoding = UNKNOWN;

    if (buffer.second == 0 || buffer.first == nullptr) {
        return FALSE;
    }

    auto pb = reinterpret_cast<LPSTR>(buffer.first.get());
    size_t size = buffer.second;

    if (SkipBOM(pb, size)) {
        size -= 3;
        m_encoding = UTF8BOM;
    } else {
        m_encoding = UTF8; // nothing else supported for now
    }

    m_styler = DocStylerRegistry::GetStyler(path);
    m_styler->Apply(m_edit);

    Write(pb, size);
    Flush();

    m_edit.SetReadOnly(m_bReadOnly);

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

    return TRUE;
}

BOOL TextFileView::SaveFileAs(const CString& path)
{
    CStringA contents = m_edit.GetText();

    CStringA strPath(path);

    FileStream file;

    try {
        file.open(strPath, "wb");
    } catch (const Exception& e) {
        ATLTRACE("Failed to open file: %s\n", e.what());
        return FALSE;
    }

    WriteBOM(file);
    file.write(contents.GetString(), contents.GetLength());

    file.close();

    m_bDirty = FALSE;

    return TRUE;
}

BOOL TextFileView::Destroy()
{
    return DestroyWindow();
}

BOOL TextFileView::IsDirty() const
{
    return m_bDirty;
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

void TextFileView::SetReadOnly(BOOL readOnly)
{
    m_bReadOnly = readOnly;
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
    auto str = m_stream.ReadString();
    if (str.IsEmpty()) {
        return FALSE;
    }
    LPCSTR pStr = str.GetString();

    m_edit.SetText(pStr);

    m_bDirty = FALSE;

    auto hr = m_stream.Reset();

    return SUCCEEDED(hr);
}
