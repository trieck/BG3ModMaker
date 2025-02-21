#include "stdafx.h"
#include "textstream.h"

HRESULT TextStream::Reset() const
{
    ATLASSERT(m_pImpl != nullptr);

    ULARGE_INTEGER uli{};

    auto hr = m_pImpl->SetSize(uli);
    if (FAILED(hr)) {
        return hr;
    }

    LARGE_INTEGER li{};
    hr = m_pImpl->Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

CStringW TextStream::ReadString()
{
    ATLASSERT(m_pImpl != nullptr);

    STATSTG statstg;
    auto hr = Stat(&statstg, STATFLAG_NONAME);
    if (FAILED(hr)) {
        return L"";
    }

    auto size = statstg.cbSize.QuadPart;

    CStringW strText;

    auto bufferLength = static_cast<int>(size / sizeof(WCHAR)) + 1;
    auto* buffer = strText.GetBuffer(bufferLength);

    LARGE_INTEGER li{};
    hr = Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        return L"";
    }

    ULONG uRead;
    hr = Read(buffer, static_cast<ULONG>(size), &uRead);
    if (FAILED(hr)) {
        return L"";
    }

    if (size != uRead) {
        return L"";
    }

    buffer[uRead / sizeof(WCHAR)] = L'\0';
    strText.ReleaseBuffer();

    return strText;
}

HRESULT TextStream::WriteV(LPCWSTR format, va_list args) const
{
    ATLASSERT(m_pImpl != nullptr && format != nullptr);

    CStringW strValue;
    strValue.FormatV(format, args);

    ULONG cb = strValue.GetLength() * sizeof(WCHAR);

    ULONG written;
    auto hr = m_pImpl->Write(strValue, cb, &written);

    return hr;
}

HRESULT TextStream::Write(LPCWSTR text) const
{
    ULONG cb = static_cast<ULONG>(wcslen(text) * sizeof(WCHAR));
    ULONG written;

    auto hr = m_pImpl->Write(text, cb, &written);

    return hr;
}

TextStream::~TextStream()
= default;

HRESULT TextStream::FinalConstruct()
{
    m_pImpl = SHCreateMemStream(nullptr, 0);
    if (!m_pImpl) {
        return E_FAIL;
    }

    return S_OK;
}

void TextStream::FinalRelease()
{
    if (m_pImpl) {
        m_pImpl.Release();
    }
}

HRESULT TextStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Read(pv, cb, pcbRead);
}

HRESULT TextStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Write(pv, cb, pcbWritten);
}

HRESULT TextStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Seek(dlibMove, dwOrigin, plibNewPosition);
}

HRESULT TextStream::SetSize(ULARGE_INTEGER libNewSize)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->SetSize(libNewSize);
}

HRESULT TextStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->CopyTo(pstm, cb, pcbRead, pcbWritten);
}

HRESULT TextStream::Commit(DWORD grfCommitFlags)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Commit(grfCommitFlags);
}

HRESULT TextStream::Revert()
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Revert();
}

HRESULT TextStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->LockRegion(libOffset, cb, dwLockType);
}

HRESULT TextStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->UnlockRegion(libOffset, cb, dwLockType);
}

HRESULT TextStream::Stat(STATSTG* pstatstg, DWORD grfStatFlag)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Stat(pstatstg, grfStatFlag);
}

HRESULT TextStream::Clone(IStream** ppstm)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Clone(ppstm);
}
