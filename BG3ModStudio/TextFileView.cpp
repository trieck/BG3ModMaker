#include "stdafx.h"

#include "Exception.h"
#include "FileStream.h"
#include "RTFFormatterRegistry.h"
#include "TextFileView.h"
#include "UTF8Stream.h"

namespace { // anonymous

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

}   // namespace

LRESULT TextFileView::OnCreate(LPCREATESTRUCT pcs)
{
    auto lRet = DefWindowProc();

    SetModify(FALSE);

    return lRet;
}

BOOL TextFileView::Create(HWND parent, _U_RECT rect, DWORD dwStyle, DWORD dwStyleEx)
{
    dwStyle |= WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
        ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE
        | ES_NOOLEDRAGDROP;

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
        m_encoding = UTF8;    // nothing else supported for now
    }

    Write(pb, read);

    for (;;) {
        read = file.read(buf, sizeof(buf));
        if (read == 0) {
            break;
        }

        Write(buf, read);
    }

    Flush();

    SetModify(FALSE);

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

    SetModify(FALSE);

    return TRUE;
}

BOOL TextFileView::SaveFileAs(const CString& path)
{
    CComObjectStack<UTF8Stream> stream;
    IStream* pStream = &stream;

    EDITSTREAM es{};
    es.dwCookie = reinterpret_cast<DWORD_PTR>(&pStream);
    es.pfnCallback = StreamOutCallback;

    StreamOut((CP_UTF8 << 16) | SF_USECODEPAGE | SF_TEXT, es);

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
    return GetModify();
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

BOOL TextFileView::WriteBOM(const IStreamBase& stream) const
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
    auto formatter = RTFFormatterRegistry::GetInstance().GetFormatter(m_path);

    auto str = formatter->Format(m_stream);

    LPCSTR pStr = str.GetString();

    EDITSTREAM es{};
    es.dwCookie = reinterpret_cast<DWORD_PTR>(&pStr);
    es.pfnCallback = StreamInCallback;

    StreamIn((CP_UTF8 << 16) | SF_USECODEPAGE | SF_RTF, es);
    if (es.dwError != NOERROR) {
        ATLTRACE("Failed to stream in text.\n");
        return FALSE;
    }

    CPoint origin;
    SetScrollPos(&origin);

    auto hr = m_stream.Reset();

    return SUCCEEDED(hr);
}
